AC_INIT([ap_db_config], [0.1], [zonkiie])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
#LT_INIT
AC_PROG_CC
AC_PROG_RANLIB
AC_HAVE_LIBRARY(odbc)
AC_HAVE_LIBRARY(odbcinst)
AC_HAVE_LIBRARY(fuse)
AC_HAVE_LIBRARY(crypt)
AC_HAVE_LIBRARY(pthread)
AC_DEFINE([CONFIGFILES], [{"/etc/ap_configfs.conf", "./ap_configfs.conf", NULL}])
AC_CONFIG_FILES([Makefile])
#AC_CONFIG_HEADERS([config.h])
EXTRA_CFLAGS=-I.
AC_OUTPUT
