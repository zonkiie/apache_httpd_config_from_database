#ifndef __configfile_parsing__
#define __configfile_parsing__

typedef struct {
	char * key;
	void ** storage;
} config_storage;

#define _cleanup_config_storage_ __attribute((cleanup(free_config_storage)))

void free_config_storage(config_storage* storage);
int parse_configstring(const char * line, char * key, char * value);
bool parse_configfile_callback(const char * configfile, void(*handle_function)(const char * key, const char * value));
bool parse_configfile_with_storage(const char * configfile, config_storage storage[]);

#endif
