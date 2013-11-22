#include <iostream>
using std::cerr;

#include <sstream>
using std::ostringstream;

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
using CryptoPP::StringSource;
using CryptoPP::StringSink;
using CryptoPP::AuthenticatedEncryptionFilter;
using CryptoPP::AuthenticatedDecryptionFilter;

#include "includes/cryptopp/hex.h"
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include "includes/cryptopp/cryptlib.h"
using CryptoPP::Exception;

//must be divisible by 16
const int PACKETSIZE = 1024;

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
string createPacket(std::string &nonce, std::string &message, std::string &newNonce) {
	
	string packet;
	string cipher;
	
	string plain = nonce + "$" + message + "$" + newNonce;

	//plain = pad(plain);
	
	AutoSeededRandomPool prng;

	SecByteBlock key(AES::DEFAULT_KEYLENGTH);
	prng.GenerateBlock(key, key.size());

	SecByteBlock iv(AES::BLOCKSIZE);
	prng.GenerateBlock(iv, iv.size());

	string hexIV;

	StringSource ss(iv, sizeof(iv), true,
	                new HexEncoder(new StringSink(hexIV)));

	try {
		GCM<AES>::Encryption e;
		e.SetKeyWithIV(key, key.size(), iv, iv.size());

		StringSource(plain, true,
		             new AuthenticatedEncryptionFilter(e,
			                                               new StringSink(cipher)));
		
	}
	catch (const Exception &e) {
		cerr << e.what() << std::endl;
		return "";
	}

	packet = hexIV + "$" + cipher;
	
	return packet;
}

std::vector<std::string> openPacket(string &packet) {

}
