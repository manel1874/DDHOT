#include <vector>

#include "OTParties.hpp"


// ================================ //
//                                  //
//              Sender              //   
//                                  //
// ================================ //

SenderParty::SenderParty(int argc, char* argv[]){
    
    
    // ++++++++++++++++++++++++++++ //
    //         Group Setup          //
    // ++++++++++++++++++++++++++++ //

    shared_ptr<OpenSSLDlogECF2m> dlog = make_shared<OpenSSLDlogECF2m>("include/configFiles/NISTEC.txt", "K-233");
    //auto dlog = make_shared<OpenSSLDlogZpSafePrime>(128);
    biginteger p = dlog->getOrder();
    cout << "\nOrder is: " << p << "\n";


    // ++++++++++++++++++++++++++++ //
    //          OT inputs           //
    // ++++++++++++++++++++++++++++ //    

    // generate two random numbers
    m0 = dlog->createRandomElement();
    m1 = dlog->createRandomElement();

    // Print for debugging

    auto ele0 = m0.get();
    ECElement * ECele0 = (ECElement*) ele0;
    biginteger test_int0 = ECele0->getX();
    cout << "\nMy first message, m0: " << test_int0;

    auto ele1 = m1.get();
    ECElement * ECele1 = (ECElement*) ele1;
    biginteger test_int1 = ECele1->getX();
    cout << "\nMy second message, m1: " << test_int1;


    // ++++++++++++++++++++++++++++ //
    //      Communication Setup     //
    // ++++++++++++++++++++++++++++ //
    
    boost::asio::io_service io_service;
    SocketPartyData senderParty, receiverParty;

    senderParty = SocketPartyData(boost_ip::address::from_string("127.0.0.1"), 8000);
    receiverParty = SocketPartyData(boost_ip::address::from_string("127.0.0.1"), 8001);
	
    channel = make_shared<CommPartyTCPSynced>(io_service, senderParty, receiverParty);
    // connect to party one
    channel->join(500, 5000);
    cout << "channel established\n";


    // ++++++++++++++++++++++++++++ //
    //          CRS Setup           //
    // ++++++++++++++++++++++++++++ //

    if(atoi(argv[2]) == 0){ // setup Messy mode

        crs_sent = genMessySetUp(dlog);

        // Send vector group element to receiver
        send_vec_ecelement(channel, crs_sent);

    } else { // setup Decryption mode 
        
        crs_sent = genDecSetUp(dlog);
        
        // Send vector group element to receiver
        send_vec_ecelement(channel, crs_sent);

    }

    // ++++++++++++++++++++++++++++ //
    //          Receive pk          //
    // ++++++++++++++++++++++++++++ //

    pk_received = receive_vec_ecelement(channel, dlog, 2);


    // ++++++++++++++++++++++++++++ //
    //       m0, m1 Encryption      //
    // ++++++++++++++++++++++++++++ //

    //biginteger p = dlog->getOrder();

    vector<shared_ptr<GroupElement>> u0__v0_m0;
    u0__v0_m0 = encryptMessage(dlog, 0, m0); // = y0 
    vector<shared_ptr<GroupElement>> u1__v1_m1; 
    u1__v1_m1 = encryptMessage(dlog, 1, m1); // = y1

    // ++++++++++++++++++++++++++++ //
    //         Send y0, y1          //
    // ++++++++++++++++++++++++++++ //
    
    vector<shared_ptr<GroupElement>> Y;
    shared_ptr<GroupElement> u0 = u0__v0_m0[0];
    shared_ptr<GroupElement> v0_m0 = u0__v0_m0[1];
    shared_ptr<GroupElement> u1 = u1__v1_m1[0];
    shared_ptr<GroupElement> v1_m1 = u1__v1_m1[1];
    

    Y.push_back(u0);
    Y.push_back(v0_m0);
    Y.push_back(u1);
    Y.push_back(v1_m1);

    send_vec_ecelement(channel, Y);


}

