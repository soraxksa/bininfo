#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>
#include <bfd.h>
#include <string>
#include <vector>
#include <map>

class Binary;
class Section;
class Symbol;

class Symbol{
public:
	enum SymbolType{
		SYM_TYPE_UKN  = 0,
		SYM_TYPE_FUNC = 1,
		SYM_TYPE_OBJECT = 2,
		SYM_TYPE_FILE   = 3
	};

	Symbol():
		type(SYM_TYPE_UKN),
		name(),
		addr(0)
	{
	}

	Symbol(Symbol::SymbolType type, std::string name, uint64_t addr):
		type(type),
		name(name),
		addr(addr)
	{
	}
         

	SymbolType type;
	std::string name;
	uint64_t addr;
};

class Section{
public:
	enum SectionType{
		SEC_TYPE_NONE = 0,
		SEC_TYPE_CODE = 1,
		SEC_TYPE_DATA = 2
	};

	Section(Binary *bin, std::string name, Section::SectionType type, uint64_t vma, uint64_t size, uint8_t *bytes):
		binary(bin),
		name(name),
		type(type),
		vma(vma),
		size(size),
		bytes(bytes)
	{
	}

	~Section()
	{
		if(bytes)
			free(bytes);
	}

	Section(Section &&other)
	{
		this->binary = other.binary;
		this->name = other.name;
		this->type = other.type;
		this->vma = other.vma;
		this->size = other.size;
		this->bytes = other.bytes;

		other.bytes = NULL;
	}

	bool contains(uint64_t addr) {return (addr >= vma) && (addr-vma < size);}

	Binary      *binary;
	std::string  name;
	SectionType  type;
	uint64_t     vma;
	uint64_t     size;
	uint8_t     *bytes;
};

class Binary{
public:
	enum BinaryType{
		BIN_TYPE_AUTO = 0,
		BIN_TYPE_ELF  = 1,
		BIN_TYPE_PE   = 2
	};

	enum BinaryArch{
		ARCH_NONE = 0,
		ARCH_X84  = 1
	};

	Binary(const std::string &filename):
		filename(filename),
		type(BIN_TYPE_AUTO),
		arch(ARCH_NONE),
		bits(0),
		entry(0)
	{
		bfd *bfd_h = open_bfd(filename);
		
		if(!bfd_h)
		{
			exit(-1);
		}

		this->entry = bfd_get_start_address(bfd_h);
		this->type_str = std::string(bfd_h->xvec->name);

		switch(bfd_h->xvec->flavour)
		{
			case bfd_target_elf_flavour:
				this->type = Binary::BIN_TYPE_ELF;
				break;
			case bfd_target_coff_flavour:
				this->type = Binary::BIN_TYPE_PE;
				break;
			case bfd_target_unknown_flavour:
			default:
				fprintf(stderr, "unsupported binary type (%s)\n", bfd_h->xvec->name);
				exit(-1);
		}

		const bfd_arch_info_type *bfd_info = bfd_get_arch_info(bfd_h);
		this->arch_str = std::string(bfd_info->printable_name);

		switch(bfd_info->mach)
		{
			case bfd_mach_i386_i386:
				this->arch = Binary::ARCH_X84;
				this->bits = 32;
				break;
			case bfd_mach_x86_64:
				this->arch = Binary::ARCH_X84;
				this->bits = 64;
				break;
			default:
				fprintf(stderr, "unsupported architecture (%s)\n", bfd_info->printable_name);
				exit(-1);
		}

		load_symbols_bfd(bfd_h);
		
	        load_dynsym_bfd(bfd_h);

		
		if(load_sections_bfd(bfd_h) < 0)
		{
			exit(-1);
		}
		

		if(bfd_h)
			bfd_close(bfd_h);
		

	}


	
	Section* get_section(const std::string &sec_name){
	        for(auto &s : sections)
			if(s.name == sec_name)
				return &s;
		return NULL;
	}

	void disas(const char *section_name);

	std::string          filename;
	BinaryType           type;
	std::string          type_str;
	BinaryArch           arch;
	std::string          arch_str;
	uint32_t             bits;
	uint64_t             entry;
	std::vector<Section> sections;
	std::map<const char *, Symbol>  symbols;


private:
	void load_symbols_bfd(bfd *bfd_h);
	void load_dynsym_bfd(bfd *bfd_h);
	int load_sections_bfd(bfd *bfd_h);
	bfd* open_bfd(const std::string &filename);
	void add_symbols(asymbol **bfd_symtab, size_t nsyms); 
};


#endif
