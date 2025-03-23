#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#define MAX_BUFFER 4096

int main(int argc, char *argv[]) {
    printf("[DEBUG] Starting fuzz_cupsd (Persistent Mode)...\n");
    fflush(stdout);

    // AFL++ Persistent Mode
    __AFL_INIT();

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[ERROR] socket failed");
        return 1;
    }

    struct sockaddr_in server = {0};
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    server.sin_port = htons(631);

    printf("[DEBUG] Connecting to CUPS server...\n");
    fflush(stdout);

    struct timeval timeout;
    timeout.tv_sec = 1;  // 1 second timeout
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("[ERROR] connect failed");
        close(sock);
        return 1;
    }

    printf("[DEBUG] Connected to CUPS server.\n");
    fflush(stdout);

    char buffer[MAX_BUFFER];

    while (__AFL_LOOP(1000)) { // AFL++ Persistent Loop
        printf("[DEBUG] Starting iteration...\n");
        fflush(stdout);

        ssize_t len = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (len <= 0) {
            printf("[WARNING] Empty input or read error.\n");
            continue;
        }

        printf("[DEBUG] Read %zd bytes from input\n", len);
        fflush(stdout);

        char request[MAX_BUFFER + 256];
        snprintf(request, sizeof(request),
                 "POST / HTTP/1.1\r\n"
                 "Host: localhost\r\n"
                 "Content-Length: %zd\r\n"
                 "Content-Type: application/ipp\r\n"
                 "\r\n", len);

        if (write(sock, request, strlen(request)) != (ssize_t)strlen(request)) {
            perror("[ERROR] write failed (request)");
            continue;
        }

        if (write(sock, buffer, len) != len) {
            perror("[ERROR] write failed (body)");
            continue;
        }

        char response[1024];
        ssize_t res_len = read(sock, response, sizeof(response));
        if (res_len > 0) {
            printf("[DEBUG] Received response from CUPS (%zd bytes)\n", res_len);
        } else {
            printf("[WARNING] No response from CUPS or timeout occurred.\n");
        }
    }

    printf("[DEBUG] Closing socket.\n");
    close(sock);
    return 0;
}
