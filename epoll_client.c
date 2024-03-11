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

int create_client_socket(const char *server_ip, int port) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(client_fd);
        return -1;
    }

    return client_fd;
}

int main() {
    const char *server_ip = "127.0.0.1"; // 服务器IP地址
    int port = 8080; // 服务器端口

    int client_fd = create_client_socket(server_ip, port);
    if (client_fd < 0) {
        return EXIT_FAILURE;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        close(client_fd);
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        perror("epoll_ctl");
        close(epoll_fd);
        close(client_fd);
        return -1;
    }

    // 发送消息到服务器
    const char *message = "Hello, Server!";
    send(client_fd, message, strlen(message), 0);

    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            perror("epoll_wait");
            break;
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == client_fd) {
                // 接收服务器的响应
                char buffer[1024];
                ssize_t received = recv(client_fd, buffer, sizeof(buffer), 0);
                if (received < 0) {
                    perror("recv");
                    close(client_fd);
                    break;
                } else if (received == 0) {
                    // 连接已关闭
                    printf("Connection closed by server.\n");
                    close(client_fd);
                    break;
                } else {
                    buffer[received] = '\0';
                    printf("Received from server: %s\n", buffer);
                }
            }
        }
    }

    close(epoll_fd);
    return EXIT_SUCCESS;
}