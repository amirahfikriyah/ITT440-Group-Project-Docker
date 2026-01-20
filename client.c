#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SZ 1024

int main() {
    const char *host = getenv("SERVER_HOST");
    const char *port_s = getenv("SERVER_PORT");
    if (!host) host = "server_c";
    int port = port_s ? atoi(port_s) : 5002;

    while (1) {
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(host, NULL, &hints, &res) != 0) {
            fprintf(stderr, "[C CLIENT] resolve failed for %s\n", host);
            sleep(2);
            continue;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
        freeaddrinfo(res);

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) { perror("socket"); sleep(2); continue; }

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("[C CLIENT] connect");
            close(sock);
            sleep(2);
            continue;
        }

        send(sock, "GET", 3, 0);

        char buf[BUF_SZ];
        memset(buf, 0, sizeof(buf));
        int n = recv(sock, buf, sizeof(buf)-1, 0);
        if (n > 0) {
            printf("[C CLIENT] %s", buf);
            fflush(stdout);
        }

        close(sock);
        sleep(10);
    }

    return 0;
}
