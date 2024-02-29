// Storing information in a "raw" pointer.  Assume that the
 
#include <hooks/hooks.h>
#include <boost/shared_ptr.hpp>
   :
 
extern "C" {
 
// context_create callout - called when the request is created.
 
int context_create(CalloutHandle& handle) {
    // Create the security information and store it in the context for this
    // packet.
    boost::shared_ptr<SecurityInformation> si(new SecurityInformation());
    handle.setContext("security_information", si);
}
 
// Other than the data type, a shared pointer has similar semantics to a "raw"
// pointer.  Only the code from "pkt4_receive" is shown here.
 
int pkt4_receive(CalloutHandle& handle) {
    // Retrieve the pointer to the SecurityInformation object
    boost::shared_ptr<SecurityInformation> si;
    handle.setContext("security_information", si);
        :
        :
    // Modify the security information
    si->setSomething(...);
 
    // The pointed-to information has been updated but the pointer has not
    // altered, so there is no need to reset the context.
}
 
// No context_destroy callout is needed to delete the allocated
// SecurityInformation object.  When the @c CalloutHandle is destroyed, the shared
// pointer object will be destroyed.  If that is the last shared pointer to the
// allocated memory, then it too will be deleted.

