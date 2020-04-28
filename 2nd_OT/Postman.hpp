
#pragma once

#include "include/comm/Comm.hpp"
#include "include/primitives/DlogOpenSSL.hpp"



void send_vec_ecelement(shared_ptr<CommParty> channel, vector<shared_ptr<GroupElement>> vec_ecelem);

vector<shared_ptr<GroupElement>> receive_vec_ecelement(vector<shared_ptr<GroupElement>> vec_ecelem, int size, DlogGroup* dlog);

