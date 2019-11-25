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

void free_fstream(FILE ** stream)
{
	if(*stream != NULL)
	{
		fclose(*stream);
		*stream = NULL;
	}
}

void exec_odbc_query_example(const char * datasource, const char * query)
{
	SQLHSTMT odbc_statement = SQL_NULL_HSTMT;   // Handle for a statement
	SQLHDBC odbc_conn = SQL_NULL_HDBC;
	SQLHENV odbc_env = SQL_NULL_HENV;
	SQLSMALLINT num_columns;
	SQLRETURN command_result;
	if(!alloc_handles_env(&odbc_env, &odbc_conn)) goto cleanup;
	if(!odbc_connect(odbc_conn, datasource)) goto cleanup;
	if(!odbc_allocate_statement(odbc_conn, &odbc_statement)) goto cleanup;
	if(!odbc_execute_query(odbc_conn, odbc_statement, query)) goto cleanup;
	if(!odbc_is_ok(command_result=SQLNumResultCols(odbc_statement,&num_columns))) goto cleanup;
	while((command_result=SQLFetch(odbc_statement)) == SQL_SUCCESS)
	{
		for(int col = 0; col < num_columns; col++)
		{
			const int maxDataLength = 1024;
			_cleanup_cstr_ char * colData = malloc(maxDataLength);
			SQLLEN retrievedDataLength = 0;
			int retcode = SQLGetData(odbc_statement, col + 1, SQL_C_CHAR, colData, maxDataLength, &retrievedDataLength);
			if(retcode == SQL_SUCCESS) printf("Col %d: %s, retrievedDataLength: %ld ", col, colData, retrievedDataLength);
		}
		printf("\n");
	}
cleanup:
	free_hstmt(&odbc_statement);
	free_hdbc(&odbc_conn);
	free_henv(&odbc_env);
}

void exec_odbc_query(char ** result_string, const char * datasource, const char * query)
{
	size_t size = 0;
	_autoclose_fstream_ FILE *stream = open_memstream (result_string, &size);
	const int maxDataLength = 1024;
	SQLHSTMT odbc_statement = SQL_NULL_HSTMT;   // Handle for a statement
	SQLHDBC odbc_conn = SQL_NULL_HDBC;
	SQLHENV odbc_env = SQL_NULL_HENV;
	SQLSMALLINT num_columns;
	SQLRETURN command_result;
	if(!alloc_handles_env(&odbc_env, &odbc_conn)) goto cleanup;
	if(!odbc_connect(odbc_conn, datasource)) goto cleanup;
	if(!odbc_allocate_statement(odbc_conn, &odbc_statement)) goto cleanup;
	if(!odbc_execute_query(odbc_conn, odbc_statement, query)) goto cleanup;
	if(!odbc_is_ok(command_result=SQLNumResultCols(odbc_statement,&num_columns))) goto cleanup;
	while((command_result=SQLFetch(odbc_statement)) == SQL_SUCCESS)
	{
		for(int col = 0; col < num_columns; col++)
		{
			_cleanup_cstr_ char * colData = malloc(maxDataLength);
			SQLLEN retrievedDataLength = 0;
			int retcode = SQLGetData(odbc_statement, col + 1, SQL_C_CHAR, colData, maxDataLength, &retrievedDataLength);
			if(retcode == SQL_SUCCESS) fprintf(stream, "%s ", colData);
		}
		fprintf(stream, "\n");
	}
	fflush (stream);
cleanup:
	free_hstmt(&odbc_statement);
	free_hdbc(&odbc_conn);
	free_henv(&odbc_env);
}

char * build_string(const char * dsn, const char * query)
{
	
}

