#ifndef __globals__
#define __globals__

extern char configfile[PATH_MAX];
extern char * result_string;
extern time_t last_update;
extern time_t update_intervall;
extern char * query;
extern char * dsn;
extern char * mountpoint;
extern char *optarg;

#define _cleanup_cstr_ __attribute((cleanup(free_cstr)))
#define _cleanup_carr_ __attribute((cleanup(free_carr))) 
#define _autoclose_fstream_ __attribute((cleanup(free_fstream)))
void free_cstr(char ** str);
void free_carr(char ***carr);
void free_fstream(FILE ** stream);
void reassign_cstr(char **str, const char * value);
int parse_configstring(const char * line, char * key, char * value);
void parse_configfile(const char * configfile);

#endif
