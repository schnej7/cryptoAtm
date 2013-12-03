/*
@file modifiedatm.cpp
@brief Top level ATM implementation file
*/
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
using std::string;
#include <vector>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <termios.h>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
using std::cin;

using namespace boost;

std::string getPin(bool show_asterisk);
bool is_number(const std::string& s);
int attempt = 3;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: atm proxy-port\n");
        return -1;
    }

    //socket setup
    unsigned short proxport = atoi(argv[1]);
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!sock) {
        printf("fail to create socket\n");
        return -1;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(proxport);
    unsigned char *ipaddr = reinterpret_cast<unsigned char *>(&addr.sin_addr);
    ipaddr[0] = 127;
    ipaddr[1] = 0;
    ipaddr[2] = 0;
    ipaddr[3] = 1;
    if (0 != connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
        printf("fail to connect to proxy\n");
        return -1;
    }

    

    //input loop
    std::string buf;
    while (1 && attempt != 0) {
        printf("atm> ");
        std::cin >> buf;
        //fgets(buf, 79, stdin);
        //buf[strlen(buf)-1] = '\0'; //trim off trailing newline

        //TODO: your input parsing code has to put data here
        char packet[1024];
        int length = 1;
        std::string username; //10 char username plus null character
        std::string pin; // 4 digit PIN plus null character

        bool isLoggedin = false; // User has not been validated

        std::vector<std::string> command;

        char_separator<char> sep(" ");
        tokenizer<char_separator<char> > tokens(buf, sep);
        BOOST_FOREACH (const std::string & t, tokens) {
            command.push_back(t);
        }

        std::vector<std::string> validCmds;
        validCmds.push_back("login");
        validCmds.push_back("withdraw");
        validCmds.push_back("transfer");
        validCmds.push_back("logout");

        bool in = false;
        for (int i = 0; i < validCmds.size(); i++) {
            if (command[0] == validCmds[i]) {
                in = true;
            }
        }

        if (!in) {
            std::cout << "Not a valid command, please try again" << std::endl;
            continue;
        }


        if ( command[0] == "login" && !isLoggedin ){ //Only available to un-validated user
            cout<<"Please enter your username: ";
            cin>>username;
            cout<<endl;

            pin = getPin(true);
        // Form login packet to send to bank
        } //End login

        if( command[0] == "withdraw" && isLoggedin){
            int withdrawAmount=0;
            bool validAmount = false;

            while(!validAmount){
                cout<<"Please enter amount > $0 and <= $500 to withdraw: ";
                cin>> withdrawAmount;

                if (withdrawAmount > 0 && withdrawAmount <= 500 && isdigit(withdrawAmount))  // Check amount entered is formed of digits, non-negative, and < $500
                    validAmount = true;
                else{
                    attempt--;    //Else decrement attempts left and prompt to retry
                    cout<<"Invalid amount entered. Please try again: "<<endl;
                }//End else
            }//End while

            //Form withdraw packet and send to bank
        } //End withdraw

        if( command[0] == "transfer" && isLoggedin){
            std::string transferToUsername;
            int transferAmount = 0 ;
            bool validAmount = false;

            cout<<"Please enter username you wish to transfer money to: ";
            cin>> transferToUsername;
            cout<<endl;

            cout<<"Please enter amount > 0 and <= $500 to transfer:";
            cin>>transferAmount;

            while(!validAmount){
                cout<<"Please enter amount > 0 and <= $500 to transfer:";
                cin>>transferAmount;

                if (transferAmount > 0 && transferAmount <= 500 && isdigit(transferAmount))  // Check amount entered is formed of digits, non-negative, and < 32767
                    validAmount = true;
                else{
                    attempt--;    //Else decrement attempts left and prompt to retry
                    cout<<"Invalid amount entered. Please try again: "<<endl;
                }//End else
            }//End while

            //Form transfer packet and send to bank

        } // End transfer

        //TODO: other commands

        //send the packet through the proxy to the bank
        if (sizeof(int) != send(sock, &length, sizeof(int), 0)) {
            printf("fail to send packet length\n");
            break;
        }
        if (length != send(sock, (void *)packet, length, 0)) {
            printf("fail to send packet\n");
            break;
        }

        //TODO: do something with response packet
        if (sizeof(int) != recv(sock, &length, sizeof(int), 0)) {
            printf("fail to read packet length\n");
            break;
        }
        if (length >= 1024) {
            printf("packet too long\n");
            break;
        }
        if (length != recv(sock, packet, length, 0)) {
            printf("fail to read packet\n");
            break;
        }
    }

    //cleanup
    if (attempt == 0)
        cout<<"Too many errors.\n";
    cout<<"Goodbye.\n";

    close(sock);
    return 0;
}

//Helper function for getpass() It reads in each character to be masked.
int getch() {
    int ch;
    termios t_old, t_new;

    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
}

//This function prompts for and receives the user-entered PIN (masked with *'s)
std::string getPin(bool show_asterisk=true){
    
    const char BACKSPACE=127;
    const char RETURN=10;

    std::string password;
    unsigned char ch=0;
    bool validPin = false;

    cout << "Please enter 4 digit pin: ";
    while(!validPin){
        while( (ch=getch())!= RETURN){
            if(ch==BACKSPACE){
                if(password.length()!=0){
                    if(show_asterisk)
                        cout <<"\b \b";
                    password.resize(password.length()-1);
                }
            } //end if BACKSPACE
            else{
                password+=ch;
                if(show_asterisk)
                    cout <<'*';
            } //end else
        }// user finished entering Pin
        validPin = is_number(password);
        if(!validPin){
            std::cout<<"\nInvalid PIN entered. Please try again using digits." <<std::endl;
            attempt--;
        }
    }//User has entered a valid Pin attempt

    cout<<endl;
    return password;
}

// This function checks the entered PIN to ensure it is a positive int (0-9)
bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}
