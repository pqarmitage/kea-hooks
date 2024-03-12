// load_unload.h
 
#ifndef _LOAD_UNLOAD_H_
#define _LOAD_UNLOAD_H_

#include <list>

#include "subnet.h"

extern "C" {

extern std::list<pqa::Subnet> ignore_subnets;
extern pqa::Changes name_changes;

}

#endif
