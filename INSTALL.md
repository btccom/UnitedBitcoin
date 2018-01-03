Building UnitedBitcoin
================

See doc/build-*.md for instructions on building the various
elements of the UnitedBitcoin Core reference implementation of Bitcoin.

Running UnitedBitcoin core
================

* Ubuntu 16.04

1. Install dependent packages
```
apt-get install -y libevent-pthreads-2.0-5 libminiupnpc-dev libboost-all-dev  
apt-get install software-properties-common
add-apt-repository ppa:bitcoin/bitcoin
apt-get update
apt-get install -y libdb4.8-dev libdb4.8++-dev
```

2. Run ubcd from command line
```
./ubcd -rpcuser=a -rpcpassword=b -datadir=/home/www/ubchain   -daemon -server -rpcport=10086 -txindex
```
Above command will run core with rpc enabled.
