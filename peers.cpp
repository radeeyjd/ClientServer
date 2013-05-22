#include "peer.h"
#include <iostream>
#include <fstream>
#include <string>

/*void Peers::Peers() {
	_numPeers = 0;
}*/

//After the execution of this method
	// 1. List of Peer is know
	// 2. Number of peers is know

int Peers::initialize(std::string peersFile) { //Get the IP and Port numbers of the peers
	std::ifstream peersList;		  		   //Create a new Stream
	std::string IP;
	int port;				  		   //Temp variables to port
	_numPeers = 0;
	peersList.open(peersFile.c_str(), std::ifstream::in);  //Open the stream
	while(peersList >> IP) {  //Create new peer					//Get value of IP
		peersList >> port;					//Get value of port
		_peers[_numPeers].setIP(IP);		//set IP
		_peers[_numPeers].setPort(port);	//set port
		_numPeers++;						//Set number of peers
//		std::cout << _numPeers <<"." << IP << port  << std::endl;
	}
	peersList.close();						//Close file
//	for(int iii = 0; iii < _numPeers - 1; iii++)
//		std::cout << _peers[iii]->getIP() << std::endl;
}

//Return Number of Peers

int Peers::getNumPeers() {
	return _numPeers;
}

Peer Peers::getPeer(int num) {
	return _peers[num];
}
	
