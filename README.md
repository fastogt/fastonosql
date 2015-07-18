About FastoRedis
===============

FastoRedis &mdash; is a crossplatform Redis, Memcached, SSDB management tool. <br />

<h3>Windows:</h3>
![](http://fastoredis.com/images/slider/win_slider.png)
<h3>MacOS X:</h3>
![](http://fastoredis.com/images/slider/mac_slider.png)
<h3>Linux:</h3>
![](http://fastoredis.com/images/slider/linux_slider.png)

Visit our site: [www.fastoredis.com](http://www.fastoredis.com)

Download
========

You can download compiled version (for Mac OS X, Windows, FreeBSD and Linux) from our site:<br />
[www.fastoredis.com](http://www.fastoredis.com)

<!-- https://www.dropbox.com/sh/u0s0i8e4m0a8i9f/oxtqKHPUZ8 -->

Contribute
==========
Contributions are always welcome! Just try to follow our coding style: [FastoRedis Coding Style](https://github.com/fasto/fastoredis/wiki/Coding-Style)

Build
=====

Build documentation: [Build FastoRedis](https://github.com/fasto/fastoredis/wiki/Build)

License
=======

Copyright (C) 2014 Fasto, Inc (http://www.fastoredis.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3 as 
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

<!-- 

Outdated build documentation:<br />
[Building FastoRedis and Dependencies (for Linux and Mac OS X)]
(https://github.com/fasto/fastoredis/wiki/Building-FastoRedis-and-Dependencies-(for-Linux-and-Mac-OS-X\))




You are lucky enough, if prebuild libraries (that are in `libs` folder) are 
already available and match your OS/Compiler. For most of you it's not &mdash; 
you need to build FastoRedis dependencies, before building FastoRedis itself.

Here is a detailed instructions on building FastoRedis dependencies for Linux and/or Mac OS X:<br />
[Building FastoRedis and Dependencies (for Linux and Mac OS X)]
(https://github.com/fasto/fastoredis/wiki/Building-FastoRedis-and-Dependencies-(for-Linux-and-Mac-OS-X\))



Windows
-------

The following steps assume that all dependencies already compiled.

Prerequisites:

* Qt should be compiled with VC2010. Tested with Qt 5.1
* Your PATH variable should have Qt bin folder
* Visual Studio 2010 should be installed and VC should be in this location: %ProgramFiles%\Microsoft Visual Studio 10.0\VC. Otherwise you need to modify VISUALC_PATH in build script.

Compiling:

    > cd build
    > build.bat

Executable will be placed to: target/debug/app/out



Linux and OS X
---------------

The following steps assume that all dependencies already compiled.

Prerequisites:

* Qt should be installed. Tested with Qt 4.8
* Your PATH variable should have Qt bin folder

Compiling:

    $ cd build
    $ chmod u+x build.sh
    $ ./build.sh

Executable will be placed to: target/debug/app/out

-->

[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/fasto/fastoredis/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

