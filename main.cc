#include "loader.h"
#include <stdio.h>
#include "memory.h"
#include <string.h>


static bool section_arg = false;
static const char* section_name = NULL;

static const char* binary_name = NULL;

static bool disas_arg = false;
static bool symbol_arg = false;
static bool list_section_arg = false;

int main(int argc, char **argv)
{
	if(argc < 2){
Usage:
		printf("Usage: %s [options] <binary>\n\n"
		       "options:-\n"
		       "-s <section name> [to hexdump a section or disassemble it if option -d is set, -s all will hexdump every section]\n"
		       "-d [disassemble a section name given by -s option]\n"
		       "-S [to list symbols]\n"
		       "-l [to list sections]\n", argv[0]);
		return -1;
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
			case 'd':
				disas_arg = true;
				break;
			case 'S':
				symbol_arg = true;
				break;
			case 'l':
				list_section_arg = true;
				break;




		}
	}

	//check args logic
	if(disas_arg && !section_arg)
	{
		printf("Error: no section was given for disassembling\n");
		return -1;
	}

	argv += ar_index;
	if(*argv == NULL)
		goto Usage;
	binary_name = *argv;


	Binary bin(binary_name);
	printf("loaded binary '%s' %s/%s (%u bits) entry@0x%016jx\n", bin.filename.c_str(), bin.type_str.c_str(), bin.arch_str.c_str(), bin.bits, bin.entry);

	if(list_section_arg)
	{
		for(int i = 0; i < bin.sections.size(); i++)
		{
			Section *sec = &bin.sections[i];
			printf(" 0x%016jx %-8ju %-20s %s\n", sec->vma, sec->size, sec->name.c_str(), sec->type == Section::SEC_TYPE_CODE ? "CODE" : "DATA");
		}
	}

	if(symbol_arg && (bin.symbols.size() > 0) )
	{
		printf("scanned symbol tables\n");
		for(auto &p : bin.symbols)
		{
			Symbol &sym = p.second;
			const char *sym_type = NULL;
			switch(sym.type)
			{
				case Symbol::SYM_TYPE_FUNC:
					sym_type = "FUNC";
					break;
				case Symbol::SYM_TYPE_OBJECT:
					sym_type = "OBJECT";
					break;
				default:
					sym_type = "UNKNOWN";
			}
			printf(" %-40s 0x%016jx %s\n", sym.name.c_str(), sym.addr, sym_type);
		}
	}
        if(section_arg && disas_arg)
	{
		bin.disas(section_name);
	}
	else if(section_arg)
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
