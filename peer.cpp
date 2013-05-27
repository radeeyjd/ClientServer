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
#include <pthread.h>
#include <sstream>

int myFilecount;
vector<int> myFiles;
pthread_t pid;

Peer::Peer() {
//		_peers = new Peers;
} 

#define addrSize sizeof(struct sockaddr_in)

void * startListen(void *);


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
	_peers = new Peers;
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
	if(filesize > 0) {
		fileRecv = recv(serverSock, buf, filesize, 0);			//Receive the files List.
		pFile = fopen("fileList", "w");							//Write the new files lis
		fwrite(buf, 1, fileRecv, pFile);
		fclose(pFile);
	}
	if(fileRecv == -1) {
		std::cout << "Send Error" << std::endl;
	}
	
//Connect with peers to receive file from them
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
	_peers->initialize("peersList");
	Peer myPeers[maxPeers];
	numofPeers = _peers->getNumPeers();
	for(int jjj = 0; jjj<numofPeers; jjj++) 			//Store list of peers to connect
	myPeers[jjj] = _peers->getPeer(jjj);
	numofFiles = files.size();							//Store number of files in the list

//Connect to peers and start receiving files
	if(numofFiles == 0 || numofPeers == 0) {	//No Files to process listen to port for incoming connection
	//To do
std::cout << "No File to copy" << std::endl;
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
			
				}	
				std::cout << "Connecting to the Peer....." << std::endl;
				if( (connect(peerSock, (struct sockaddr *)&mypeer, addrSize)) == -1) {
					std::cout << "Connection to Peer failed" << std::endl;
				
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
//	pthread_create(&pid, NULL, startListen, NULL);

}


