#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <sstream>
using std::ostringstream;
using std::stringstream;

#include <string>
using std::string;

#include "util.h"
#include "includes/cryptopp/osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include "includes/cryptopp/aes.h"
using CryptoPP::AES;

#include "includes/cryptopp/gcm.h"
using CryptoPP::GCM;

#include "includes/cryptopp/secblock.h"
using CryptoPP::SecByteBlock;

#include "includes/cryptopp/filters.h"
using CryptoPP::Redirector;
using CryptoPP::StringSource;
using CryptoPP::StringSink;
using CryptoPP::AuthenticatedEncryptionFilter;
using CryptoPP::AuthenticatedDecryptionFilter;

#include "includes/cryptopp/hex.h"
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include "includes/cryptopp/cryptlib.h"
using CryptoPP::Exception;

#include "includes/cryptopp/sha.h"
using CryptoPP::SHA;

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

//must be divisible by 16
//const int PACKETSIZE = 1024;

string makeNonce() {

    const unsigned int BLOCKSIZE = 4;
    byte scratch[BLOCKSIZE];

	AutoSeededRandomPool prng;
	prng.GenerateBlock(scratch, BLOCKSIZE);

	unsigned int nonceInt = 0;
	
	for(int i = (BLOCKSIZE - 1); i >= 0; i--){
		nonceInt += (nonceInt << 8) + scratch[i];
	}

	ostringstream oss;
	oss << nonceInt;
	string nonceStr = oss.str();

    return nonceStr;
}

/*this is a function that will take a message and break
  it into  an appropiately formatted and sized packet (based on PACKETSIZE)
  inputs are the recived nonce, and the message
  output is a list of strings where each string is an (encrypted?) package
  with the IV (for AES) tacked onto the front
  format before encryption should be something along the lines of:
  <IV> <nonce> <message> <new nonce> <padding>:
*/
string createPacket(string &hexKey, string &nonce, string &message, string &newNonce) {
	
	string plain = nonce + "$" + message + "$" + newNonce;

	string hexIV = generateAESIV();

	string cipher = encryptAES(plain, hexKey, hexIV);

	string packet = hexIV + "$" + cipher;
	
	return packet;
}

std::vector<string> openPacket(string &packet, string &key) {
	string hexIV = packet.substr(0, 48);
	string cipher = packet.substr(49);

	string plain = decryptAES(cipher, key, hexIV);
	plain = unPad(plain);

	std::vector<string> results;
	
	boost::char_separator<char> sep("$");
	boost::tokenizer<boost::char_separator<char> > tokens(plain, sep);
	BOOST_FOREACH (const std::string & t, tokens) {
		results.push_back(t);
	}

	return results;
}

string generateAESKey(){

	AutoSeededRandomPool prng;

	SecByteBlock key(AES::DEFAULT_KEYLENGTH);
	prng.GenerateBlock(key, key.size());
	
	string hexKey;

	StringSource ss(key, sizeof(key), true,
	                new HexEncoder(new StringSink(hexKey)));

	return hexKey;
}

string generateAESIV() {

	AutoSeededRandomPool prng;

	SecByteBlock iv(AES::BLOCKSIZE);
	prng.GenerateBlock(iv, iv.size());

	string hexIV;

	StringSource ss(iv, sizeof(iv), true,
	                new HexEncoder(new StringSink(hexIV)));

	return hexIV;

}

string encryptAES(string &plain, string &hexKey, string &hexIV){

	string key;
	string iv;
	string cipher;
	plain = pad(plain);

	const int TAG_SIZE = 12;
	
	StringSource keyss(hexKey, true,
	                   new HexDecoder(new StringSink(key)));

	StringSource ivss(hexIV, true,
	                  new HexDecoder(new StringSink(iv)));


	try {
		GCM<AES>::Encryption e;
		e.SetKeyWithIV((byte*)key.data(), key.size(), (byte*)iv.data(), iv.size());

		StringSource(plain, true,
		             new AuthenticatedEncryptionFilter(e,
		                                               new StringSink(cipher),
		                                               false, TAG_SIZE));
		
	}
	catch (const Exception &e) {
		cerr << e.what() << endl;
		return "";
	}

	return cipher;

}

string decryptAES(string &cipher, string &hexKey, string &hexIV) {

	string iv;
	string plain;
	string key;

	const int TAG_SIZE = 12;

	StringSource keyss(hexIV, true,
	                   new HexDecoder(new StringSink(iv)));

	StringSource ivss(hexKey, true,
	                  new HexDecoder(new StringSink(key)));

	try {
		GCM<AES>::Decryption d;
		d.SetKeyWithIV((byte*)key.data(), key.size(), (byte*)iv.data(), iv.size());

		AuthenticatedDecryptionFilter df(d, new StringSink(plain),
		                                 AuthenticatedDecryptionFilter::DEFAULT_FLAGS,
		                                 TAG_SIZE);

		StringSource(cipher, true, new Redirector(df));

		if(true == df.GetLastResult()){
			return plain;
		}
		else {
			return "";
		}

	}
	catch(const Exception& e) {
		cerr << e.what() << endl;
		return "";
	}

}

string pad(string &packet) {

	//std::cout << "Before pad size: "<< packet << " is " << (int)packet.size() << " characters" << std::endl;
    if (packet.size() < 959) {
        packet += "~";
    }
    while (packet.size() < 959) {
        packet += "a";
    }
    packet += "\0";

    //std::cout << "After pad size: "<< packet << " is " << (int)packet.size() << " characters" << std::endl;

	return packet;
}

string unPad(string &packet){

	int position = -1;
	bool marker = false;

	for(int i = 0; i < packet.size(); ++i){
		if(packet[i] == '~'){
			position = i;
			marker = true;
			break;
		}
	}

	if(marker){
		packet = packet.substr(0,position);
	}
	return packet;
}

string SHA1( string data ){
    CryptoPP::SHA1 sha1;
    std::string hash = "";
    CryptoPP::StringSource(data, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
    return hash;
}

string compoundSHA1( string data1, string data2 ){
    return SHA1( SHA1( data1 ) + SHA1( data2 ) );
}

string updateSHA1( string hash, string data ){
    return SHA1( hash + SHA1( data ) );
}

string generateSecret( int len ){
    std::string secret = "";
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for( int i = 0; i < len; i++ ){
        secret += alphanum[ rand() % (sizeof(alphanum) - 1) ];
    }
    return secret;
}

void* generateAESKey( byte* key, byte* iv ){
    byte a_key[ CryptoPP::AES::DEFAULT_KEYLENGTH ];
    byte a_iv[ CryptoPP::AES::BLOCKSIZE ];

    key = a_key;
    iv  = a_iv;

    memset( a_key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH );
    memset( a_iv, 0x00, CryptoPP::AES::BLOCKSIZE );
}

string AESEncrypt( byte* key, byte* iv, string plaintext ){
    string ciphertext;
    CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, iv );

    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink( ciphertext ) );
    stfEncryptor.Put( reinterpret_cast<const unsigned char*>( plaintext.c_str() ), plaintext.length() + 1 );
    stfEncryptor.MessageEnd();
    return ciphertext;
}

string AESDecrypt( byte* key, byte* iv, string ctxt ){
    return ctxt;
}

string itos( int i ){
    stringstream ss;
    ss << i;
    return ss.str();
}
