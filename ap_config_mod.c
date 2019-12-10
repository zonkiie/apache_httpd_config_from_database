/*
 * code parts from httpd/modules/core/mod_macro.c
 */
#include <includes.h>

#include "httpd.h"
#include "http_config.h"
#include "http_log.h"

#include "apr.h"
#include "apr_strings.h"
#include "apr_hash.h"

#define BEGIN_CMD "<Command"
#define EXEC_CMD "Exec"
#define END_CMD "</Command>"
#define LOGFILE_CMD "/dev/shm/ap_mod_conf.log"

typedef struct
{
    char *name;                    /* lower case name of the command */
    apr_array_header_t *arguments; /* of char*, command parameter names */
    apr_array_header_t *contents;  /* of char*, command body */
    char *location;                /* of command definition, for error messages */
} ap_command_t;

typedef struct
{
    int index;                    /* current element */
    int char_index;               /* current char in element */
    int length;                   /* cached length of the current line */
    apr_array_header_t *contents; /* array of char * */
    ap_configfile_t *next;        /* next config once this one is processed */
    ap_configfile_t **upper;      /* hack: where to update it if needed */
} array_contents_t;


static apr_hash_t *ap_commands = NULL;

#define empty_string_p(p) (!(p) || *(p) == '\0')
#define trim(line) while (*(line) == ' ' || *(line) == '\t') (line)++

/*
  return configuration-parsed arguments from line as an array.
  the line is expected not to contain any '\n'?
*/
static apr_array_header_t *get_arguments(apr_pool_t * pool, const char *line)
{
    apr_array_header_t *args = apr_array_make(pool, 1, sizeof(char *));

    trim(line);
    while (*line) {
        char *arg = ap_getword_conf(pool, &line);
        char **new = apr_array_push(args);
        *new = arg;
        trim(line);
    }

    return args;
}

/*
  Get next config if any.
  this may be called several times if there are continuations.
*/
static int next_one(array_contents_t * ml)
{
    if (ml->next) {
        ap_assert(ml->upper);
        *(ml->upper) = ml->next;
        return 1;
    }
    return 0;
}

/*
  returns next char if possible
  this may involve switching to enclosing config.
*/
static apr_status_t array_getch(char *ch, void *param)
{
    array_contents_t *ml = (array_contents_t *) param;
    char **tab = (char **) ml->contents->elts;

    while (ml->char_index >= ml->length) {
        if (ml->index >= ml->contents->nelts) {
            /* maybe update */
            if (ml->next && ml->next->getch && next_one(ml)) {
                apr_status_t rc = ml->next->getch(ch, ml->next->param);
                if (*ch==LF)
                    ml->next->line_number++;
                return rc;
            }
            return APR_EOF;
        }
        ml->index++;
        ml->char_index = 0;
        ml->length = ml->index >= ml->contents->nelts ?
            0 : strlen(tab[ml->index]);
    }

    *ch = tab[ml->index][ml->char_index++];
    return APR_SUCCESS;
}

/*
  returns a buf a la fgets.
  no more than a line at a time, otherwise the parsing is too much ahead...
  NULL at EOF.
*/
static apr_status_t array_getstr(void *buf, size_t bufsize, void *param)
{
    array_contents_t *ml = (array_contents_t *) param;
    char *buffer = (char *) buf;
    char next = '\0';
    size_t i = 0;
    apr_status_t rc = APR_SUCCESS;

    /* read chars from stream, stop on newline */
    while (i < bufsize - 1 && next != LF &&
           ((rc = array_getch(&next, param)) == APR_SUCCESS)) {
        buffer[i++] = next;
    }

    if (rc == APR_EOF) {
        /* maybe update to next, possibly a recursion */
        if (next_one(ml)) {
            ap_assert(ml->next->getstr);
            /* keep next line count in sync! the caller will update
               the current line_number, we need to forward to the next */
            ml->next->line_number++;
            return ml->next->getstr(buf, bufsize, ml->next->param);
        }
        /* else that is really all we can do */
        return APR_EOF;
    }

    buffer[i] = '\0';

    return APR_SUCCESS;
}

/*
  close the array stream?
*/
static apr_status_t array_close(void *param)
{
    array_contents_t *ml = (array_contents_t *) param;
    /* move index at end of stream... */
    ml->index = ml->contents->nelts;
    ml->char_index = ml->length;
    return APR_SUCCESS;
}



