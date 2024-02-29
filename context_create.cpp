// Storing information in a "raw" pointer.  Assume that the
 
#include <hooks/hooks.h>
   :
 
extern "C" {
 
// context_create callout - called when the request is created.
int context_create(CalloutHandle& handle) {
    // Create the security information and store it in the context
    // for this packet.
    SecurityInformation* si = new SecurityInformation();
    handle.setContext("security_information", si);
}
 
// Callouts that use the context
int pkt4_receive(CalloutHandle& handle) {
    // Retrieve the pointer to the SecurityInformation object
    SecurityInformation* si;
    handle.getContext("security_information", si);
        :
        :
    // Set the security information
    si->setSomething(...);
 
    // The pointed-to information has been updated but the pointer has not been
    // altered, so there is no need to call setContext() again.
}
 
int pkt4_send(CalloutHandle& handle) {
    // Retrieve the pointer to the SecurityInformation object
    SecurityInformation* si;
    handle.getContext("security_information", si);
        :
        :
    // Retrieve security information
    bool active = si->getSomething(...);
        :
}
 
// Context destruction.  We need to delete the pointed-to SecurityInformation
// object because we will lose the pointer to it when the @c CalloutHandle is
// destroyed.
int context_destroy(CalloutHandle& handle) {
    // Retrieve the pointer to the SecurityInformation object
    SecurityInformation* si;
    handle.getContext("security_information", si);
 
    // Delete the pointed-to memory.
    delete si;
}

