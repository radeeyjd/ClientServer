#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <sstream>

const int chunkSize = 65536;

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
			chunkName.append("temp/");
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
			ifs.read(buf, (fileSize % chunkSize));
			ofs.write(buf, ifs.gcount());
			ifs.close();
		}
	}
	ofs.close();
}
	
int main() {
	chunkFile("Kannazhaga.mp3", 6824885);
	mergeFile("Kannazhaga.mp3", 6824885);

}

				
		
