# apache_httpd_config_from_database
Get apache http server config parts from database. These parts are either given as fuse filesystem, mmap'd file or dumped to file.

## Info for autotools
[https://thoughtbot.com/blog/the-magic-behind-configure-make-make-install](https://thoughtbot.com/blog/the-magic-behind-configure-make-make-install)

## Tutorials which help with unixodbc
 * [http://www.unixodbc.org/doc/ProgrammerManual/Tutorial/resul.html](http://www.unixodbc.org/doc/ProgrammerManual/Tutorial/resul.html)
 * [https://www.easysoft.com/developer/languages/c/odbc_tutorial.html](https://www.easysoft.com/developer/languages/c/odbc_tutorial.html)
 * [https://www.easysoft.com/developer/languages/c/examples/ReadingMultipleLongTextFields.html](https://www.easysoft.com/developer/languages/c/examples/ReadingMultipleLongTextFields.html)
 * Code examples: [https://www.easysoft.com/developer/languages/c/examples/index.html](https://www.easysoft.com/developer/languages/c/examples/index.html)

## Required Libraries
 * unixodbc
 * unixodbc-dev
 * libfuse
 * libfuse-dev

## How to compile
 * Create autotools Files with
    sh ./bootstrap.sh
 * Configure
    ./configure
 * Build
    make

 
