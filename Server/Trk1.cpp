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

int main() {
	int serverSock, sent, newsockfd;
	struct hostent *serv_addr;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(10090);

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
				case '0': {
					std::cout << "New peer Joined the system.....!" << std::endl;
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
						return -1;
					}
					
					pFile = fopen("fileList", "rb");
					fseek(pFile, 0, SEEK_END);
					fileSize = ftell(pFile);
					rewind(pFile);
					fread(buf,1, fileSize,pFile);
					fclose(pFile);
					sent = send(newsockfd, &fileSize, sizeof(int), 0);
					sent = send(newsockfd, buf, fileSize, 0);
					if(sent == -1) {
						std::cout << "Send Error " << std::endl;
						return -1;
					}
					close(newsockfd);
					break;	
			}
			
		}	
	}
	
		
	}
}