/*
  warn if anything non blank appears, but ignore comments...
*/
static void warn_if_non_blank(const char * what,
                              char * ptr,
                              ap_configfile_t * cfg)
{
    char * p;
    for (p=ptr; *p; p++) {
        if (*p == '#')
            break;
        if (*p != ' ' && *p != '\t') {
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL, APLOGNO(02989)
                         "%s on line %d of %s: %s",
                         what, cfg->line_number, cfg->name, ptr);
            break;
        }
    }
}

/*
  get read lines as an array till end_token.
  counts nesting for begin_token/end_token.
  it assumes a line-per-line configuration (thru getline).
  this function could be exported.
  begin_token may be NULL.
*/
static char *get_lines_till_end_token(apr_pool_t * pool,
                                      ap_configfile_t * config_file,
                                      const char *end_token,
                                      const char *begin_token,
                                      const char *where,
                                      apr_array_header_t ** plines)
{
    apr_array_header_t *lines = apr_array_make(pool, 1, sizeof(char *));
    char line[MAX_STRING_LEN];  /* sorry, but this is expected by getline:-( */
    int macro_nesting = 1, any_nesting = 1;
    int line_number_start = config_file->line_number;

    while (!ap_cfg_getline(line, MAX_STRING_LEN, config_file)) {
        char *ptr = line;
        char *first, **new;
        /* skip comments */
        if (*line == '#')
            continue;
        first = ap_getword_conf_nc(pool, &ptr);
        if (first) {
            /* detect nesting... */
            if (!strncmp(first, "</", 2)) {
                any_nesting--;
                if (any_nesting < 0) {
                    ap_log_error(APLOG_MARK, APLOG_WARNING,
                                 0, NULL, APLOGNO(02793)
                                 "bad (negative) nesting on line %d of %s",
                                 config_file->line_number - line_number_start,
                                 where);
                }
            }
            else if (!strncmp(first, "<", 1)) {
                any_nesting++;
            }

            if (!strcasecmp(first, end_token)) {
                /* check for proper closing */
                char * endp = (char *) ap_strrchr_c(line, '>');

                /* this cannot happen if end_token contains '>' */
                if (endp == NULL) {
                  return "end directive missing closing '>'";
                }

                warn_if_non_blank(
                    APLOGNO(02794) "non blank chars found after directive closing",
                    endp+1, config_file);

                macro_nesting--;
                if (!macro_nesting) {
                    if (any_nesting) {
                        ap_log_error(APLOG_MARK,
                                     APLOG_WARNING, 0, NULL, APLOGNO(02795)
                                     "bad cumulated nesting (%+d) in %s",
                                     any_nesting, where);
                    }
                    *plines = lines;
                    return NULL;
                }
            }
            else if (begin_token && !strcasecmp(first, begin_token)) {
                macro_nesting++;
            }
        }
        new = apr_array_push(lines);
        *new = apr_psprintf(pool, "%s" APR_EOL_STR, line); /* put EOL back? */
    }

    return apr_psprintf(pool, "expected token not found: %s", end_token);
}

