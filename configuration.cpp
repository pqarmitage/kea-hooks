/*
 * The following code is for the confuration of third.so
 *
"hooks-libraries": [
    {
        "library": "/opt/first.so"
    },
    {
        "library": "/opt/second.so",
        "parameters": {
        }
    },
    {
        "library": "/opt/third.so",
        "parameters": {
            "mail": "spam@example.com",
            "floor": 13,
            "debug": false,
            "users": [ "alice", "bob", "charlie" ],
            "languages": {
                "french": "bonjour",
                "klingon": "yl'el"
            }
        }
    }
]
*/

int load(LibraryHandle& handle) {
    ConstElementPtr mail  = handle.getParameter("mail");
    ConstElementPtr floor = handle.getParameter("floor");
    ConstElementPtr debug = handle.getParameter("debug");
    ConstElementPtr users = handle.getParameter("users");
    ConstElementPtr lang  = handle.getParameter("languages");
 
    // String handling example
    if (!mail) {
        // Handle missing 'mail' parameter here.
        return (1);
    }
    if (mail->getType() != Element::string) {
        // Handle incorrect 'mail' parameter here.
        return (1);
    }
    std::string mail_str = mail->stringValue();
 
    // In the following examples safety checks are omitted for clarity.
    // Make sure you do it properly similar to mail example above
    // or you risk dereferencing null pointer or at least throwing
    // an exception!
 
    // Integer handling example
    int floor_num = floor->intValue();
 
    // Boolean handling example
    bool debug_flag = debug->boolValue();
 
    // List handling example
    std::cout << "There are " << users->size() << " users defined." << std::endl;
    for (int i = 0; i < users->size(); i++) {
        ConstElementPtr user = users->get(i);
        std::cout << "User " << user->stringValue() << std::endl;
    }
 
    // Map handling example
    std::cout << "There are " << lang->size() << " languages defined." << std::endl;
    if (lang->contains("french")) {
       std::cout << "One of them is French!" << std::endl;
    }
    ConstElementPtr greeting = lang->find("klingon");
    if (greeting) {
       std::cout << "Lt. Worf says " << greeting->stringValue() << std::endl;
    }
 
    // All validation steps were successful. The library has all the parameters
    // it needs, so we should report a success.
    return (0);
}
