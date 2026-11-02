#include <stdio.h>
#include <stdlib.h>
#include "algoritmos.h"

int main() {
    int opcao = 0;

    while (1) {
        printf("========================================================\n");
        printf("       SELECAO DE ALGORITMO DE ORDENACAO\n");
        printf("========================================================\n");
        printf("1. Merge Sort\n");
        printf("2. Heap Sort\n");
        printf("3. Bucket Sort\n");
        printf("0. Sair\n");
        printf("========================================================\n");
        printf("Escolha uma opcao: ");
        
        if (scanf("%d", &opcao) != 1) {
            printf("\nOpcao invalida!\n\n");
            while (getchar() != '\n'); // Limpa o buffer de entrada
            continue;
        }

        printf("\n");

        switch (opcao) {
            case 1:
                Resultado_merge_sort();
                break;
            case 2:
                Resultado_heap_sort();
                break;
            case 3:
                Resultado_bucket_sort_lote();
                break;
            case 0:
                printf("Encerrando o programa...\n");
                return 0;
            default:
                printf("Opcao invalida! Tente novamente.\n\n");
                break;
        }

        printf("\nPressione ENTER para voltar ao menu...");
        getchar(); // Captura o '\n' restante do scanf
        getchar(); // Aguarda o ENTER
        printf("\n");
    }

    return 0;
}