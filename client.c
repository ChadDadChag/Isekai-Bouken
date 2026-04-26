#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080

int sock;

void* receive_messages(void* arg)
{
    char buffer[1024];

    while(1)
    {
        memset(buffer,0,sizeof(buffer));

        int bytes=recv(sock,buffer,sizeof(buffer),0);

        if(bytes<=0)
        {
            printf("Disconnected from server\n");
            exit(0);
        }

        printf("%s",buffer);
        fflush(stdout);
    }
}

int main()
{
    struct sockaddr_in serv_addr;

    sock=socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0)
    {
        perror("Socket error");
        return -1;
    }

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(PORT);

    inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr);

    if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to server\n");

    pthread_t recv_thread;
    pthread_create(&recv_thread,NULL,receive_messages,NULL);

    char msg[1024];

    while(1)
    {
        fgets(msg,sizeof(msg),stdin);
        send(sock,msg,strlen(msg),0);
    }

    close(sock);
    return 0;
}