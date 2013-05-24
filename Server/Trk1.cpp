#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <sys/types.h> 

#define SIZE sizeof(struct sockaddr_in)
const int chunkSize = 65536;


int main() {
	int serverSock, sent, newsockfd;
	struct hostent *serv_addr;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(10099);

	//create a socket
	if((serverSock = socket( AF_INET, SOCK_STREAM, 0)) == -1) {
		std::cout << "Socket call failed" << std::endl;
		return -1;	
	}	
	
	//bind the socket
	if((bind(serverSock, (struct sockaddr *)&server, sizeof(server))) == -1) {
		std::cout << "Bind Call failed" << std::endl;
		return -1;
	}

	//Start Listening to connections
	if(listen(serverSock, 5) == -1) {
		std::cout << "Listen call failed" << std::endl;
		return -1;	
	}
	while(1) {
			
	//Accept connection
		if((newsockfd = accept(serverSock, NULL, NULL)) == -1) {
			std::cout << "Accept call failed " << std::endl;
			return -1;
		}	
			if(fork() == 0) {
			std::cout << "New Connection..... " << std::endl;
			char req;
			int rec, sent;
			rec = recv(newsockfd, &req, 1, 0);
			//Switch on the request type
			switch(req) {
				case 'F': {
					std::cout << "New file request.....!" << std::endl;
					int fnameSize;
					char fname[20];
					rec = recv(newsockfd, &fnameSize, sizeof(size_t), 0);
			//std::cout << fnameSize << std::endl;
					rec = recv(newsockfd, fname, fnameSize, 0); //Assume it already has the file
					FILE *pFile;
			//std::cout << "Hello";
					char buf[65536], fname1[100];
					int fileSize, n_chunks;				//Get the file size
			std::cout << fname << std::endl;
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
							return -1;
						}
					}
						if((fileSize % chunkSize) != 0) {
						fread(buf, 1, (fileSize %chunkSize), pFile);
						sent = send(newsockfd, buf, (fileSize % chunkSize), 0);
					}
					

			
		}
					close(newsockfd);
					break;	
	}
			
		}	
	}
	
		
	}
}