static const char *cmd_section(cmd_parms * cmd, void *dummy, const char *arg)
{
    apr_pool_t *pool;
    char *endp, *name, *where;
	const char *errmsg;
	ap_command_t *command;
    if (ap_commands == NULL) {
        pool = cmd->pool;
        ap_commands = apr_hash_make(pool);
        ap_assert(ap_commands != NULL);
        apr_pool_cleanup_register(pool, &ap_commands,
                                  ap_pool_cleanup_set_null,
                                  apr_pool_cleanup_null);
    }
    else {
        pool = apr_hash_pool_get(ap_commands);
    }

    endp = (char *) ap_strrchr_c(arg, '>');
    if (endp == NULL) {
        return BEGIN_CMD "> directive missing closing '>'";
    }

    if (endp == arg) {
        return BEGIN_CMD " command definition: empty name";
    }

    warn_if_non_blank(APLOGNO(02801) "non blank chars found after "
                      BEGIN_CMD " closing '>'",
                      endp+1, cmd->config_file);

    /* coldly drop '>[^>]*$' out */
    *endp = '\0';
    name = ap_getword_conf(pool, &arg);
    if (empty_string_p(name)) {
        return BEGIN_CMD " command definition: name not found";
    }

    ap_str_tolower(name);
    command = apr_hash_get(ap_commands, name, APR_HASH_KEY_STRING);

    if (command != NULL) {
        /* already defined: warn about the redefinition */
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL, APLOGNO(02802)
                     "command '%s' multiply defined: "
                     "%s, redefined on line %d of \"%s\"",
                     command->name, command->location,
                     cmd->config_file->line_number, cmd->config_file->name);
    }
    else {
        /* allocate a new command */
        command = (ap_command_t *) apr_palloc(pool, sizeof(ap_command_t));
        command->name = name;
    }

    //debug(fprintf(stderr, "command_section: name=%s\n", name));

    /* get command arguments */
    command->location = apr_psprintf(pool,
                                   "defined on line %d of \"%s\"",
                                   cmd->config_file->line_number,
                                   cmd->config_file->name);
    //debug(fprintf(stderr, "command_section: location=%s\n", command->location));

    where =
        apr_psprintf(pool, "command '%s' (%s)", command->name, command->location);

    /*if (looks_like_an_argument(name)) {
        ap_log_error(APLOG_MARK, APLOG_WARNING, 0, NULL, APLOGNO(02803)
                     "%s better prefix a command name with any of '%s'",
                     where, ARG_PREFIX);
    }

    /* get command parameters */
    //command->arguments = get_arguments(pool, arg);*/

    errmsg = get_lines_till_end_token(pool, cmd->config_file,
                                      END_CMD, BEGIN_CMD,
                                      where, &command->contents);

    if (errmsg) {
        return apr_psprintf(cmd->temp_pool,
                            "%s" APR_EOL_STR "\tcontents error: %s",
                            where, errmsg);
    }

    /* store the new command */
    apr_hash_set(ap_commands, name, APR_HASH_KEY_STRING, command);
	return NULL;
}

/*
  create an array config stream insertion "object".
  could be exported.
*/
static ap_configfile_t *make_array_config(apr_pool_t * pool,
                                          apr_array_header_t * contents,
                                          const char *where,
                                          ap_configfile_t * cfg,
                                          ap_configfile_t ** upper)
{
    array_contents_t *ls =
        (array_contents_t *) apr_palloc(pool, sizeof(array_contents_t));
    ap_assert(ls!=NULL);

    ls->index = 0;
    ls->char_index = 0;
    ls->contents = contents;
    ls->length = ls->contents->nelts < 1 ?
        0 : strlen(((char **) ls->contents->elts)[0]);
    ls->next = cfg;
    ls->upper = upper;

    return ap_pcfg_open_custom(pool, where, (void *) ls, array_getch, array_getstr, array_close);
}

static const char *exec_cmd(cmd_parms * cmd, void *dummy, const char *arg)
{
	FILE * logfile = fopen(LOGFILE_CMD, "w+");
	fprintf(logfile, "Start\n");
    char *name, *recursion, *where;
    const char *errmsg;
	ap_command_t * command;
    apr_array_header_t *replacements;
    apr_array_header_t *contents;
    where = apr_psprintf(cmd->temp_pool, "command '%s' (%s) used on line %d of \"%s\"", command->name, command->location, cmd->config_file->line_number, cmd->config_file->name);
	
	
	
	/* the current "config file" is replaced by a string array...
       at the end of processing the array, the initial config file
       will be returned there (see next_one) so as to go on. */
    //cmd->config_file = make_array_config(cmd->temp_pool, contents, where, cmd->config_file, &cmd->config_file);
	char line[MAX_STRING_LEN];
	while (!ap_cfg_getline(line, MAX_STRING_LEN, cmd->config_file)) {
		fprintf(logfile, "Line: %s\n", line);
	}
	fclose(logfile);
	return NULL;
}

static const command_rec mod_cmds[] = {
	AP_INIT_RAW_ARGS(BEGIN_CMD, cmd_section, NULL, EXEC_ON_READ | OR_ALL, "Beginning of a cmd definition section."),
    AP_INIT_RAW_ARGS(EXEC_CMD, exec_cmd, NULL, EXEC_ON_READ | OR_ALL, "Use of a command."),
	{NULL}
};


AP_DECLARE_MODULE(mod_ap_config) = {
    STANDARD20_MODULE_STUFF,    /* common stuff */
        NULL,                   /* create per-directory config */
        NULL,                   /* merge per-directory config structures */
        NULL,                   /* create per-server config structure */
        NULL,                   /* merge per-server config structures */
        mod_cmds,               /* configuration commands */
        NULL                    /* register hooks */
};

// To be removed...
//int main(){}
