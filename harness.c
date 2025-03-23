#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, char *argv[]) {
    static int sock = -1;
    if (argc < 2) return 1;

    if (sock == -1) {  // Initialize socket once
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return 1;
        struct sockaddr_in server = {0};
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(631);
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
            close(sock);
            return 1;
        }
    }

#ifdef __AFL_HAVE_MANUAL_CONTROL
printf("[DEBUG] Starting harness\n");
fflush(stdout);

    __AFL_INIT();  // Initialize AFL persistent mode
    while (__AFL_LOOP(10000)) {  // Loop up to 10,000 times per run
#endif

        int fd = open(argv[1], O_RDONLY);
        if (fd < 0) return 1;
        char buffer[4096];
        ssize_t len = read(fd, buffer, sizeof(buffer));
        close(fd);
        if (len <= 0) return 1;

        char request[4096 + 256];
        int req_len = snprintf(request, sizeof(request),
                               "POST / HTTP/1.1\r\n"
                               "Host: localhost\r\n"
                               "Content-Length: %zd\r\n"
                               "Content-Type: application/ipp\r\n"
                               "\r\n", len);
        if (req_len < 0 || req_len >= sizeof(request)) return 1;

        if (write(sock, request, req_len) != req_len) return 1;
        if (write(sock, buffer, len) != len) return 1;

#ifdef __AFL_HAVE_MANUAL_CONTROL
    }  // End of AFL loop
#endif

    close(sock);
    return 0;
}