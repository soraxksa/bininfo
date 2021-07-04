#include <stdio.h>
#include <stdlib.h>
#include "memory.h"


int main(int argc, char **argv){

	if(argc != 2){
		printf("[USAGE]: %s filename\n", argv[0]);
		return -1;
	}

	dump_file(argv[1]);
}


