#include <stdio.h>
#include <stdlib.h>
#include "fat.h"

#define STR_EQ(x, y) (strcmp(x, y)==0)

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
static uint16_t fat[4096];

static dir_entry_t root[CLUSTER_SIZE / sizeof(dir_entry_t)];

int entradaVazia() {

    int i;
    for (i = 0; i < CLUSTER_SIZE/sizeof(dir_entry_t); i++) {
        if (root[i].filename[0] == 0) return i;
    }

    return -1;
    
}

int clusterVazio() {

    int i;
    for (i = 10; i < FAT_TAM; i++) {
        if (fat[i] == 0) return i;
    }

    return -1;

}

int salvaNoArquivo(FILE *arq) {

    fseek(arq, CLUSTER_SIZE, SEEK_SET);
    fwrite(fat, sizeof(uint16_t), FAT_TAM, arq);

    fseek(arq, CLUSTER_SIZE*9, SEEK_SET);
    fwrite(root, sizeof(dir_entry_t), CLUSTER_SIZE/sizeof(dir_entry_t), arq);

}

int init() {

    int i;

    // Inicializa a FAT
    fat[0] = 0xfffd;
    for (i = 1; i < 9; i++) {
        fat[i] = 0xfffe;
    }

    fat[9] = 0xffff;

    for (i = 10; i < FAT_TAM; i++) {
        fat[i] = 0;
    }

    memset(root, 0, sizeof(root));

    FILE *fat_part = fopen("fat.part", "wb");
    if (!fat_part) exit(1);

    uint8_t boot = 0xbb;
    for (i = 0; i < CLUSTER_SIZE/sizeof(boot); i++) {
        fwrite(&boot, sizeof(boot), sizeof(boot), fat_part);
    }

    fwrite(fat, sizeof(uint16_t), sizeof(fat)/sizeof(uint16_t), fat_part);
    fwrite(root, sizeof(dir_entry_t), CLUSTER_SIZE/sizeof(dir_entry_t), fat_part);

    data_cluster cluster;
    memset(&cluster, 0, sizeof(data_cluster));

    for (i = 0; i < FAT_TAM-10; i++) {
        fwrite(&cluster, 1, CLUSTER_SIZE, fat_part);
    }

    fclose(fat_part);

    return 0;

}

int load() {

    FILE *fat_part = fopen("fat.part", "rb+");
    if (!fat_part) exit(1);

    int i;

    fseek(fat_part, CLUSTER_SIZE, SEEK_SET);
    fread(fat, sizeof(uint16_t), FAT_TAM, fat_part);
    fread(root, sizeof(dir_entry_t), CLUSTER_SIZE/sizeof(dir_entry_t), fat_part);

    fclose(fat_part);

    return 0;

}

int mkdir(char *nome, int dir) {

    int i;
    FILE *fat_part = fopen("fat.part", "rb+");

    for (i = 0; i < CLUSTER_SIZE/sizeof(dir_entry_t); i++) {
        if (STR_EQ(nome, root[i].filename)) return 1;
    }

    // Índice da primeira entrada vazia no diretório raiz (0-32)
    int entrada = entradaVazia();
    int cluster = clusterVazio();

    if (entrada == -1 || cluster == -1) return 1;

    data_cluster novo;
    dir_entry_t entrada_dir;
    memset(&novo, 0, sizeof(data_cluster));

    memset(&entrada_dir, 0, sizeof(dir_entry_t));
    strncpy(entrada_dir.filename, nome, 18);
    entrada_dir.attributes = dir;
    entrada_dir.first_block = cluster;
    entrada_dir.size = 1;

    memcpy(&root[entrada], &entrada_dir, sizeof(dir_entry_t));

    fat[cluster] = 0xffff;

    salvaNoArquivo(fat_part);
    fclose(fat_part);
    return 0;

}

