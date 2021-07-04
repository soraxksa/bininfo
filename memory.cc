#include "memory.h"


void dump_file(const char*filename){
	
	FILE *fd = fopen(filename, "r");
	char buf[1024];
	int addr = 0;
	int buf_size;
	while( (buf_size = fread(buf, 1, 1024, fd)) == 1024)
	{
		dump(buf, 1024, addr);
		addr += 1024;
	}
	dump(buf, buf_size, addr);
}


void dump(const char *data_buffer, size_t length, uint64_t addr = 0) 
{   
	unsigned char byte;
	unsigned int i, j;
	for(i=0; i < length; i++) 
	{
		if(i%16 == 0)
		{
			printf("0x%016x| ", addr);
			addr+=16;
		}
		byte = data_buffer[i];
		printf("%02x ", byte);
		if(((i%16)==15) || (i==length-1)) 
		{         
			for(j=0; j < 15-(i%16); j++)
				printf("   ");

			printf("| ");

			for(j=(i-(i%16)); j <= i; j++) 
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



