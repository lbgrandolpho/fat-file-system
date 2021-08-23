#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include <string.h>

#define CLUSTER_SIZE 1024
#define FAT_TAM 4096

/* entrada de diretorio, 32 bytes cada */
typedef struct {
    uint8_t filename[18];
    uint8_t attributes;
    uint8_t reserved[7];
    uint16_t first_block;
    uint32_t size;
} dir_entry_t;

/* diretorios (incluindo ROOT), 32 entradas de diretorio
com 32 bytes cada = 1024 bytes ou bloco de dados de 1024 bytes*/
typedef union {
    dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
    uint8_t data[CLUSTER_SIZE];
} data_cluster;

int init();
int load();
int mkdir(char *nome, int dir);
int ls(char *nome);
int create(char *nome);
int read(char *nome);
int append(char *nome, char *str, int app);
int write(char *nome, char *str);
int unlink(char *nome);

#endif