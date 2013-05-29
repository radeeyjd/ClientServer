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

Status::Status() {
	_numFiles = 0;
	_local[] = { 0 };
	_system[] = { 0 };
}

int Status::numberofFiles() {
//Return number of files in the system as seen by the peer
	return _numFiles;
}

void Status::incFiles(int n_files) {
//New file is addded to the peer
	_numFiles += n_files;
}

float Status::fractionPresentLocally(int fileNumber) {
//returns the fraction of the file present in the peer
	if(_numFiles < fileNumber) {
		return -1;
	}
	else {
	return _local[fileNumber];
	}
}

float fractionPresent(int filenumber) {
//returns the fraction of file present in the system
	if(_numFiles < fileNumber) {
		return -1;
	}
	else {
	return _system[fileNumber];
	}
}
