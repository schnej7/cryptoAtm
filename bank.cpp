/**
    @file bank.cpp
    @brief Top level bank implementation file
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <cstring>
#include <string>
using std::string;

#include <regex.h>
#include <crypt.h>
#include <vector>
using std::vector;

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "acct.h"
#include "util.cpp"

void *client_thread(void *arg);
void *console_thread(void *arg);

//Used to store the accounts
std::vector <Acct> users;
std::string bankSecret = generateSecret( 40 );

std::vector<byte *> keys;
std::vector<bool> keysInUse;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: bank listen-port\n");
        return -1;
    }

    //Setup bank data first
    users.push_back( Acct("Alice", "1234", 100, bankSecret) );
    users.push_back( Acct("Bob", "6543", 50, bankSecret) );
    users.push_back( Acct("Eve", "1122", 0, bankSecret) );

    unsigned short ourport = atoi(argv[1]);

    //socket setup
    int lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!lsock) {
        printf("fail to create socket\n");
        return -1;
    }

    //listening address
    sockaddr_in addr_l;
    addr_l.sin_family = AF_INET;
    addr_l.sin_port = htons(ourport);
    unsigned char *ipaddr = reinterpret_cast<unsigned char *>(&addr_l.sin_addr);
    ipaddr[0] = 127;
    ipaddr[1] = 0;
    ipaddr[2] = 0;
    ipaddr[3] = 1;
    if (0 != bind(lsock, reinterpret_cast<sockaddr *>(&addr_l), sizeof(addr_l))) {
        printf("failed to bind socket\n");
        return -1;
    }
    if (0 != listen(lsock, SOMAXCONN)) {
        printf("failed to listen on socket\n");
        return -1;
    }

    pthread_t cthread;
    pthread_create(&cthread, NULL, console_thread, NULL);

    //loop forever accepting new connections
    while (1) {
        sockaddr_in unused;
        socklen_t size = sizeof(unused);
        int csock = accept(lsock, reinterpret_cast<sockaddr *>(&unused), &size);
        if (csock < 0)  //bad client, skip it
            continue;

        pthread_t thread;
        pthread_create(&thread, NULL, client_thread, (void *)(intptr_t)csock);
    }
}

void *client_thread(void *arg) {

    int length;
    int atm;
    char *charPacket = new char[1024];
    std::vector<string> input;
    std::string sessionKey;

    int csock = *((int *)(&arg));

    std::string packet;

    recvPacket(csock, length, packet);

    std::vector<std::string> results;
    for (unsigned int i = 0; i < keys.size(); ++i) {
        if (keysInUse[i]) {
            continue;
        }
        std::string encoded;

        CryptoPP::StringSource(keys[i], CryptoPP::AES::DEFAULT_KEYLENGTH, true,
                               new CryptoPP::HexEncoder(
                                   new CryptoPP::StringSink(encoded)
                               ) // HexEncoder
                              ); // StringSource

        results = openPacket(packet, encoded);
        if (!results.empty() && results[1] == "handshake") {
            sessionKey = encoded;
            keysInUse[i] = true;
            atm = i;
            break;
        }
    }

    if (results.empty()) {
        keysInUse[atm] = false;
        return NULL;
    }

    std::string bankNonce = makeNonce();
    std::string message = "handshakeResponse";
    length = 1024;

    packet = createPacket(sessionKey, results[0], message, bankNonce);
    sendPacket(csock, length, packet);

    printf("[bank] client ID #%d connected\n", csock);

    //input loop
    string pinHash;
    int current; //current user number
    while (1) {
        string message = "";
        //read the packet from the ATM
        if (!recvPacket(csock, length, packet)) {
            keysInUse[atm] = false;
            break;
        }
        vector<string> results = openPacket(packet, sessionKey);

        if (results.empty()) {
            keysInUse[atm] = false;
            break;
        }

        /*for(int i = 0; i < results.size(); i++){
            cout << results[i] << endl;
            }*/

        if (!(results[2] == bankNonce)) {
            cout << "ATM " << atm << " SECURITY COMPROMISED!" << endl;
            keysInUse[atm] = false;
            break;
        }

        vector<string> command = parseCommand(results[1]);

        if (command[0] == "login") {
            //cout << "inside login" << endl;
            string username = command[1];
            pinHash = command[2];
            bool validUser = false;
            for ( int i = 0; i < users.size(); i++ ) {
                if ( users[i].compareName(username, bankSecret) ) {
                    current = i;
                    validUser = true;
                }
            }
            if (validUser) {
                if (users[current].validatePin(pinHash, bankSecret)) {
                    if (!users[current].loggedIn) {
                        message = "success";
                        users[current].loggedIn = true;
                    } else {
                        message = "failure";
                    }

                } else { 
                    message = "failure";
                }
            } else { 
                message = "failure";
            }
        } else if (command[0] == "withdraw") {
            int amount = stoi(command[1]);
            int current_balance = users[current].getBalanceSecure(pinHash, bankSecret);
            if (current_balance - amount < 0) {
                message = "overdraft";
            } else if (current_balance - amount >= 0 && current_balance - amount < 10) {
                users[current].setBalanceSecure(current_balance - amount, pinHash, bankSecret);
                message = "low";
            } else {
                users[current].setBalanceSecure(current_balance - amount, pinHash, bankSecret);
                message = "success";
            }
        } else if (command[0] == "balance") {
            message = std::to_string(users[current].getBalanceSecure(pinHash, bankSecret));
        } else if (command[0] == "transfer") {
            int amount = stoi(command[1]);
            int userBalance = users[current].getBalanceSecure(pinHash, bankSecret);
            string otherUser = command[2];
            int other;

            bool validUser = false;
            for ( int i = 0; i < users.size(); i++ ) {
                if ( users[i].compareName(otherUser, bankSecret) ) {
                    other = i;
                    validUser = true;
                }
            }
            if (validUser) {
                int otherBalance = users[other].getBalance();
                if (userBalance - amount < 0) {
                    message = "overdraft";
                } else if (userBalance - amount >= 0 && userBalance - amount < 10) {
                    users[current].setBalanceSecure(userBalance - amount, pinHash, bankSecret);
                    users[other].setBalanceSecure(otherBalance + amount, pinHash, bankSecret);
                    message = "low";
                } else if (otherBalance + amount > 1000000000) {
                    message = "overflow";
                } else {
                    users[current].setBalanceSecure(userBalance - amount, pinHash, bankSecret);
                    users[other].setBalance(otherBalance + amount);
                    message = "success";
                }
            } else {
                message = "failure";
            }

        } else if (command[0] == "logout") {
            keysInUse[atm] = false;
            users[current].loggedIn = false;
            break;
        }

        //TODO: process packet data

        //TODO: put new data in packet

        //send the new packet back to the client
        bankNonce = makeNonce();
        packet = createPacket(sessionKey, results[0], message, bankNonce);

        if (!sendPacket(csock, length, packet)) {
            keysInUse[atm] = false;
            break;
        }

    }

    printf("[bank] client ID #%d disconnected\n", csock);

    close(csock);
    return NULL;
}

