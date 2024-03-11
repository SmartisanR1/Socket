#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define MAXBUF 256

void child_process(void)
{
    sleep(2);
    char msg[MAXBUF];
    struct sockaddr_in addr = {0};
    int n, sockfd, num = 1;
    srandom(getpid());
    /* Create socket and connect to server */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    printf("child {%d} connected \n", getpid());
    while (1)
    {
        int sl = (random() % 10) + 1;
        num++;
        sleep(sl);
        sprintf(msg, "Test message %d from client %d", num, getpid());
        n = write(sockfd, msg, strlen(msg)); /* Send message */
    }
}

int main()
{
    char buffer[MAXBUF];
    int fds[5];
    struct sockaddr_in addr;
    struct sockaddr_in client;
    int addrlen, n, i, max = 0;
    int sockfd, commfd;
    fd_set rset;
    for (i = 0; i < 5; i++)
    {
        if (fork() == 0)
        {
            child_process();
            exit(0);
        }
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置SO_REUSEADDR选项允许重用本地地址和端口
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("Error setting SO_REUSEADDR option");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2000);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    listen(sockfd, 5);

    for (i = 0; i < 5; i++)
    {
        memset(&client, 0, sizeof(client));
        addrlen = sizeof(client);
        fds[i] = accept(sockfd, (struct sockaddr *)&client, &addrlen);
        if (fds[i] > max)
            max = fds[i];
    }

    while (1)
    {
        FD_ZERO(&rset);
        for (i = 0; i < 5; i++)
        {
            FD_SET(fds[i], &rset);
        }

        puts("round again");
        select(max + 1, &rset, NULL, NULL, NULL);

        for (i = 0; i < 5; i++)
        {
            if (FD_ISSET(fds[i], &rset))
            {
                memset(buffer, 0, MAXBUF);
                read(fds[i], buffer, MAXBUF);
                puts(buffer);
            }
        }
    }
    return 0;
}
