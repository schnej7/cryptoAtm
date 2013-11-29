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
    byte ivBytes[16];
    strcpy( (char*) ivBytes, generateSecret( 16 ).c_str() );
    this->iv = ivBytes;

    //Convert the pin hash to bytes
    byte pinBytes[160];
    strcpy( (char*) pinBytes, this->pin.c_str() );

    //Generate an AES key to encrypt the name and balance
    byte* tempAESKey = (byte*) generateAESKey( pinBytes, this->iv );

    //Encrypt and store name and balance
    this->name = AESEncrypt( tempAESKey, this->iv, a_name );
    this->balance = AESEncrypt( tempAESKey, this->iv, itos(a_balance) );
}

bool Acct::validatePin(std::string pinHash, std::string bankSecret){
    return this->pin == updateSHA1( pinHash, bankSecret );
}

bool Acct::compareName(std::string a_name, std::string bankSecret){
    //Convert the pin hash to bytes
    byte pinBytes[160];
    strcpy( (char*) pinBytes, this->pin.c_str() );

    //Generate an AES key
    byte* tempAESKey = (byte*) generateAESKey( pinBytes, this->iv );

    printf("%s, %s\n", this->name.c_str(), AESEncrypt( tempAESKey, this->iv, a_name ).c_str() );

    return this->name == AESEncrypt( tempAESKey, this->iv, a_name );
}

int Acct::getBalance(){
    //Convert the pin hash to bytes
    byte pinBytes[160];
    strcpy( (char*) pinBytes, this->pin.c_str() );

    //Generate an AES key
    byte* tempAESKey = (byte*) generateAESKey( pinBytes, this->iv );

    return std::atoi( AESDecrypt( tempAESKey, this->iv, this->balance ).c_str() );
}

void Acct::setBalance( int a_balance ){
    //Convert the pin hash to bytes
    byte pinBytes[160];
    strcpy( (char*) pinBytes, this->pin.c_str() );

    //Generate an AES key
    byte* tempAESKey = (byte*) generateAESKey( pinBytes, this->iv );

    this->balance = AESEncrypt( tempAESKey, this->iv, itos(a_balance) );
}

int Acct::getBalanceSecure(std::string pinHash, std::string bankSecret){
    //Convert the pin hash to bytes
    byte pinBytes[160];
    strcpy( (char*) pinBytes, updateSHA1( pinHash, bankSecret ).c_str() );

    //Generate an AES key
    byte* tempAESKey = (byte*) generateAESKey( pinBytes, this->iv );

    return atoi( AESDecrypt( tempAESKey, this->iv, this->balance ).c_str() );
}

void Acct::setBalanceSecure( int a_balance, std::string pinHash, std::string bankSecret ){
    //Convert the pin hash to bytes
    byte pinBytes[160];
    strcpy( (char*) pinBytes, this->pin.c_str() );

    //Generate an AES key
    byte* tempAESKey = (byte*) generateAESKey( pinBytes, this->iv );

    this->balance = AESEncrypt( tempAESKey, this->iv, itos(a_balance) );
}
