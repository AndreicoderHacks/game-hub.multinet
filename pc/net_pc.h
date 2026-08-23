#ifndef NET_PC_H
#define NET_PC_H

#include <stdint.h>

/* Wrapper minimal peste UDP, non-blocking.
 * Suficient pentru host-ul autoritativ / clientul de Pong.
 */

int  net_pc_open(uint16_t local_port);              /* creeaza socket, bind local; returneaza fd sau -1 */
void net_pc_close(int fd);

/* trimite catre ip:port (ip ca string, ex "192.168.1.42") */
int  net_pc_send(int fd, const char *ip, uint16_t port, const void *data, int len);

/* citeste un pachet daca exista (non-blocking). returneaza nr de bytes, 0 daca nu-i nimic, -1 la eroare */
int  net_pc_recv(int fd, void *buf, int buflen);

/* ca net_pc_recv, dar completeaza si IP-ul expeditorului in out_ip (trebuie sa aiba minim 16 bytes) */
int  net_pc_recv_from(int fd, void *buf, int buflen, char *out_ip, int out_ip_len);

#endif /* NET_PC_H */
