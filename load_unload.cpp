// load_unload.c++
 
#include <hooks/hooks.h>
#include "library_common.h"
 
using namespace isc::hooks;
 
extern "C" {
 
int load(LibraryHandle&) {
	return (0);
}
 
int unload() {
	return (0);
}
 
}
