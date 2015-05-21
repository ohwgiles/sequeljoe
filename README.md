SequelJoe
=========

Inspired by the excellent, but unfortunately OS-X only [SequelPro](http://www.sequelpro.com)

![SequelJoe](https://raw.github.com/ohwgiles/sequeljoe/master/src/res/sequeljoe.png)

Simple Qt-based database administration tool. Supports MariaDB/Mysql, PostgreSQL, SQLite and (with some [trickery](https://github.com/ohwgiles/qt-sqlcipher)) Sqlcipher.

SequelJoe is still pretty raw around the edges, and I don't have a lot of time for this project; so please do fork/contribute/steal

```
$ # first install development libraries for qt5, libssh2, libmariadb, libsqlite
$ git clone https://github.com/ohwgiles/sequeljoe.git
$ mkdir -p path/to/build && cd path/to/build
$ cmake path/to/source -DCMAKE_BUILD_TYPE=Release
$ make
$ sudo make install
```
