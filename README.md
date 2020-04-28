# apache_httpd_config_from_database
Get apache http server config parts from database. These parts are either given as fuse filesystem, mmap'd file or dumped to file. Additionally, an apache module can be used.
# Warning
If using the apache module, you should know what you are doing!

## Required Libraries
 * unixodbc
 * unixodbc-dev
 * libfuse
 * libfuse-dev
 * libapr1-dev
 * libaprutil1
 * libaprutil1-dbd-odbc
 * libaprutil1-dbd-sqlite3
 * libaprutil1-dev
 * 

## How to compile
 * Create autotools Files with
    sh ./autogen.sh
 * Configure
    ./configure
 * Build
    make
    
## Command Line to compile and install module and restart apache
    sudo apxs -I. -a -i -c mod_cmd_config.c lib_apache_config.a; sudo systemctl restart apache2
or simply
	make apmod

## mod_cmd_config: Example apache config file /etc/apache2/mods-available/cmd_config.conf - only to see if arg parsing works - will be changed later
    <IfModule mod_cmd_config.c>
    <Command load_vhosts>
    Exec "/usr/sbin/ls"
    </Command>
    </IfModule>

## Example apache config file /etc/apache2/mods-available/db_config.conf
	DBC_DBDriver "sqlite3"
	DBC_DBDSN "/dev/shm/mydb.sqlite"
	ExecuteSQL "select '<VirtualHost *:80>'||x'0a'||'ServerName ' || vhost_name ||x'0a'||'DocumentRoot ' || target || x'0a'||'</VirtualHost>'||x'0a' as vhostdata from vhosts;"
## Description
Every Line which contains an ExecuteSQL Command is replaced with the Database Result of this query.
Note that this module does only support one Database Connection.
If you use a Database Server on the save host, please order the startup sequence so that the Database Server is started before the Apache Webserver.
Variable names must start with a Dollar Sign ($) and end with a non-alphabetical char. Variable names may contain an underscore.
### NOTE:
Because of naming conflicts with mod_dbd, we use here other directives than mod_dbd.
When this module was called mod_db_config while developing, everything worked fine when using the directive DBDriver. After renaming to db_config, I've got segfaults and illegal reads when debugging the module. Then after some sleepless nights, I tried to rename the directive DBDriver to DBC_DBDriver and after another minor bug, everything worked fine again. So the lesson fpr module developer is: Use apache2ctl -L to find out wether your directives are already defined. When you don't see a directive with your designated name, you can choose the name.

## Example DB Create script
see create_db.sh

## Note
You can't use mod_macro with mod_db_config or mod_cmd_config. I've implemented an own simple template "engine" for mod_db_config.

## Memory debugging of apache module I've used
    sudo valgrind --trace-children=yes apache2ctl restart

# ap_configfs
## Example for configfile (/etc/ap_configfs.conf, ./ap_configfs.conf)
    dsn=Driver=SQLITE3;Database=/tmp/testdb.sqlite
    query=select 'Use Vhost example example.com /home/example.com' as entry union select 'Use VHost example2 example2.com /home/example2.com' as entry union select 'Use Redirect example3 example3.com https://www.google.de' as entry;
    mountpoint=my_mountpoint
