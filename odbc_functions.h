#ifndef __odbc_functions__
#define __odbc_functions__

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sqlucode.h>
#include <odbcinst.h>

#define _cleanup_hdbc_ __attribute((cleanup(free_hdbc)))
#define _cleanup_hstmt_ __attribute((cleanup(free_hstmt)))
#define _cleanup_henv_ __attribute((cleanup(free_henv)))
void free_hdbc(SQLHDBC * hdbc);
void free_hstmt(SQLHSTMT * hstmt);
void free_henv(SQLHENV * env);
bool config_odbc(const char * driver, const char * dsn);
bool odbc_is_ok(SQLRETURN value);
bool alloc_handles_env(SQLHENV * henv, SQLHDBC * hdbc);
bool odbc_connect(SQLHDBC hdbc, const char * datasource);
bool odbc_allocate_statement(SQLHDBC hdbc, SQLHSTMT *odbc_statement);
bool extract_error(char ** target, SQLHDBC hdbc, const char * subject, SQLRETURN result);
void get_odbc_datasources();
bool odbc_execute_query(SQLHDBC odbc_conn, SQLHSTMT odbc_statement, const char* query);

#endif
