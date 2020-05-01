#pragma once

#include "Postman.hpp"

class SenderParty {

private:

    shared_ptr<CommParty> channel;

    shared_ptr<DlogGroup> dlog;

    vector<shared_ptr<GroupElement>> crs_sent;

    shared_ptr<GroupElement> m0, m1;

    vector<shared_ptr<GroupElement>> pk_received;


public:

    SenderParty(int argc, char* argv[]);

    vector<shared_ptr<GroupElement>> genMessySetUp(shared_ptr<DlogGroup> dlog);

    vector<shared_ptr<GroupElement>> genDecSetUp(shared_ptr<DlogGroup> dlog);

    void run();

    vector<shared_ptr<GroupElement>> encryptMessage(shared_ptr<DlogGroup> dlog, int message_number, shared_ptr<GroupElement> mi);


};

class ReceiverParty {
private:

public:

    shared_ptr<CommParty> channel;

    int sigma;

    DlogGroup* dlog;

    vector<shared_ptr<GroupElement>> crs_received;
    
    biginteger r;

    shared_ptr<GroupElement> g;
    shared_ptr<GroupElement> h;

    ReceiverParty(int argc, char* argv[]);

    void run();

};