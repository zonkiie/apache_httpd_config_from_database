# apache_httpd_config_from_database
Get apache http server config parts from database. These parts are either given as fuse filesystem, mmap'd file or dumped to file.

## Info for autotools
 * [https://thoughtbot.com/blog/the-magic-behind-configure-make-make-install](https://thoughtbot.com/blog/the-magic-behind-configure-make-make-install)

## Tutorials which help with unixodbc
 * [http://www.unixodbc.org/doc/ProgrammerManual/Tutorial/resul.html](http://www.unixodbc.org/doc/ProgrammerManual/Tutorial/resul.html)
 * [https://www.easysoft.com/developer/languages/c/odbc_tutorial.html](https://www.easysoft.com/developer/languages/c/odbc_tutorial.html)
 * [https://www.easysoft.com/developer/languages/c/examples/ReadingMultipleLongTextFields.html](https://www.easysoft.com/developer/languages/c/examples/ReadingMultipleLongTextFields.html)
 * Code examples: [https://www.easysoft.com/developer/languages/c/examples/index.html](https://www.easysoft.com/developer/languages/c/examples/index.html)

## Info for fuse
 * [https://github.com/libfuse/libfuse/wiki/Option-Parsing](https://github.com/libfuse/libfuse/wiki/Option-Parsing)
 
## Info for Multithreading
 * [http://www.cs.kent.edu/~ruttan/sysprog/lectures/multi-thread/multi-thread.html](http://www.cs.kent.edu/~ruttan/sysprog/lectures/multi-thread/multi-thread.html)

## Apache Modules with dynamic vhosts
 * [https://sourceforge.net/projects/dbd-modules/](https://sourceforge.net/projects/dbd-modules/), [https://github.com/joneschrisan/dbd-modules](https://github.com/joneschrisan/dbd-modules)

## Reading file content
  * [https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c](https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c)
## Required Libraries
 * unixodbc
 * unixodbc-dev
 * libfuse
 * libfuse-dev

## How to compile
 * Create autotools Files with
    sh ./autogen.sh
 * Configure
    ./configure
 * Build
    make
    
## Command Line to compile and install module and restart apache
    sudo apxs -I. -a -i -c ap_config_mod.c lib_apache_config.a; sudo systemctl restart apache2

## Example apache config file /etc/apache2/mods-available/mod_ap_config.conf - only to see if arg parsing works - will be changed later
    <IfModule ap_config_mod.c>
    <Command load_vhosts>
    Exec "/usr/sbin/ls"
    </Command>
    </IfModule>

    

## Example for configfile (/etc/ap_configfs.conf, ./ap_configfs.conf)
    dsn=Driver=SQLITE3;Database=/tmp/testdb.sqlite
    query=select 'Use Vhost example example.com /home/example.com' as entry union select 'Use VHost example2 example2.com /home/example2.com' as entry union select 'Use Redirect example3 example3.com https://www.google.de' as entry;
    mountpoint=my_mountpoint
