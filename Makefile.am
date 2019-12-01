AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = ap_config ap_configfs
noinst_LIBRARIES = lib_apache_config.a
lib_apache_config_a_SOURCES = ap_db_vhostreader.c odbc_functions.c globals.c
ap_config_SOURCES = ap_config.c
ap_config_LDADD = lib_apache_config.a
ap_config_LDFLAGS = 
ap_configfs_SOURCES = ap_configfs.c
ap_configfs_LDADD = lib_apache_config.a
ap_configfs_CFLAGS = -D_FILE_OFFSET_BITS=64
#AM_LDFLAGS = -l
