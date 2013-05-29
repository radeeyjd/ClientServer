#include "peer.h"
#include <iostream>
using namespace std;

int main() {
	Peer *P = new Peer;
cout << "Main: Join" << endl;
	P->join();
cout << "Main: Inserting" << endl;
	P->insert("nQPT6.jpg");
cout << "Main: Leave" << endl;
	P->leave();
	delete(P);
}
