#include <iostream>
#include <pthread.h>
using namespace std;

void *worker_thread(void *arg) {
	cout << "Hi from thread" << endl;
	pthread_exit(NULL);
}

int main() {
	pthread_t myThread;
	for(int iii = 0; iii < 10; iii++) {
		if(pthread_create(&myThread, NULL, &worker_thread, (int*)iii) != 0)
			cout << "Error creating thread" << endl;
	}
	pthread_exit(NULL);
}
	
