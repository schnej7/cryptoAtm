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
#include <fstream>
#include "util.h"
#include "includes/cryptopp/filters.h"
using CryptoPP::StringSource;
using CryptoPP::StringSink;
#include "includes/cryptopp/hex.h"
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

using namespace boost;
using std::cin;
using std::cerr;
using std::cout;
using std::endl;

std::string getPin(bool show_asterisk);
bool is_number(const std::string &s);
bool handshake(int csock, std::string atmNumber);
int attempt = 3;
std::string sessionKey = "";
std::string atmNonce;
std::string message;
std::string bankNonce = "";
std::string packet;


int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: atm proxy-port atm-number(1-50)\n");
        return -1;
    }

    //socket setup
    unsigned short proxport = atoi(argv[2]);
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

    if (!handshake(sock, argv[1])) {
        cerr << "SECURITY COMPROMISED!" << endl;
        exit(1);
    }

    //input loop
    std::string buf;
    while (1 && attempt != 0) {
        printf("atm> ");
        std::cin >> buf;
        //fgets(buf, 79, stdin);
        //buf[strlen(buf)-1] = '\0'; //trim off trailing newline

        int length = 1024;
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

        if ( command[0] == "login" && !isLoggedin ) { //Only available to un-validated user
            cout << "Please enter your username: ";
            cin >> username;
            while (username.size() > 10) {
                cout << "Usernames are 10 characters or less!" << endl;
                cout << "Please enter your username: ";
                cin >> username;
            }

            pin = getPin(true);
            pin = SHA1(pin);
            atmNonce = makeNonce();
            message = "login " + username + " " + pin;
            packet = createPacket(sessionKey, atmNonce, message, bankNonce);

            sendPacket(sock, length, packet);
            recvPacket(sock, length, packet);

            std::vector<std::string> results = openPacket(packet, sessionKey);

            if (results[0] != atmNonce) {
                return false;
            } else if (results[1] != "failure") {
                cout << "\nInvalid Username or Pin!" << endl;
                attempt --;
            } else if ( results[1] == "success") {
                cout << "Successfully logged in!" << endl;
                isLoggedin = true;
            }

        } else {
            cout << "You are already logged in" << endl;
        }
        //End login

        if ( command[0] == "withdraw" && isLoggedin) {
            int withdrawAmount = 0;
            bool validAmount = false;

            while (!validAmount) {
                cout << "Please enter amount > $0 and <= $500 to withdraw: ";
                cin >> withdrawAmount;

                if (withdrawAmount > 0 && withdrawAmount <= 500 && isdigit(withdrawAmount))  // Check amount entered is formed of digits, non-negative, and < $500
                    validAmount = true;
                else {
                    attempt--;    //Else decrement attempts left and prompt to retry
                    cout << "Invalid amount entered. Please try again: " << endl;
                }//End else
            }//End while

            //Form withdraw packet and send to bank
        } //End withdraw

        if ( command[0] == "transfer" && isLoggedin) {
            std::string transferToUsername;
            int transferAmount = 0 ;
            bool validAmount = false;

            cout << "Please enter username you wish to transfer money to: ";
            cin >> transferToUsername;
            cout << endl;

            cout << "Please enter amount > 0 and <= $500 to transfer:";
            cin >> transferAmount;

            while (!validAmount) {
                cout << "Please enter amount > 0 and <= $500 to transfer:";
                cin >> transferAmount;

                if (transferAmount > 0 && transferAmount <= 500 && isdigit(transferAmount))  // Check amount entered is formed of digits, non-negative, and < 32767
                    validAmount = true;
                else {
                    attempt--;    //Else decrement attempts left and prompt to retry
                    cout << "Invalid amount entered. Please try again: " << endl;
                }//End else
            }//End while

            //Form transfer packet and send to bank

        } // End transfer
    }

    //cleanup
    if (attempt == 0)
        cout << "Too many errors.\n";
    cout << "Goodbye.\n";

    close(sock);
    return 0;
}

bool handshake(int csock, std::string atmNumber) {

    bankNonce = "";
    atmNonce = makeNonce();
    message = "handshake";
    int length = 1024;

    //construct filename from command-line argument
    std::string filename = std::string(atmNumber) + ".key";

    //read in key from file
    std::ifstream input_file;
    input_file.open(filename.c_str());
    if (input_file.is_open()) {
        input_file >> sessionKey;
    } else {
        return false;
    }
    input_file.close();

    if (atmNonce == "") {
        atmNonce = "";
        return false;
    }

    //std::vector<string> openPacket(string & packet, string & key);

    if (sessionKey == "") {
        return false;
    }
    packet = createPacket(sessionKey, atmNonce, message, bankNonce);


    //send the packet through the proxy to the bank
    if (!sendPacket(csock, length, packet)) {
        return false;
    }

    if (!recvPacket(csock, length, packet)) {
        return false;
    }


    std::vector<std::string> results = openPacket(packet, sessionKey);
    //cout << results[1] << endl;
    if (results[0] != atmNonce) {
        return false;
    } else if (results[1] != "handshakeResponse") {
        return false;
    }

    return true;
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
std::string getPin(bool show_asterisk = true) {

    const char BACKSPACE = 127;
    const char RETURN = 10;

    std::string password;
    unsigned char ch = 0;
    bool validPin = false;

    cout << "Please enter 4 digit pin: ";
    while ( (ch = getch()) != RETURN) {
        if (ch == BACKSPACE) {
            if (password.length() != 0) {
                if (show_asterisk)
                    cout << "\b \b";
                password.resize(password.length() - 1);
            }
        } //end if BACKSPACE
        else {
            password += ch;
            if (show_asterisk)
                cout << '*';
        } //end else
    }// user finished entering Pin
    validPin = is_number(password);

    while (!validPin) {
        password = "";
        while ( (ch = getch()) != RETURN) {
            if (ch == BACKSPACE) {
                if (password.length() != 0) {
                    if (show_asterisk)
                        cout << "\b \b";
                    password.resize(password.length() - 1);
                }
            } //end if BACKSPACE
            else {
                password += ch;
                if (show_asterisk)
                    cout << '*';
            } //end else
        }// user finished entering Pin
        validPin = is_number(password);
        if (!validPin) {
            std::cout << "\nInvalid PIN entered. Please try again using digits." << std::endl;
            cout << "Please enter 4 digit pin: ";
        }
    }//User has entered a valid Pin attempt

    return password;
}

// This function checks the entered PIN to ensure it is a positive int (0-9)
bool is_number(const std::string &s) {
    if (s.size() != 4) {
        return false;
    }
    return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) {
        return !std::isdigit(c);
    }) == s.end();
}