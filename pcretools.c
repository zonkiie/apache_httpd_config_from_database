#include <includes.h>

void remove_from_string(char * string, size_t start, size_t count)
{
	char * reststring = string + start + count;
	size_t restlen = 0;
	restlen = strlen(reststring);
	size_t full_length = strlen(string);
	memmove((string + start), reststring, restlen);
	memset(string + full_length - count, 0, count);
}

void insert_into_string(char * string, size_t offset, char * to_insert)
{
	size_t new_pos_rest = offset + strlen(to_insert);
	char * left = (string + offset);
	char * right = (string + new_pos_rest);
	size_t to_move = strlen(left);
	memmove(right, left, to_move);
	memcpy(left, to_insert, strlen(to_insert));
}

int preg_replace_r(char ** text, char * search, char * replacement, int flags)
{
	if(flags == -1) flags = PCRE_NEWLINE_ANY|PCRE_DOTALL|PCRE_CASELESS;  /* PCRE_NEWLINE_ANY|PCRE_DOTALL|PCRE_UNGREEDY|PCRE_CASELESS, */
	pcre *re;
	const char *error;
	int erroffset, ovector[OVECCOUNT], j, subject_length;
	const char *submatch;

	
	subject_length = (int)strlen(*text);
	re = pcre_compile(
		search,               /* the pattern */
		                      /* default options */
		flags,
		&error,               /* for error message */
		&erroffset,           /* for error offset */
		NULL);                /* use default character tables */

	/* Compilation failed: print the error message and exit */

	if (re == NULL)
	{
		printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
		return 1;
	}
	
	int rc = pcre_exec(
		re,                   /* the compiled pattern */
		NULL,                 /* no extra data - we didn't study the pattern */
		*text,              /* the subject string */
		subject_length,       /* the length of the subject */
		0,                    /* start at offset 0 in the subject */
		PCRE_NEWLINE_ANY,                    /* default options */
		ovector,              /* output vector for substring information */
		OVECCOUNT);           /* number of elements in the output vector */
	
	if(rc <= 0)
	{
		switch(rc) {
			case PCRE_ERROR_NOMATCH      : fprintf(stderr, "String did not match the pattern\n");        break;
			case PCRE_ERROR_NULL         : fprintf(stderr, "Something was null\n");                      break;
			case PCRE_ERROR_BADOPTION    : fprintf(stderr, "A bad option was passed\n");                 break;
			case PCRE_ERROR_BADMAGIC     : fprintf(stderr, "Magic number bad (compiled re corrupt?)\n"); break;
			case PCRE_ERROR_UNKNOWN_NODE : fprintf(stderr, "Something kooky in the compiled re\n");      break;
			case PCRE_ERROR_NOMEMORY     : fprintf(stderr, "Ran out of memory\n");                       break;
			default                      : fprintf(stderr, "Unknown error\n");                           break;
		} /* end switch */
		return -1;
	}
	for(j=0; j<rc; j++) {
        pcre_get_substring(*text, ovector, rc, j, &(submatch));
        printf("Match(%2d/%2d): (%2d,%2d): '%s'\n", j, rc-1, ovector[j*2], ovector[j*2+1], submatch);
		pcre_free_substring(submatch);

      } /* end for */
	
	fprintf(stderr, "Len: %ld\n", strlen(*text));
	for(j = rc - 1; j >= 0; j--)
	{
		pcre_get_substring(*text, ovector, rc, j, &(submatch));
		int matchlength = ovector[j*2 + 1] - ovector[j*2];
		remove_from_string(*text, ovector[j*2], matchlength);
		*text = realloc(*text, strlen(*text) + strlen(replacement));
		//insert_into_string(*text, ovector[j*2], replacement);
		memmove(*text + ovector[j*2] + strlen(replacement), *text + ovector[j*2], strlen(*text + ovector[j*2] + 1));
		memcpy(*text + ovector[j*2], replacement, strlen(replacement));
		pcre_free_substring(submatch);
	}
	
	pcre_free(re);
	if(*text != NULL) *text = realloc(*text, strlen(*text) + 1);
	return 0;
}


