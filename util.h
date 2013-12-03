#ifndef __util_h__
#define __util_h__

#include <vector>
#include <string>
using std::string;

typedef unsigned char byte;

string makeNonce();
bool recvPacket(int sock, int length, string &packet);
bool sendPacket(int sock, int length, string &packet);
string createPacket(string &key, string &nonce, string &message, string &newNonce);
std::vector<string> openPacket(string &packet, string &hexKey);
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
byte* generateAESKey(string key);
string AESEncrypt( byte* key, byte* iv, string plaintext );
string AESDecrypt( byte* key, byte* iv, string ctxt );
string itos( int i );
void generateRandomKey(std::string name, byte* key, long unsigned int length);
string charToString(char * old );

#endif
