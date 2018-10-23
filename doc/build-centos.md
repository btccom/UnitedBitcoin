CENTOS BUILD NOTES
====================

CentOS Version
---------------------

    LSB Version:	:core-4.1-amd64:core-4.1-noarch:cxx-4.1-amd64:cxx-4.1-noarch:desktop-4.1-amd64:desktop-4.1-noarch:languages-4.1-amd64:languages-4.1-noarch:printing-4.1-amd64:printing-4.1-noarch
    Distributor ID:	CentOS
    Description:	CentOS Linux release 7.5.1804 (Core) 
    Release:	7.5.1804
    Codename:	Core

Dependencies
---------------------

These dependencies are required:

 Library     | Purpose          | Description
 ------------|------------------|----------------------
 libssl      | Crypto           | Random Number Generation, Elliptic Curve Cryptography
 libboost    | Utility          | Library for threading, data structures, etc
 libevent    | Networking       | OS independent asynchronous networking

Optional dependencies:

 Library     | Purpose          | Description
 ------------|------------------|----------------------
 miniupnpc   | UPnP Support     | Firewall-jumping support
 libdb4.8    | Berkeley DB      | Wallet storage (only needed when wallet enabled)
 qt          | GUI              | GUI toolkit (only needed when GUI enabled)
 protobuf    | Payments in GUI  | Data interchange format used for payment protocol (only needed when GUI enabled)
 libqrencode | QR codes in GUI  | Optional for generating QR codes (only needed when GUI enabled)
 univalue    | Utility          | JSON parsing and encoding (bundled version will be used unless --with-system-univalue passed to configure)
 libzmq3     | ZMQ notification | Optional, allows generating ZMQ notifications (requires ZMQ version >= 4.x)

For the versions used, see [dependencies.md](dependencies.md)

Memory Requirements
--------------------

C++ compilers are memory-hungry. It is recommended to have at least 1.5 GB of
memory available when compiling UnitedBitcoin Core. On systems with less, gcc can be
tuned to conserve memory with additional CXXFLAGS:


    ./configure CXXFLAGS="--param ggc-min-expand=1 --param ggc-min-heapsize=32768"

Dependency Build Instructions: CentOS
----------------------------------------------
Build requirements:

    sudo yum install build-essential libtool autotools-dev automake pkg-config libssl-dev libevent-dev bsdmainutils
    sudo yum install gcc-c++ libtool make autoconf automake openssl-devel libevent-devel boost-devel libdb4-devel libdb4-cxx-devel

BerkeleyDB is required for the wallet.

You can build and install BerkeleyDB using the following commands:

    wget 'http://download.oracle.com/berkeley-db/db-5.1.29.NC.tar.gz'
    tar -xzf db-5.1.29.NC.tar.gz
    cd db-5.1.29.NC/build_unix/
    ../dist/configure --enable-cxx --disable-shared --with-pic --prefix=/usr/local
    make install

--------------------- 

CentOS has its own libdb-dev and libdb++-dev packages, but these will install
BerkeleyDB 5.1 or later, which break binary wallet compatibility with the distributed executables which
are based on BerkeleyDB 4.8. If you do not care about wallet compatibility,
pass `--with-incompatible-bdb` to configure.

See the section "Disable-wallet mode" to build UnitedBitcoin Core without wallet.

Dependencies for the GUI: CentOS
-----------------------------------------

If you want to build UnitedBitcoin-Qt, make sure that the required packages for Qt development
are installed. Either Qt 5 or Qt 4 are necessary to build the GUI.
If both Qt 4 and Qt 5 are installed, Qt 5 will be used. Pass `--with-gui=qt4` to configure to choose Qt4.
To build without GUI pass `--without-gui`.

To build with Qt 5 (recommended) you need the following:

    sudo yuminstall qt5-qttools-devel qt5-qtbase-devel protobuf-devel

Once these are installed, they will be found by configure and a UnitedBitcoin-qt executable will be
built by default.

Notes
-----
The release is built with GCC and then "strip ubcd" to strip the debug
symbols, which reduces the executable size by about 90%.

Boost
-----
If you need to build Boost yourself:

	sudo su
	./bootstrap.sh
	./bjam install

Disable-wallet mode
--------------------
When the intention is to run only a P2P node without a wallet, UnitedBitcoin may be compiled in
disable-wallet mode with:

    ./configure --disable-wallet

In this case there is no dependency on Berkeley DB 4.8.

Mining is also possible in disable-wallet mode, but only using the `getblocktemplate` RPC
call not `getwork`.

Additional Configure Flags
--------------------------
A list of additional configure flags can be displayed with:

    ./configure --help


Build Example:
-----------------------------------
This example lists the steps necessary to build a command line only:

    git clone https://github.com/UnitedBitcoin/UnitedBitcoin.git
    cd UnitedBitcoin/
    ./autogen.sh
    ./configure --with-incompatible-bdb
    make
