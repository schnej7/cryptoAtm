#include "util.h"
#include <string>
#include <iostream>

int main() {

	std::string packet = "nonce$message$nonce";
	pad(packet);
	unPad(packet);
	std::cout << "unpadded: " << packet << std::endl;
	return 0;
}
