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
#include <string.h>
#include <regex.h>
#include <crypt.h>
#include <vector>
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
	if(!lsock)
	{
		printf("fail to create socket\n");
		return -1;
	}
	
	//listening address
	sockaddr_in addr_l;
	addr_l.sin_family = AF_INET;
	addr_l.sin_port = htons(ourport);
	unsigned char* ipaddr = reinterpret_cast<unsigned char*>(&addr_l.sin_addr);
	ipaddr[0] = 127;
	ipaddr[1] = 0;
	ipaddr[2] = 0;
	ipaddr[3] = 1;
	if(0 != bind(lsock, reinterpret_cast<sockaddr*>(&addr_l), sizeof(addr_l)))
	{
		printf("failed to bind socket\n");
		return -1;
	}
	if(0 != listen(lsock, SOMAXCONN))
	{
		printf("failed to listen on socket\n");
		return -1;
	}
	
	pthread_t cthread;
	pthread_create(&cthread, NULL, console_thread, NULL);
	
	//loop forever accepting new connections
	while(1)
	{
		sockaddr_in unused;
		socklen_t size = sizeof(unused);
		int csock = accept(lsock, reinterpret_cast<sockaddr*>(&unused), &size);
		if(csock < 0)	//bad client, skip it
			continue;
			
		pthread_t thread;
		pthread_create(&thread, NULL, client_thread, (void*)(intptr_t)csock);
	}
}

void *client_thread(void *arg) {

    int length;
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
            break;
        }
    }

    if(results.empty()){
        return NULL;
    }

    std::string bankNonce = makeNonce();
    std::string message = "handshakeResponse";
    length = 1024;

    packet = createPacket(sessionKey, results[0], message, bankNonce);
    sendPacket(csock, length, packet);

    printf("[bank] client ID #%d connected\n", csock);

    //input loop
    while (1) {
        //read the packet from the ATM
        if (sizeof(int) != recv(csock, &length, sizeof(int), 0)) {
            break;
        }
        if (length > 1024) {
            printf("packet too long\n");
            break;
        }
        if (length != recv(csock, charPacket, length, 0)) {
            printf("[bank] fail to read packet\n");
            break;
        }

        //TODO: process packet data

        //TODO: put new data in packet

        //send the new packet back to the client
        if (sizeof(int) != send(csock, &length, sizeof(int), 0)) {
            printf("[bank] fail to send packet length\n");
            break;
        }
        if (length != send(csock, (void *)charPacket, length, 0)) {
            printf("[bank] fail to send packet\n");
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
