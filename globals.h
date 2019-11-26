#ifndef __globals__
#define __globals__

#define _cleanup_cstr_ __attribute((cleanup(free_cstr)))
#define _autoclose_fstream_ __attribute((cleanup(free_fstream)))
void free_cstr(char ** str);
void free_fstream(FILE ** stream);

extern char configfile[PATH_MAX];
extern char * result_string;
extern time_t last_update;
extern char * query;


#endif
