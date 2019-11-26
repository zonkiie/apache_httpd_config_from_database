#ifndef __ap_db_vhostreader__
#define __ap_db_vhostreader__


//void exec_obdc_query(const char * datasource, const char * username, const char * password, char * query);
void exec_odbc_query_example(const char * datasource, const char * query);
void exec_odbc_query(char ** result_string, const char * datasource, const char * query);
char * build_string(const char * datasource, const char * query);

#endif
