#ifndef __util_h__
#define __util_h__

#include <string>
using std::string;

string makeNonce();
string createPacket(string &nonce, string &message, string &newNonce);

#endif
