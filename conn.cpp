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

    /**
    sender inputs: [0] [setup type: 0 (messy), 1 (decryption)] [input0] [input 1]
    receiver inputs: [1] [element choice: 0, 1] 

    **/


    shared_ptr<CommParty> channel = make_shared<CommPartyTCPSynced>(io_service, senderParty, receiverParty);
    // connect to party one
    channel->join(500, 5000);
    cout<<"channel established"<<endl;

    // initiate a discrete log group
    DlogGroup* dlog = new OpenSSLDlogECF2m("include/configFiles/NISTEC.txt", "K-233");
    //auto dlog = make_shared<OpenSSLDlogZpSafePrime>(128);
    biginteger p = dlog->getOrder();
    cout << "\nOrder is: " << p << "\n";

    //important elements to be generally initialized
    vector<shared_ptr<GroupElement>> crs_received;
    vector<shared_ptr<GroupElement>> crs_sent;
    /**
    shared_ptr<GroupElement> g0;
    shared_ptr<GroupElement> g1;
    shared_ptr<GroupElement> h0;
    shared_ptr<GroupElement> h1;
    **/
    biginteger r;

    shared_ptr<GroupElement> g;
    shared_ptr<GroupElement> h;

    // setup mode
    if(atoi(argv[1]) == 0){ // sender prepares CRS elements
        if(atoi(argv[2]) == 0){ // setup Messy mode
            // generate two random group generators
            auto g0 = dlog->createRandomGenerator();
            auto g1 = dlog->createRandomGenerator();

            // generate two random elements in Zp
            shared_ptr<PrgFromOpenSSLAES> gen = get_seeded_prg();
            biginteger r0 = getRandomInRange(0, p-1, gen.get());
            biginteger r1 = getRandomInRange(0, p-1, gen.get());

            // define h0 and h1
            auto h0 = dlog->exponentiate(g0.get(), r0);
            auto h1 = dlog->exponentiate(g1.get(), r1);

            shared_ptr<GroupElement> crs_sent[4] = {g0, h0, g1, h1};

            /**
            // send CRS elements to receiver
            auto g0_send = g0->generateSendableData();
            auto g0_sendStr = g0_send->toString();
            channel->writeWithSize(g0_sendStr);
            cout << "g0 sent to Typhon\n";
            **/

            // Send vector group element to receiver
            for (shared_ptr<GroupElement> elem : crs_sent){
                auto elem_send = elem->generateSendableData();
                auto elem_sendStr = elem_send->toString();
                channel->writeWithSize(elem_sendStr);
            }

        } else { // setup Decryption mode 
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

            /**
            // send one group element to receiver

            auto g0_send = g0->generateSendableData();
            auto g0_sendStr = g0_send->toString();
            channel->writeWithSize(g0_sendStr);
            cout << "g0 sent to Typhon\n";
            **/
            
           // Send vector group element to receiver
           for (shared_ptr<GroupElement> elem : crs_sent){
               auto elem_send = elem->generateSendableData();
               auto elem_sendStr = elem_send->toString();
               channel->writeWithSize(elem_sendStr);
           }

        }

    } else { // receiver receives CRS elements
        
        /**
        // receive one group element from sender
        
        shared_ptr<GroupElement> g0_receivedStr;
	    shared_ptr<GroupElementSendableData> elem_sendable = make_shared<ECElementSendableData>(dlog->getOrder(), dlog->getOrder());
	    //shared_ptr<GroupElementSendableData> elem_sendable = make_shared<ZpElementSendableData>(dlog->getOrder());
	    vector<byte> raw_g0_receivedStr;
	    channel->readWithSizeIntoVector(raw_g0_receivedStr);
	    elem_sendable->initFromByteVector(raw_g0_receivedStr);
	    g0_receivedStr = dlog->reconstructElement(true, elem_sendable.get());

        cout << g0_receivedStr;
        **/

        for (int i=0; i < 4; i++){

            shared_ptr<GroupElement> crs_elem;
            shared_ptr<GroupElementSendableData> elem_receivable = make_shared<ECElementSendableData>(dlog->getOrder(), dlog->getOrder());
            //shared_ptr<GroupElementSendableData> elem_receivable = make_shared<ZpElementSendableData>(dlog->getOrder());
            vector<byte> raw_element;
            channel->readWithSizeIntoVector(raw_element);
            elem_receivable->initFromByteVector(raw_element);
            crs_elem = dlog->reconstructElement(true, elem_receivable.get());

            crs_received.push_back(crs_elem);
        }
        

    }

    // First computation part
    if(atoi(argv[1]) == 1){ // receiver computation (prepare keys )

        // key generation
        shared_ptr<PrgFromOpenSSLAES> gen = get_seeded_prg();
        biginteger r = getRandomInRange(0, p-1, gen.get());

        auto g_sigma = crs_received[2 * atoi(argv[2])];
        auto h_sigma = crs_received[2 * atoi(argv[2]) + 1];
        auto g = dlog->exponentiate(g_sigma.get(), r);
        auto h = dlog->exponentiate(h_sigma.get(), r);


        // send pk = (g, h)
        shared_ptr<GroupElement> pk[2] = {g, h};

        for (shared_ptr<GroupElement> elem : pk){
            auto elem_send = elem->generateSendableData();
            auto elem_sendStr = elem_send->toString();
            channel->writeWithSize(elem_sendStr);
           }

    } else { // sender computation (prepare encrypted inputs)

        // receive pk
        vector<shared_ptr<GroupElement>> pk_received;
        for (int i=0; i < 2; i++){
            shared_ptr<GroupElement> pk_elem;
            shared_ptr<GroupElementSendableData> pk_elem_receivable = make_shared<ECElementSendableData>(dlog->getOrder(), dlog->getOrder());
            //shared_ptr<GroupElementSendableData> pk_elem_receivable = make_shared<ZpElementSendableData>(dlog->getOrder());
            vector<byte> raw_pk_elem;
            channel->readWithSizeIntoVector(raw_pk_elem);
            pk_elem_receivable->initFromByteVector(raw_pk_elem);
            pk_elem = dlog->reconstructElement(true, pk_elem_receivable.get());

            pk_received.push_back(pk_elem);

        }

        // generate two elements
        shared_ptr<GroupElement> m0 = dlog->createRandomElement();
        shared_ptr<GroupElement> m1 = dlog->createRandomElement();

        // Encrypt m0
        //Generate s0 and t0
        shared_ptr<PrgFromOpenSSLAES> gen = get_seeded_prg();
        biginteger s0 = getRandomInRange(0, p-1, gen.get());
        biginteger t0 = getRandomInRange(0, p-1, gen.get());
        // define u0
        auto g0_s0 = dlog->exponentiate(crs_sent[0].get(), s0);
        auto h0_t0 = dlog->exponentiate(crs_sent[2].get(), t0);
        auto u0 = dlog->multiplyGroupElements(g0_s0.get(), h0_t0.get());
        //define v0
        auto gsig_s0 = dlog->exponentiate(pk_received[0].get(), s0);
        auto hsig_t0 = dlog->exponentiate(pk_received[1].get(), t0);
        auto v0 = dlog->multiplyGroupElements(gsig_s0.get(), hsig_t0.get());
        // define v0_m0
        auto v0_m0 = dlog->multiplyGroupElements(v0.get(), m0.get());

        // Encrypt m1
        //Generate s1 and t1
        shared_ptr<PrgFromOpenSSLAES> gen = get_seeded_prg();
        biginteger s1 = getRandomInRange(0, p-1, gen.get());
        biginteger t1 = getRandomInRange(0, p-1, gen.get());
        // define u1
        auto g1_s1 = dlog->exponentiate(crs_sent[1].get(), s1);
        auto h1_t1 = dlog->exponentiate(crs_sent[3].get(), t1);
        auto u1 = dlog->multiplyGroupElements(g1_s1.get(), h1_t1.get());
        //define v1
        auto gsig_s1 = dlog->exponentiate(pk_received[0].get(), s1);
        auto hsig_t1 = dlog->exponentiate(pk_received[1].get(), t1);
        auto v1 = dlog->multiplyGroupElements(gsig_s1.get(), hsig_t1.get());
        // define v1_m1
        auto v1_m1 = dlog->multiplyGroupElements(v1.get(), m1.get());

        // send y0 = (u0, v0_m0) and y1 = (u1, v1_m1)
        shared_ptr<GroupElement> Y[4] = {u0, v0_m0, u1, v1_m1};
        for (shared_ptr<GroupElement> elem : Y){
            auto elem_send = elem->generateSendableData();
            auto elem_sendStr = elem_send->toString();
            channel->writeWithSize(elem_sendStr);
        }

        cout << "\nMy first message, m0: " << m0.get();
        cout << "\nMy second message, m1: " << m1.get();
    
    }


    // Final decryption
    if(atoi(argv[1]) == 1){
        // Receive Y
        vector<shared_ptr<GroupElement>> Y_received;
        for (int i=0; i < 4; i++){
            shared_ptr<GroupElement> Y_elem;
            shared_ptr<GroupElementSendableData> Y_elem_receivable = make_shared<ECElementSendableData>(dlog->getOrder(), dlog->getOrder());
            //shared_ptr<GroupElementSendableData> pk_elem_receivable = make_shared<ZpElementSendableData>(dlog->getOrder());
            vector<byte> raw_Y_elem;
            channel->readWithSizeIntoVector(raw_Y_elem);
            Y_elem_receivable->initFromByteVector(raw_Y_elem);
            Y_elem = dlog->reconstructElement(true, Y_elem_receivable.get());

            Y_received.push_back(Y_elem);
        }

        int sigma = atoi(argv[1]);

        auto usig = Y_received[2 * sigma];
        auto vsig_msig = Y_received[2 * sigma + 1];

        auto vsig_computed = dlog->exponentiate(usig.get(), r);
        auto msig = dlog->multiplyGroupElements(vsig_msig.get(), vsig_computed.get());

        cout << "\nMy chosen element is: " << msig.get();

    }
    

}
