Zentoshi integration/staging tree
=================================

https://zentoshi.com

What is Zentoshi?
----------------

Zentoshi is an experimental digital currency that enables instant payments to
anyone, anywhere in the world. Zentoshi uses peer-to-peer technology to operate
with no central authority: managing transactions and issuing money are carried
out collectively by the network. Zentoshi Core is the name of open source
software which enables the use of this currency.


How do I build the software?
----------------------------

The example assumes you have a basic build environment installed:


    git clone https://github.com/zentoshi/zentoshi
    cd zentoshi/depends
    make HOST=x86_64-linux-gnu
    cd ..
    ./autogen.sh
    CONFIG_SITE=$PWD/depends/x86_64-linux-gnu/share/config.site ./configure --prefix=/
    make


License
-------

Zentoshi Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/zentoshi/zentoshi/tags) are created
regularly to indicate new official, stable release versions of Zentoshi Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).

