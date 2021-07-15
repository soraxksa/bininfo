#include <bfd.h>
#include "loader.h"




int load_binary(std::string &fname, Binary *bin, Binary::BinaryType type)
{
	return load_binary_bfd(fname, bin, type);
}



void unload_binary(Binary *bin)
{
	for(auto &s : bin->sections)
	{
		if(s.bytes)
			free(s.bytes);
	}

}
