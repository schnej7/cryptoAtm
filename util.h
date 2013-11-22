#ifndef __util_h__
#define __util_h__

#include <string>

std::string makeNonce();
std::string pad(std::string &packet);
std::string unPad(std::string &packet);

#endif
