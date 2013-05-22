/*Client*/
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <cstdlib>
#include <cstdio>

#define addrSize sizeof(struct sockaddr_in)

int main() {
	int sockfd, n; //Socket file descriptor
	std::string message;
	struct hostent *serv_addr;
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	serv_addr = gethostbyname("localhost");
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(10089);

	//create a socket
	if( (sockfd = socket( AF_INET, SOCK_STREAM, 0)) == -1 ) {
		std::cout << "Socket call failed" << std::endl;
		return -1;	
	}	
	std::cout << "Connecting to the server....." << std::endl;
	if( (connect(sockfd, (struct sockaddr *)&server, addrSize)) == -1) {
		std::cout << "Connection failed" << std::endl;
		return -1;
	}
	std::cout << "Connected to Server!" << std::endl;
	FILE *pFile;
	char buf[74];
	int filerecv, fileSize;
	char fileName[4];
	filerecv = recv(sockfd, &fileName, 4, 0);
	filerecv = recv(sockfd, &fileSize, sizeof(int), 0);
	pFile = fopen(fileName, "wb");
	filerecv = recv(sockfd, buf, fileSize, 0);
	fwrite(buf, 1, 74, pFile);
	std::cout << "Received File " << fileName << std::endl;
	fclose(pFile);
	close(sockfd);
	
}
