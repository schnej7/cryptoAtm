#include "acct.h"

#include <string>

Acct::Acct( std::string a_name, std::string a_pin, int a_balance ){
    this->name = a_name;
    this->pin = a_pin;
    this->balance = a_balance;
}

std::string Acct::getName(){
    return this->name;
}

std::string Acct::getPin(){
    return this->pin;
}

int Acct::getBalance(){
    return this->balance;
}

void Acct::setBalance( int a_balance ){
    this->balance = a_balance;
}
