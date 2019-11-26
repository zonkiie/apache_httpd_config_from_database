#include <includes.h>

char configfile[PATH_MAX];
char * result_string = NULL;
time_t last_update;
char * query = NULL;

/**
 * for usage with cleanup attribute
 */
void free_cstr(char ** str)
{
	if(*str == NULL) return;
	free(*str);
	*str = NULL;
}

void free_fstream(FILE ** stream)
{
	if(*stream != NULL)
	{
		fclose(*stream);
		*stream = NULL;
	}
}