int ls(char *nome) {

    FILE *fat_part = fopen("fat.part", "rb+");
    int i;

    if (nome == NULL || STR_EQ(nome, "/")) {

        for (i = 0; i < CLUSTER_SIZE/sizeof(dir_entry_t); i++) {
            if (root[i].filename[0] != 0) {
                printf("%s\n", root[i].filename);
            }
        }

    }

    else {
        for (i = 0; i < CLUSTER_SIZE/sizeof(dir_entry_t); i++) {
            if (STR_EQ(nome, root[i].filename)) break;
        }
        if (i == CLUSTER_SIZE/sizeof(dir_entry_t)) return 1;

        data_cluster cluster;
        fseek(fat_part, root[i].first_block * CLUSTER_SIZE, SEEK_SET);
        fread(&cluster, sizeof(data_cluster), CLUSTER_SIZE/sizeof(data_cluster), fat_part);

        for (i = 0; i < 32; i++) {
            if (cluster.dir[i].filename[0]) {
                printf("%s\n", cluster.dir[i].filename);
            }
        }
    }

    fclose(fat_part);

    return 0;

}

int create(char *nome) {

    return mkdir(nome, 0);

}

#define min(x, y) ((x) > (y) ? (y) : (x))
int append(char *nome, char *str, int app) {

    FILE *fat_part = fopen("fat.part", "rb+");
    int i, endereco;
    for (i = 0; i < CLUSTER_SIZE/sizeof(dir_entry_t); i++) {
        if (STR_EQ(nome, root[i].filename)) break;
    }
    if (i == CLUSTER_SIZE/sizeof(dir_entry_t)) return 1;
    endereco = i;

    data_cluster cluster;
    int m, comeco = 0;

    if (app) {
        fseek(fat_part, root[endereco].first_block * CLUSTER_SIZE, SEEK_SET);
        fread(&cluster, sizeof(data_cluster), 1, fat_part);

        for (i = 0; i < CLUSTER_SIZE && cluster.data[i]; i++);
        comeco = i;
    }

    else {
        memset(&cluster, 0, sizeof(data_cluster));
    }

    if (comeco < CLUSTER_SIZE) {
        i = CLUSTER_SIZE - strlen(cluster.data);
        m = strlen(str);
        m = min(m, i);
        for (i = 0; i < m; i++) {
            cluster.data[i + comeco] = str[i];
        }
    }

    salvaNoArquivo(fat_part); // Salva FAT e Root

    // Salva o bloco dos dados
    fseek(fat_part, root[endereco].first_block * CLUSTER_SIZE, SEEK_SET);
    fwrite(&cluster, sizeof(data_cluster), 1, fat_part);

    fclose(fat_part);

    return 0;

}

int write(char *nome, char *str) {

    return append(nome, str, 0);

}

int read(char *nome) {

    FILE *fat_part = fopen("fat.part", "rb+");
    int i, endereco;
    for (i = 0; i < CLUSTER_SIZE/sizeof(dir_entry_t); i++) {
        if (STR_EQ(nome, root[i].filename)) break;
    }
    if (i == CLUSTER_SIZE/sizeof(dir_entry_t)) return 1;
    endereco = i;

    data_cluster cluster;
    fseek(fat_part, root[endereco].first_block * CLUSTER_SIZE, SEEK_SET);
    fread(&cluster, sizeof(data_cluster), 1, fat_part);

    printf("%s\n", cluster.data);

    fclose(fat_part);

}

int unlink(char *nome) {

    FILE *fat_part = fopen("fat.part", "rb+");
    int i, endereco;
    for (i = 0; i < CLUSTER_SIZE/sizeof(dir_entry_t); i++) {
        if (STR_EQ(nome, root[i].filename)) break;
    }
    if (i == CLUSTER_SIZE/sizeof(dir_entry_t)) return 1;
    endereco = i;

    data_cluster cluster;
    memset(&cluster, 0, sizeof(data_cluster));
    fseek(fat_part, root[endereco].first_block * CLUSTER_SIZE, SEEK_SET);
    fwrite(&cluster, sizeof(data_cluster), 1, fat_part);

    memset(root + endereco, 0, sizeof(dir_entry_t));

    fat[endereco+10] = 0;

    salvaNoArquivo(fat_part);
    return 0;

}