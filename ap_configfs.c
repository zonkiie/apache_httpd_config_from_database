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
		exec_odbc_query(&result_string, dsn, "select 'Use Vhost example example.com /home/example.com' as entry union select 'Use VHost example2 example2.com /home/example2.com' as entry union select 'Use Redirect example3 example3.com https://www.google.de' as entry;");
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

int main(int argc, char *argv[])
{
// 	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
// 	struct fuse_cmdline_opts opts;
// 	struct stat stbuf;
// 
// 	if (fuse_parse_cmdline(&args, &opts) != 0)
// 		return 1;
// 	fuse_opt_free_args(&args);
// 
// 	if (!opts.mountpoint) {
// 		fprintf(stderr, "missing mountpoint parameter\n");
// 		return 1;
// 	}
// 
// 	if (stat(opts.mountpoint, &stbuf) == -1) {
// 		fprintf(stderr ,"failed to access mountpoint %s: %s\n",
// 			opts.mountpoint, strerror(errno));
// 		free(opts.mountpoint);
// 		return 1;
// 	}
// 	free(opts.mountpoint);
// 	if (!S_ISREG(stbuf.st_mode)) {
// 		fprintf(stderr, "mountpoint is not a regular file\n");
// 		return 1;
// 	}
// 
	//get_odbc_datasources();
	
	//_cleanup_cstr_ char * dsn = strdup("DSN=sqlite\0Database=/tmp/testdb.sqlite\0\0");
	/*_cleanup_cstr_ char * driver = strdup("SQLite3");
	if(!config_odbc(driver, dsn)) return EXIT_FAILURE;*/
	//exec_obdc_query(dsn, "select 'two' as one, 'four' as two, 'six' as three union select 10+10 as one, 20+20 as two, 30+30 as three;");
	strcpy(configfile, "");
	dsn = strdup("Driver=SQLITE3;Database=/tmp/testdb.sqlite;");
	/*
	_cleanup_cstr_ char * vhostlist = NULL;
	exec_odbc_query(&vhostlist, dsn, "select 'Use Vhost example example.com /home/example.com' as entry union select 'Use VHost example2 example2.com /home/example2.com' as entry union select 'Use Redirect example3 example3.com https://www.google.de' as entry;");
	printf("Vhostlist: %s\n", vhostlist);
	return 0;*/
	
	int show_help = 0, show_vhostlist = 0;
	int option_index = 0, c = 0;
	while(1)
	{
		static struct option long_options[] = {
			{"help", no_argument, 0, 0},
			{"configfile", required_argument, 0, 0},
			{"dsn", required_argument, 0, 0},
			{"query", required_argument, 0, 0},
			{"update_intervall", required_argument, 0, 0},
			{0, 0, 0, 0}
		};
		c = getopt_long (argc, argv, "hc:", long_options, &option_index);
		if(c == -1) break;
		switch(c)
		{
			case 0:
			{
				char* oname = (char*)long_options[option_index].name;
				if(!strcmp(oname, "help")) show_help = 1;
				if(!strcmp(oname, "dsn")) reassign_cstr(&dsn, optarg);
				if(!strcmp(oname, "configfile")) strcpy(configfile, optarg);
				if(!strcmp(oname, "query")) reassign_cstr(&query, optarg);
				if(!strcmp(oname, "update_intervall")) update_intervall = atoi(optarg);
				break;
			}
			case 1:
			{
				break;
			}
			case 'h':
			{
				show_help = 1;
				break;
			}
			case 'c':
			{
				strcpy(configfile, optarg);
				break;
			}
			default:
				abort();
		}
	}
	if(strcmp(configfile, ""))
	{
		parse_configfile(configfile);
	}
	if(show_help)
	{
		puts("Command line has priority over Config file!");
		puts("--configfile=<configfile>, -c=<configfile>");
		puts("--dsn=<dsn>");
		puts("--query=<query>");
		exit(0);
	}

	atexit(cleanup);
	return fuse_main(argc, argv, &null_oper, NULL);
}
