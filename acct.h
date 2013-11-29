#ifndef _acct_h_
#define _acct_h_

#include <string>
#include "util.h"

class Acct{
    private:
        std::string name;
        std::string pin;
        std::string balance;
        byte* iv;
    public:
        Acct( std::string a_name, std::string a_pin, int a_ballance, std::string bankSecret );
        bool validatePin(std::string pinHash, std::string bankSecret );
        bool compareName( std::string a_name,  std::string bankSecret );
        int getBalance(); //For use only on in bank console
        void setBalance( int a_balance ); //For use only on in bank console
        int getBalanceSecure( std::string pinHash, std::string bankSecret ); //For use when interacting with atm
        void setBalanceSecure( int a_balance, std::string pinHash, std::string bankSecret ); //For use when interacting with atm
};

#endif
