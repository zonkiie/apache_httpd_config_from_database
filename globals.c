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

/**
 * size of an char ** array (number of elements until the first NULL element)
 * @param the array to count
 * @return number of elements in the array
 */
int get_carr_size(char ** carr)
{
	if(carr == NULL) return 0;
    int i = 0;
    for(; carr[i] != NULL; i++);
    return i;
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
	if(!write (fd_ziel, " ", 1)) return -1;
	char *ziel = NULL;
	if ((ziel =
        mmap (0, getpagesize(), PROT_READ | PROT_WRITE,
              MAP_SHARED, fd_ziel, 0)) == MAP_FAILED) {
      perror ("mmap");
      exit (EXIT_FAILURE);
	}
	strcpy(ziel, content);
	munmap(ziel, getpagesize());
	close(fd_ziel);
	return 0;
}

