#include <cstdlib> 
#include "custom_init.h"

float* data;
float* inVals;

int main() {
	data = (float*)malloc(sizeof(float) * 4);
	data[2] = 0.0f;
	data[3] = 1.0f;

	float EEC_UEI_test1; //input params from shmem
	float EEC_UEI_test2; //input params from shmem

	custom_init customInst;
	float* shmptr = customInst.comm_init();

	/* Run Continuously */
	for (;;) {
		shmptr=customInst.update(data);
			/* wait 5 us */
		usleep(5000);
		std::cout << "shmptr OUTSIDE comm_init() is:" << shmptr << "\n";

		EEC_UEI_test1 = customInst.inVal1;
		EEC_UEI_test2 = customInst.inVal2;
		std::cout << "Read in params EEC_UEI_test1&2: " << shmptr[0] << " " << shmptr[1] << "\n";

		usleep(50000);

		data[2] = (float)(rand()%1000);
		data[3] = (float)(rand()%1000);

		std::cout << "Read back value from wrote to shmptr[0], shmptr[1], shmptr[2], shmptr[3]:" << shmptr[0] << " " << shmptr[1] << " " << shmptr[2] << " " << shmptr[3] << '\n';
	}
}
