/*******
 * Structure FROM: https://github.com/libfuse/libfuse/blob/master/example/null.c
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <includes.h>

// To avoid a memory leak from fuse args, we define this macro
#define _cleanup_fuse_args_ __attribute((cleanup(free_fuse_args)))

pthread_mutex_t a_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_configfile_list();

void free_fuse_args(struct fuse_args *args)
{
	fuse_opt_free_args(args);
}

void cleanup()
{
	free_cstr(&result_string);
	free_cstr(&dsn);
	free_cstr(&query);
	free_cstr(&mountpoint);
}

void handle_configfile_entries(const char * key, const char * value)
{
	if(!strcmp(key, "dsn") && dsn == NULL) reassign_cstr(&dsn, value);
	if(!strcmp(key, "query") && query == NULL) reassign_cstr(&query, value);
	if(!strcmp(key, "update_intervall") && value != NULL) update_intervall = atoi(value);
	if(!strcmp(key, "mountpoint") && value != NULL) reassign_cstr(&mountpoint, value);
}

void signalhandler(int sig)
{
	if(sig == SIGHUP)
	{
		init_configfile_list();
		if(strcmp(configfile, "")) parse_configfile_callback(configfile, handle_configfile_entries);
	}
	if(sig == SIGTERM)
	{
		cleanup();
	}
}

/*static int null_getattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)*/
static int apconfigfs_getattr(const char *path, struct stat *stbuf)
{
	//(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	stbuf->st_mode = S_IFREG | 0644;
	stbuf->st_nlink = 1;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_size = (1ULL << 32); /* 4G */
	stbuf->st_blocks = 0;
	stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);

	return 0;
}

static int apconfigfs_open(const char *path, struct fuse_file_info *fi)
{
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	return 0;
}

static int apconfigfs_read(const char *path, char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;
	size_t len = 0;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

// 	if (offset >= (1ULL << 32))
// 		return 0;

	memset(buf, 0, size);
	strcpy(buf, "");
	time_t current_time = time(NULL);
	if(last_update == 0 || current_time - last_update > update_intervall)
	{
		int rc;
		//rc = pthread_mutex_lock(&a_mutex);
		while((rc = pthread_mutex_trylock(&a_mutex)) == EBUSY) sleep(1);
		free_cstr(&result_string);
		exec_odbc_query(&result_string, dsn, query);
		len = strlen(result_string);
		last_update = current_time;
		rc = pthread_mutex_unlock(&a_mutex);
	}
// 	fprintf(stderr, "Result string: %s, Offset: %ld, Size: %ld\n", result_string, offset, size);
	if(offset > len) return 0;
	if(result_string != NULL && result_string + offset != NULL) strcpy(buf, result_string + offset);
	return strlen(buf);
	//return size;
}

static struct fuse_operations apconfigfs_oper = {
	.getattr	= apconfigfs_getattr,
	.open		= apconfigfs_open,
	.read		= apconfigfs_read,
};

static void help()
{
	puts("Command line has priority over Config file!");
	puts("--configfile <configfile>, -c <configfile>");
	puts("--dsn <dsn>");
	puts("--query <query>");
	puts("--update_intervall <update_intervall>");
}

struct params {
     char *configfile;
     char *query;
     char *dsn;
	 int print_config;
	 int update_intervall;
};

enum {
     KEY_HELP,
     KEY_VERSION,
};

#define PARAMS_OPT(t, p, v) { t, offsetof(struct params, p), v }

static struct fuse_opt ap_configfs_opts[] = {
	PARAMS_OPT("--configfile %s", configfile, 0),
	PARAMS_OPT("--dsn %s", dsn, 0),
	PARAMS_OPT("--query %s", query, 0),
	PARAMS_OPT("--print_config", print_config, 1),
	PARAMS_OPT("--update_intervall %d", update_intervall, 0),
	FUSE_OPT_KEY("-h",             KEY_HELP),
	FUSE_OPT_KEY("--help",         KEY_HELP),
	FUSE_OPT_END
};


static int parse_cmdline_options(void *data, const char *arg, int key, struct fuse_args *outargs)
{
	switch(key)
	{
		case KEY_HELP:
			help();
			fuse_opt_add_arg(outargs, "-h");
			fuse_main(outargs->argc, outargs->argv, &apconfigfs_oper, NULL);
			free_fuse_args(outargs);
			exit(0);
	}
	return 1;
}

void init_configfile_list()
{
	const char * configfile_list[] = CONFIGFILES;
	for(int i = 0; configfile_list[i] != NULL; i++)
	{
		if(!access(configfile_list[i], F_OK))
		{
			strcpy(configfile, configfile_list[i]);
			break;
		}
	}
}


/**
 * @see https://github.com/libfuse/libfuse/wiki/Option-Parsing
 */
int main(int argc, char *argv[])
{
	signal(SIGHUP, signalhandler);
	signal(SIGTERM, signalhandler);
	struct params cparams;

    memset(&cparams, 0, sizeof(cparams));
	cparams.update_intervall = -1;
	strcpy(configfile, "");
	init_configfile_list();

	atexit(cleanup);
	_cleanup_fuse_args_ struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	fuse_opt_parse(&args, &cparams, ap_configfs_opts, parse_cmdline_options);
	if(cparams.configfile != NULL) strcpy(configfile, cparams.configfile);
	if(cparams.dsn != NULL) reassign_cstr(&dsn, cparams.dsn);
	if(cparams.query != NULL) reassign_cstr(&query, cparams.query);
	if(cparams.update_intervall != -1) update_intervall = cparams.update_intervall;
	if(strcmp(configfile, "")) parse_configfile_callback(configfile, handle_configfile_entries);
	// Preparation work for storing config in local instead of global variables.
	/*_cleanup_cstr_ char * bal = NULL;
	if(strcmp(configfile, "")) parse_configfile_with_storage(configfile, (config_storage[]){{"bal", &bal},{NULL, NULL}});
	printf("Bal: %s\n", bal);
	return(0);*/
	if(mountpoint != NULL) fuse_opt_add_arg(&args, mountpoint);
	if(cparams.print_config)
	{
		printf("Configfile: %s\ndsn: %s\nquery: %s\nconfigured mountpoint: %s\nupdate_intervall: %ld\n", configfile, dsn, query, mountpoint, update_intervall);
		return 0;
	}
	return fuse_main(args.argc, args.argv, &apconfigfs_oper, NULL);
}
