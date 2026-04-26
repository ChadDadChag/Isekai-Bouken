#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "armory.h"

pthread_mutex_t armory_mutex=PTHREAD_MUTEX_INITIALIZER;
int excalibur_available=1;

void handle_armory(Player *player)
{
    char buffer[1024];

    send(player->socket,"\n--- ARMORY ---\n",strlen("\n--- ARMORY ---\n"),0);
    send(player->socket,"Choose: Excalibur / Elder_Wand\n",strlen("Choose: Excalibur / Elder_Wand\n"),0);

    recv(player->socket,buffer,sizeof(buffer),0);

    buffer[strcspn(buffer,"\n")]=0;

    if(strcmp(buffer,"Excalibur")==0)
    {
        if(player->role!=WARRIOR)
        {
            char *msg="Only a Warrior can wield Excalibur!\n";
            send(player->socket,msg,strlen(msg),0);
            return;
        }

        printf("Thread %ld trying for Excalibur\n",pthread_self());

        pthread_mutex_lock(&armory_mutex);

        if(excalibur_available)
        {
            excalibur_available=0;
            player->hasSword=1;

            char *msg="You have obtained Excalibur!\n";
            send(player->socket,msg,strlen(msg),0);
        }
        else
        {
            char *msg="Excalibur already taken!\n";
            send(player->socket,msg,strlen(msg),0);
        }

        pthread_mutex_unlock(&armory_mutex);
    }

    else if(strcmp(buffer,"Elder_Wand")==0)
    {
        if(player->role!=MAGE)
        {
            char *msg="Only Mage can take Elder Wand!\n";
            send(player->socket,msg,strlen(msg),0);
            return;
        }

        player->hasStaff=1;

        char *msg="You obtained Elder Wand!\n";
        send(player->socket,msg,strlen(msg),0);
    }

    else
    {
        char *msg="Invalid choice!\n";
        send(player->socket,msg,strlen(msg),0);
    }
}