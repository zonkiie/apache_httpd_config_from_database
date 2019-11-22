#ifndef __ap_db_vhostreader__
#define __ap_db_vhostreader__

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sqlucode.h>
#include <odbcinst.h>

#define _cleanup_cstr_ __attribute((cleanup(free_cstr))) 
void free_cstr(char ** str);
bool config_odbc(const char * driver, const char * dsn);
//void exec_obdc_query(const char * datasource, const char * username, const char * password, char * query);
void exec_obdc_query(const char * datasource, const char * query);
void get_odbc_datasources();

#endif
