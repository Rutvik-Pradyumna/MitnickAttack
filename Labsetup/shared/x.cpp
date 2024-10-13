#include<bits/stdc++.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<poll.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
using namespace std;

struct sockaddr_in sAddr,cAddr;
int adrlen=sizeof(sAddr);
void createSocket(int &sfd){
    if((sfd=socket(AF_INET, SOCK_STREAM, 0))<0){
        perror("socket err");
        exit(1);
    }
}

void setSockOpt(int &sfd){
    int opt=1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))<0){
        perror("sockopt err");
        exit(1);
    }
}

void bindSocket(int &sfd,int port){
    sAddr.sin_family=AF_INET;
    sAddr.sin_addr.s_addr=inet_addr("10.9.0.5");
    sAddr.sin_port=htons(port);
    if(bind(sfd, (struct sockaddr*)&sAddr, adrlen)<0){
        perror("bind err");
        exit(1);
    }
}

void listenSocket(int &sfd){
    if(listen(sfd,3)<0){
        perror("listen err");
        exit(1);
    }
}

int main(){
    // opening sfd
    int sfd;
    createSocket(sfd);
    setSockOpt(sfd);
    bindSocket(sfd,6000);
    listenSocket(sfd);

    while(1){
        accept(sfd, NULL, NULL);
        cout<<"req came"<<endl;
        cout.flush();
    }
}
