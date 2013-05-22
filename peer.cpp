#include "peer.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <unistd.h>
#include <vector>

/*Peer::Peer() {
	_state = "unknown";
	IP = NULL;
	port = NULL;
}*/

#define addrSize sizeof(struct sockaddr_in)

void Peer::setIP(std::string s_IP) { //Set the IP of Peer	
	IP = s_IP;	
}

void Peer::setPort(int s_port) { 	 //Set the port number of peer
	port = s_port;
}	

long int Peer::getIP() {  			 //Set the IP of peer
	return inet_addr(IP.c_str());
}

long int Peer::getPort() {			//Get the port of a peer
	return htons(port);
}
	
int Peer::join() {
	// 1. Connect to server get Peers List
	// 2. Get the files list
	// 3. Start receiving file
	
//Connect to Server

	int serverSock, sent;
	struct hostent *serv_addr;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(10089);

//Create a Socket
	//create a socket
	if( (serverSock = socket( AF_INET, SOCK_STREAM, 0)) == -1 ) {
		std::cout << "Socket call failed" << std::endl;
		return -1;	
	}	
	std::cout << "Connecting to the server....." << std::endl;
	if( (connect(serverSock, (struct sockaddr *)&server, addrSize)) == -1) {
		std::cout << "Connection failed" << std::endl;
		return -1;
	}
	char req = '0';
	sent = send(serverSock, &req, sizeof(char), 0);
	std::cout << "Connected to Server!" << std::endl;
	FILE *pFile;
	char buf[255];
	int fileRecv, filesize;
	fileRecv = recv(serverSock, &filesize, sizeof(int), 0);
	fileRecv = recv(serverSock, buf, filesize, 0);
	pFile = fopen("peersList", "w");
	fwrite(buf, 1, fileRecv, pFile);
	fclose(pFile);

	
	fileRecv = recv(serverSock, &filesize, sizeof(int), 0);
	fileRecv = recv(serverSock, buf, filesize, 0);
	pFile = fopen("fileList", "w");
	fwrite(buf, 1, fileRecv, pFile);
	fclose(pFile);

	//Connect with peers to receive file from them
	vector<std::string> files;
	vector<int> repStatus;
	int numofPeers, stat, numofFiles;
	ifstream iFiles;
	iFiles.open("fileList", std::ifstream::in);
	std::string bufFile;
	if(iFiles >> bufFile) {
		iFiles >> stat;
		repStatus.push_back(stat);
		files.push_back(bufFile);
	}
	else {
		std::cout << "No files in System" << std::endl;	
	}
	while(iFiles >> bufFile) {
		iFiles >> stat;
		files.push_back(bufFile);	
		repStatus.push_back(stat);
	}

	//Get the list of peers
	_peers.initialize("peersList");
	//cout << _peers.getNumPeers() << endl;
	Peer *myPeers[maxPeers];
	*myPeers = *_peers.getPeers(); //Servers IP and Port


	
	//Get the number of peers and connect with them to start receiving file
	numofPeers = _peers.getNumPeers();
	numofFiles = files.size();
std::cout <<"Peers"<< numofPeers << std::endl ;
	//Connect to the peers and start getting the files
for(int iii = 0; iii < numofPeers; iii++) {
		std::cout << myPeers[iii]->getIP() << std::endl;		
		std::cout << iii << std::endl;
	}
// Start Listening to a port for connections
// Connect to all client and start receiving files from them
//	close(serverSock);
	
}

Peer::~Peer() {
	std::cout << "Bye Bye" << std::endl;
}




