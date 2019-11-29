/*******
 * Structure FROM: https://github.com/libfuse/libfuse/blob/master/example/null.c
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <includes.h>

void cleanup()
{
	free_cstr(&result_string);
	free_cstr(&dsn);
}

/*static int null_getattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)*/
static int null_getattr(const char *path, struct stat *stbuf)
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

static int null_open(const char *path, struct fuse_file_info *fi)
{
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	return 0;
}

static int null_read(const char *path, char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

// 	if (offset >= (1ULL << 32))
// 		return 0;

	memset(buf, 0, size);
	//strcpy(buf, "Bla");
	time_t current_time = time(NULL);
	if(current_time - last_update > update_intervall)
	{
		last_update = current_time;
		free_cstr(&result_string);
		exec_odbc_query(&result_string, dsn, query);
	}
	strcpy(buf, result_string + offset);
	return strlen(buf);
	//return size;
}

static struct fuse_operations null_oper = {
	.getattr	= null_getattr,
	.open		= null_open,
	.read		= null_read,
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
			exit(0);
	}
	return 1;
}

void init_configfile_list()
{
	const char * configfile_list[] = {"/etc/ap_configfs.conf", "./ap_configfs.conf", NULL};
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
	struct params cparams;

    memset(&cparams, 0, sizeof(cparams));
	strcpy(configfile, "");
	init_configfile_list();
	//_cleanup_cstr_ char * vhostlist = NULL;

	atexit(cleanup);
	//return fuse_main(argc, argv, &null_oper, NULL);
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	fuse_opt_parse(&args, &cparams, ap_configfs_opts, parse_cmdline_options);
	if(cparams.configfile != NULL) strcpy(configfile, cparams.configfile);
	if(cparams.dsn != NULL) reassign_cstr(&dsn, cparams.dsn);
	if(cparams.query != NULL) reassign_cstr(&query, cparams.query);
	if(strcmp(configfile, "")) parse_configfile(configfile);

	//return fuse_main(args.argc, args.argv, &null_oper, NULL);
}
