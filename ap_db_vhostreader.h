#ifndef __ap_db_vhostreader__
#define __ap_db_vhostreader__


#define _cleanup_cstr_ __attribute((cleanup(free_cstr)))
#define _autoclose_fstream_ __attribute((cleanup(free_fstream)))
void free_cstr(char ** str);
void free_fstream(FILE ** stream);

//void exec_obdc_query(const char * datasource, const char * username, const char * password, char * query);
void exec_odbc_query_example(const char * datasource, const char * query);
void exec_odbc_query(char ** result_string, const char * datasource, const char * query);
char * build_string(const char * dsn, const char * query);

#endif
