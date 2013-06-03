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
#include <netdb.h>
#include <bitset>
#include <sys/types.h> 

int myFilecount;					//Stores the number of files in the system
vector<int> myFiles;				//Stores the list of files
vector< bitset<100> > myFileChunks;	//Each file is chunked into maximum of 100 chunks
pthread_t pid,sid;					//Thread for the server and peerTracker
pthread_t peer_threads[maxPeers];

Peer::Peer() {
//		_peers = new Peers;
} 

#define addrSize sizeof(struct sockaddr_in)

void * startListen(void *);
void * trackerListen(void *); 
void * receiveFiles(void * arg);
 
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

int hosttoIP(char* hostname, char* IP) {
//Function to resolve the hostname -> IP
	struct hostent *addr;		//to resolve the address
	struct in_addr **addr_list;	//to store the address

	if((addr = gethostbyname( hostname )) == NULL) {
		std::cout << "Error resolving hostname" << std::endl;
	}		
	addr_list = (struct in_addr **)addr->h_addr_list;
	for(int iii =0; addr_list[iii] != NULL; iii++) {	//Copy the address to the string
		strcpy(IP, inet_ntoa(*addr_list[iii]));
//		std::cout << IP << std::endl;
	}
	return 0;
}
	
int Peer::join() {
	// 1. Connect to peer Tracker get Peers List
	// 2. Get the files list
	// 3. Start receiving file
	
//Connect to Server
	myStatus = new Status;		//Create a new object Status to store the status of files in the systesm
	int serverSock, sent;		//Create a new server sock to connect to tracker
	struct hostent *serv_addr;	
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	char IP[20];
	hosttoIP("localhost", IP);				//Resolve the host to IP strinf
	server.sin_addr.s_addr = inet_addr(IP);	//IP address of tracker	
	server.sin_port = htons(10089);			//Trackers Port
	_peers = new Peers;
//Contact the Tracker to get files and peers List
	if( (serverSock = socket( AF_INET, SOCK_STREAM, 0)) == -1 ) {
		std::cout << "Socket call failed" << std::endl;
	}	
//	std::cout << "Connecting to the server....." << std::endl;

	if( (connect(serverSock, (struct sockaddr *)&server, addrSize)) == -1) {
		std::cout << "Connection failed" << std::endl;
	}
	else {
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
			fwrite(buf, 1, filesize, pFile);
			fclose(pFile);
		}
		if(fileRecv == -1) {
			std::cout << "Send Error" << std::endl;
		}
	}
//Connect with peers to receive file from them
	vector<int> repStatus;
	int numofPeers, stat, numofFiles;
	ifstream iFiles;
	long f_Size;
//Initialize file list
	iFiles.open("fileList", std::ifstream::in);
	std::string bufFile;
	if(iFiles >> bufFile) {
		iFiles >> f_Size;
		iFiles >> stat;
		repStatus.push_back(stat);
		files.push_back(bufFile);
		filesSize.push_back(f_Size);
		myFiles.push_back(0);
	}
	else {
//Start a tracket to listen to the incoming connections
//		std::cout << "No files in System" << std::endl;			//No files start listening to the port for incoming connection
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
//std::cout << "No File to copy Starting the peerTracker" << std::endl;
	pthread_create(&sid, NULL, trackerListen, NULL); 
	}
	else {
		for(int iii = 0; iii < numofFiles; iii++) { 
//Split the file into multiple chunks and receive the chunks from different peers at the same time
				pthread_create(&peer_threads[iii], NULL,receiveFiles, NULL);
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
	pthread_create(&pid, NULL, startListen, NULL);
//std::cout << "Listen call started" << endl;
}


void * startListen(void * arg) {
//std::cout << "Starting to listen " << std::endl;
	int serverSock, sent, newsockfd;
	struct hostent *serv_addr;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(10090);

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
//			std::cout << "New Connection..... " << std::endl;
			char request;
			int rec, sent;
			rec = recv(newsockfd, &request, sizeof(char), 0);
			if(rec == -1) {
				std::cout << "Receive error " << std::endl;
			}
//std::cout << "Request type " << request << std::endl;
			//Switch on the request type
			switch(request) {
				case 'F': {
//					std::cout << "New file request.....!" << std::endl;
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
//	std::cout << "Files Sent " << std::endl;
			
					}
					close(newsockfd);	
					break;
				}
	default: {
//New file is being added to the system
//std::cout << "New file T/F request" << std::endl;
		char buf[chunkSize], fname[20];
//		std::cout << "New file received" << std::endl;
		int n_Chunks, fnameSize, rec, sent;
		long fileSize;
		rec = recv(newsockfd, &fnameSize, sizeof(int), 0);
		rec = recv(newsockfd, fname, fnameSize, 0);
		FILE *pFile;
//		std::cout << "New file " << fname << " received" << std::endl;
		rec = recv(newsockfd, &fileSize, sizeof(long), 0);
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
	close(newsockfd);	
	fclose(pFile);
	break;
	}
				
   }
  }
 }
}

