all:
	g++ atm.cpp -m64 -o atm -lcryptopp
	g++ bank.cpp acct.cpp util.cpp -m64 -o bank -lpthread -lcryptopp
	g++ proxy.cpp -m64 -o proxy -lpthread
	g++ util.cpp test.cpp -m64 -o util -lcryptopp

32:
	g++ atm.cpp -m32 -o atm -lcryptopp
	g++ bank.cpp acct.cpp util.cpp -m32 -o bank -lpthread -lcryptopp
	g++ proxy.cpp -m32 -o proxy -lpthread
	g++ util.cpp test.cpp -m32 -o util -lcryptopp

debug:
	g++ atm.cpp -g -m64 -o atm -lcryptopp
	g++ bank.cpp acct.cpp util.cpp -g -m64 -o bank -lpthread -lcryptopp
	g++ proxy.cpp -g -m64 -o proxy -lpthread
	g++ util.cpp test.cpp -g -m64 -o util -lcryptopp

debug32:
	g++ atm.cpp -g -m32 -o atm -lcryptopp
	g++ bank.cpp acct.cpp util.cpp -g -m32 -o bank -lpthread -lcryptopp
	g++ proxy.cpp -g -m32 -o proxy -lpthread
	g++ util.cpp test.cpp -g -m32 -o util -lcryptopp

install:
	apt-get install gcc-multilib
	apt-get install libboost-all-dev
	apt-cache pkgnames | grep -i crypto++ apt-get install libcrypto++9 libcrypto++9-dbg libcrypto++-dev
