#include "acct.h"
#include "util.h"

#include <string>

Acct::Acct( std::string a_name, std::string a_pin, int a_balance, std::string bankSecret ){
    this->pin = compoundSHA1( a_pin, bankSecret );
    this->name = a_name;
    this->balance = a_balance;
}

bool Acct::validatePin(std::string pinHash, std::string bankSecret){
    return this->pin == updateSHA1( pinHash, bankSecret );
}

bool Acct::compareName(std::string a_name, std::string bankSecret){
    return this->name == a_name;
}

int Acct::getBalance(){
    return this->balance;
}

void Acct::setBalance( int a_balance ){
    this->balance = a_balance;
}

int Acct::getBalanceSecure(std::string pinHash, std::string bankSecret){
    return this->balance;
}

void Acct::setBalanceSecure( int a_balance, std::string pinHash, std::string bankSecret ){
    this->balance = a_balance;
}
