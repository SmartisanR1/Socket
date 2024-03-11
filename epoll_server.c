#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define MAX_EVENTS 10
#define MAX_CLIENTS 100
#define PORT 8080

int create_socket() {
    int server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int setup_epoll(int server_fd) {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl");
        close(epoll_fd);
        return -1;
    }

    return epoll_fd;
}

void handle_connections(int epoll_fd, int server_fd) {
    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            perror("epoll_wait");
            break;
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                struct sockaddr_in client_addr;
                socklen_t addrlen = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }

                printf("Accepted new connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
                    perror("epoll_ctl");
                    close(client_fd);
                }
            } else {
                // Handle client events here
                printf("Event on fd %d\n", events[n].data.fd);
                // Read from client_fd, send response, etc.
            }
        }
    }
}

int main() {
    int server_fd = create_socket();
    if (server_fd < 0) {
        return EXIT_FAILURE;
    }

    int epoll_fd = setup_epoll(server_fd);
    if (epoll_fd < 0) {
        close(server_fd);
        return EXIT_FAILURE;
    }

    handle_connections(epoll_fd, server_fd);

    close(epoll_fd);
    close(server_fd);
    return EXIT_SUCCESS;
}