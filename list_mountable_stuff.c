/*
 * Program that lists mountable drives/partitions in linux systems.
 *
 * Copyright (c) 2019 Gaurav Kumar Yadav <gaurav712@protonmail.com>
 * for license information, see the LICENSE file distributed with this source
 *
 * I said "mountable partitions and drives as well".
 * In linux systems partions are named as sd[a-z][1-N], (N is some number) for drive named sd[a-z] and we can mount those partitions for whatever.
 *
 * I used to think that all the drives have at least one partition but I was wrong. I encountered a situation where a drive was listed as "sdb" and had no partitions.
 * I use dmenu(https://tools.suckless.org/dmenu) to list all mountable drives but as I said I didn't know all disks don't need to have partitions.
 * So, this piece of code searches for drives and partitions and outputs a list for dmenu.
 *
 * e.g. let's assume my /dev says:
 *          sda-|
 *              |-sda1
 *              |-sda2
 *              |-sda3
 *          sdb-|-sdb1
 *          sdc
 *          sdd-|-sdd1
 *              |-sdd2
 *
 *      then, this code will throw up:
 *          sda1
 *          sda2
 *          sda3
 *          sdb1
 *          sdc
 *          sdd1
 *          sdd2
 *      Actually, it outputs as "device(size)" just because why not? But that's irrelevant.
 *
 * It might be worthless for ya as everyone has different use cases.
 */

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <stdlib.h>
#include <stropts.h>
#include <unistd.h>

#define MAX_DEVICES 20

/* Checks if current entry is a either drive or partition */
static int checkEntry(struct dirent *dir);
/* Removes entry for parent drive of the partition "partName" */
static void removeDriveWithPart(char devices[MAX_DEVICES][5], char partName[5]);
/* Calculates device size in Gigs */
static double getSize(char *device);

int main(void) {

    short count = 0;
    DIR *dirp;
    struct dirent *dir;
    char devices[MAX_DEVICES][5];

    /* Open /dev */
    if((dirp = opendir("/dev")) == NULL) {
        perror("Error opening directory!");
        exit(1);
    }

    /* To check if there are any devices */
    strcpy(devices[0], "nil");

    /* Listing contents of /dev */
    dir = readdir(dirp);

    while(dir) {
        if(checkEntry(dir)) {
            strcpy(devices[count], dir->d_name);
            count++;
            dir = readdir(dirp);
        } else
            dir = readdir(dirp);
    }
    /* As a marker for list's ending */
    strcpy(devices[count], "nil");

    /* Exit if there are no devices */
    if(!(strcmp(devices[0], "nil"))) {
        perror("No devices!");
        exit(2);
    }

    /* Remove device names from devices[][] if it has partitions */
    count--;
    while(count >= 0) {
        if(strlen(devices[count]) > 3)
            /* It's a name longer than 3 letter, means it's a partition and we need to remove it's parent disk */
            removeDriveWithPart(devices, devices[count]);
        count--;
    }

    /* Print the devices' list */
    count = 0;
    while(strcmp(devices[count], "nil")) {
        printf("%-5s (%.2lfGB)\n", devices[count], getSize(devices[count]));
        count++;
    }

    /* Close directory to make system happy */
    closedir(dirp);

    return 0;
}

int
checkEntry(struct dirent *dir) {
    if(dir->d_name[0] == 's' && dir->d_name[1] == 'd') {
        if(dir->d_name[2] == 'a') /* it's the primary storage device */
            return 0;
        else
            return 1;
    } else
        return 0;
}

void
removeDriveWithPart(char devices[MAX_DEVICES][5], char partName[5]) {

    short count = 0;
    char temp[5], devName[4];

    strcpy(devName, partName); /* just to keep partName unchanged as changing it will overwrite the devices[][] */
    devName[3] = '\0'; /* truncating partition name to get it parent device's name */

    /* Finding the parent drive in the list */
    while(strcmp(devName, devices[count]))
        count++;

    /* Deleting entry and aligning the list all over again! */
    while(1) {
        strcpy(temp, devices[count + 1]);
        if(!(strcmp(temp, "nil"))) {
            strcpy(devices[count], temp);
            return;
        }
        strcpy(devices[count], temp);
        count++;
    }
}

double
getSize(char *device) {

    u_int64_t arg;
    int fd;
    double devSize;
    char devName[10];

    strcpy(devName, "/dev/");
    strcat(devName, device);

    if((fd = open(devName, O_RDONLY)) == -1) {
        perror("Error opening device!");
        exit(3);
    }

    if((ioctl(fd, BLKGETSIZE64, &arg) == -1)) {
        perror("Error getting size!");
        exit(4);
    }

    devSize = arg/(1024.0*1024.0*1024.0);

    close(fd);
    return devSize;
}

