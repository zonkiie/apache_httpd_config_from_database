AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = ap_config ap_configfs
noinst_LIBRARIES = lib_apache_config.a 
lib_apache_config_a_SOURCES = ap_db_vhostreader.c odbc_functions.c globals.c configfile_parsing.c pcretools.c
lib_apache_config_a_CFLAGS = -fPIC
ap_config_SOURCES = ap_config.c
ap_config_LDADD = lib_apache_config.a
ap_config_LDFLAGS = 
ap_configfs_SOURCES = ap_configfs.c
ap_configfs_LDADD = lib_apache_config.a
ap_configfs_CFLAGS = -D_FILE_OFFSET_BITS=64
CLEANFILES = *.so *.la *.lo *.slo
#AM_LDFLAGS = -l

#ap_config_mod_so_SOURCES = ap_config_mod.c
#ap_config_mod_so_LDADD = lib_apache_config.a
#ap_config_mod_so_LDFLAGS = 
#ap_config_mod_so_CFLAGS = -I/usr/include/apr-1.0/ -I/usr/include/apache2/ -I.
#ap_config_mod_so_COMPILE = apxs -I. -a -c ap_config_mod.c
# install-exec-local: ap_config_mod.c
# 	apxs -I. -a -c $^
all-local: mod_cmd_config.so mod_db_config.so
mod_cmd_config.so: mod_cmd_config.c lib_apache_config.a
	apxs -I. -a -lpcre -c -o $@ $^
mod_db_config.so: mod_db_config.c lib_apache_config.a
	apxs -I. -a -lpcre -c -o $@ $^
apmod:
	sudo apxs -I. -a -i -lpcre -c mod_cmd_config.c lib_apache_config.a
	sudo apxs -I. -a -i -lpcre -c mod_db_config.c lib_apache_config.a
	sudo systemctl restart apache2
