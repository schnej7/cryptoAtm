#ifndef __util_h__
#define __util_h__

#include <vector>
#include <string>
using std::string;

string makeNonce();
string createPacket(string &key, string &nonce, string &message, string &newNonce);
std::vector<string> openPacket(string &packet, string &key);
string generateAESKey();
string generateAESIV();
string encryptAES(string &plain, string &hexKey, string &hexIV);
string decryptAES(string &cipher, string &hexKey, string &hexIV);
string pad(string &packet);
string unPad(string &packet);

#endif
