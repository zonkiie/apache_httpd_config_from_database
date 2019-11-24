#include <includes.h>

/**
 * for usage with cleanup attribute
 */
void free_cstr(char ** str)
{
	if(*str == NULL) return;
	free(*str);
	*str = NULL;
}

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

// source from: http://www.unixodbc.org/doc/ProgrammerManual/Tutorial/resul.html , https://www.easysoft.com/developer/languages/c/odbc_tutorial.html
// https://www.easysoft.com/developer/languages/c/examples/ReadingMultipleLongTextFields.html
//void exec_obdc_query(const char * datasource, const char * username, const char * password, char * query)
void exec_obdc_query(const char * datasource, const char * query)
{
	SQLHSTMT V_OD_hstmt = SQL_NULL_HSTMT;   // Handle for a statement
	SQLINTEGER V_OD_err;
	SQLLEN bind_err;
	SQLSMALLINT V_OD_mlen,V_OD_colanz;
	SQLRETURN V_OD_erg;
	SQLHDBC V_OD_hdbc = SQL_NULL_HDBC;
	SQLHENV V_OD_Env = SQL_NULL_HENV;
	char
	V_OD_stat[10], // Status SQL
	V_OD_msg[200];
	if(!alloc_handles_env(&V_OD_Env, &V_OD_hdbc)) goto cleanup;
	// 3. Connect to the datasource "web" 
	V_OD_erg = SQLDriverConnect(V_OD_hdbc, NULL, (SQLCHAR*)datasource, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		fprintf(stderr, "Error SQLConnect %d\n",V_OD_erg);
		SQLGetDiagRec(SQL_HANDLE_DBC, V_OD_hdbc,1, V_OD_stat, &V_OD_err,V_OD_msg,100,&V_OD_mlen);
		fprintf(stderr, "%s (%d)\n",V_OD_msg,V_OD_err);
		goto cleanup;
	}
	printf("Connected !\n");
	V_OD_erg=SQLAllocHandle(SQL_HANDLE_STMT, V_OD_hdbc, &V_OD_hstmt);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		fprintf(stderr, "Fehler im AllocStatement %d\n",V_OD_erg);
		SQLGetDiagRec(SQL_HANDLE_DBC, V_OD_hdbc,1, V_OD_stat,&V_OD_err,V_OD_msg,100,&V_OD_mlen);
		fprintf(stderr, "%s (%d)\n",V_OD_msg,V_OD_err);
		goto cleanup;
	}
	V_OD_erg=SQLExecDirect(V_OD_hstmt,(SQLCHAR*)query,SQL_NTS);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		fprintf(stderr, "Error in Select %d\n",V_OD_erg);
		SQLGetDiagRec(SQL_HANDLE_DBC, V_OD_hdbc,1, V_OD_stat,&V_OD_err,V_OD_msg,100,&V_OD_mlen);
		fprintf(stderr, "%s (%d)\n",V_OD_msg,V_OD_err);
		goto cleanup;
	}
	V_OD_erg=SQLNumResultCols(V_OD_hstmt,&V_OD_colanz);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) goto cleanup;
	printf("Number of Columns %d\n",V_OD_colanz);
	while((V_OD_erg=SQLFetch(V_OD_hstmt)) == SQL_SUCCESS)
	{
		for(int col = 0; col < V_OD_colanz; col++)
		{
			const int maxDataLength = 1024;
			_cleanup_cstr_ char * colData = malloc(maxDataLength);
			SQLLEN retrievedDataLength = 0;
			int retcode = SQLGetData(V_OD_hstmt, col + 1, SQL_C_CHAR, colData, maxDataLength, &retrievedDataLength);
			if(retcode == SQL_SUCCESS) printf("Col %d: %s, retrievedDataLength: %ld ", col, colData, retrievedDataLength);
		}
		printf("\n");
	}
cleanup:
	free_hstmt(&V_OD_hstmt);
	free_hdbc(&V_OD_hdbc);
	free_henv(&V_OD_Env);
}
