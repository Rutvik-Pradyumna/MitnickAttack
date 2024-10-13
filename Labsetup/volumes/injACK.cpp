#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <cstdlib>
#include <linux/if_ether.h>
using namespace std;
class PacketSender {
private:
    int sfd;
    const char* saddr = "10.9.0.6";
    const char* daddr = "10.9.0.5";
    int sport = 1023;
    int dport = 514;
    long long seq_num = 323398485;
    long long ack_num;

    struct pseudohdr {
        unsigned int saddr;
        unsigned int daddr;
        unsigned char reserved;
        unsigned char protocol;
        unsigned short tcp_len;
    };

public:
    PacketSender(long long ackNum) : ack_num(ackNum) {
        sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (sfd == -1) {
            perror("socket ");
            exit(EXIT_FAILURE);
        }
    }

    ~PacketSender() {
        close(sfd);
    }
    
    unsigned short checksum(unsigned short* buff, int _16bitword) {
        unsigned long sum = 0;
        while (_16bitword--) {
            sum += htons(*(buff)++);
        }
        sum = ((sum >> 16) + (sum & 0xFFFF));
        sum += (sum >> 16);
        return (unsigned short)(~sum);
    }

    unsigned short tcp_checksum(struct iphdr* ip, struct tcphdr* tcp, int tcplen) {
        struct pseudohdr pseudo{};
        pseudo.saddr = ip->saddr;
        pseudo.daddr = ip->daddr;
        pseudo.reserved = 0;
        pseudo.protocol = 6;  // TCP
        pseudo.tcp_len = htons(tcplen);

        int pseudo_size = sizeof(pseudo) + tcplen;
        unsigned short* pseudo_packet = (unsigned short*)malloc(pseudo_size);

        memcpy(pseudo_packet, &pseudo, sizeof(pseudo));
        memcpy(pseudo_packet + (sizeof(pseudo) / 2), tcp, tcplen);

        unsigned short result = checksum(pseudo_packet, pseudo_size / 2);
        free(pseudo_packet);

        return result;
    }

    void sendPacket() {
        char buff[1024];
        memset(buff, 0, sizeof(buff));

        int total_len = 0;

        struct iphdr* ip = (struct iphdr*)(buff);
        ip->version = 4;
        ip->ihl = 5;
        ip->tos = 0;
        ip->id = htons(10201);
        ip->ttl = 64;
        ip->protocol = 6; // TCP
        ip->check = 0;
        ip->saddr = inet_addr(saddr);
        ip->daddr = inet_addr(daddr);

        total_len += sizeof(struct iphdr);

        struct tcphdr* tcp = (struct tcphdr*)(buff + total_len);
        tcp->source = htons(sport);
        tcp->dest = htons(dport);
        tcp->seq = htonl(seq_num);
        tcp->ack_seq = htonl(ack_num);
        tcp->doff = 5; // Header size in 32-bit words
        tcp->ack = 1;
        tcp->window = htons(65535);
        tcp->check = 0;
	cout<<ack_num<<endl;
        total_len += (unsigned int)tcp->doff * 4;

        ip->tot_len = htons(total_len);
        ip->check = htons(checksum((unsigned short*)ip, ip->ihl * 2));
        tcp->check = htons(tcp_checksum(ip, tcp, sizeof(struct tcphdr)));

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("10.9.0.5");

        int sz = sendto(sfd, buff, total_len, 0, (struct sockaddr*)&addr, sizeof(addr));
        if (sz < 0) {
            perror("send ");
        } else if (sz == total_len) {
            std::cout << "Successfully sent SYN packet to X terminal\n";
        }
    }
};

int main() {
    long long ackNum;
    cout << "Enter ACK number: ";
    cin >> ackNum;
    PacketSender sender(ackNum);
    sender.sendPacket();
    return 0;
}

