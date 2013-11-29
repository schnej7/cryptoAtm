#ifndef __util_h__
#define __util_h__

#include <string>
using std::string;

typedef unsigned char byte;

string createPacket(string &nonce, string &message, string &newNonce);
string makeNonce();
string pad(std::string &packet);
string unPad(std::string &packet);
string SHA1( string data );
string compoundSHA1( string data1, string data2 );
string updateSHA1( string hash, string data );
string generateSecret( int len );
void* generateAESKey( byte *key, byte *iv );
string AESEncrypt( byte* key, byte* iv, string plaintext );

#endif
