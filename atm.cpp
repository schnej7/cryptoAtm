/**
    @file atm.cpp
    @brief Top level ATM implementation file
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

using namespace boost;

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
    while (1) {
        printf("atm> ");
        cin >> buf;
        //fgets(buf, 79, stdin);
        //buf[strlen(buf)-1] = '\0';    //trim off trailing newline

        //TODO: your input parsing code has to put data here
        char packet[1024];
        int length = 1;

        std::vector<std::string> command;

        char_separator<char> sep(" ");
        tokenizer<char_separator<char> > tokens(buf, sep);
        BOOST_FOREACH (const std::string & t, tokens) {
            command.push_back(t);
        }

        std::vector<std::string> validCmds;
        validCmds.push_back("login");
        validCmds.push_back("withdraw");
        validCmds.push_back("deposit");
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
    close(sock);
    return 0;
}
