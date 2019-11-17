#ifndef __ap_db_vhostreader__
#define __ap_db_vhostreader__

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sqlucode.h>
#include <odbcinst.h>

#define _cleanup_cstr_ __attribute((cleanup(free_cstr))) 
void free_cstr(char ** str);
void get_odbc_datasources();

#endif