void *deposit( char *msgbuf ) {
    std::string messageString( msgbuf );
    int begin = strlen( "deposit " );
    int length = messageString.length();
    int spaceLoc = 0;
    for ( int i = begin; i < messageString.length(); i++ ) {
        if ( messageString[i] == ' ' ) {
            spaceLoc = i;
            break;
        }
    }
    std::string user = messageString.substr( begin, spaceLoc - begin );
    std::string amount = messageString.substr( spaceLoc + 1, length - spaceLoc );
    for ( int i = 0; i < users.size(); i++ ) {
        if ( users[i].compareName(user, bankSecret) && users[i].getBalance() > 0 && atoi(amount.c_str()) > 0 ) {
            users[i].setBalance( users[i].getBalance() + atoi(amount.c_str()) );
        }
    }
}

void *balance( char *msgbuf ) {
    std::string messageString( msgbuf );
    int begin = strlen( "balance " );
    int length = messageString.length() - begin;
    std::string user = messageString.substr( begin, length );
    for ( int i = 0; i < users.size(); i++ ) {
        if ( users[i].compareName(user, bankSecret) ) {
            printf("%d\n", users[i].getBalance());
        }
    }
}

void *console_thread(void *arg) {

    //Generate our keys
    for (unsigned int i = 1; i <= 10; ++i) {
        byte *key = new byte[CryptoPP::AES::DEFAULT_KEYLENGTH];
        generateRandomKey(std::to_string((int)i), key, CryptoPP::AES::DEFAULT_KEYLENGTH);
        keys.push_back(key);
        keysInUse.push_back(false);
    }

    char buf[80];

    // Compile regular expressions
    regex_t depositregex;
    regex_t balanceregex;
    int reti;
    reti = regcomp(&depositregex, "^deposit [a-zA-Z]+ [0-9]+$", REG_EXTENDED);
    if ( reti ) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    reti = regcomp(&balanceregex, "^balance [a-zA-Z]+$", REG_EXTENDED);
    if ( reti ) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }

    while (1) {
        printf("bank> ");
        fgets(buf, 79, stdin);
        buf[strlen(buf) - 1] = '\0'; //trim off trailing newline
        char msgbuf[100]; //msg buffer for regex errors

        reti = regexec(&depositregex, buf, 0, NULL, 0);
        if ( !reti ) {
            //Deposit command
            deposit( buf );
            continue;
        } else if ( reti == REG_NOMATCH ) {
            //Not deposit
        } else {
            regerror(reti, &depositregex, msgbuf, sizeof(msgbuf));
            fprintf(stderr, "Regex match failed: %s\n", msgbuf);
            exit(1);
        }

        reti = regexec(&balanceregex, buf, 0, NULL, 0);
        if ( !reti ) {
            //Balance command
            balance( buf );
            continue;
        } else if ( reti == REG_NOMATCH ) {
            //Not balance
        } else {
            regerror(reti, &depositregex, msgbuf, sizeof(msgbuf));
            fprintf(stderr, "Regex match failed: %s\n", msgbuf);
            exit(1);
        }
    }
}
