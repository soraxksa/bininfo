#include <bfd.h>
#include <stdio.h>
#include "loader.h"
#include <map>
#include <set>
#include <capstone/capstone.h>



bfd* Binary::open_bfd(const std::string &filename)
{
	static bool bfd_inited = false;

	if(!bfd_inited)
	{
		bfd_init();
		bfd_inited = true;
	}

	bfd *bfd_h = bfd_openr(filename.c_str(), NULL);
	if(!bfd_h)
	{
		fprintf(stderr, "failed to open binary '%s' (%s)\n", filename.c_str(), bfd_errmsg(bfd_get_error())); 
		return NULL;
	}

	if(!bfd_check_format(bfd_h, bfd_object))
	{
		fprintf(stderr, "file '%s' does not look like an executable (%s)\n", filename.c_str(), bfd_errmsg(bfd_get_error())); 
		bfd_close(bfd_h);
		return NULL;
	}

	//dumb bfd versions set a wrong_formaterror before detecting the format and then neglect to unset it once
	// we unset it manually.
	bfd_set_error(bfd_error_no_error);

	if(bfd_get_flavour(bfd_h) == bfd_target_unknown_flavour)
	{
		fprintf(stderr, "unrecognized format for binary '%s' (%s)\n", filename.c_str(), bfd_errmsg(bfd_get_error())); 
		bfd_close(bfd_h);
		return NULL;
	}

	return bfd_h;

}


void Binary::load_symbols_bfd(bfd *bfd_h)
{
	int64_t n = bfd_get_symtab_upper_bound(bfd_h);
	if(n == 0)
		return;
	if(n < 0)
	{
		fprintf(stderr, "failed to read symtab (%s)\n", bfd_errmsg(bfd_get_error()));
		exit(-1);
	}

	asymbol **bfd_symtab = (asymbol**) malloc(n);

	int64_t nsyms = bfd_canonicalize_symtab(bfd_h, bfd_symtab);
	if(nsyms < 0)
	{
		fprintf(stderr, "failed to read symtab (%s)\n", bfd_errmsg(bfd_get_error()));
		free(bfd_symtab);
		exit(-1);
	}

	add_symbols(bfd_symtab, nsyms);
	if(bfd_symtab)
		free(bfd_symtab);
}

void Binary::load_dynsym_bfd(bfd *bfd_h)
{
	int64_t n = bfd_get_dynamic_symtab_upper_bound(bfd_h);
	if(n == 0)
		return;
	if(n < 0)
	{
		fprintf(stderr, "failed to read dynamic symtab (%s)\n", bfd_errmsg(bfd_get_error()));
		exit(-1);
	}

	asymbol **bfd_dynsym = (asymbol**) malloc(n);

	int64_t nsyms = bfd_canonicalize_dynamic_symtab(bfd_h, bfd_dynsym);
	if(nsyms < 0)
	{
		fprintf(stderr, "failed to read dynamic symtab (%s)\n", bfd_errmsg(bfd_get_error()));
		free(bfd_dynsym);
		exit(-1);
	}

	add_symbols(bfd_dynsym, nsyms);

	if(bfd_dynsym)
		free(bfd_dynsym);
}


int Binary::load_sections_bfd(bfd *bfd_h)
{
	for(asection *bfd_sec = bfd_h->sections; bfd_sec; bfd_sec = bfd_sec->next)
	{
		int64_t bfd_sec_flags = bfd_sec->flags;

		Section::SectionType sectype = Section::SEC_TYPE_NONE;
		if(bfd_sec_flags & SEC_CODE){
			sectype = Section::SEC_TYPE_CODE;
		}
		else if(bfd_sec_flags & SEC_DATA){
			sectype = Section::SEC_TYPE_DATA;
		}
		else{
			continue;
		}

		uint64_t vma     = bfd_section_vma( bfd_sec);
		uint64_t size    = bfd_section_size( bfd_sec);
		const char* name = bfd_section_name( bfd_sec);
		if(name == NULL) name = "<unnamed>";


		uint8_t *bytes= (uint8_t*) malloc(size);
		if(!bfd_get_section_contents(bfd_h, bfd_sec, bytes, 0, size))
		{
			fprintf(stderr, "failed to read section '%s' (%s)\n", name, bfd_errmsg(bfd_get_error()));
			return -1;
		}

		this->sections.push_back( Section(this, std::string(name), sectype, vma, size, bytes));


	}
	return 1;
}

void Binary::add_symbols(asymbol **bfd_symtab, size_t nsyms)
{
 	std::set<const char *> weak_sym;

	for(int i = 0; i < nsyms; i++)
	{
		if(bfd_symtab[i]->flags & BSF_WEAK)
		{
			weak_sym.insert(bfd_symtab[i]->name);
		}
		else if(bfd_symtab[i]->flags & BSF_FUNCTION)
		{       if(weak_sym.find(bfd_symtab[i]->name) != weak_sym.end())
			{
				Symbol symbol = Symbol(Symbol::SYM_TYPE_FUNC, bfd_symtab[i]->name, bfd_asymbol_value(bfd_symtab[i]));
				weak_sym.erase(bfd_symtab[i]->name);
				this->symbols.erase(bfd_symtab[i]->name);
				this->symbols[bfd_symtab[i]->name] = std::move(symbol);

			}else{
				Symbol func = Symbol(Symbol::SYM_TYPE_FUNC, bfd_symtab[i]->name, bfd_asymbol_value(bfd_symtab[i]));
				this->symbols[bfd_symtab[i]->name] = std::move(func);
			}
		}else if(bfd_symtab[i]->flags & BSF_OBJECT)
		{
			if(weak_sym.find(bfd_symtab[i]->name) != weak_sym.end())
			{
				Symbol symbol = Symbol(Symbol::SYM_TYPE_OBJECT, bfd_symtab[i]->name, bfd_asymbol_value(bfd_symtab[i]));
				weak_sym.erase(bfd_symtab[i]->name);
				this->symbols.erase(bfd_symtab[i]->name);
				this->symbols[bfd_symtab[i]->name] = std::move(symbol);

			}else{
				Symbol symbol = Symbol(Symbol::SYM_TYPE_OBJECT, bfd_symtab[i]->name, bfd_asymbol_value(bfd_symtab[i]));
				this->symbols[bfd_symtab[i]->name] = std::move(symbol);
			} 
		}
	}
 
}


void Binary::disas(const char *section_name)
{
	
	Section *sec = this->get_section(section_name);
	if(sec == NULL)
	{
		printf("the binary does not have a section:%d\n", section_name);
		exit(-1);
	}

	csh dis;
	if(cs_open(CS_ARCH_X86, CS_MODE_64, &dis) != CS_ERR_OK)
	{
		printf("Failed to open Capstone\n");
		exit(-1);
	}

	cs_insn *insns;
	size_t n = cs_disasm(dis, sec->bytes, sec->size, sec->vma, 0, &insns);
        if(n <= 0)
	{
		printf("Disassembly error: %s\n", cs_strerror(cs_errno(dis)));
		exit(-1);
	}


	for(size_t i = 0; i < n; i++)
	{
		printf("0x%016jx: ", insns[i].address);
		/*
		for(size_t j = 0; j < 16; j++) {
			if(j < insns[i].size) printf("%02x ", insns[i].bytes[j]);
			else printf("   ");
		}
		*/
                printf("%-12s %s\n", insns[i].mnemonic, insns[i].op_str);
	}

	cs_free(insns, n);
	cs_close(&dis);
}
