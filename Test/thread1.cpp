#include <iostream>
#include <pthread.h>
using namespace std;

pthread_t myThread;

void *worker_thread(void *arg) {
	cout << "Hi from thread" << endl;
	while(0) {
	cout << "Hi " << endl;
//	sleep(100);
//	pthread_exit(myThread);
	}
	pthread_exit(NULL);
}

int main() {
		pthread_create(&myThread, NULL, &worker_thread, NULL);
	cout << "Back again " << endl;
	pthread_join(myThread, NULL);
	cout << "Back?" << endl;
}
	
