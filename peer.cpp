#include "peer.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <cstring>

int myFilecount;
vector<int> myFiles;
Peer::Peer() {
//	_peers = new Peers;
} 

#define addrSize sizeof(struct sockaddr_in)

void Peer::setIP(std::string s_IP) { //Set the IP of Peer	
	IP = inet_addr(s_IP.c_str());
//std::cout << "New IP" << IP;
}

void Peer::setPort(int s_port) { 	 //Set the port number of peer
	port = htons(s_port);
//std::cout << "New Port " << port << std::endl;
}	

in_addr_t Peer::getIP() {  			 //Set the IP of peer
//std::cout <<"Returned IP " << IP << std::endl; 
	return IP;
}

in_port_t Peer::getPort() {			//Get the port of a peer
//std::cout <<"Returned Port " <<  port << std::endl;
	return port;
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
//std::cout << "Server "<< server.sin_addr.s_addr << " " << htons(10090) << std::endl;


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
	if(sent == -1) {
		std::cout << "Send Error" << std::endl;
	}
//	std::cout << "Connected to Server!" << std::endl;
	FILE *pFile;
	char buf[255];
	int fileRecv, filesize;
	fileRecv = recv(serverSock, &filesize, sizeof(int), 0);
		if(fileRecv == -1) {
		std::cout << "Send Error" << std::endl;
	}

	fileRecv = recv(serverSock, buf, filesize, 0);
	if(fileRecv == -1) {
		std::cout << "Send Error" << std::endl;
	}
	pFile = fopen("peersList", "w");
	if(pFile == NULL) {
		std::cout << "Error Reading file" << std::endl;
		return -1;
	}
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
		myFiles.push_back(0);
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
	_peers = new Peers;
	_peers->initialize("peersList");
	//cout << _peers.getNumPeers() << endl;
	Peer myPeers[maxPeers];
	numofPeers = _peers->getNumPeers();
	for(int jjj = 0; jjj<numofPeers; jjj++)
	myPeers[jjj] = _peers->getPeer(jjj);
//	std::cout << numofPeers << std::endl;
//		std::cout << std::endl;
//	for(int iii = 0; iii < numofPeers && iii <11; iii++) {
//		myPeers[iii] = _peers->getPeer(iii);
//		std::cout << myPeers[iii].getIP() <<" "<< myPeers[iii].getPort() << std::endl;
//		std::cout << myPeers[iii].IP << " " << myPeers[iii].port << std::endl;
//	} 
//		std::cout << std::endl;
		
	//Get the number of peers and connect with them to start receiving file
	numofFiles = files.size();
	
//	for(int iii = 0; iii < files.size(); iii++)
//		std::cout << files[iii];

	//Connect to peers and start receiving files
	if(numofFiles == 0) {
		//No Files to process listen to port for incoming connection
	}
	else {	
		for(int iii = 0; iii < numofFiles; iii++) { //Change to num of file
			if(fork() > 0) {
				char buf[65536];
				int peerSock, recvd;
				struct hostent *peer_addr;
				struct sockaddr_in mypeer;
				mypeer.sin_family = AF_INET;
				mypeer.sin_addr.s_addr = myPeers[iii].getIP();
				mypeer.sin_port = myPeers[iii].getPort();
//std::cout <<"Connecting to " << std::endl << myPeers[iii].getIP() << " "<< myPeers[iii].getPort() << std::endl;
				if( (peerSock = socket( AF_INET, SOCK_STREAM, 0)) == -1 ) {
					std::cout << "Peer Socket call failed" << std::endl;
					return -1;	
				}	
				std::cout << "Connecting to the Peer....." << std::endl;
				if( (connect(peerSock, (struct sockaddr *)&mypeer, addrSize)) == -1) {
					std::cout << "Connection to Peer failed" << std::endl;
					return -1;
				}
				char req = 'F';
				std::string fname;
				fname = files[iii];
				sent = send(peerSock, &req, sizeof(char), 0);  			//Send Size of Filename
//				recvd = recv(peerSock, &req, sizeof(char), 0);			//Sen
				int fnameSize = files[iii].size();
				sent = send(peerSock, &fnameSize, sizeof(size_t), 0); //Send file size
				sent = send(peerSock, fname.c_str(), fnameSize, 0);	//Send file name
				int fileSize;
				recvd = recv(peerSock, &fileSize, sizeof(int), 0);
				recvd = recv(peerSock, buf, fileSize, 0);
				FILE *pFile;
				char fname1[100];
				memcpy(fname1, fname.c_str(), fname.size());	
				pFile = fopen(fname.c_str(), "wb");
				fwrite(buf, 1, fileSize, pFile);
				fclose(pFile);
//sent = send(peerSock, &req, sizeof(char), 0);
				if(sent == -1) {
					std::cout << "Send Error" << std::endl;
				}
//std::cout << "Received " << req << std::endl;
			}		
		}
	}

	// Start Listening to a port for connections
	// Connect to all client and start receiving files from them
	

close(serverSock);
}

Peer::~Peer() {
}




