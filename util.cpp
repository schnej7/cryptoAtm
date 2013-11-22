#include <iostream>
#include <sstream>
#include "util.h"
#include "includes/cryptopp/osrng.h"

//must be divisible by 16
const int PACKETSIZE = 1024;

std::string makeNonce() {

    const unsigned int BLOCKSIZE = 4;
    byte scratch[BLOCKSIZE];

    CryptoPP::AutoSeededRandomPool prng;
    prng.GenerateBlock(scratch, BLOCKSIZE);

    int nonceInt = 0;

    for (int i = (BLOCKSIZE - 1); i >= 0; i--) {
        nonceInt += (nonceInt << 8) + scratch[i];
    }

    std::ostringstream oss;
    oss << nonceInt;
    std::string nonceStr = oss.str();

    return nonceStr;
}

/*this is a function that will take an arbitrary message and break
  it into appropiately formatted and sized packets (based on PACKETSIZE)
  inputs are the recived nonce, and the message
  output is a list of strings where each string is an (encrypted?) package
  with the IV (for AES) tacked onto the front
  format before encryption should be something along the lines of:
  <nonce> <# of packets> <new nonce> <message> <padding>:
*/
std::vector<std::string> createPackets(std::string nonce, std::string &message) {
    std::vector<std::string> packets;
    return packets;
}

std::string pad(std::string &packet) {

	std::cout << "Before pad size: "<< packet << " is " << (int)packet.size() << " characters" << std::endl;
    if (packet.size() < 1006) {
        packet += "~";
    }
    while (packet.size() < 1006) {
        packet += "a";
    }
    packet += "\0";

	std::cout << "After pad size: "<< packet << " is " << (int)packet.size() << " characters" << std::endl;

	return packet;
}

std::string unPad(std::string &packet){

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