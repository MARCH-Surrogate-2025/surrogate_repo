//
// Created by surrogate on 24. 8. 7.
//

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>

#define SHM_INFO_COUNT 30
#define SHM_KEYS 3832
#define SHM_SIZE 1024

static int SharedMemoryCreate(int &shmid);
static int SharedMemoryWrite(int &shmid, char *data, int size);
static int SharedMemoryRead(int &shmid, char *data);
static int SharedMemoryFree(int &shmid);

typedef struct _shm_info{
    char         str_ip[40];
    unsigned int int_ip;
    unsigned int int_id;
} SHM_INFOS;

typedef struct _grpc_info{
    double wheel_vel;
    double steering_pos;
} GRPC_INFOS;


static int SharedMemoryCreate(int &shmid)
{
    shmid = shmget((key_t)SHM_KEYS, SHM_SIZE, 0666|IPC_CREAT);

    if(shmid == -1) {
        perror("shmget failed");
        return 1;
    }
    else {
        SharedMemoryFree(shmid);
        shmid = shmget((key_t)SHM_KEYS, SHM_SIZE, IPC_CREAT| 0666);

        if(shmid == -1)
        {
            perror("Shared memory create fail");
            return 1;
        }
    }

    return 0;
}

static int SharedMemoryWrite(int &shmid, char *data, int size)
{
    void *shmaddr;
    if(size > SHM_SIZE)
    {
        printf("Shared memory size over");
        return 1;
    }

    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    {
        perror("Shmat failed");
        return 1;
    }

    memcpy((char *)shmaddr, data, size);

    if(shmdt(shmaddr) == -1)
    {
        perror("Shmdt failed");
        return 1;
    }
    return 0;
}

static int SharedMemoryRead(int &shmid, char *data)
{
    void *shmaddr;
    char mess[SHM_SIZE] = {0};

    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    {
        perror("Shmat failed");
        return 1;
    }

    memcpy(data, (char *)shmaddr, sizeof(mess));

    if(shmdt(shmaddr) == -1)
    {
        perror("Shmdt failed");
        return 1;
    }
    return 0;
}

static int SharedMemoryFree(int &shmid)
{
    if(shmctl(shmid, IPC_RMID, 0) == -1)
    {
        perror("Shmctl failed : ");
        return 1;
    }
    return 0;
}

#endif //SHARED_MEMORY_H
