#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include <time.h>
#include <iostream>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

class custom_init
{
public:
	float EEC_UEI_test1;
	float EEC_UEI_test2;
	float* data;
	static float* shmptr_master;
	float* getShmptr();
	float* shmptr;
	int first_pass;
	int nshare;
	static int shmid_master;
	int getShmid();
	float inVal1;
	float inVal2;
	static float* comm_init();
	custom_init(); //Class contructor
	float* update(int* data);
	float* getShmPtr();
	int  N_SHARE;
	char THR_SHARE[];
};