#ifndef __pcretools__
#define __pcretools__

#define OVECCOUNT 30    /* should be a multiple of 3 */

void remove_from_string(char * string, size_t start, size_t count);
void insert_into_string(char * string, size_t offset, char * to_insert);
int preg_replace_r(char ** text, char * search, char * replacement, int flags);

#endif
