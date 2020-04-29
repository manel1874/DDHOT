#include "OTParties.hpp"

// ================================ //
//                                  //
//        Main Functionality        //   
//                                  //
// ================================ //

int main(int argc, char* argv[]) {

    cout << "Here";
    
    int partyNum = atoi(argv[1]);
    
    
    if (partyNum == 0) { // sender
        // create sender party with input elements
        SenderParty sender(argc, argv);
        // run sender
        sender.run();
    } else if (partyNum == 1) { // receiver 
        // create receiver party with input elements
        ReceiverParty receiver(argc, argv);
        // run receiver
        receiver.run();
    } else {
        std::cerr << "partyId must be 0 or 1" << std::endl;
        return 1;
    }
    

    return 0;

}
     
    