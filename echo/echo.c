#include <fcntl.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SZ 256

void echo_server(int sockfd) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buf[BUF_SZ];

    while (true) {
        int clientfd = accept(sockfd, (struct sockaddr*) &client_addr, &client_len);

        while(true) {
            bzero(buf, BUF_SZ);

            size_t received = recv(clientfd, buf, BUF_SZ - 1, 0); 
            printf("Received buffer of size: %zu\n", received);

            if (received == 0) {
                close(clientfd);
                break;
            }

            size_t to_send = strlen(buf);
            size_t sent = send(clientfd, buf, to_send, 0);
            while (sent != to_send) {
               to_send = strlen(buf) - sent; 
               sent = send(clientfd, buf + sent, to_send, 0); 
            }
        }
    }
}

void print_errno(void) {
    perror("");
}

int init_socket(uint16_t portno, uint32_t address) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portno);
    server_addr.sin_addr.s_addr = htonl(address);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "error on open().\n");
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "error on bind().\n");
        exit(1);
    }

    if(listen(sockfd, 5) == -1) {
        fprintf(stderr, "error on listen().\n");
        exit(1);
    }

    printf("Starting server. Listening at %" PRIu16 "\n", portno);

    return sockfd;
}

int main(int argc, char* argv[argc + 1]) {
    uint16_t portno = 7;
    uint32_t host = INADDR_LOOPBACK;

    if (argc > 1) {
        long usr_port = strtol(*++argv, NULL, 0);
        if (usr_port > UINT16_MAX) {
            fprintf(stderr, 
                    "port %ld is out of bound. Using default value %" PRIu16 "\n", usr_port, portno);
        } else {
            portno = (uint16_t) usr_port;
        }
    }

    atexit(print_errno);

    int sockfd = init_socket(portno, host);
    echo_server(sockfd);

    return EXIT_SUCCESS;
}
