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
#include "apr_dbd.h"
#include "apr_strings.h"
#include "apr_lib.h"

#include "mod_dbd.h"

#define EXEC_SQL "ExecuteSQL"
#define LOGFILE "/dev/shm/mod_db_config.log"

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

static const char *exec_sql(cmd_parms * cmd, void *dummy, const char *arg)
{
    char *sql, *where, *line = NULL;
	ap_dbd_t *dbd = NULL;
	apr_dbd_results_t *res = NULL;
	apr_dbd_row_t *row = NULL;
	
	// This will fail because cmd->server is null as it's called in the startup context
	if((dbd = ap_dbd_open(cmd->temp_pool, cmd->server)) == NULL) return "Could not dbd open!";
	//if ((dbd = dbd_acquire_fn(r)) == NULL) {
	
    apr_array_header_t *replacements;
    apr_array_header_t *contents;
	if((sql = ap_getword_conf(cmd->temp_pool, &arg)) == NULL) return "No command given.";
	
	where = apr_psprintf(cmd->temp_pool, "File '%s' (%d)", cmd->config_file->name, cmd->config_file->line_number);
	
	contents = apr_array_make(cmd->temp_pool, 1, sizeof(char *));
	
	int sqlstate = apr_dbd_select(dbd->driver, cmd->temp_pool, dbd->handle, &res, sql, 1);
	int rows = apr_dbd_num_tuples(dbd->driver, res);
	int cols = apr_dbd_num_cols(dbd->driver, res);
	while(!apr_dbd_get_row(dbd->driver, cmd->temp_pool, res, &row, -1))
	{
		char **new = apr_array_push(contents);
		line = apr_pstrdup(cmd->temp_pool, "");
		for(int i = 0; i < cols; i++)
		{
			char * col = (char*)apr_dbd_get_entry(dbd->driver, row, i);
			line = apr_pstrcat(cmd->temp_pool, line, " ", col, NULL);
			debug_printf("Col %d: %s, line: %s\n", i, col, line);
		}
		line = apr_pstrcat(cmd->temp_pool, line, "\n", NULL);
		*new = apr_pstrdup(cmd->temp_pool, line);
	}
	ap_dbd_close(cmd->server, dbd);

	/* the current "config file" is replaced by a string array...
       at the end of processing the array, the initial config file
       will be returned there (see next_one) so as to go on. */
    cmd->config_file = make_array_config(cmd->temp_pool, contents, where, cmd->config_file, &cmd->config_file);
	return NULL;
}

// Here the live config of apache starts
// Lets try it...

typedef struct {
	const char * servername;
	apr_array_header_t * serveraliases;
	int vhost_type;
	const char * target;
	const char * ssl_pem_file;
	const char * ssl_crt_file;
	server_rec * server;
} ap_config_server;

/// server_rec, 
/// @see mod_vhost_alias.c:

/* optional functions imported from mod_dbd */
static APR_OPTIONAL_FN_TYPE(ap_dbd_prepare) *dbd_prepare_fn = NULL;
static APR_OPTIONAL_FN_TYPE(ap_dbd_acquire) *dbd_acquire_fn = NULL;

static void * create_server_config(apr_pool_t *p, server_rec *s)
{
	debug_printf("Server rec name: %s\n", s->context);
	if (!dbd_prepare_fn) {
		dbd_prepare_fn = 
					APR_RETRIEVE_OPTIONAL_FN(ap_dbd_prepare);
		dbd_acquire_fn = 
					APR_RETRIEVE_OPTIONAL_FN(ap_dbd_acquire);
	}
    if (!dbd_prepare_fn || !dbd_acquire_fn)
        return "mod_dbd must be enabled to use ap_config_mod";

	return NULL;
}

// The vhosts should only be appended, not replaced.
static void * merge_server_config(apr_pool_t *p, void *parentv, void *childv)
{
	debug_line;
	return NULL;
}

// In Configfile you can give these args:
// GetArgs "Arg 1" "Arg 2" "Arg 3" Arg4
static const char *get_args(cmd_parms *cmd, void *dc, int argc, char *const argv[])
{
	FILE *logfile = fopen(LOGFILE, "w+");
	for(int i = 0; i < argc; i++)
	{
		fprintf(logfile, "Arg %d: %s\n", i, argv[i]);
	}
	fclose(logfile);
}

static const command_rec mod_cmds[] = {
    AP_INIT_RAW_ARGS(EXEC_SQL, exec_sql, NULL, EXEC_ON_READ | OR_ALL, "Use of a command."),
// 	AP_INIT_TAKE_ARGV("GetArgs", get_args, NULL, EXEC_ON_READ | OR_ALL, "Test for parsing args."),
	{NULL}
};

static int translate_path(request_rec *r)
{
    int nelts = r->server->names->nelts;
	debug_printf("Server_Rec: %s, Hostname: %s, nelts: %d\n", r->server->server_hostname, r->hostname, nelts);
	debug_printf("Server Context: %s\n", r->server->context);
	char * document_root = ap_context_document_root(r);
	debug_printf("Document Root: %s\n", document_root);
	//debug_printf("Server Module Config: %s\n", config[0]);
    char **names = (char **) r->server->names->elts;

    for (int i = 0; i < nelts; i++) {
		debug_printf("El %d: %s\n", i, names[i]);
	}
	//return DECLINED;
	return OK;
}

static void register_hooks(apr_pool_t *p)
{
    static const char * const aszPre[]={ "mod_macro.c", "mod_dbd.c", NULL };
	debug_line;
	
	//ap_hook_handler(translate_path, NULL, NULL, APR_HOOK_FIRST);
    ap_hook_translate_name(translate_path, aszPre, NULL, APR_HOOK_MIDDLE);
}


AP_DECLARE_MODULE(mod_db_config) = {
    STANDARD20_MODULE_STUFF,    /* common stuff */
        NULL,                   /* create per-directory config */
        NULL,                   /* merge per-directory config structures */
        create_server_config,                   /* create per-server config structure */
        merge_server_config,                   /* merge per-server config structures */
        mod_cmds,               /* configuration commands */
        register_hooks                    /* register hooks */
};
