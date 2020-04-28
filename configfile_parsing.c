#include <includes.h>

void free_config_storage(config_storage* storage)
{
	for(int st_i = 0; storage[st_i].key != NULL && storage[st_i].storage != NULL; st_i++)
	{
		free(storage[st_i].storage);
		storage[st_i].storage = NULL;
	}
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

/**
 * Stores the configuration data in variables.
 * @param configfile: The configfile to parse
 * @param storage: An storage array of type config_storage. We can pass the key and the storage variable (should be char * string, passed by reference).
 * @return true if reading and parsing was successfull, false if an error occured
 * Please remember to free your variables or struct.
 * if in your configfile this line appears:
 * key=val
 * and your initialization routine has this code
 * <code>
	const char * configfile = "myconfig.conf";
	_cleanup_cstr_ char * key = NULL;
	parse_configfile_with_storage(configfile, (config_storage[]){{"key", &key},{NULL, NULL}});
	printf("Key: %s\n", key);
 * </code>
 * then the output will look like:
 * Key: val
 */
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
						*(storage[st_i].storage) = strdup(value);
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
