#ifndef __configfile_parsing__
#define __configfile_parsing__

typedef struct {
	char * key;
	void * storage;
} config_storage;

int parse_configstring(const char * line, char * key, char * value);
bool parse_configfile_callback(const char * configfile, void(*handle_function)(const char * key, const char * value));
bool parse_configfile_with_storage(const char * configfile, config_storage storage[]);

#endif
