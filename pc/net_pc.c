#include "net_pc.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int net_pc_open(uint16_t local_port)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(local_port);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return -1;
    }

    /* non-blocking, ca sa nu ne inghete game loop-ul asteptand pachete */
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    return fd;
}

void net_pc_close(int fd)
{
    if (fd >= 0) close(fd);
}

int net_pc_send(int fd, const char *ip, uint16_t port, const void *data, int len)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    return (int)sendto(fd, data, len, 0, (struct sockaddr *)&addr, sizeof(addr));
}

int net_pc_recv(int fd, void *buf, int buflen)
{
    int n = (int)recvfrom(fd, buf, buflen, 0, NULL, NULL);
    if (n < 0) return 0; /* nimic disponibil (EWOULDBLOCK) sau eroare minora - tratam la fel */
    return n;
}

int net_pc_recv_from(int fd, void *buf, int buflen, char *out_ip, int out_ip_len)
{
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);
    int n = (int)recvfrom(fd, buf, buflen, 0, (struct sockaddr *)&src, &srclen);
    if (n < 0) return 0;
    if (out_ip && out_ip_len > 0)
        inet_ntop(AF_INET, &src.sin_addr, out_ip, out_ip_len);
    return n;
}
