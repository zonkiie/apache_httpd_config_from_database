#include <includes.h>

void free_hdbc(SQLHDBC * hdbc)
{
    if (* hdbc!=SQL_NULL_HDBC) {
        SQLDisconnect(* hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, * hdbc);
		* hdbc = SQL_NULL_HDBC;
    }
}

void free_hstmt(SQLHSTMT * hstmt)
{
	if (* hstmt!=SQL_NULL_HSTMT) {
        SQLFreeHandle(SQL_HANDLE_STMT, * hstmt);
		* hstmt = SQL_NULL_HSTMT;
	}
}

void free_henv(SQLHENV * env)
{
    if (*env!=SQL_NULL_HENV) {
		SQLFreeHandle(SQL_HANDLE_ENV, *env);
		*env = SQL_NULL_HENV;
    }
}

bool odbc_is_ok(SQLRETURN value)
{
	if(value == SQL_SUCCESS || value == SQL_SUCCESS_WITH_INFO) return true;
	else return false;
}

void get_odbc_datasources()
{
	SQLHENV env;
	char dsn[256];
	char desc[256];
	SQLSMALLINT dsn_ret;
	SQLSMALLINT desc_ret;
	SQLUSMALLINT direction;
	SQLRETURN ret;

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
	SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

	direction = SQL_FETCH_FIRST;
	while(SQL_SUCCEEDED(ret = SQLDataSources(env, direction,
											dsn, sizeof(dsn), &dsn_ret,
											desc, sizeof(desc), &desc_ret))) {
		direction = SQL_FETCH_NEXT;
		printf("%s - %s\n", dsn, desc);
		if (ret == SQL_SUCCESS_WITH_INFO) printf("\tdata truncation\n");
	}
	SQLFreeHandle(SQL_HANDLE_ENV, env);
}

bool config_odbc(const char * driver, const char * dsn)
{
	// use SQLConfigDataSource
	bool ret = SQLConfigDataSource(NULL, ODBC_ADD_SYS_DSN, driver, dsn);
	if(!ret)
	{
		DWORD errCode;
		char errBuf[SQL_MAX_MESSAGE_LENGTH];
		WORD msgLen;
		SQLInstallerError(1, &errCode, errBuf, SQL_MAX_MESSAGE_LENGTH, &msgLen);
		fprintf(stderr, "File: %s, Line: %d, Message: %s\n", __FILE__, __LINE__, errBuf);
		return false;
	}
	return true;
}

/** allocate Environment handle and register version
*/
bool alloc_handles_env(SQLHENV * henv, SQLHDBC * hdbc)
{
	if(!odbc_is_ok(SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&(*henv))))
	{
		fprintf(stderr, "Error AllocHandle\n");
		return false;
	}
	if(!odbc_is_ok(SQLSetEnvAttr(*henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0)))
	{
		fprintf(stderr, "Error SetEnv\n");
		return false;
	}
	if(!odbc_is_ok(SQLAllocHandle(SQL_HANDLE_DBC, *henv, &(*hdbc))))
	{
		fprintf(stderr, "Error AllocHDB.\n");
		return false;
	}
	SQLSetConnectAttr(*hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)5, 0);
	return true;
}

bool extract_error(char ** target, SQLHDBC hdbc, const char * subject, SQLRETURN result)
{
	char message_stat[10],  message_text[200];
	SQLSMALLINT errortext_len;
	SQLINTEGER error_number;
	SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, message_stat, &error_number, message_text, 100, &errortext_len);
	if(asprintf(target, "%s %d %s (%d)", subject, result, message_text, error_number) > 0) return true;
	else return false;
}

bool odbc_connect(SQLHDBC hdbc, const char * datasource)
{
	SQLRETURN odbc_result;
	if(!odbc_is_ok(odbc_result = SQLDriverConnect(hdbc, NULL, (SQLCHAR*)datasource, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE)))
	{
		_cleanup_cstr_ char * error_text = NULL;
		extract_error(&error_text, hdbc, "Error SQLConnect", odbc_result);
		fprintf(stderr, "%s\n",error_text);
		return false;
	}
	return true;
}

bool odbc_allocate_statement(SQLHDBC hdbc, SQLHSTMT *odbc_statement)
{
	SQLRETURN odbc_result;
	if(!odbc_is_ok(odbc_result=SQLAllocHandle(SQL_HANDLE_STMT, hdbc, odbc_statement)))
	{
		_cleanup_cstr_ char * error_text = NULL;
		extract_error(&error_text, hdbc, "Error in AllocStatement ", odbc_result);
		fprintf(stderr, "%s\n",error_text);
		return false;
	}
	return true;
}

bool odbc_execute_query(SQLHDBC odbc_conn, SQLHSTMT odbc_statement, const char* query)
{
	SQLRETURN odbc_result;
	if(!odbc_is_ok(odbc_result=SQLExecDirect(odbc_statement,(SQLCHAR*)query,SQL_NTS)))
	{
		_cleanup_cstr_ char * error_text = NULL;
		extract_error(&error_text, odbc_conn, "Error in Select ", odbc_result);
		fprintf(stderr, "%s\n",error_text);
		return false;
	}
	return true;
}

