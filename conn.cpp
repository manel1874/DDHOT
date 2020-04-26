#include "include/comm/Comm.hpp"
#include "include/primitives/DlogOpenSSL.hpp"

int main(int argc, char* argv[]){

    boost::asio::io_service io_service;
    SocketPartyData senderParty, receiverParty;

    if(atoi(argv[1]) == 0){
        senderParty = SocketPartyData(boost_ip::address::from_string("127.0.0.1"), 8000);
        receiverParty = SocketPartyData(boost_ip::address::from_string("127.0.0.1"), 8001);
    } else {
        senderParty = SocketPartyData(boost_ip::address::from_string("127.0.0.1"), 8001);
        receiverParty = SocketPartyData(boost_ip::address::from_string("127.0.0.1"), 8000);

    }

    shared_ptr<CommParty> channel = make_shared<CommPartyTCPSynced>(io_service, me, other);
    // connect to party one
    channel->join(500, 5000);
    cout<<"channel established"<<endl;

    // initiate a discrete log group
    DlogGroup* dlog = new OpenSSLDlogECF2m("include/configFiles/NISTEC.txt", "K-233");

    // setup mode
    if(atoi(argv[1]) == 0){ // Sender prepares CRS elements
        if(atoi(argv[2]) == 0){ // setup Messy mode
            // generate two random group generators
            auto g0 = dlog->createRandomGenerator();
            auto g1 = dlog->createRandomGenerator();

            biginteger p = dlog->getOrder();

            // generate two random elements in Zp
            shared_ptr<PrgFromOpenSSLAES> gen = get_seeded_prg();
            biginteger r0 = getRandomInRange(0, p-1, gen.get());
            biginteger r1 = getRandomInRange(0, p-1, gen.get());

            // define h0 and h1
            auto h0 = dlog->exponentiate(g0.get(), r0);
            auto h1 = dlog->exponentiate(g1.get(), r1);

            shared_ptr<GroupElement> crs_sent[4] = {g0, h0, g1, h1};

            // send CRS elements to receiver
            channel->write((byte*) crs_sent, 100);
            cout << "CRS sent to Typhon\n";

        } else {// setup Decryption mode 
            // generate random group generators
            auto g0 = dlog->createRandomGenerator();

            biginteger p = dlog->getOrder();

            // generate two random elements in Zp
            shared_ptr<PrgFromOpenSSLAES> gen = get_seeded_prg();
            biginteger y = getRandomInRange(0, p-1, gen.get());
            biginteger x = getRandomInRange(0, p-1, gen.get());

            // define g1, h0 and h1
            auto g1 = dlog->exponentiate(g0.get(), y);
            auto h0 = dlog->exponentiate(g0.get(), x);
            auto h1 = dlog->exponentiate(g1.get(), x);

            shared_ptr<GroupElement> crs_sent[4] = {g0, h0, g1, h1};

            // send CRS elements to receiver
            channel->write((byte*) crs_sent, 100);
            cout << "CRS sent to Typhon\n";

            me->transfe

            }

    } else {
        byte* crs_received = new byte[100];
        channel->read(crs_received, 100);
        cout << crs_received[1];
    }

    if(atoi(argv[1]) == 0){
        channel->write((byte*) "Typhon, please behave yourself...\n", 100);
        cout << "Message sent to Typhon\n";
    } else {
        byte* helloTyphon = new byte[100];
        channel->read(helloTyphon, 100);
        cout << helloTyphon;
    }
    

}
