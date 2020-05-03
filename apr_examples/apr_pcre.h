#ifndef __apr_pcre__
#define __apr_pcre__

static const char *collect_section_string(cmd_parms *cmd, void *dummy, const char *arg);
static int build_vhost_entry_from_template_r(cmd_parms *cmd, char ** text, int num_els, char *const els[]);
static const char * do_replacements(cmd_parms *cmd, void *dummy, int argc, char *const argv[]);

#endif
