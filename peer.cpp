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
	server.sin_addr.s_addr = inet_addr("127.0.0.1");	//IP address of tracker	
	server.sin_port = htons(10089);						//Trackers Port

//Contact the Tracker to get files and peers List
	if( (serverSock = socket( AF_INET, SOCK_STREAM, 0)) == -1 ) {
		std::cout << "Socket call failed" << std::endl;
		return -1;	
	}	
	std::cout << "Connecting to the server....." << std::endl;

	if( (connect(serverSock, (struct sockaddr *)&server, addrSize)) == -1) {
		std::cout << "Connection failed" << std::endl;
		return -1;
	}
//'0' -- Joining the system
	char req = '0';
	sent = send(serverSock, &req, sizeof(char), 0);
	if(sent == -1) {
		std::cout << "Send Error" << std::endl;
	}

//Start receiving file
	FILE *pFile;
	char buf[255];
	int fileRecv, filesize;
	fileRecv = recv(serverSock, &filesize, sizeof(int), 0); //Receive the size of the file
	if(fileRecv == -1) {
		std::cout << "Send Error" << std::endl;
	}

	fileRecv = recv(serverSock, buf, filesize, 0);		 	//Receive peerslist file
	if(fileRecv == -1) {
		std::cout << "Send Error" << std::endl;
	}

	pFile = fopen("peersList", "w");
	if(pFile == NULL) {
		std::cout << "Error Reading file" << std::endl;
		return -1;
	}
	fwrite(buf, 1, fileRecv, pFile);						//Write the new peerlist
	fclose(pFile);

//Start receiving file List
	fileRecv = recv(serverSock, &filesize, sizeof(int), 0);	//Receive size of file
	if(fileRecv == -1) {
		std::cout << "Send Error" << std::endl;
	}

	fileRecv = recv(serverSock, buf, filesize, 0);			//Receive the files List
	if(fileRecv == -1) {
		std::cout << "Send Error" << std::endl;
	}

	pFile = fopen("fileList", "w");							//Write the new files lis
	fwrite(buf, 1, fileRecv, pFile);
	fclose(pFile);

//Connect with peers to receive file from them
	vector<std::string> files;
	vector<int> repStatus;
	int numofPeers, stat, numofFiles;
	ifstream iFiles;

//Initialize file list
	iFiles.open("fileList", std::ifstream::in);
	std::string bufFile;
	if(iFiles >> bufFile) {
		iFiles >> stat;
		repStatus.push_back(stat);
		files.push_back(bufFile);
		myFiles.push_back(0);
	}
	else {
		std::cout << "No files in System" << std::endl;			//No files start listening to the port for incoming connection
	}
	while(iFiles >> bufFile) {
		iFiles >> stat;
		files.push_back(bufFile);	
		repStatus.push_back(stat);
	}

//Get the list of peers
//Initialize new peers list
	_peers = new Peers;
	_peers->initialize("peersList");
	Peer myPeers[maxPeers];
	numofPeers = _peers->getNumPeers();
	for(int jjj = 0; jjj<numofPeers; jjj++) 			//Store list of peers to connect
	myPeers[jjj] = _peers->getPeer(jjj);
	numofFiles = files.size();							//Store number of files in the list

//Connect to peers and start receiving files
	if(numofFiles == 0) {	//No Files to process listen to port for incoming connection
	//To do
	}
	else {		
		for(int iii = 0; iii < numofFiles; iii++) { 
		//Create threads
				char buf[chunkSize];
				int peerSock, recvd, n_Chunks, peer_ID;
				peer_ID = 0;
				struct hostent *peer_addr;
				struct sockaddr_in mypeer;
				mypeer.sin_family = AF_INET;
				mypeer.sin_addr.s_addr = myPeers[peer_ID].getIP();		//Connect to iiith peer
				mypeer.sin_port = myPeers[peer_ID].getPort();			//iith port
				peer_ID++;
				if(peer_ID >= numofPeers)	//Check if you have connected with all the peers
					peer_ID--;
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
				int fnameSize = files[iii].size();
				sent = send(peerSock, &fnameSize, sizeof(size_t), 0); //Send file size
				sent = send(peerSock, fname.c_str(), fnameSize, 0);	//Send file name
				int fileSize;
				recvd = recv(peerSock, &fileSize, sizeof(int), 0);
			
				FILE *pFile;	
				pFile = fopen(fname.c_str(), "a");

				if(fileSize <= chunkSize) {	//For small file get in a single attempt
				recvd = recv(peerSock, buf, fileSize, 0);
				fwrite(buf, 1, fileSize, pFile);
				}
				else {
					n_Chunks = fileSize / chunkSize;				
					for(int c_Chunk = 0; c_Chunk < n_Chunks; c_Chunk++) {  //For large file split it into chunks 
						recvd = recv(peerSock, buf, chunkSize, 0);
						fwrite(buf, 1, chunkSize, pFile);
					}
					if((fileSize % chunkSize) != 0) {					   //Send remaining data
						recvd = recv(peerSock, buf, (fileSize % chunkSize), 0);
						fwrite(buf, 1, recvd, pFile);
					}
				}	
				fclose(pFile);
				close(peerSock);
			}		
		}

	// Start Listening to a port for connections
	// Connect to all client and start receiving files from them

	close(serverSock);
}

Peer::~Peer() {
}




