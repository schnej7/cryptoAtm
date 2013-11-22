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
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <regex.h>
#include <crypt.h>
#include <vector>
#include "acct.h"

void* client_thread(void* arg);
void* console_thread(void* arg);

std::vector <Acct> users;

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: bank listen-port\n");
		return -1;
	}

    //Setup bank data first
    users.push_back( Acct("Alice", "salty", 100) );
    //users[1] = {"Bob", "salty", 50 };
    //users[2] = { "Eve", "salty", 0 };
	
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
		pthread_create(&thread, NULL, client_thread, (void*)csock);
	}
}

void* client_thread(void* arg)
{
	int csock = *((int*)(&arg));
	
	printf("[bank] client ID #%d connected\n", csock);
	
	//input loop
	int length;
	char packet[1024];
	while(1)
	{
		//read the packet from the ATM
		if(sizeof(int) != recv(csock, &length, sizeof(int), 0))
			break;
		if(length >= 1024)
		{
			printf("packet too long\n");
			break;
		}
		if(length != recv(csock, packet, length, 0))
		{
			printf("[bank] fail to read packet\n");
			break;
		}
		
		//TODO: process packet data
		
		//TODO: put new data in packet
		
		//send the new packet back to the client
		if(sizeof(int) != send(csock, &length, sizeof(int), 0))
		{
			printf("[bank] fail to send packet length\n");
			break;
		}
		if(length != send(csock, (void*)packet, length, 0))
		{
			printf("[bank] fail to send packet\n");
			break;
		}

	}

	printf("[bank] client ID #%d disconnected\n", csock);

	close(csock);
	return NULL;
}

void* deposit( char* msgbuf )
{
}

void* balance( char* msgbuf )
{
}

void* console_thread(void* arg)
{
	char buf[80];

    // Compile regular expressions
    regex_t depositregex;
    regex_t balanceregex;
    int reti;
    reti = regcomp(&depositregex, "^deposit [a-zA-Z]+ [0-9]+$", REG_EXTENDED);
    if( reti ){ fprintf(stderr, "Could not compile regex\n"); exit(1); }
    reti = regcomp(&balanceregex, "^balance [a-zA-Z]+$", REG_EXTENDED);
    if( reti ){ fprintf(stderr, "Could not compile regex\n"); exit(1); }

	while(1)
	{
		printf("bank> ");
		fgets(buf, 79, stdin);
		buf[strlen(buf)-1] = '\0';	//trim off trailing newline
        char msgbuf[100]; //msg buffer for regex errors
		
		//TODO: your input parsing code has to go here
        reti = regexec(&depositregex, buf, 0, NULL, 0);
        if( !reti ){
            //Deposit command
            printf("Deposit command\n");
            deposit( msgbuf );
            continue;
        }
        else if( reti == REG_NOMATCH ){
            //Not deposit
        }
        else{
                regerror(reti, &depositregex, msgbuf, sizeof(msgbuf));
                fprintf(stderr, "Regex match failed: %s\n", msgbuf);
                exit(1);
        }

        reti = regexec(&balanceregex, buf, 0, NULL, 0);
        if( !reti ){
            //Balance command
            printf("Balance command\n");
            balance( msgbuf );
            continue;
        }
        else if( reti == REG_NOMATCH ){
            //Not balance
        }
        else{
                regerror(reti, &depositregex, msgbuf, sizeof(msgbuf));
                fprintf(stderr, "Regex match failed: %s\n", msgbuf);
                exit(1);
        }
	}
}
