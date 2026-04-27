#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include "file_ops.h"

void save_to_file(Player *player)
{
    int fd=open("data.txt",O_WRONLY | O_CREAT | O_APPEND,0644);

    if(fd<0)
    {
        perror("File open failed");
        return;
    }

    struct flock lock;

    lock.l_type=F_WRLCK;
    lock.l_whence=SEEK_SET;
    lock.l_start=0;
    lock.l_len=0;

    fcntl(fd,F_SETLKW,&lock);

    char buffer[200];
    if(player->role==WARRIOR)
    {
        snprintf(buffer,sizeof(buffer),"Player %s won as role Warrior\n",player->name);
    }
    else
    {
        snprintf(buffer,sizeof(buffer),"Player %s won as role Mage\n",player->name);
    }

    write(fd,buffer,strlen(buffer));

    lock.l_type=F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);

    close(fd);
}