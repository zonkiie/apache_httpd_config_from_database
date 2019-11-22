/*******
 * Structure FROM: https://github.com/libfuse/libfuse/blob/master/example/null.c
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <includes.h>

static int null_getattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)
{
	(void) fi;

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
	strcpy(buf, "Bla");
	return size;
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
	get_odbc_datasources();
	//_cleanup_cstr_ char * dsn = strdup("DSN=sqlite\0Database=/tmp/testdb.sqlite\0\0");
	_cleanup_cstr_ char * dsn = strdup("Driver=SQLITE3;Database=/tmp/testdb.sqlite;");
	/*_cleanup_cstr_ char * driver = strdup("SQLite3");
	if(!config_odbc(driver, dsn)) return EXIT_FAILURE;*/
	exec_obdc_query(dsn, "select 1+1, 2+2, 3+3;");
	return fuse_main(argc, argv, &null_oper, NULL);
}
