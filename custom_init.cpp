#include "custom_init.h"
using namespace std;

static union {
	char c[4];
	int t;
}
key;

custom_init::custom_init() {
	first_pass = 1;
	std::cout << "From constructor--fisr_pass is:" << first_pass << "\n";
}

float* custom_init::shmptr_master = NULL;
int custom_init::shmid_master = NULL;

float* custom_init::comm_init() {
	
		printf("******In FIRST PASS inside comm_init()******\n");
		int  N_SHARE = 6;      /* Number of Shared Value, in and out */
		char THR_SHARE[] = "SHEM";
		int length = sizeof(THR_SHARE) / sizeof(char);
		/* create - attach to share memory */
		strncpy(key.c, THR_SHARE, 4);
		shmid_master = shmget(key.t, N_SHARE, 0666);
		std::cout << "shmid is: " << shmid_master << "\n";

		if (shmid_master == -1) {
			printf("Error attaching to shared memory from custom driver eec_uei.\n");
			printf("Errno is: %s, key.c is: %53c, key.t is: %d\n", strerror(errno), key.c, key.t);
		}
		else {
			printf("Attached to shared memory successfully from eec_uei.bin.\n");
		}

		/* get pointer to shm */
		shmptr_master = (float*)shmat(shmid_master, NULL, 0);
		if (shmptr_master == NULL) {
			printf("Error getting shared memory from custom driver eec_uei.\n");
		}
		else {
			printf("Got shared memory successfully from eec_uei.bin.\n");
		}

	std::cout << "shmptr_master inside comm_init() is:" << shmptr_master << "\n";
	return shmptr_master;
}

float* custom_init::getShmptr() {
	return shmptr_master;
}

int custom_init::getShmid() {
	return shmid_master;
}

float* custom_init::update(int* data) {
	if (custom_init::first_pass == 1) {
		custom_init::shmptr = custom_init::comm_init();
		custom_init::first_pass = 0;
	}
	custom_init::shmptr = getShmptr();
	std::cout << "custom_init::update() is called\n";
	std::cout << "fisr_pass is:" << first_pass << "\n";
	std::cout << "key.c is: " << key.c << " key.t is: " << key.t << "\n";
	std::cout << "shmid is: " << getShmid() << "\n";
	/* wait 50 us */
	usleep(5000);

		std::cout << "shmptr inside update() is:" << custom_init::shmptr << "\n";
		std::cout << "shmptr val BEFORE: " << shmptr[2] << " " << shmptr[3] << " " << shmptr[4] << " " << shmptr[5] << '\n';

		shmptr[2] = data[0];
		shmptr[3] = data[1];
		shmptr[4] = data[2];
		shmptr[5] = data[3];
		std::cout << "shmptr val AFTER: " << shmptr[2] << " " << shmptr[3] << " " << shmptr[4] << " " << shmptr[5] << '\n';

		std::cout << "Shem writing complete..\n";
		std::cout << "shmptr INSIDE update() is:" << shmptr << "\n";
		/* wait 50 us */
		usleep(5000);
		return shmptr;
}