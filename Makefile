all:
	g++ atm.cpp util.cpp -m64 -o atm -lcryptopp -std=c++11
	g++ bank.cpp acct.cpp -m64 -o bank -lpthread -lcryptopp -std=c++11
	#g++ proxy.cpp -m64 -o proxy -lpthread

32:
	g++ atm.cpp util.cpp -m32 -o atm -lcryptopp -std=c++11
	g++ bank.cpp acct.cpp -m32 -o bank -lpthread -lcryptopp -std=c++11
	#g++ proxy.cpp -m32 -o proxy -lpthread

debug:
	g++ atm.cpp util.cpp -g -m64 -o atm -lcryptopp -std=c++11
	g++ bank.cpp acct.cpp -g -m64 -o bank -lpthread -lcryptopp -std=c++11
	g++ proxy.cpp -g -m64 -o proxy -lpthread

debug32:
	g++ atm.cpp util.cpp -g -m32 -o atm -lcryptopp -std=c++11
	g++ bank.cpp acct.cpp -g -m32 -o bank -lpthread -lcryptopp -std=c++11
	g++ proxy.cpp -g -m32 -o proxy -lpthread

install:
	apt-get install gcc-multilib
	apt-get install libboost-all-dev
	apt-get install libcrypto++9 libcrypto++9-dbg libcrypto++-dev
