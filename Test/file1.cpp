#include <iostream>
#include <fcntl.h>
#include <unistd.h>
using namespace std;

int main() {
	int fd, fd2;
	ssize_t nread;
	char buf[1024];
	
	//Open a file
	fd = open ("data", O_RDONLY);
	fd2 = open ("data", O_RDONLY);
	//read the data
	nread = read(fd, buf, 1024);
	//close the file
	cout << fd <<" "<<fd2 << " " << nread;
	close(fd);
}
