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

// source from: http://www.unixodbc.org/doc/ProgrammerManual/Tutorial/resul.html , https://www.easysoft.com/developer/languages/c/odbc_tutorial.html
// https://www.easysoft.com/developer/languages/c/examples/ReadingMultipleLongTextFields.html
//void exec_obdc_query(const char * datasource, const char * username, const char * password, char * query)
void exec_obdc_query(const char * datasource, const char * query)
{
	SQLHSTMT V_OD_hstmt = SQL_NULL_HSTMT;;   // Handle for a statement
	SQLINTEGER V_OD_err;
	SQLLEN V_OD_rowanz, bind_err;
	SQLSMALLINT V_OD_mlen,V_OD_colanz;
	SQLRETURN V_OD_erg;
	SQLHDBC V_OD_hdbc = SQL_NULL_HDBC;
	SQLHENV V_OD_Env = SQL_NULL_HENV;
	char
	V_OD_stat[10], // Status SQL
	V_OD_msg[200];
	// 1. allocate Environment handle and register version 
	V_OD_erg=SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&V_OD_Env);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		fprintf(stderr, "Error AllocHandle\n");
		exit(0);
	}
	V_OD_erg=SQLSetEnvAttr(V_OD_Env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0); 
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		fprintf(stderr, "Error SetEnv\n");
		goto cleanup;
	}
	V_OD_erg = SQLAllocHandle(SQL_HANDLE_DBC, V_OD_Env, &V_OD_hdbc); 
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		fprintf(stderr, "Error AllocHDB %d.\n",V_OD_erg);
		goto cleanup;
	}
	SQLSetConnectAttr(V_OD_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)5, 0);
	// 3. Connect to the datasource "web" 
	//V_OD_erg = SQLConnect(V_OD_hdbc, (SQLCHAR*) datasource, SQL_NTS, (SQLCHAR*) username, SQL_NTS, (SQLCHAR*) password, SQL_NTS);
	V_OD_erg = SQLDriverConnect(V_OD_hdbc, NULL, (SQLCHAR*)datasource, SQL_NTS,
									NULL, 0, NULL,
									SQL_DRIVER_COMPLETE);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		fprintf(stderr, "Error SQLConnect %d\n",V_OD_erg);
		SQLGetDiagRec(SQL_HANDLE_DBC, V_OD_hdbc,1, 
		              V_OD_stat, &V_OD_err,V_OD_msg,100,&V_OD_mlen);
		fprintf(stderr, "%s (%d)\n",V_OD_msg,V_OD_err);
		SQLFreeHandle(SQL_HANDLE_ENV, V_OD_Env);
		exit(0);
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
	}
	V_OD_erg=SQLNumResultCols(V_OD_hstmt,&V_OD_colanz);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) goto cleanup;
	printf("Number of Columns %d\n",V_OD_colanz);
	V_OD_erg=SQLRowCount(V_OD_hstmt,&V_OD_rowanz);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		fprintf(stderr, "Number of RowCount %d\n",V_OD_erg);
		goto cleanup;
	}
	printf("Number of Rows %ld\n",V_OD_rowanz);
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
	if (V_OD_hstmt!=SQL_NULL_HSTMT) {
        SQLFreeHandle(SQL_HANDLE_STMT, V_OD_hstmt);
		V_OD_hstmt = SQL_NULL_HSTMT;
	}

    if (V_OD_hdbc!=SQL_NULL_HDBC) {
        SQLDisconnect(V_OD_hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, V_OD_hdbc);
		V_OD_hdbc = SQL_NULL_HDBC;
    }

    if (V_OD_Env!=SQL_NULL_HENV) {
		SQLFreeHandle(SQL_HANDLE_ENV, V_OD_Env);
		V_OD_Env = SQL_NULL_HENV;
    }
	
}
