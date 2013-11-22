#ifndef __util_h__
#define __util_h__

#include <string>
using std::string;

string createPacket(string &nonce, string &message, string &newNonce);
string makeNonce();
string pad(std::string &packet);
string unPad(std::string &packet);

#endif
