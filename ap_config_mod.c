/*
 * code from httpd/modules/core/mod_macro.c
 */
#include <includes.h>

#include "httpd.h"
#include "http_config.h"
#include "http_log.h"

#include "apr.h"
#include "apr_strings.h"
#include "apr_hash.h"

#define BEGIN_CMD "<Command"

static const char *cmd_section(cmd_parms * cmd, void *dummy, const char *arg)
{
	return NULL;
}

static const command_rec mod_cmds[] = {
	AP_INIT_RAW_ARGS(BEGIN_CMD, cmd_section, NULL, EXEC_ON_READ | OR_ALL, "Beginning of a cmd definition section."),
	{NULL}
};


AP_DECLARE_MODULE(mod_ap_config) = {
    STANDARD20_MODULE_STUFF,    /* common stuff */
        NULL,                   /* create per-directory config */
        NULL,                   /* merge per-directory config structures */
        NULL,                   /* create per-server config structure */
        NULL,                   /* merge per-server config structures */
        mod_cmds,             /* configuration commands */
        NULL                    /* register hooks */
};

// To be removed...
int main(){}
