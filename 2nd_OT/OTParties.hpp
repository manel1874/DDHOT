#pragma once

#include "Postman.hpp"

class SenderParty {
private:

    shared_ptr<CommParty> channel;

    DlogGroup* dlog;

    vector<shared_ptr<GroupElement>> crs_sent;

    shared_ptr<GroupElement> m0, m1;

    vector<shared_ptr<GroupElement>> pk_received;

public:

    SenderParty(int argc, char* argv[]);

    vector<shared_ptr<GroupElement>> genMessySetUp(DlogGroup* dlog, biginteger p);

    vector<shared_ptr<GroupElement>> genDecSetUp(DlogGroup* dlog, biginteger p);

    void run();

    shared_ptr<GroupElement> encryptMessage(biginteger p, int message_number, shared_ptr<GroupElement> mi);

};

class ReceiverParty {
private:

    shared_ptr<CommParty> channel;

    int sigma;

    DlogGroup* dlog;

    vector<shared_ptr<GroupElement>> crs_received;
    
    biginteger r;

    shared_ptr<GroupElement> g;
    shared_ptr<GroupElement> h;

public:

    ReceiverParty(int argc, char* argv[]);

    void run();

};