int Peer::insert(std::string filename) {
//Connect to peers and send the new file
//Connect to server and update the peer list
//std::cout << "Insert hit " << std::endl;
	int i_numofPeers;
	long newfileSize;
	Peer i_myPeers[maxPeers];
	i_numofPeers = _peers->getNumPeers();
	for(int jjj = 0; jjj<i_numofPeers; jjj++) 			//Store list of peers to connect
		i_myPeers[jjj] = _peers->getPeer(jjj);
//std::cout << "Connecting to " << i_numofPeers << " peers" << std::endl;
	for(int iii = 0; iii < i_numofPeers; iii++) {		
//Connect to each peer and send the new file
		int inSock, sent;
		struct hostent *i_peer_addr;
		struct sockaddr_in i_peer;
		i_peer.sin_family = AF_INET;
		i_peer.sin_addr.s_addr = i_myPeers[iii].getIP();		//Connect to iiith peer
		i_peer.sin_port = i_myPeers[iii].getPort();			//iith port
	
		if( (inSock = socket( AF_INET, SOCK_STREAM, 0)) == -1 ) {
				std::cout << "Peer Socket call failed" << std::endl;
		}	
		if( (connect(inSock, (struct sockaddr *)&i_peer, addrSize)) == -1) {
				std::cout << "Connection to Peer failed" << std::endl;				
		}
	
		char req = 'U';
		sent = send(inSock, &req, sizeof(char), 0);  

		int i_fnameSize = filename.size();
		sent = send(inSock, &i_fnameSize, sizeof(int), 0); //Send file size
		sent = send(inSock, filename.c_str(), i_fnameSize, 0);	//Send file name

		long i_newfileSize;

		FILE *inpFile;
		inpFile = fopen(filename.c_str(), "rb");
		fseek(inpFile, 0, SEEK_END);
		i_newfileSize = ftell(inpFile);
		rewind(inpFile);

		sent = send(inSock, &i_newfileSize, sizeof(long), 0);
		char buf[chunkSize];
		int i_nChunks;
		if(i_newfileSize < chunkSize) {
			fread(buf, 1, chunkSize, inpFile);
			sent = send(inSock, buf, i_newfileSize,0);
		}
		else {
			i_nChunks = i_newfileSize / chunkSize;
			for(int i_cChunks = 0; i_cChunks < i_nChunks; i_cChunks++) {
				fread(buf,1, chunkSize, inpFile);
				sent = send(inSock, buf, chunkSize, 0);
			}
			if((i_newfileSize % chunkSize) > 0) {
				fread(buf, 1, (i_newfileSize % chunkSize), inpFile);
				sent = send(inSock, buf, (i_newfileSize % chunkSize), 0);
			}
			}
		}


 	
//Contact the server update the file List
//Connect to Server
	FILE *pFile;
	pFile = fopen(filename.c_str(), "rb");
	fseek(pFile, 0, SEEK_END);
	newfileSize = ftell(pFile);
	rewind(pFile);

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
//	std::cout << "Connecting to the server....." << std::endl;

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
	sent = send(serverSock, &fnSize , sizeof(size_t), 0); //Send file name size
	sent = send(serverSock, filename.c_str(), fnSize, 0);	//Send file name
//std::cout << "FS " << newfileSize << std::endl;
	sent = send(serverSock, &newfileSize, sizeof(long), 0); //send size of file
//std::cout << fnSize << " " << filename.c_str() << std::endl;
	close(serverSock);
//	return 0;
}
	
int Peer::leave() {
	pthread_join(pid, NULL);
	pthread_join(sid, NULL);
	pthread_exit(&sid);	
	pthread_exit(&pid);
//Connect to all peer and let them know that the peer is exiting
	int numofPeers;
	long newfileSize;
	Peer myPeers[maxPeers];
	numofPeers = _peers->getNumPeers();
	for(int jjj = 0; jjj<numofPeers; jjj++) 			//Store list of peers to connect
		myPeers[jjj] = _peers->getPeer(jjj);
	for(int iii = 0; iii < numofPeers; iii++) {			//Connect to each peer and send the exit code 
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
//			std::cout << "Connecting to the Peer....." << std::endl;

			if( (connect(peerSock, (struct sockaddr *)&mypeer, sizeof(mypeer))) == -1) {
					std::cout << "Connection to Peer failed" << std::endl;
			}	
//'U' - Updating the files
			int reqst = -1;	
			sent = send(peerSock, &reqst, sizeof(int), 0);  			//Send Size of Filename
			if(sent == -1) {
			std::cout << "Send Error in Leave" << std::endl;
			}
		}
	}
}

Peer::~Peer() {
//		pthread_join(pid, NULL);	
}

