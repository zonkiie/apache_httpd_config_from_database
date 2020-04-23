#include <includes.h>

void remove_from_string(char * string, size_t start, size_t count)
{
	char * reststring = string + start + count;
	if(*reststring == NULL) return;
	size_t full_length = strlen(string);
	size_t restlen = full_length - start - count;
	memmove((string + start), reststring, restlen);
	memset(string + full_length - count, 0, count);
}

void insert_into_string(char * string, size_t offset, char * to_insert)
{
	size_t insert_length = strlen(to_insert), new_pos_rest = offset + insert_length, current_string_length = strlen(string);
	char * left = (string + offset);
	char * right = (string + new_pos_rest);
	size_t to_move = strlen(left);
	memmove(right, left, to_move);
	memcpy(left, to_insert, insert_length);
	memset(string + current_string_length + insert_length, 0, 1);
}

int substr_from_offsets(char ** dest, const char * text, size_t start, size_t end)
{
	if(end < start) return -1;
	*dest = strndup(text + start, end - start);
	return strlen(*dest);
}

const char * pcre_str_error(int ecode)
{
	const char * errortext;
	switch(ecode) {
		case PCRE_ERROR_NOMATCH      : errortext = "String did not match the pattern";        break;
		case PCRE_ERROR_NULL         : errortext = "Something was null";                      break;
		case PCRE_ERROR_BADOPTION    : errortext = "A bad option was passed";                 break;
		case PCRE_ERROR_BADMAGIC     : errortext = "Magic number bad (compiled re corrupt?)"; break;
		case PCRE_ERROR_UNKNOWN_NODE : errortext = "Something kooky in the compiled re";      break;
		case PCRE_ERROR_NOMEMORY     : errortext = "Ran out of memory";                       break;
		default                      : errortext = "Unknown error";                           break;
	} /* end switch */
	return errortext;
}

void cleanup_pcre_t(pcre **re)
{
	if(*re != NULL)
	{
		pcre_free(*re);
		*re = NULL;
	}
}

int pcre_match_string_all(pregmatches** matches, const char * text, const char * search, int flags)
{
	if(flags == -1) flags = PCRE_NEWLINE_ANY|PCRE_DOTALL|PCRE_CASELESS|PCRE_UNGREEDY;  /* PCRE_NEWLINE_ANY|PCRE_DOTALL|PCRE_UNGREEDY|PCRE_CASELESS, */
	_cleanup_pcre_t_ pcre *re;
	const char *error;
	int erroffset, ovector[OVECCOUNT], subject_length, rc, offset = 0;
	*matches = (pregmatches*)malloc(sizeof(pregmatches*)+sizeof(pregmatch));
	(*matches)->count = 0;
	
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
		fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
		return 1;
	}
	
	while((rc = pcre_exec(
			re,											/* the compiled pattern */
			NULL,										/* no extra data - we didn't study the pattern */
			text,										/* the subject string */
			(subject_length = (int)strlen(text)),		/* the length of the subject */
			offset,										/* start at offset 0 in the subject */
			PCRE_NEWLINE_ANY,							/* default options */
			ovector,									/* output vector for substring information */
			OVECCOUNT)									/* number of elements in the output vector */
	) > 0)
	{
		
		if(rc < -1)
		{
			const char * errortext = pcre_str_error(rc);
			fprintf(stderr, "%s\n", errortext);
			return -1;
		}
		
		//fprintf(stderr, "Start: %d, End: %d\n", ovector[0], ovector[1]); 
		(*matches)->matches[(*matches)->count] = (pregmatch){.start = ovector[0], .end = ovector[1]};
		offset = ovector[1];
		(*matches) = realloc((*matches), sizeof(pregmatches*) + ((++(*matches)->count) + 1)*sizeof(pregmatch));
	}
	return (*matches)->count;
}

int pcre_replace_r(char ** text, const char * search, const char * replacement, int flags)
{
	if(flags == -1) flags = PCRE_NEWLINE_ANY|PCRE_DOTALL|PCRE_CASELESS|PCRE_UNGREEDY;  /* PCRE_NEWLINE_ANY|PCRE_DOTALL|PCRE_UNGREEDY|PCRE_CASELESS, */
	pregmatches *matches = NULL;
	int matchcount = pcre_match_string_all(&matches, *text, search, flags);
	
	for(int j = matchcount - 1; j >= 0; j--)
	{
		int matchlength = matches->matches[j].end - matches->matches[j].start;
		remove_from_string(*text, matches->matches[j].start, matchlength);
		*text = realloc(*text, strlen(*text) + strlen(replacement) + 1);
		insert_into_string(*text, matches->matches[j].start, replacement);
		
		/*memmove(*text + matches->matches[j].start + strlen(replacement), *text + matches->matches[j].start, strlen(*text + matches->matches[j].start));
		memcpy(*text + matches->matches[j].start, replacement, strlen(replacement));*/
	}
	free(matches);
	if(*text != NULL) *text = realloc(*text, strlen(*text) + 1);
	return 0;
}

int get_pcre_matches_r(char *** strmatches, const char * text, const char * search, int flags)
{
	pregmatches *matches = NULL;
	int matchcount = pcre_match_string_all(&matches, text, search, flags);
	*strmatches = (char**)malloc((matchcount + 1) * sizeof(char*));
	for(int i = 0; i < matchcount; i++)
	{
		char * singlematch = NULL;
		substr_from_offsets(&singlematch, text, matches->matches[i].start, matches->matches[i].end);
		(*strmatches)[i] = singlematch;
	}
	(*strmatches)[matchcount] = NULL;
	free(matches);
	return matchcount;
}

int get_variable_names_r(char *** vars, const char * text)
{
	return get_pcre_matches_r(vars, text, "\\$\\w+\\b", -1);
}

char ** get_pcre_matches(const char * text, const char * search, int flags)
{
	return NULL;
}