/*
void SenderParty::run() {

    
    // ++++++++++++++++++++++++++++ //
    //          Receive pk          //
    // ++++++++++++++++++++++++++++ //

    pk_received = receive_vec_ecelement(channel, dlog, 2);


    // ++++++++++++++++++++++++++++ //
    //       m0, m1 Encryption      //
    // ++++++++++++++++++++++++++++ //

    biginteger p = dlog->getOrder();

    shared_ptr<GroupElement> u0, v0_m0 = encryptMessage(dlog, p, 0, m0); // = y0
    shared_ptr<GroupElement> u1, v1_m1 = encryptMessage(dlog, p, 1, m1); // = y1

    // ++++++++++++++++++++++++++++ //
    //         Send y0, y1          //
    // ++++++++++++++++++++++++++++ //

    vector<shared_ptr<GroupElement>> Y;
    Y.push_back(u0);
    Y.push_back(v0_m0);
    Y.push_back(u1);
    Y.push_back(v1_m1);

    send_vec_ecelement(channel, Y);

}
*/

// ---------------------------------- //
// ------- Auxiliar Functions ------- //
// ---------------------------------- //

vector<shared_ptr<GroupElement>> SenderParty::genMessySetUp(shared_ptr<DlogGroup> dlog){

    vector<shared_ptr<GroupElement>> crs;
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

    crs.push_back(g0);
    crs.push_back(h0);
    crs.push_back(g1);
    crs.push_back(h1);

    return crs;

}

vector<shared_ptr<GroupElement>> SenderParty::genDecSetUp(shared_ptr<DlogGroup> dlog){

    vector<shared_ptr<GroupElement>> crs;
    
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

    crs.push_back(g0);
    crs.push_back(h0);
    crs.push_back(g1);
    crs.push_back(h1);

    return crs;

}

vector<shared_ptr<GroupElement>> SenderParty::encryptMessage(shared_ptr<DlogGroup> dlog, int message_number, shared_ptr<GroupElement> mi){

    biginteger p = dlog->getOrder();
    vector<shared_ptr<GroupElement>> ui__vi_mi;
    
    // Encrypt mi
    // Generate si and ti
    shared_ptr<PrgFromOpenSSLAES> gen_encmi = get_seeded_prg();
    biginteger si = getRandomInRange(0, p-1, gen_encmi.get());
    biginteger ti = getRandomInRange(0, p-1, gen_encmi.get());
    // Define ui (recall that crs_sent = {g0, h0, g1, h1})
    auto gi_si = dlog->exponentiate(crs_sent[2 * message_number].get(), si);
    auto hi_ti = dlog->exponentiate(crs_sent[2 * message_number + 1].get(), ti);
    auto ui = dlog->multiplyGroupElements(gi_si.get(), hi_ti.get());
    // Define vi
    auto gsig_si = dlog->exponentiate(pk_received[0].get(), si);
    auto hsig_ti = dlog->exponentiate(pk_received[1].get(), ti);
    auto vi = dlog->multiplyGroupElements(gsig_si.get(), hsig_ti.get());
    // Define vi_mi
    auto vi_mi = dlog->multiplyGroupElements(vi.get(), mi.get());

    ui__vi_mi.push_back(ui);
    ui__vi_mi.push_back(vi_mi);

    return ui__vi_mi;

}


// ================================ //
//                                  //
//              Receiver            //   
//                                  //
// ================================ //

