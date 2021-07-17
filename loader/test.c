#include "loader.h"
#include <stdio.h>
#include "../memory.h"
#include <string.h>


static bool section_arg = false;
static const char* section_name = NULL;

static const char* binary_name = NULL;

int main(int argc, char **argv)
{
	if(argc < 2){
Usage:
		printf("Usage: %s [-s <section name>] <binary>\n", argv[0]);
		return 1;
	}

	int ar_index;
	for(ar_index = 1; ar_index < argc && argv[ar_index][0] == '-'; ar_index++)
	{
		switch(argv[ar_index][1])
		{
			case 's':
				section_arg = true;
				ar_index++;
				section_name = argv[ar_index];
				break;

		}
	}

	argv += ar_index;
	if(*argv == NULL)
		goto Usage;
	binary_name = *argv;


	Binary bin(binary_name);
	printf("loaded binary '%s' %s/%s (%u bits) entry@0x%016jx\n", bin.filename.c_str(), bin.type_str.c_str(), bin.arch_str.c_str(), bin.bits, bin.entry);
	for(int i = 0; i < bin.sections.size(); i++)
	{
		Section *sec = &bin.sections[i];
		printf(" 0x%016jx %-8ju %-20s %s\n", sec->vma, sec->size, sec->name.c_str(), sec->type == Section::SEC_TYPE_CODE ? "CODE" : "DATA");
	}

	if(bin.symbols.size() > 0)
	{
		printf("scanned symbol tables\n");
		for(int i = 0; i < bin.symbols.size(); i++)
		{
			Symbol *sym = &bin.symbols[i];
			printf(" %-40s 0x%016jx %s\n", sym->name.c_str(), sym->addr, (sym->type & Symbol::SYM_TYPE_FUNC) ? "FUNC" : "");
		}
	}

	if(section_arg)
	{
		if(strcmp(section_name, "all") == 0)
		{
			printf("--------------------sections--------------------\n");
			for(auto &s : bin.sections)
			{
				printf("############### %s ###############\n", s.name.c_str());
				dump( (const char*)s.bytes, s.size, s.vma);
				printf("\n");
			} 
		}
		else	
		{
			printf("--------------------section---------------------\n");
			for(auto &s : bin.sections)
			{
				if(strcmp(s.name.c_str(), section_name) == 0)
				{
					printf("############### %s ###############\n", s.name.c_str());
					dump( (const char*)s.bytes, s.size, s.vma);
					printf("\n");
					break;
				}
			}
		}

		

	}

	return 0;
	
}
