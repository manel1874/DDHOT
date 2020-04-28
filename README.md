# DDHOT
Implementation of OT under DDH assumption based on the following work: "A Framework for Efficient and Composable Oblivious Transfer"

There are two possible setup modes:

- SetupMessy:
  o Receiver'security: computational
  o Sender's security: statistical
- SetupDec:
  o Receiver's security: statistical
  o Sender'security: computational


In order to run:

compile by:

g++ conn.cpp -I/home/manel -I/home/manel/boost_1_71_0 -std=c++11 libscapi.a -lboost_system -L/home/manel/boost_1_71_0/stage/lib -lssl -lcrypto -lgmp -g -pthread

run in terminal 1
./a.out 0 0

run in terminal 2
./a.out 1 1


