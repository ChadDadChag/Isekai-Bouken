#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include "armory.h"

pthread_mutex_t armory_mutex=PTHREAD_MUTEX_INITIALIZER;
int excalibur_available=1;

void handle_armory(Player *player)
{
    char buffer[1024];

    send(player->socket,"\n--- ARMORY ---\n",strlen("\n--- ARMORY ---\n"),0);

    while(1)
    {
        send(player->socket,"Choose: Excalibur / Divine_Spear / Elder_Wand\n",52,0);

        memset(buffer,0,sizeof(buffer));
        recv(player->socket,buffer,sizeof(buffer),0);

        buffer[strcspn(buffer,"\n")]=0;

        if(strcmp(buffer,"Excalibur")==0)
        {
            if(player->role!=WARRIOR)
            {
                send(player->socket,"Only Warrior can use Excalibur. Try again.\n",48,0);
                continue;
            }

            pthread_mutex_lock(&armory_mutex);

            if(excalibur_available)
            {
                excalibur_available=0;
                player->hasSword=1;

                pthread_mutex_unlock(&armory_mutex);

                send(player->socket,"You obtained Excalibur.\n",25,0);
                break;
            }
            else
            {
                pthread_mutex_unlock(&armory_mutex);

                send(player->socket,"Excalibur taken. Giving Divine Spear.\n",44,0);
                player->hasSpear=1;
                break;
            }
        }

        else if(strcmp(buffer,"Divine_Spear")==0)
        {
            if(player->role!=WARRIOR)
            {
                send(player->socket,"Only Warrior can use Divine Spear. Try again.\n",51,0);
                continue;
            }

            player->hasSpear=1;
            send(player->socket,"You obtained Divine Spear.\n",29,0);
            break;
        }

        else if(strcmp(buffer,"Elder_Wand")==0)
        {
            if(player->role!=MAGE)
            {
                send(player->socket,"Only Mage can use Elder Wand. Try again.\n",47,0);
                continue;
            }

            player->hasStaff=1;
            send(player->socket,"You obtained Elder Wand.\n",27,0);
            break;
        }

        else
        {
            send(player->socket,"Invalid choice. Try again.\n",29,0);
        }
    }
}