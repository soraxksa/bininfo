#include "loader.h"
#include <stdio.h>
#include "../memory.h"



int main(int argc, char **argv)
{
	if(argc < 2){
		printf("Usage: %s <binary>\n", argv[0]);
		return 1;
	}


	Binary bin(argv[1]);
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

	printf("--------------------sections--------------------\n");
	for(auto &s : bin.sections)
	{
		printf("############### %s ###############\n", s.name.c_str());
		dump( (const char*)s.bytes, s.size, s.vma);
		printf("\n");
	}

	return 0;
	
}
