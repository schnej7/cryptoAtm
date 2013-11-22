Downloads Required:
c++ boost: libboost-all-dev

Download cryptopp Libraries:
apt-cache pkgnames | grep -i crypto++
apt-get install libcrypto++9 libcrypto++9-dbg libcrypto++-dev

compiler flags:

bank, proxy: -lpthread
bank, atm, util: -lcryptopp
