#include <stdio.h>
#include "memory.h"


void dump(const char *data_buffer, size_t length, uint64_t addr /* = 0*/) 
{   
	for(int i=0; i < length; i++) 
	{
		if(i%16 == 0)
		{
			printf("0x%016x| ", addr);
			addr+=16;
		}

		unsigned char byte = data_buffer[i];
		printf("%02x ", byte);

		if(((i%16)==15) || (i==length-1)) 
		{         
			for(int j=0; j < 15-(i%16); j++)
				printf("   ");

			printf("| ");

			for(int j=(i-(i%16)); j <= i; j++) 
			{
				byte = data_buffer[j];

				if((byte > 31) && (byte < 127))
					printf("%c", byte); 
				else               
					printf(".");
			}         

			printf("\n");
		}    
	}  
} 


