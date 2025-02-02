#include<bits/stdc++.h>
#include<pcap.h>
#include<netinet/ether.h>
#include<netinet/ip.h>
#include<netinet/udp.h>
#include<netinet/tcp.h>
#include<netinet/if_ether.h>
using namespace std;
char errbuff[PCAP_ERRBUF_SIZE];
char *device;
pcap_t *handler;
bpf_u_int32 netp;
bpf_u_int32 maskp;

void findAllDevs(){
    pcap_if_t *alldevsp;
    if(pcap_findalldevs(&alldevsp,errbuff)<0){
        cout<<"findalldevs err : "<<errbuff<<endl;
        exit(0);
    }
    pcap_if_t *dev=alldevsp;
    while (dev != NULL) {
        cout << "Interface name: " << dev->name << endl;
        if(dev->description) cout << "Interface desc: " << dev->description << endl;
        pcap_addr_t *dev_addr; //interface address that used by pcap_findalldevs()

        /* check if the device capturable*/
        for (dev_addr = dev->addresses; dev_addr != NULL; dev_addr = dev_addr->next) {
            if (dev_addr->addr->sa_family == AF_INET && dev_addr->addr && dev_addr->netmask) {
                const string interface_ip = inet_ntoa(reinterpret_cast<struct sockaddr_in*>(dev_addr->addr)->sin_addr);
                const string interface_netmask = inet_ntoa(reinterpret_cast<struct sockaddr_in*>(dev_addr->netmask)->sin_addr);
                cout<<"ip : "<<interface_ip<<endl;
                cout<<"netmask : "<<interface_netmask<<endl;
            }
        }
        cout<<"----------------------\n"<<endl;
        dev=dev->next;
    }
    // Free the list when done
    pcap_freealldevs(alldevsp);
}

void getDevInfo(){
    if(pcap_lookupnet(device,&netp,&maskp,errbuff)<0){
        cout<<"lookupnet err : "<<errbuff<<endl;
        exit(0);
    }
    // printing ip & subnetmask in human readable format
    char ip[13];
    char subnet_mask[13];
    struct in_addr addr;

    addr.s_addr=netp;
    strcpy(ip,inet_ntoa(addr));
    if (ip == NULL) {
        perror("inet_ntoa");
        exit(0);
    }

    addr.s_addr = maskp;
    strcpy(subnet_mask, inet_ntoa(addr));
    if (subnet_mask == NULL) {
        perror("inet_ntoa");
        exit(0);
    }

    cout<<"ip : "<<ip<<endl;
    cout<<"mask : "<<subnet_mask<<endl;
}

void printTCPHdr(char *packet,int sz){
    // ethhdr len is not needed to consider as packets are from layer-3
    unsigned short iphdrlen;
	
	struct iphdr *iph = (struct iphdr *)( packet + sizeof(struct ethhdr) );
	iphdrlen = iph->ihl*4;
	
	struct tcphdr *tcph=(struct tcphdr*)(packet + sizeof(struct ethhdr) + iphdrlen );
			
	int header_size =  iphdrlen + tcph->doff*4;

	printf("TCP Header\n");
	printf("   |-Source Port      : %u\n",ntohs(tcph->source));
	printf("   |-Destination Port : %u\n",ntohs(tcph->dest));
	printf("   |-Sequence Number    : %u\n",ntohl(tcph->seq));
	printf("   |-Acknowledge Number : %u\n",ntohl(tcph->ack_seq));
 	printf("   |-Header Length      : %d DWORDS or %d BYTES\n" ,(unsigned int)tcph->doff,(unsigned int)tcph->doff*4);
	printf("   |-Urgent Flag          : %d\n",(unsigned int)tcph->urg);
	printf("   |-Acknowledgement Flag : %d\n",(unsigned int)tcph->ack);
	printf("   |-Push Flag            : %d\n",(unsigned int)tcph->psh);
	printf("   |-Reset Flag           : %d\n",(unsigned int)tcph->rst);
	printf("   |-Synchronise Flag     : %d\n",(unsigned int)tcph->syn);
	printf("   |-Finish Flag          : %d\n",(unsigned int)tcph->fin);
	printf("   |-Window         : %d\n",ntohs(tcph->window));
	printf("   |-Checksum       : %d\n",ntohs(tcph->check));
	printf("   |-Urgent Pointer : %d\n",tcph->urg_ptr);

    // **correct way of printing tcp-data**
    if(sz>(iph->ihl * 4 + tcph->doff*4)){
        char *payload = packet + sizeof(struct ethhdr) + iph->ihl * 4 + tcph->doff*4;
        cout << "Data : " << payload << endl;
    }
}

