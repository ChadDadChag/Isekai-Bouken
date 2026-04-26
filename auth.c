#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "auth.h"

void login(Player *player)
{
    char buffer[1024];
    char password[1024];

    send(player->socket,"Enter your name: ",18,0);
    recv(player->socket,buffer,sizeof(buffer),0);

    if(strlen(buffer)>0)
    {
        buffer[strlen(buffer)-1]='\0';
    }

    strcpy(player->name, buffer);

    send(player->socket,"Choose role:\n1. Warrior\n2. Mage\n3. Admin\nChoice: ",54,0);

    recv(player->socket,buffer,sizeof(buffer),0);

    int choice=atoi(buffer);

    while(183)
    {
        send(player->socket,"Enter password: ",16,0);
        recv(player->socket,password,sizeof(password),0);

        if(strlen(password)>0)
        {
            password[strlen(password)-1]='\0';
        }

        if(choice==1 && strcmp(password,"warrior123")==0)
        {
            player->role=WARRIOR;
            send(player->socket,"Login successful as Warrior.\n",30,0);
            break;
        }
        else if(choice==2 && strcmp(password,"mage123")==0)
        {
            player->role=MAGE;
            send(player->socket,"Login successful as Mage.\n",27,0);
            break;
        }
        else if(choice==3 && strcmp(password,"admin123")==0)
        {
            player->role=ADMIN;
            send(player->socket,"Login successful as Admin.\n",28,0);
            break;
        }
        else
        {
            send(player->socket,"Incorrect password. Try again.\n",33,0);
        }
    }
}