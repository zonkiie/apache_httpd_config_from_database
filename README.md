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

## Example for configfile (/etc/ap_configfs.conf, ./ap_configfs.conf)
    dsn=Driver=SQLITE3;Database=/tmp/testdb.sqlite
    query=select 'Use Vhost example example.com /home/example.com' as entry union select 'Use VHost example2 example2.com /home/example2.com' as entry union select 'Use Redirect example3 example3.com https://www.google.de' as entry;
    mountpoint=my_mountpoint
