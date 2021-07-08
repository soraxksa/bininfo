#include "memory.h"




void dump_file(const char *filename){
	
	FILE *fd = fopen(filename, "rb");
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


char* file_to_bin(const char *filename)
{
	FILE *fd = fopen(filename, "rb");
	if(fd == NULL)
	{
		printf("ERROR: could not open file:%s. in function file_to_bin()\n", filename);
	}

	size_t buf_size = 1024;
	size_t buf_used = 0;

	char *buf = malloc(buf_size);
	char *buf_tmp = buf;

	while( (buf_used = fread(buf_tmp, 1, 1024, fd)) == 1024)
	{
		buf_end += buf_used;
		if(buf_end - buf + 12
	}


}


