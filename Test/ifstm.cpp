#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main() {
	std::ifstream peersList;
	string IP, port;
	peersList.open("PF", std::ifstream::in);
	if(!peersList.eof())
	{
	do{
		peersList >> IP;
		peersList >> port;	
		cout << IP << " " << port << endl;
	}while(peersList.good());
	}
	peersList.close();
}
