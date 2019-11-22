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
//void exec_obdc_query(const char * datasource, const char * username, const char * password, char * query)
void exec_obdc_query(const char * datasource, const char * query)
{
	SQLHSTMT V_OD_hstmt;   // Handle for a statement
	SQLINTEGER V_OD_err,V_OD_id,V_OD_rowanz;
	SQLSMALLINT V_OD_mlen,V_OD_colanz;
	SQLRETURN V_OD_erg;
	SQLHDBC V_OD_hdbc;
	SQLHENV V_OD_Env;
	char
	V_OD_stat[10], // Status SQL
	V_OD_buffer[200],
	V_OD_msg[200];
	SQLBindCol(V_OD_hstmt,1,SQL_C_CHAR, &V_OD_buffer,200,&V_OD_err);
	SQLBindCol(V_OD_hstmt,2,SQL_C_ULONG,&V_OD_id,sizeof(V_OD_id),&V_OD_err);
	// 1. allocate Environment handle and register version 
	V_OD_erg=SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&V_OD_Env);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		printf("Error AllocHandle\n");
		exit(0);
	}
	V_OD_erg=SQLSetEnvAttr(V_OD_Env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0); 
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		printf("Error SetEnv\n");
		SQLFreeHandle(SQL_HANDLE_ENV, V_OD_Env);
		exit(0);
	}
	V_OD_erg = SQLAllocHandle(SQL_HANDLE_DBC, V_OD_Env, &V_OD_hdbc); 
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		printf("Error AllocHDB %d.\n",V_OD_erg);
		SQLFreeHandle(SQL_HANDLE_ENV, V_OD_Env);
		exit(0);
	}
	SQLSetConnectAttr(V_OD_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)5, 0);
	// 3. Connect to the datasource "web" 
	/*V_OD_erg = SQLConnect(V_OD_hdbc, (SQLCHAR*) datasource, SQL_NTS,
                                     (SQLCHAR*) username, SQL_NTS,
                                     (SQLCHAR*) password, SQL_NTS);*/
	V_OD_erg = SQLDriverConnect(V_OD_hdbc, NULL, datasource, SQL_NTS,
									NULL, 0, NULL,
									SQL_DRIVER_COMPLETE);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		printf("Error SQLConnect %d\n",V_OD_erg);
		SQLGetDiagRec(SQL_HANDLE_DBC, V_OD_hdbc,1, 
		              V_OD_stat, &V_OD_err,V_OD_msg,100,&V_OD_mlen);
		printf("%s (%d)\n",V_OD_msg,V_OD_err);
		SQLFreeHandle(SQL_HANDLE_ENV, V_OD_Env);
		exit(0);
	}
	printf("Connected !\n");
	V_OD_erg=SQLAllocHandle(SQL_HANDLE_STMT, V_OD_hdbc, &V_OD_hstmt);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		printf("Fehler im AllocStatement %d\n",V_OD_erg);
		SQLGetDiagRec(SQL_HANDLE_DBC, V_OD_hdbc,1, V_OD_stat,&V_OD_err,V_OD_msg,100,&V_OD_mlen);
		printf("%s (%d)\n",V_OD_msg,V_OD_err);
        SQLDisconnect(V_OD_hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC,V_OD_hdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, V_OD_Env);
		exit(0);
	}
	V_OD_erg=SQLExecDirect(V_OD_hstmt,query,SQL_NTS);   
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		printf("Error in Select %d\n",V_OD_erg);
		SQLGetDiagRec(SQL_HANDLE_DBC, V_OD_hdbc,1, V_OD_stat,&V_OD_err,V_OD_msg,100,&V_OD_mlen);
		printf("%s (%d)\n",V_OD_msg,V_OD_err);
	}
	V_OD_erg=SQLNumResultCols(V_OD_hstmt,&V_OD_colanz);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		goto cleanup;
	}
	printf("Number of Columns %d\n",V_OD_colanz);
	V_OD_erg=SQLRowCount(V_OD_hstmt,&V_OD_rowanz);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
	{
		printf("Number ofRowCount %d\n",V_OD_erg);
		goto cleanup;
	}
	printf("Number of Rows %d\n",V_OD_rowanz);
	if(V_OD_rowanz > 0)
	{
		while((V_OD_erg=SQLFetch(V_OD_hstmt)) != SQL_NO_DATA)
		{
			printf("Result: %d %s\n",V_OD_id,V_OD_buffer);
			V_OD_erg=SQLFetch(V_OD_hstmt);  
		}
	}
cleanup:
	SQLFreeHandle(SQL_HANDLE_STMT,V_OD_hstmt);
	SQLDisconnect(V_OD_hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC,V_OD_hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, V_OD_Env);
	
}