void * startListen(void * arg) {
std::cout << "Starting to listen " << std::endl;
	int serverSock, sent, newsockfd;
	struct hostent *serv_addr;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(10091);

	//create a socket
	if((serverSock = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
		std::cout << "Socket call failed" << std::endl;
	}	
	
	//bind the socket
	if((bind(serverSock, (struct sockaddr *)&server, sizeof(server))) == -1) {
		std::cout << "Bind Call failed" << std::endl;

	}

	//Start Listening to connections
	if(listen(serverSock, 5) == -1) {
		std::cout << "Listen call failed" << std::endl;

	}
	while(1) {
			
	//Accept connection
		if((newsockfd = accept(serverSock, NULL, NULL)) == -1) {
			std::cout << "Accept call failed " << std::endl;

		}	
			if(fork() == 0) {
			std::cout << "New Connection..... " << std::endl;
			char req;
			int rec, sent;
			rec = recv(newsockfd, &req, 1, 0);
			if(rec == -1) {
				std::cout << "Receive error " << std::endl;
			}
std::cout << "Request type " << req << std::endl;
			//Switch on the request type
			switch(req) {
				case 'F': {
					std::cout << "New file request.....!" << std::endl;
					int fnameSize;
					char fname[20], fdir[30];
					rec = recv(newsockfd, &fnameSize, sizeof(size_t), 0);
			//std::cout << fnameSize << std::endl;
					rec = recv(newsockfd, fname, fnameSize, 0); //Assume it already has the file
					FILE *pFile;
			//std::cout << "Hello";
					char buf[65536], fname1[100];
					int fileSize, n_chunks;				//Get the file size
			//std::cout << fname << std::endl;
					pFile = fopen(fname, "rb");
					fseek(pFile, 0, SEEK_END);
					fileSize = ftell(pFile);
					rewind(pFile);
					sent = send(newsockfd, &fileSize, sizeof(int), 0);
								
					if(fileSize <= chunkSize) {
					fread(buf,1, fileSize,pFile);
					fclose(pFile);
					sent = send(newsockfd, buf, fileSize, 0);	
					}
					else {
						n_chunks = fileSize / chunkSize;		
						for(int c_Chunk = 0; c_Chunk < n_chunks; c_Chunk++) {
							fread(buf, 1, chunkSize, pFile);
							sent = send(newsockfd, buf, chunkSize, 0);	
							if(sent == -1) {
							std::cout << "Send Error " << std::endl;

						}
					}
						if((fileSize % chunkSize) != 0) {
						fread(buf, 1, (fileSize %chunkSize), pFile);
						sent = send(newsockfd, buf, (fileSize % chunkSize), 0);
					}
	std::cout << "Files Sent " << std::endl;
			
					}
					close(newsockfd);	
					break;
				}
	case 'U': {
//New file is being added to the system
std::cout << "New file T/F request" << std::endl;
		char buf[chunkSize], fname[20];
		std::cout << "New file received" << std::endl;
		int n_Chunks, fnameSize, rec, sent, fileSize;
		rec = recv(newsockfd, &fnameSize, sizeof(size_t), 0);
		rec = recv(newsockfd, fname, fnameSize, 0);
		FILE *pFile;
		rec = recv(newsockfd, &fileSize, sizeof(int), 0);
		pFile = fopen(fname, "wa");
//Open and start writing the files
		if(fileSize <= chunkSize) {	//For small file get in a single attempt
			rec = recv(newsockfd, buf, fileSize, 0);
			fwrite(buf, 1, fileSize, pFile);
			}
		else {
			n_Chunks = fileSize / chunkSize;
			for(int c_Chunk = 0; c_Chunk < n_Chunks; c_Chunk++) {  //For large file split it into chunks 
				rec = recv(newsockfd, buf, chunkSize, 0);
				fwrite(buf, 1, chunkSize, pFile);
			}
			if((fileSize % chunkSize) != 0) {					   //Send remaining data
				rec = recv(newsockfd, buf, (fileSize % chunkSize), 0);
				fwrite(buf, 1, rec, pFile);
		}
	}	
	fclose(pFile);
	break;
	}
				
	}
 }
}
}

int Peer::insert(std::string filename) {
//Connect to peers and send the new file
std::cout << "Insert hit " << std::endl;
	int numofPeers;
	Peer myPeers[maxPeers];
	numofPeers = _peers->getNumPeers();
	for(int jjj = 0; jjj<numofPeers; jjj++) 			//Store list of peers to connect
		myPeers[jjj] = _peers->getPeer(jjj);

	for(int iii = 0; iii < numofPeers; iii++) {			//Connect to each peer and send the new file
	if(fork() == 0) {
		char buf[chunkSize];
		int peerSock, sent, recvd, n_Chunks;
		struct hostent *peer_addr;
		struct sockaddr_in mypeer;
		mypeer.sin_family = AF_INET;
		mypeer.sin_addr.s_addr = myPeers[iii].getIP();		//Connect to iiith peer
		mypeer.sin_port = myPeers[iii].getPort();			//iith port

		if( (peerSock = socket( AF_INET, SOCK_STREAM, 0)) == -1 ) {
				std::cout << "Peer Socket call failed" << std::endl;		
		}	
		std::cout << "Connecting to the Peer....." << std::endl;

		if( (connect(peerSock, (struct sockaddr *)&mypeer, addrSize)) == -1) {
				std::cout << "Connection to Peer failed" << std::endl;
		}
//'U' - Updating the files
		char req = 'U';	
		std::string fname;
		fname = files[iii];
		int n_chunks;
		sent = send(peerSock, &req, sizeof(char), 0);  			//Send Size of Filename
		int fnameSize = files[iii].size();
		sent = send(peerSock, &fnameSize, sizeof(size_t), 0); //Send file size
		sent = send(peerSock, fname.c_str(), fnameSize, 0);	//Send file name
		int fileSize;
//Start Sending the new file
		FILE *pFile;
		pFile = fopen(fname.c_str(), "rb");
		fseek(pFile, 0, SEEK_END);
		fileSize = ftell(pFile);
		rewind(pFile);
		sent = send(peerSock, &fileSize, sizeof(int), 0);
//Split files into chunks and send it across							
		if(fileSize <= chunkSize) {
			fread(buf,1, fileSize,pFile);
			fclose(pFile);
			sent = send(peerSock, buf, fileSize, 0);	
		}
		else {
			n_chunks = fileSize / chunkSize;		
			for(int c_Chunk = 0; c_Chunk < n_chunks; c_Chunk++) {
				fread(buf, 1, chunkSize, pFile);
				sent = send(peerSock, buf, chunkSize, 0);	
				if(sent == -1) {
					std::cout << "Send Error " << std::endl;
				}
			}
				if((fileSize % chunkSize) != 0) {
					fread(buf, 1, (fileSize %chunkSize), pFile);
					sent = send(peerSock, buf, (fileSize % chunkSize), 0);
					if(sent == -1) {
					std::cout << "Send Error " << std::endl;
				}
			}
std::cout << "New Files Sent " << std::endl;
		}
		fclose(pFile);
	}

 }
//Contact the server update the file List
//Connect to Server
	int serverSock, sent;
	struct hostent *serv_addr;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");	//IP address of tracker	
	server.sin_port = htons(10089);						//Trackers Port
	_peers = new Peers;
//Contact the Tracker to Update the file list
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
	char req = '1';
	sent = send(serverSock, &req, sizeof(char), 0);
	if(sent == -1) {
		std::cout << "Send Error" << std::endl;
	}
	
//Send the newly inserted file name
	size_t fnSize = filename.size();
//	char newfn[20];
//	memcpy(newfn, filename.c_str(), filename.length());
	sent = send(serverSock, &fnSize , sizeof(size_t), 0); //Send file size
	sent = send(serverSock, filename.c_str(), fnSize, 0);	//Send file name
//std::cout << fnSize << " " << filename.c_str() << std::endl;
	close(serverSock);
//	return 0;
}

Peer::~Peer() {
//		pthread_join(pid, NULL);	
}

void chunkFile(char *fname, long fileSize) {
//get the base file name and the size of the file to chunk
	std::ifstream ifs;			//Input file
	ifs.open(fname, std::ios::in | std::ios::binary);
	if(ifs.is_open()) {
		std::ofstream ofs;
		int n_Chunks;
		std::string chunkName;
		char *buf = new char[chunkSize];
		n_Chunks = fileSize / chunkSize;
		for(int c_Chunks = 0; c_Chunks < n_Chunks; c_Chunks++) {	//Iterate till the entire file is being chunked minus the excess
			chunkName.clear();
			chunkName.append(fname);
			chunkName.append(".");	
			std::ostringstream intbuf;
			intbuf.clear();
			intbuf << c_Chunks;
			chunkName.append(intbuf.str());
			ofs.open(chunkName.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
			if(ofs.is_open()) {
				ifs.read(buf, chunkSize);
				ofs.write(buf, ifs.gcount());
				ofs.close();
			}
		}
		if(fileSize % chunkSize > 0) {	//If some data is left after chunking the file
			n_Chunks++;
			chunkName.clear();
			chunkName.append(fname);
			chunkName.append(".");	
			std::ostringstream intbuf;
			intbuf.clear();
			intbuf << n_Chunks;
			chunkName.append(intbuf.str());
			ofs.open(chunkName.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
			if(ofs.is_open()) {
				ifs.read(buf, (fileSize % chunkSize));
				ofs.write(buf, ifs.gcount());
				ofs.close();
			}
		}
	ifs.close();	
	}
}

void mergeFile(char *fname, long fileSize) {
//Get the base file name
//Merge the small files and produce the final file
	std::ofstream ofs;
	ofs.open(fname, std::ios::out | std::ios::binary);
	if(ofs.is_open()) {
		int n_Chunks;
		n_Chunks = fileSize /chunkSize;
		std::string chunkName;
		std::ifstream ifs;
		char *buf = new char[chunkSize];
		for(int c_Chunks = 0; c_Chunks < n_Chunks; c_Chunks++) {
			chunkName.clear();
			chunkName.clear();
			chunkName.append(fname);
			chunkName.append(".");	
			std::ostringstream intbuf;
			intbuf.clear();
			intbuf << c_Chunks;
			chunkName.append(intbuf.str());

			ifs.open(chunkName.c_str(), std::ios::in | std::ios::binary);
			ifs.read(buf, chunkSize);
			ofs.write(buf, ifs.gcount());
			ifs.close();
		}
		if(fileSize % chunkSize > 0) {
//if there is some excessive data in the file
			n_Chunks++;
			chunkName.clear();
			chunkName.clear();
			chunkName.append(fname);
			chunkName.append(".");	
			std::ostringstream intbuf;
			intbuf.clear();
			intbuf << n_Chunks;
			chunkName.append(intbuf.str());

			ifs.open(chunkName.c_str(), std::ios::in | std::ios::binary);
			ifs.read(buf, (fileSize % chunkSize));	//read the data
			ofs.write(buf, ifs.gcount());			//write in the final file
			ifs.close();
		}
	}
	ofs.close();
}
	

