#include "acct.h"
#include "util.h"

#include <string>
#include <stdio.h>
#include <string.h>
#include <cstdlib>

Acct::Acct( std::string a_name, std::string a_pin, int a_balance, std::string bankSecret ){
    //Store the pin hash
    this->pin = compoundSHA1( a_pin, bankSecret );

    //Generate a random IV
    this->iv =  generateSecret( 16 );

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
    //Generate an AES key
    byte* tempAESKey = generateAESKey( this->pin );

    //Convert IV to bytes
    byte ivBytes[16];
    strcpy( (char*) ivBytes, this->iv.c_str() );

    this->balance = AESEncrypt( tempAESKey, ivBytes, itos(a_balance) );
}

int Acct::getBalanceSecure(std::string pinHash, std::string bankSecret){
    //Generate an AES key
    byte* tempAESKey = generateAESKey( updateSHA1( pinHash, bankSecret ) );

    //Convert IV to bytes
    byte ivBytes[16];
    strcpy( (char*) ivBytes, this->iv.c_str() );

    if( this->validatePin( pinHash, bankSecret ) ){
        return atoi( AESDecrypt( tempAESKey, ivBytes, this->balance ).c_str() );
    }
    else{
        return -1;
    }
}

void Acct::setBalanceSecure( int a_balance, std::string pinHash, std::string bankSecret ){
    //Generate an AES key
    byte* tempAESKey = generateAESKey( updateSHA1( pinHash, bankSecret ) );

    //Convert IV to bytes
    byte ivBytes[16];
    strcpy( (char*) ivBytes, this->iv.c_str() );

    if( this->validatePin( pinHash, bankSecret ) ){
        this->balance = AESEncrypt( tempAESKey, ivBytes, itos(a_balance) );
    }
}
