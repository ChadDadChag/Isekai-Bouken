#ifndef PLAYER_H
#define PLAYER_H

typedef enum
{
    WARRIOR,
    MAGE,
    ADMIN
} Role;

typedef struct
{
    int hp;
    int socket;
    Role role;
    int hasSword;
    int hasStaff;
    char name[20];
} Player;

#endif