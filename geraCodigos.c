#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void gerar_arquivo(int quantidade) {
    char nome_arquivo[50];
    
    sprintf(nome_arquivo, "codigos_%d.txt", quantidade);
    
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        printf("Erro ao criar o arquivo %s!\n", nome_arquivo);
        return;
    }

    for (int i = 0; i < quantidade; i++) {
        int codigo = (rand() % 900000) + 100000;
        fprintf(arquivo, "%d\n", codigo);
    }

    fclose(arquivo);
    printf("-> Arquivo '%s' gerado com sucesso.\n", nome_arquivo);
}

int main() {
    srand((unsigned int)time(NULL));

    int tamanhos[] = {500, 5000, 50000, 500000, 5000000};
    int num_arquivos = sizeof(tamanhos) / sizeof(tamanhos[0]);

    printf("Iniciando a geracao dos arquivos...\n");
    printf("Isso pode levar alguns segundos para os arquivos maiores.\n\n");

    for (int i = 0; i < num_arquivos; i++) {
        gerar_arquivo(tamanhos[i]);
    }

    printf("\nTodos os arquivos foram gerados!\n");

    return 0;
}