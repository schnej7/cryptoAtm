all:
	g++ atm.cpp -m64 -o atm
	g++ bank.cpp -m64 -o bank -lpthread
	g++ proxy.cpp -m64 -o proxy -lpthread
