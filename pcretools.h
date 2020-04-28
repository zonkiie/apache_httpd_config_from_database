#ifndef __pcretools__
#define __pcretools__

#define OVECCOUNT 30    /* should be a multiple of 3 */
#define _cleanup_pcre_t_ __attribute((cleanup(cleanup_pcre_t)))

typedef struct
{
	int start, end;
} pregmatch;

// https://en.wikipedia.org/wiki/Flexible_array_member
typedef struct
{
	int count;
	pregmatch matches[];
} pregmatches;

void remove_from_string(char * string, size_t start, size_t count);
void insert_into_string(char * string, size_t offset, const char * to_insert);
int preg_replace_r(char ** text, char * search, char * replacement, int flags);
int substr_from_offsets(char ** dest, const char * text, size_t start, size_t end);
const char * pcre_str_error(int ecode);
void cleanup_pcre_t(pcre **re);
int pcre_match_string_all(pregmatches** matches, const char * text, const char * search, int flags);
int pcre_replace_r(char ** text, const char * search, const char * replacement, int flags);
int get_pcre_matches_r(char *** strmatches, const char * text, const char * search, int flags);
char ** get_pcre_matches(const char * text, const char * search, int flags);
int get_variable_names_r(char *** vars, const char * text);

#endif
