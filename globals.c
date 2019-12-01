#include <includes.h>

char configfile[PATH_MAX];
char * result_string = NULL;
volatile time_t last_update = 0;
volatile time_t update_intervall = 30;
char * query = NULL;
char * dsn = NULL;
char * mountpoint = NULL;

/**
 * for usage with cleanup attribute
 */
void free_cstr(char ** str)
{
	if(*str == NULL) return;
	free(*str);
	*str = NULL;
}

/**
*  Iterate through C array and free all Elements
*/
void free_carr(char ***carr)
{
    if(*carr == NULL) return;
    for(int i = 0; (*carr)[i] != NULL; i++)
	{
		free((*carr)[i]);
	}
	free(*carr);
}

void free_fstream(FILE ** stream)
{
	if(*stream != NULL)
	{
		fclose(*stream);
		*stream = NULL;
	}
}

void reassign_cstr(char **str, const char * value)
{
	free_cstr(str);
	*str = strdup(value);
}

int mmapwrite(const char *filename, const char * content)
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	umask (0);
	int fd_ziel = open (filename, O_RDWR | O_EXCL | O_CREAT, mode);
	if (lseek (fd_ziel, getpagesize() - 1, SEEK_SET) == -1) {
      perror ("lseek");
      exit (EXIT_FAILURE);
	}
	write (fd_ziel, " ", 1);
	char *ziel = NULL;
	if ((ziel =
        mmap (0, getpagesize(), PROT_READ | PROT_WRITE,
              MAP_SHARED, fd_ziel, 0)) == -1) {
      perror ("mmap");
      exit (EXIT_FAILURE);
	}
	strcpy(ziel, content);
	munmap(ziel, getpagesize());
	close(fd_ziel);
	return 0;
}

int parse_configstring(const char * line, char * key, char * value)
{
	size_t s = sscanf(line, "%[^#^=]=%[^\n]", key, value);
	return s<=0?-1:s;
}

bool parse_configfile_callback(const char * configfile, void(*handle_function)(const char * key, const char * value))
{
	FILE *f = fopen(configfile, "r");
	if(f != NULL)
	{
		_cleanup_cstr_ char *line = NULL;
		ssize_t linelen;
		size_t n;
		while((linelen = getline(&line, &n, f)) >= 0)
		{
			_cleanup_cstr_ char * key = (char*)malloc(256), * value = (char*)malloc(2048);
			if(parse_configstring(line, key, value) >= 0) handle_function(key, value);
			free_cstr(&line);
		}
		fclose(f);
		return true;
	} else {
		fprintf(stderr, "Config file %s not found!\n", configfile);
		return false;
	}
}

