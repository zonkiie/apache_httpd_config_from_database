#include <includes.h>

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

bool parse_configfile_with_storage(const char * configfile, config_storage storage[])
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
			if(parse_configstring(line, key, value) >= 0) 
			{
				for(int st_i = 0; storage[st_i].key != NULL && storage[st_i].storage != NULL; st_i++)
				{
					if(!strcmp(storage[st_i].key, key))
					{
						storage[st_i].storage = strdup(value);
						break;
					}
				}
			}
			free_cstr(&line);
		}
		fclose(f);
		return true;
	} else {
		fprintf(stderr, "Config file %s not found!\n", configfile);
		return false;
	}
}
