all:
	g++ atm.cpp -m64 -o atm
	g++ bank.cpp -m64 -o bank -lpthread
	g++ proxy.cpp -m64 -o proxy -lpthread
	g++ util.cpp test.cpp -m64 -o util -lcryptopp