#ifndef __debug_macros__
#define __debug_macros__

#define debug_line { FILE *logfile = fopen(LOGFILE_CMD, "a+"); fprintf(logfile, "%s (line %d)\n", __FUNCTION__, __LINE__); fclose(logfile); }
#define debug_printf(formatstring, ...) { FILE *logfile = fopen(LOGFILE_CMD, "a+"); fprintf(logfile, "%s (line %d): ", __FUNCTION__, __LINE__); fprintf(logfile, formatstring, __VA_ARGS__);  fclose(logfile); }

#endif