void * receiveFiles(void * arg) {
	int i_numofPeers;
	long newfileSize;
	Peer i_myPeers[maxPeers];
	sleep(5);
	i_numofPeers = 0;
	for(int jjj = 0; jjj<i_numofPeers; jjj++) 			//Store list of peers to connect
//std::cout << "Connecting to " << i_numofPeers << " peers" << std::endl;
	for(int iii = 0; iii < i_numofPeers; iii++) {		
//Connect to each peer and send the new file
		int inSock, sent;
		struct hostent *i_peer_addr;
		struct sockaddr_in i_peer;
		i_peer.sin_family = AF_INET;
		if( (inSock = socket( AF_INET, SOCK_STREAM, 0)) == -1 ) {
				std::cout << "Peer Socket call failed" << std::endl;
		}	
		if( (connect(inSock, (struct sockaddr *)&i_peer, addrSize)) == -1) {
				std::cout << "Connection to Peer failed" << std::endl;				
		}
	
		char req = 'U';
		sent = send(inSock, &req, sizeof(char), 0);  
		std::string filename;
		int i_fnameSize = filename.size();
		sent = send(inSock, &i_fnameSize, sizeof(int), 0); //Send file size
		sent = send(inSock, filename.c_str(), i_fnameSize, 0);	//Send file name

		long i_newfileSize;

		FILE *inpFile;
		inpFile = fopen(filename.c_str(), "rb");
		fseek(inpFile, 0, SEEK_END);
		i_newfileSize = ftell(inpFile);
		rewind(inpFile);

		sent = send(inSock, &i_newfileSize, sizeof(long), 0);
		char buf[chunkSize];
		int i_nChunks;
		if(i_newfileSize < chunkSize) {
			fread(buf, 1, chunkSize, inpFile);
			sent = send(inSock, buf, i_newfileSize,0);
		}
		else {
			i_nChunks = i_newfileSize / chunkSize;
			for(int i_cChunks = 0; i_cChunks < i_nChunks; i_cChunks++) {
				fread(buf,1, chunkSize, inpFile);
				sent = send(inSock, buf, chunkSize, 0);
			}
			if((i_newfileSize % chunkSize) > 0) {
				fread(buf, 1, (i_newfileSize % chunkSize), inpFile);
				sent = send(inSock, buf, (i_newfileSize % chunkSize), 0);
			}
		}
	}
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
			chunkName.append("temp/");
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
			chunkName.append("temp/");
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
			chunkName.append("temp/");
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

void * trackerListen(void *arg) {
	int serverSock, sent, newsockfd;
	struct hostent *serv_addr;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(10089);

	//create a socket
	if((serverSock = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
		std::cout << "Socket call failed" << std::endl;
	}	
	
	//bind the socket
	if((bind(serverSock, (struct sockaddr *)&server, sizeof(server))) == -1) {
		std::cout << "Bind Call failed" << std::endl;
	}
	else {
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
			char req;
			int rec, sent;
			rec = recv(newsockfd, &req, 1, 0);
			//Switch on the request type
			switch(req) {
				case '0': {
					FILE *pFile;
					char buf[255];
					int fileSize;				//Get the file size
					pFile = fopen("peersList", "rb");
					fseek(pFile, 0, SEEK_END);
					fileSize = ftell(pFile);
					rewind(pFile);
					fread(buf,1, fileSize,pFile);
					fclose(pFile);
					sent = send(newsockfd, &fileSize, sizeof(int), 0);
					sent = send(newsockfd, buf, fileSize, 0);
					if(sent == -1) {
						std::cout << "Send Error " << std::endl;
					}

					pFile = fopen("fileList", "rb");
					fseek(pFile, 0, SEEK_END);
					fileSize = ftell(pFile);
					rewind(pFile);
					fread(buf,1, fileSize,pFile);
					fclose(pFile);
//std::cout << fileSize << std::endl;
					sent = send(newsockfd, &fileSize, sizeof(int), 0);
					if(sent == -1) {
						std::cout << "Send Error " << std::endl;
					}
					if(fileSize != 0)
					sent = send(newsockfd, buf, fileSize, 0);
					if(sent == -1) {
						std::cout << "Send Error " << std::endl;
					}

					//Get the Peer IP and append in the file
					char client_IP[20];
					struct sockaddr_in client;
					socklen_t len = sizeof(client);		
					if(getpeername(newsockfd, (struct sockaddr*)&client, &len) == -1) {
						std::cout << "Cannot get IP of Client" << std::endl;
					}
					inet_ntop(AF_INET, &client.sin_addr, client_IP, sizeof(client_IP));	
					std::cout << "New peer " << client_IP <<" Joined the system.....!" << std::endl;
					std::ofstream ofs;	//Open output stream
					ofs.open("peersList", std::ofstream::out|std::ofstream::app);
					ofs << client_IP << " 10091" << std::endl; //Make a new entry
					ofs.close();
					break;	
			}
			case '1': {	//Add the new inserted file to the list
				size_t fnSize;
				long newfileSize;
				int rec;
				std::string nfname;
				char newfile[100];
				rec = recv(newsockfd, &fnSize, sizeof(size_t), 0);
				rec = recv(newsockfd,newfile, fnSize, 0);
				rec = recv(newsockfd, &newfileSize, sizeof(long), 0);
				newfile[fnSize] = '\0';
				std::ofstream oFiles;
				oFiles.open("fileList", std::ios::out|std::ios::app);
				oFiles << newfile << " "<< newfileSize << " 1" << std::endl;
				oFiles.close();
				break;
				}
				
			}	
			close(newsockfd);

		}
	  }
	}
}