ReceiverParty::ReceiverParty(int argc, char* argv[]) {

    // ++++++++++++++++++++++++++++ //
    //          OT inputs           //
    // ++++++++++++++++++++++++++++ //    
    
    sigma = atoi(argv[2]);

    // ++++++++++++++++++++++++++++ //
    //      Communication Setup     //
    // ++++++++++++++++++++++++++++ //
    
    boost::asio::io_service io_service;
    SocketPartyData senderParty, receiverParty;

    senderParty = SocketPartyData(boost_ip::address::from_string("127.0.0.1"), 8000);
    receiverParty = SocketPartyData(boost_ip::address::from_string("127.0.0.1"), 8001);
	
    channel = make_shared<CommPartyTCPSynced>(io_service, receiverParty, senderParty);
    // connect to party one
    channel->join(500, 5000);
    cout<<"channel established"<<endl;

    // ++++++++++++++++++++++++++++ //
    //         Group Setup          //
    // ++++++++++++++++++++++++++++ //

    shared_ptr<DlogGroup> dlog = make_shared<OpenSSLDlogECF2m>("include/configFiles/NISTEC.txt", "K-233");
    //auto dlog = make_shared<OpenSSLDlogZpSafePrime>(128);
    biginteger p = dlog->getOrder();
    cout << "\nOrder is: " << p << "\n";

    // ++++++++++++++++++++++++++++ //
    //          CRS Setup           //
    // ++++++++++++++++++++++++++++ //    

    crs_received = receive_vec_ecelement(channel, dlog, 4);

    // ++++++++++++++++++++++++++++ //
    //        Key Generation        //
    // ++++++++++++++++++++++++++++ //

    //biginteger p = dlog->getOrder();

    shared_ptr<PrgFromOpenSSLAES> gen = get_seeded_prg();
    r = getRandomInRange(0, p-1, gen.get());

    auto g_sigma = crs_received[2 * sigma];
    auto h_sigma = crs_received[2 * sigma + 1];
    auto g = dlog->exponentiate(g_sigma.get(), r);
    auto h = dlog->exponentiate(h_sigma.get(), r);


    // ++++++++++++++++++++++++++++ //
    //           Send pk            //
    // ++++++++++++++++++++++++++++ //
    vector<shared_ptr<GroupElement>> pk;
    pk.push_back(g);
    pk.push_back(h);

    send_vec_ecelement(channel, pk);


    // ++++++++++++++++++++++++++++ //
    //        Receive y0, y1        //
    // ++++++++++++++++++++++++++++ //

    vector<shared_ptr<GroupElement>> Y_received = receive_vec_ecelement(channel, dlog, 4);

    auto usig = Y_received[2 * sigma];
    auto vsig_msig = Y_received[2 * sigma + 1];

    auto vsig_computed = dlog->exponentiate(usig.get(), r);
    auto vsig_comp_inverse = dlog->getInverse(vsig_computed.get());
    auto msig = dlog->multiplyGroupElements(vsig_msig.get(), vsig_comp_inverse.get());


    // ++++++++++++++++++++++++++++ //
    //         Print m_sigma        //
    // ++++++++++++++++++++++++++++ //

    auto ele = msig.get();
    ECElement * ECele = (ECElement*) ele;
    biginteger test_int = ECele->getX();
    cout << "\nMy chosen element is: " << test_int;

}

/*
void ReceiverParty::run() {

    // ++++++++++++++++++++++++++++ //
    //        Key Generation        //
    // ++++++++++++++++++++++++++++ //

    biginteger p = dlog->getOrder();

    shared_ptr<PrgFromOpenSSLAES> gen = get_seeded_prg();
    r = getRandomInRange(0, p-1, gen.get());

    auto g_sigma = crs_received[2 * sigma];
    auto h_sigma = crs_received[2 * sigma + 1];
    auto g = dlog->exponentiate(g_sigma.get(), r);
    auto h = dlog->exponentiate(h_sigma.get(), r);


    // ++++++++++++++++++++++++++++ //
    //           Send pk            //
    // ++++++++++++++++++++++++++++ //
    vector<shared_ptr<GroupElement>> pk;
    pk.push_back(g);
    pk.push_back(h);

    send_vec_ecelement(channel, pk);


    // ++++++++++++++++++++++++++++ //
    //        Receive y0, y1        //
    // ++++++++++++++++++++++++++++ //

    vector<shared_ptr<GroupElement>> Y_received = receive_vec_ecelement(channel, dlog, 4);

    auto usig = Y_received[2 * sigma];
    auto vsig_msig = Y_received[2 * sigma + 1];

    auto vsig_computed = dlog->exponentiate(usig.get(), r);
    auto vsig_comp_inverse = dlog->getInverse(vsig_computed.get());
    auto msig = dlog->multiplyGroupElements(vsig_msig.get(), vsig_comp_inverse.get());


    // ++++++++++++++++++++++++++++ //
    //         Print m_sigma        //
    // ++++++++++++++++++++++++++++ //

    auto ele = msig.get();
    ECElement * ECele = (ECElement*) ele;
    biginteger test_int = ECele->getX();
    cout << "\nMy chosen element is: " << test_int;

}
*/