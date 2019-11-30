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

int parse_configstring(const char * line, char * key, char * value)
{
	size_t s;
	s = sscanf(line, "%[^#^=]=%[^\n]", key, value);
	if(s <= 0) return -1;
	else return s;
}


void parse_configfile(const char * configfile)
{
	FILE *f = fopen(configfile, "r");
	if(f != NULL)
	{
		char *line = NULL;
		ssize_t linelen;
		size_t n;
		while((linelen = getline(&line, &n, f)) >= 0)
		{
			//char * key = (char*)alloca(256), * value = (char*)alloca(2048);
			_cleanup_cstr_ char * key = (char*)malloc(256), * value = (char*)malloc(2048);
			if(parse_configstring(line, key, value) >= 0)
			{
				if(!strcmp(key, "dsn") && dsn == NULL) reassign_cstr(&dsn, value);
				if(!strcmp(key, "query") && query == NULL) reassign_cstr(&query, value);
				if(!strcmp(key, "update_intervall") && value != NULL) update_intervall = atoi(value);
				if(!strcmp(key, "mountpoint") && value != NULL) reassign_cstr(&mountpoint, value);
			}
			free_cstr(&line);
			line = NULL;
		}
		free_cstr(&line);
		fclose(f);
	} else {
		fprintf(stderr, "Config file %s not found!\n", configfile);
		exit(-1);
	}
	
}
