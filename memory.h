
#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdio.h>
#include <stdint.h>

void dump_file(const char*filename);
void dump(const char *data_buffer, size_t length, uint64_t addr = 0);


#endif
