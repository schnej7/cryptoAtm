#ifndef _acct_h_
#define _acct_h_

#include <string>

class Acct{
    private:
        std::string name;
        std::string pin;
        int balance;
    public:
        Acct( std::string a_name, std::string a_pin, int a_ballance );
        std::string getName();
        std::string getPin();
        int getBalance();
        void setBalance( int a_balance );
};

#endif