void printIPHdr(char *packet,int sz){
    
    struct iphdr *ipHeader=(struct iphdr*)(packet+sizeof(struct ethhdr));
    struct sockaddr_in source,dest;
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = ipHeader->saddr;
    
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = ipHeader->daddr;

    printf("IP Header\n");
    printf("   |-IP Version        : %d\n",(unsigned int)ipHeader->version);
    printf("   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)ipHeader->ihl,((unsigned int)(ipHeader->ihl))*4);
    printf("   |-Type Of Service   : %d\n",(unsigned int)ipHeader->tos);
    printf("   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(ipHeader->tot_len));
    printf("   |-Identification    : %d\n",ntohs(ipHeader->id));
    printf("   |-TTL      : %d\n",(unsigned int)ipHeader->ttl);
    printf("   |-Protocol : %d\n",(unsigned int)ipHeader->protocol);
    printf("   |-Checksum : %d\n",ntohs(ipHeader->check));
    printf("   |-Source IP        : %s\n" , inet_ntoa(source.sin_addr) );
    printf("   |-Destination IP   : %s\n" , inet_ntoa(dest.sin_addr) );

    printTCPHdr(packet,sz);

}

void printEthHdr(char *packet,int sz){
    if(sz>sizeof(struct ethhdr)){
        struct ethhdr *ethHdr=(struct ethhdr*)packet;
        
        cout<<"\nGot a Packet :"<<endl;
        printf("\nEthernet Header\n");
        printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",ethHdr->h_source[0],ethHdr->h_source[1],ethHdr->h_source[2],ethHdr->h_source[3],ethHdr->h_source[4],ethHdr->h_source[5]);
        printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",ethHdr->h_dest[0],ethHdr->h_dest[1],ethHdr->h_dest[2],ethHdr->h_dest[3],ethHdr->h_dest[4],ethHdr->h_dest[5]);
        printf("\t|-Protocol : %d\n",ntohs(ethHdr->h_proto));
        printIPHdr(packet,sz);
    }
}

void decodePacket(u_char *args, const struct pcap_pkthdr *packet_header, const u_char *packet){
    printEthHdr((char*)packet,packet_header->caplen);
}

int main(){
    findAllDevs();
    cout<<"Select a device from above : ";
    string devStr;
    cin>>devStr;
    device = new char[devStr.length() + 1];
    strcpy(device,devStr.c_str());
    getDevInfo();

    // initialise session
    if((handler=pcap_open_live(device,BUFSIZ,0,1000,errbuff))==NULL){
        cout<<"openlive err : "<<errbuff<<endl;
    }

    cout<<"data link type : "<<pcap_datalink(handler)<<endl;

    // filter code :
    char filter[]="src host 10.9.0.6 or dst host 10.9.0.6 and tcp";
    struct bpf_program fp;
    if (pcap_compile(handler, &fp, filter, 0, netp) == -1) {
        printf("Bad filter - %s\n", pcap_geterr(handler));
        exit(0);
    }
    if (pcap_setfilter(handler, &fp) == -1) {
        printf("Error setting filter - %s\n", pcap_geterr(handler));
        exit(0);
    }

    pcap_loop(handler,0,decodePacket,NULL);

    return 0;
}

/*
    // get device
    char *device = pcap_lookupdev(errbuff);
    if (device == NULL) {
        cout<<"lookupdev err : "<<errbuff<<endl;
        exit(0);
    }
    cout<<"device : "<<device<<endl;
*/
