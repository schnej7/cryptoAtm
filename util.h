#ifndef __util_h__
#define __util_h__

#include <vector>
#include <string>
using std::string;

typedef unsigned char byte;

string makeNonce();
string createPacket(string &key, string &nonce, string &message, string &newNonce);
std::vector<string> openPacket(string &packet, string &key);
string generateAESKey();
string generateAESIV();
string encryptAES(string &plain, string &hexKey, string &hexIV);
string decryptAES(string &cipher, string &hexKey, string &hexIV);
string pad(string &packet);
string unPad(string &packet);
string SHA1( string data );
string compoundSHA1( string data1, string data2 );
string updateSHA1( string hash, string data );
string generateSecret( int len );
void* generateAESKey( byte *key, byte *iv );
string AESEncrypt( byte* key, byte* iv, string plaintext );
string AESDecrypt( byte* key, byte* iv, string ctxt );
string itos( int i );

#endif
