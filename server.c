#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "player.h"
#include "armory.h"
#include "auth.h"
#include "file_ops.h"

#define PORT 8080
#define MAX_CLIENTS 3

pthread_t threads[MAX_CLIENTS];
int thread_count=0;

int clients[MAX_CLIENTS];
int client_count=0;
pthread_mutex_t clients_mutex=PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t demon_mutex=PTHREAD_MUTEX_INITIALIZER;
int demon_hp=100;

pthread_mutex_t ready_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ready_cond=PTHREAD_COND_INITIALIZER;

int ready_players=0;
int total_players=3;

struct msgbuf{
    long mtype;
    char mtext[100];
};

int msgid;

void* ipc_listener(void* arg)
{
    struct msgbuf msg;
    while(1)
    {
        msgrcv(msgid,&msg,sizeof(msg.mtext),1,0);
        printf("[IPC RECEIVED]: %s\n",msg.mtext);
    }
}

void remove_client(int sock)
{
    pthread_mutex_lock(&clients_mutex);

    for(int i=0;i<client_count;i++)
    {
        if(clients[i]==sock)
        {
            clients[i]=clients[client_count-1];
            client_count--;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void broadcast(char *msg)
{
    pthread_mutex_lock(&clients_mutex);

    for(int i=0;i<client_count;i++)
    {
        if(send(clients[i],msg,strlen(msg),0)<=0)
        {
            int dead_sock=clients[i];
            clients[i]=clients[client_count-1];
            client_count--;
            i--;
            close(dead_sock);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void send_story(int sock)
{
    char *lines[]={
        "\n--- THE VOID ---\n",
        "You awaken in an endless void...\n",
        "A voice echoes around you...\n\n",
        "Goddess: You have been summoned to be a hero and save humanity from destruction.\n",
        "Goddess: The Demon King threatens this world.\n",
        "Goddess: Prepare yourself before battle.\n\n",
        "Teleporting you to the Armory...\n\n"
    };

    for(int i=0;i<7;i++)
    {
        send(sock,lines[i],strlen(lines[i]),0);
        sleep(1);
    }
}

void wait_for_players(Player *player)
{
    int sock=player->socket;
    char buffer[100];

    send(sock,"\nYou are ready for battle.\n",30,0);
    send(sock,"Waiting for other players...\n",29,0);

    pthread_mutex_lock(&ready_mutex);

    ready_players++;

    if(ready_players<total_players)
    {
        while(ready_players<total_players)
        {
            pthread_cond_wait(&ready_cond,&ready_mutex);
        }
    }
    else
    {
        pthread_cond_broadcast(&ready_cond);
    }

    pthread_mutex_unlock(&ready_mutex);

    send(sock,"\nAll players ready. Entering Demon Castle...\n",49,0);
}

void demon_fight(Player *player)
{
    char buffer[1024];
    int sock=player->socket;

    send(sock,"\nYou have entered the Demon King's Castle.\n",47,0);

    while(1)
    {
        if(player->hp<=0)
        {
            char msg[100];
            snprintf(msg,sizeof(msg),"%s has been defeated.\n",player->name);
            broadcast(msg);
            break;
        }

        int current_hp;

        pthread_mutex_lock(&demon_mutex);
        current_hp=demon_hp;
        pthread_mutex_unlock(&demon_mutex);

        char status[100];
        snprintf(status,sizeof(status),"\nYour HP: %d | Demon HP: %d\n",player->hp,current_hp);
        send(sock,status,strlen(status),0);

        if(player->role==WARRIOR)
        {
            send(sock,"Enter action (Divine_Strike or chat <msg>): \n",46,0);
        }
        else
        {
            send(sock,"Enter action (Cast_Fireball or chat <msg>): \n",46,0);
        }

        memset(buffer,0,sizeof(buffer));
        if(recv(sock,buffer,sizeof(buffer),0)<=0)
        {
            remove_client(sock);
            break;
        }

        buffer[strcspn(buffer,"\r\n")]=0;

        if(strncmp(buffer,"chat ",5)==0)
        {
            char msg[200];
            snprintf(msg,sizeof(msg),"[CHAT] %s: %s\n",player->name,buffer+5);
            broadcast(msg);
            continue;
        }
        else if(strncmp(buffer,"Divine_Strike",13)==0)
        {
            send(sock,"Your Divine Strike landed on the Demon\n",39,0);
        }
        else if(strncmp(buffer,"Cast_Fireball",13)==0)
        {
            send(sock,"Your Fireball landed on the Demon\n",34,0);
        }
        else
        {
            continue;
        }

        int damage;

        if(player->hasSword)
        {
            damage=rand()%20+15;
        }
        else if(player->hasSpear)
        {
            damage=rand()%15+10;
        }
        else if(player->hasStaff)
        {
            damage=rand()%20+10;
        }

        pthread_mutex_lock(&demon_mutex);

        if(demon_hp>0)
        {
            demon_hp-=damage;
            if(demon_hp<0)
            {
                demon_hp=0;
            }

            char msg[150];
            snprintf(msg,sizeof(msg),"%s dealt %d damage. Demon HP = %d\n",player->name,damage,demon_hp);
            broadcast(msg);
        }

        if(demon_hp<=0)
        {
            pthread_mutex_unlock(&demon_mutex);

            broadcast("Demon King defeated.\n");

            struct msgbuf msg;
            msg.mtype=1;
            strcpy(msg.mtext,"Demon King defeated");
            msgsnd(msgid,&msg,sizeof(msg.mtext),0);

            break;
        }

        pthread_mutex_unlock(&demon_mutex);

        int chance=rand()%100;

        if(chance<25)
        {
            int demon_damage=rand()%6+3;
            player->hp-=demon_damage;

            char msg2[150];
            snprintf(msg2,sizeof(msg2),"Demon attacked %s for %d damage\n",player->name,demon_damage);
            broadcast(msg2);
        }
        else
        {
            broadcast("Demon missed the attack.\n");
        }

        sleep(1);
    }
}

void send_ending(int sock)
{
    char *lines[]={
        "Congratulations!\n",
        "You and your party have defeated the Demon.\n",
        "Everyone praises your party as the heroes who have saved humanity.\n",
        "You have now fulfilled your purpose and can go back to your world.\n",
        "\n--- VOID ---\n",
        "You once again awaken in an endless void...\n",
        "A voice echoes around you...\n\n",
        "Goddess: Thank you O Brave Soul\n",
        "...\n",
        "..\n",
        ".\n",
        "Suddenly you are back in Prof Thangu's class and your life returns back to normal.\n"
    };

    for(int i=0;i<12;i++)
    {
        send(sock,lines[i],strlen(lines[i]),0);
        sleep(1);
    }
}

void* handle_client(void* arg)
{
    Player *player=(Player*)arg;
    int sock=player->socket;

    login(player);

    send_story(sock);

    handle_armory(player);

    wait_for_players(player);

    sleep(1);

    demon_fight(player);

    send_ending(sock);

    if(player->hp>0)
    {
        save_to_file(player);
    }

    send(sock,"\nGame Over.\n",12,0);

    remove_client(sock);

    close(sock);
    free(player);
    return NULL;
}

int main()
{
    int server_fd;
    struct sockaddr_in address;
    int addrlen=sizeof(address);

    srand(time(NULL));

    msgid=msgget(1234,0666|IPC_CREAT);

    pthread_t ipc_thread;
    pthread_create(&ipc_thread,NULL,ipc_listener,NULL);

    server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd<0)
    {
        perror("Socket failed");
        exit(1);
    }

    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(PORT);

    if(bind(server_fd,(struct sockaddr*)&address,sizeof(address))<0)
    {
        perror("Bind failed");
        exit(1);
    }

    if(listen(server_fd,5)<0)
    {
        perror("Listen failed");
        exit(1);
    }

    printf("Server running on port %d...\n",PORT);

    while(1)
    {
        int client_socket=accept(server_fd,(struct sockaddr*)&address,(socklen_t*)&addrlen);

        if(client_socket<0)
        {
            perror("Accept failed");
            continue;
        }

        printf("New client connected\n");

        pthread_mutex_lock(&clients_mutex);
        if(client_count<MAX_CLIENTS)
            clients[client_count++]=client_socket;
        else
        {
            pthread_mutex_unlock(&clients_mutex);
            close(client_socket);
            continue;
        }
        pthread_mutex_unlock(&clients_mutex);

        Player *player=malloc(sizeof(Player));
        player->socket=client_socket;
        player->hasSword=0;
        player->hasStaff=0;
        player->hasSpear=0;
        player->hp=100;

        if(thread_count<MAX_CLIENTS)
        {
            pthread_create(&threads[thread_count++],NULL,handle_client,player);
        }
        else
        {
            printf("Max clients reached\n");
            close(client_socket);
            free(player);
        }
    }

    return 0;
}