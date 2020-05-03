#include <includes.h>

#include <httpd.h>
#include <http_config.h>

#include <apr.h>
#include <apr_strings.h>
#include <apr_hash.h>
#include <apr_dbd.h>
#include <apr_strings.h>
#include <apr_lib.h>

static apr_hash_t *vhost_templates = NULL;
static apr_hash_t *vhost_template_args = NULL;
static apr_array_header_t *variables = NULL;
static char * vhost_template = NULL;

/**
 * Set the template content when the VHostTemplate directive appears in config file.
 * @param cmd The Apache command environment
 * @param mconfig not used
 * @param arg The template parameters
 * @return char* NULL when no error occurs, an error string when an error occurs
 */
static const char *collect_section_string(cmd_parms *cmd, void *dummy, const char *arg)
{
	char line[MAX_STRING_LEN];
	apr_pool_t *pool = cmd->temp_pool;
	char *section_string = apr_pstrdup(pool, "")/*apr_pstrcat(pool, BEGIN_TEMPLATE_SECTION, " ", arg, NULL)*/, ** arg_array;
	// Tookenize Parameters
	if(apr_tokenize_to_argv(arg, &arg_array, pool) != 0) return "Error in apr_tokenize_to_argv!";
	char *name = apr_pstrdup(pool, arg_array[0]);
	if(vhost_templates == NULL) vhost_templates = apr_hash_make(pool);
	if(vhost_template_args == NULL) vhost_template_args = apr_hash_make(pool);
	apr_hash_set(vhost_template_args, name, APR_HASH_KEY_STRING, arg);
	while (!ap_cfg_getline(line, MAX_STRING_LEN, cmd->config_file))
	{
        char *currline;
		char * trimmed_line = apr_pstrdup(pool, line);
		apr_collapse_spaces(trimmed_line, trimmed_line);
        /* skip comments */
        if (*trimmed_line == '#')
            continue;
		if ((currline = strstr(trimmed_line, END_TEMPLATE_SECTION)) == trimmed_line)
		{
			break;
		}
		section_string = apr_pstrcat(pool, section_string, "\n", line, NULL); 
	}
	apr_hash_set(vhost_templates, name, APR_HASH_KEY_STRING, section_string);
	return NULL;
}

/**
 * Replace vars from els/num_els in template and store result in text 
 * @param cmd The Apache command environment
 * @param text The complete vhost entry with replaced arguments
 * @param num_els The number of arguments
 * @param els The arguments
 * @return int 0 if everything is ok, otherwise -1
 */
static int build_vhost_entry_from_template_r(cmd_parms *cmd, char ** text, int num_els, char *const els[])
{
	char * macro_name = apr_pstrdup(cmd->temp_pool, els[0]), * content = NULL, * template_content = NULL, * template_args = NULL;
	// Get Vhost Template and Params from global Hash Map
	if((content = apr_hash_get(vhost_templates, macro_name, APR_HASH_KEY_STRING)) != NULL) template_content = apr_pstrdup(cmd->temp_pool, content);
	if((content = apr_hash_get(vhost_template_args, macro_name, APR_HASH_KEY_STRING)) != NULL) template_args = apr_pstrdup(cmd->temp_pool, content);
	// Store Params into local array "set_params"
	char **set_params = NULL;
	// Tokenize Macro Parameters into set_params
	if(apr_tokenize_to_argv(template_args, &set_params, cmd->temp_pool) != 0) return -1;
	int num_params = get_carr_size(set_params);
	_cleanup_cstr_ char * tmp_content = strdup(template_content);
// 	fprintf(stderr, "template_content: %s\n--endcontent\n", template_content);
	for(int i = 1; i < num_els; i++)
	{
		//Security check when one value is null
		if(set_params[i] == NULL || els[i] == NULL)
		{
			printf(stderr, "Param[%d](%s) or Arg[%d](%s) is NULL!\n", i, set_params[i], i, els[i]);
			return -1;
		}
		_cleanup_cstr_ char * param = NULL;
		// Assume that Variable names begin with a to-quote char
		if(!asprintf(&param, "\\%s\\b", set_params[i])) return -1;
		// Replace args with supplied parameters
		pcre_replace_r(&tmp_content, param, els[i], -1);
	}
// 	fprintf(stderr, "Tmp_content: %s\n--endcontent\n", tmp_content);
	*text = apr_pstrdup(cmd->temp_pool, tmp_content);
	return 0;
}

/**
 * Apply Template and do replacements when UseTemplate appears in Config file or in a passed string
 * @param cmd The Apache command environment
 * @param dummy not used
 * @param argc The number of arguments
 * @param argv The arguments
 * @return NULL if everything is ok
 */
static const char * do_replacements(cmd_parms *cmd, void *dummy, int argc, char *const argv[])
{
    apr_array_header_t *contents = apr_array_make(cmd->temp_pool, 1, sizeof(char *));
	char **new = apr_array_push(contents);
	char * string_contents = NULL;
	char * where = apr_psprintf(cmd->temp_pool, "File '%s' (%d)", cmd->config_file->name, cmd->config_file->line_number);
	// Replacements with arguments
	build_vhost_entry_from_template_r(cmd, &string_contents, argc, argv);
	// We add a linebreak here
	*new = apr_pstrcat(cmd->temp_pool, apr_pstrdup(cmd->temp_pool, string_contents), "\n", NULL);
	cmd->config_file = make_array_config(cmd->temp_pool, contents, where, cmd->config_file, &cmd->config_file);
	return NULL;
}

