all:
	g++ peer.h peer.cpp peers.cpp Tester.cpp -o Tester -lpthread
