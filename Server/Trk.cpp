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
	server.sin_port = htons(10089);

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
						return -1;
					}

					pFile = fopen("fileList", "rb");
					fseek(pFile, 0, SEEK_END);
					fileSize = ftell(pFile);
					rewind(pFile);
					fread(buf,1, fileSize,pFile);
					fclose(pFile);
std::cout << fileSize << std::endl;
					sent = send(newsockfd, &fileSize, sizeof(int), 0);
					if(sent == -1) {
						std::cout << "Send Error " << std::endl;
					}
					if(fileSize != 0)
					sent = send(newsockfd, buf, fileSize, 0);
					if(sent == -1) {
						std::cout << "Send Error " << std::endl;
						return -1;
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
			
		}	
			close(newsockfd);

	}

	
		
	}
}
