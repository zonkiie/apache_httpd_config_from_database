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

## Memory debugging of apache module I've used
    sudo valgrind --trace-children=yes apache2ctl restart

# ap_configfs
## Example for configfile (/etc/ap_configfs.conf, ./ap_configfs.conf)
    dsn=Driver=SQLITE3;Database=/tmp/testdb.sqlite
    query=select 'Use Vhost example example.com /home/example.com' as entry union select 'Use VHost example2 example2.com /home/example2.com' as entry union select 'Use Redirect example3 example3.com https://www.google.de' as entry;
    mountpoint=my_mountpoint

# mod_db_config
mod_db_config has git it's own repository. You can find it at [https://github.com/zonkiie/mod_db_config](https://github.com/zonkiie/mod_db_config) .
