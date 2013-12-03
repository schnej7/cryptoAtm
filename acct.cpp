#include "acct.h"
#include "util.h"

#include <string>
#include <stdio.h>
#include <string.h>
#include <climits>
#include <cstdlib>

Acct::Acct( std::string a_name, std::string a_pin, int a_balance, std::string bankSecret ){
    //Store the pin hash
    this->pin = compoundSHA1( a_pin, bankSecret );

    //Generate a random IV
    this->iv =  generateSecret( 16 );
    this->loggedIn = false;

    //Convert IV to bytes
    byte ivBytes[16];
    strcpy( (char*) ivBytes, this->iv.c_str() );

    //Generate an AES key to encrypt the name and balance
    byte* tempAESKey = generateAESKey( this->pin );

    //Encrypt and store name and balance
    this->name = AESEncrypt( tempAESKey, ivBytes, a_name );
    this->balance = AESEncrypt( tempAESKey, ivBytes, itos(a_balance) );
}

bool Acct::validatePin(std::string pinHash, std::string bankSecret){
    return this->pin == updateSHA1( pinHash, bankSecret );
}

bool Acct::compareName(std::string a_name, std::string bankSecret){
    //Generate an AES key
    byte* tempAESKey = generateAESKey( this->pin );

    //Convert IV to bytes
    byte ivBytes[16];
    strcpy( (char*) ivBytes, this->iv.c_str() );

    return this->name == AESEncrypt( tempAESKey, ivBytes, a_name );
}

int Acct::getBalance(){
    //Generate an AES key
    byte* tempAESKey = generateAESKey( this->pin );

    //Convert IV to bytes
    byte ivBytes[16];
    strcpy( (char*) ivBytes, this->iv.c_str() );

    return std::atoi( AESDecrypt( tempAESKey, ivBytes , this->balance ).c_str() );
}

void Acct::setBalance( int a_balance ){
    if( a_balance < 0 || a_balance > INT_MAX ){
        return;
    }

    //Generate an AES key
    byte* tempAESKey = generateAESKey( this->pin );

    //Convert IV to bytes
    byte ivBytes[16];
    strcpy( (char*) ivBytes, this->iv.c_str() );

    this->balance = AESEncrypt( tempAESKey, ivBytes, itos(a_balance) );
}

int Acct::getBalanceSecure(std::string pinHash, std::string bankSecret){
    if( this->validatePin( pinHash, bankSecret ) ){
        return this->getBalance();
    }
    else{
        return -1;
    }
}

void Acct::setBalanceSecure( int a_balance, std::string pinHash, std::string bankSecret ){
    if( this->validatePin( pinHash, bankSecret ) ){
        this->setBalance( a_balance );
    }
}
