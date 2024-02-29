#include <util/multi_threading_mgr.h>
 
int commandHandler(CalloutHandle& handle) {
    ...
    {
        // Enter the critical section.
        isc::util::MultiThreadingCriticalSection ct;
        <add the subnet>
        // Leave the critical section.
    }
    ...
}
