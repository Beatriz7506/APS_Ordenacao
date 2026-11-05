#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "algoritmosDeOrdenacao.h"

static const int tamanhos[] = {500, 5000, 50000, 500000, 5000000};

static void formatarNumero(unsigned long long valor, char *destino, size_t capacidade)
{
    char digitos[32];
    size_t quantidade = 0;
    size_t posicao = 0;

    do {
        digitos[quantidade++] = (char)('0' + valor % 10);
        valor /= 10;
    } while (valor > 0 && quantidade < sizeof(digitos));

    while (quantidade > 0 && posicao + 1 < capacidade) {
        destino[posicao++] = digitos[--quantidade];
        if (quantidade > 0 && quantidade % 3 == 0 && posicao + 1 < capacidade) {
            destino[posicao++] = '.';
        }
    }
    destino[posicao] = '\0';
}

static int carregarNumeros(const char *nome, int numeros[], int tamanho)
{
    FILE *arquivo = fopen(nome, "r");
    int i;

    if (arquivo == NULL) return 0;
    for (i = 0; i < tamanho; i++) {
        if (fscanf(arquivo, "%d", &numeros[i]) != 1) {
            fclose(arquivo);
            return 0;
        }
    }
    fclose(arquivo);
    return 1;
}

static void imprimirMemoria(const char *rotulo, size_t bytes)
{
    if (bytes >= 1024 * 1024) {
        printf("%s%.2f MB\n", rotulo, (double)bytes / (1024.0 * 1024.0));
    } else {
        printf("%s%zu Bytes (%.2f KB)\n", rotulo, bytes, (double)bytes / 1024.0);
    }
}

static int executar(int escolha, int n)
{
    char caminho[80];
    char nomeArquivo[50];
    int *originais = NULL;
    int *merge = NULL;
    int *heap = NULL;
    int *aux = NULL;
    unsigned int *bucket = NULL;
    int precisaMerge = escolha == 2 || escolha == 4;
    int precisaHeap = escolha == 3 || escolha == 4;
    int precisaBucket = escolha == 4;
    size_t memoriaMerge = (size_t)n * sizeof(int);
    size_t memoriaBucket = 0;
    size_t memoriaHeap = 0;
    size_t quantidadeBaldes = 0;
    size_t *quantidadesPorBalde = NULL;
    Metricas metricasMerge = {0, 0};
    HeapMetricas metricasHeap = {0, 0};
    unsigned long long movimentosBucket = 0;
    unsigned long long movimentosInternosBucket = 0;
    unsigned long long comparacoesBucket = 0;
    double tempoMerge = 0.0, tempoBucket = 0.0, tempoHeap = 0.0;
    clock_t inicio, fim;
    int j;

    sprintf(nomeArquivo, "codigos_%d.txt", n);
    sprintf(caminho, "output/%s", nomeArquivo);
    originais = (int *)malloc((size_t)n * sizeof(int));
    if (precisaMerge) {
        merge = (int *)malloc((size_t)n * sizeof(int));
        aux = (int *)malloc((size_t)n * sizeof(int));
    }
    if (precisaHeap) heap = (int *)malloc((size_t)n * sizeof(int));
    if (precisaBucket) bucket = (unsigned int *)malloc((size_t)n * sizeof(unsigned int));

    if (originais == NULL || (precisaMerge && (merge == NULL || aux == NULL)) ||
        (precisaHeap && heap == NULL) || (precisaBucket && bucket == NULL)) {
        printf("Erro de memoria para a entrada de %d numeros.\n", n);
        goto falha;
    }

    if (!carregarNumeros(caminho, originais, n)) {
        sprintf(caminho, "%s", nomeArquivo);
        if (!carregarNumeros(caminho, originais, n)) {
            printf("Nao foi possivel ler %s na pasta atual nem em output.\n", nomeArquivo);
            goto falha;
        }
    }

    if (precisaMerge) {
        for (j = 0; j < n; j++) merge[j] = originais[j];
        inicio = clock();
        mergeSort(merge, aux, 0, n - 1, &metricasMerge);
        fim = clock();
        tempoMerge = (double)(fim - inicio) / CLOCKS_PER_SEC;
        printf("========================================================\n");
        printf("          RELATORIO DE DESEMPENHO - MERGE SORT          \n");
        printf("========================================================\n\n");
        printf("Arquivo: %s\n", nomeArquivo);
        printf("- Tamanho da Entrada        : %d codigos\n", n);
        printf("- Tempo de Execucao         : %f segundos\n", tempoMerge);
        printf("- Quantidade de comparacoes : %llu\n", metricasMerge.qtde_comparacoes);
        printf("- Quantidade de movimentos  : %llu\n", metricasMerge.qtde_movimentos);
        imprimirMemoria("- Memoria Extra Usada: ", memoriaMerge);
        printf("--------------------------------------------------------\n");
    }

    if (precisaBucket) {
        for (j = 0; j < n; j++) bucket[j] = (unsigned int)originais[j];
        inicio = clock();
        if (!bucketSort(bucket, (size_t)n, &movimentosBucket, &movimentosInternosBucket,
                        &quantidadeBaldes, &quantidadesPorBalde)) {
            printf("Erro ao executar Bucket Sort para %d numeros.\n", n);
            goto falha;
        }
        fim = clock();
        tempoBucket = (double)(fim - inicio) / CLOCKS_PER_SEC;
        comparacoesBucket = bucketSortQuantidadeComparacoes();
        memoriaBucket = bucketSortMemoriaExtra(quantidadeBaldes, quantidadesPorBalde);
        printf("\n========== RESULTADO BUCKET SORT ==========\n");
        printf("Arquivo: %s\n", nomeArquivo);
        char tamanhoEntradaTexto[32], movimentosTexto[32], movimentosForaTexto[32];
        char movimentosDentroTexto[32], comparacoesTexto[32];
        formatarNumero((unsigned long long)n, tamanhoEntradaTexto, sizeof(tamanhoEntradaTexto));
        formatarNumero(movimentosBucket, movimentosTexto, sizeof(movimentosTexto));
        formatarNumero(movimentosBucket - movimentosInternosBucket,
                       movimentosForaTexto, sizeof(movimentosForaTexto));
        formatarNumero(movimentosInternosBucket, movimentosDentroTexto, sizeof(movimentosDentroTexto));
        formatarNumero(comparacoesBucket, comparacoesTexto, sizeof(comparacoesTexto));
        printf("Tamanho da entrada: %s codigos\n", tamanhoEntradaTexto);
        printf("Tempo de Execucao: %f segundos\n", tempoBucket);
        printf("Quantidade de comparacoes na ordenacao dos baldes: %s\n", comparacoesTexto);
        printf("Movimentos fora dos baldes (entrada e saida): %s\n", movimentosForaTexto);
        printf("Movimentos dentro dos baldes (ordenacao): %s\n", movimentosDentroTexto);
        printf("Total de movimentos: %s\n", movimentosTexto);
        imprimirMemoria("Memoria Extra Usada: ", memoriaBucket);
        printf("===========================================\n");
    }

    if (precisaHeap) {
        for (j = 0; j < n; j++) heap[j] = originais[j];
        inicio = clock();
        heap_sort_com_metricas(heap, (size_t)n, &metricasHeap);
        fim = clock();
        tempoHeap = (double)(fim - inicio) / CLOCKS_PER_SEC;
        printf("\n========== RESULTADO HEAP SORT ==========\n");
        printf("Arquivo: %s\n", nomeArquivo);
        printf("Tamanho da entrada: %d codigos\n", n);
        printf("Tempo de Execucao: %f segundos\n", tempoHeap);
        printf("Quantidade de comparacoes: %llu\n", metricasHeap.comparacoes);
        printf("Quantidade de movimentos: %llu\n", metricasHeap.movimentos);
        printf("Memoria extra dinamica: 0 Bytes (uso auxiliar O(1))\n");
        printf("=========================================\n");
    }

    if (escolha == 4) {
        char memoriaBucketTexto[32], memoriaMergeTexto[32], memoriaHeapTexto[32];
        char movimentosBucketTexto[32], movimentosMergeTexto[32], movimentosHeapTexto[32];
        char comparacoesBucketTexto[32], comparacoesMergeTexto[32], comparacoesHeapTexto[32];

        formatarNumero((unsigned long long)memoriaBucket, memoriaBucketTexto, sizeof(memoriaBucketTexto));
        formatarNumero((unsigned long long)memoriaMerge, memoriaMergeTexto, sizeof(memoriaMergeTexto));
        formatarNumero((unsigned long long)memoriaHeap, memoriaHeapTexto, sizeof(memoriaHeapTexto));
        formatarNumero(movimentosBucket, movimentosBucketTexto, sizeof(movimentosBucketTexto));
        formatarNumero(metricasMerge.qtde_movimentos, movimentosMergeTexto, sizeof(movimentosMergeTexto));
        formatarNumero(metricasHeap.movimentos, movimentosHeapTexto, sizeof(movimentosHeapTexto));
        formatarNumero(comparacoesBucket, comparacoesBucketTexto, sizeof(comparacoesBucketTexto));
        formatarNumero(metricasMerge.qtde_comparacoes, comparacoesMergeTexto, sizeof(comparacoesMergeTexto));
        formatarNumero(metricasHeap.comparacoes, comparacoesHeapTexto, sizeof(comparacoesHeapTexto));

        printf("\n================ COMPARACAO DOS ALGORITMOS ================\n");
        printf("Arquivo: %s (%d numeros)\n", nomeArquivo, n);
        printf("%-18s %14s %18s %18s %18s\n",
               "Algoritmo", "Tempo (s)", "Memoria extra", "Movimentos", "Comparacoes");
        printf("%-18s %14.6f %15s B %18s %18s\n", "Bucket Sort", tempoBucket,
               memoriaBucketTexto, movimentosBucketTexto, comparacoesBucketTexto);
        printf("%-18s %14.6f %15s B %18s %18s\n", "Merge Sort", tempoMerge,
               memoriaMergeTexto, movimentosMergeTexto, comparacoesMergeTexto);
        printf("%-18s %14.6f %15s B %18s %18s\n", "Heap Sort", tempoHeap,
               memoriaHeapTexto, movimentosHeapTexto, comparacoesHeapTexto);
        printf("============================================================\n");
    }

    free(quantidadesPorBalde);
    free(originais);
    free(merge);
    free(aux);
    free(heap);
    free(bucket);
    return 1;

falha:
    free(quantidadesPorBalde);
    free(originais);
    free(merge);
    free(aux);
    free(heap);
    free(bucket);
    return 0;
}

static int escolherTamanho(void)
{
    int opcao;
    int total = (int)(sizeof(tamanhos) / sizeof(tamanhos[0]));

    printf("\nEscolha o arquivo de entrada:\n");
    for (int i = 0; i < total; i++) printf("%d - %d numeros\n", i + 1, tamanhos[i]);
    printf("Opcao: ");
    if (scanf("%d", &opcao) != 1) {
        int caractere;
        while ((caractere = getchar()) != '\n' && caractere != EOF) {}
        return 0;
    }
    if (opcao < 1 || opcao > total) {
        printf("Opcao de arquivo invalida.\n");
        return 0;
    }
    return tamanhos[opcao - 1];
}

int main(void)
{
    int escolha;

    do {
        printf("\n========== MENU DE ORDENACAO ==========\n");
        printf("1 - Executar Bucket Sort\n");
        printf("2 - Executar Merge Sort\n");
        printf("3 - Executar Heap Sort\n");
        printf("4 - Comparar os tres algoritmos\n");
        printf("0 - Sair\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &escolha) != 1) {
            int caractere;
            while ((caractere = getchar()) != '\n' && caractere != EOF) {}
            escolha = -1;
        }

        switch (escolha) {
            case 1:
            case 2:
            case 3:
            case 4: {
                int tamanho = escolherTamanho();
                if (tamanho == 0) break;
                if (escolha == 1) {
                    char entrada[80], saida[100];
                    char *argumentos[3];
                    
                    sprintf(entrada, "output/codigos_%d.txt", tamanho);
                    
                    FILE *teste = fopen(entrada, "r");
                    if (teste != NULL) {
                        fclose(teste);
                    } else {
                        sprintf(entrada, "codigos_%d.txt", tamanho);
                    }

                    sprintf(saida, "bucket_codigos_%d.txt", tamanho);
                    argumentos[0] = (char *)"bucket_sort";
                    argumentos[1] = entrada;
                    argumentos[2] = saida;
                    
                    if (Resultado_buckt_sort(3, argumentos) != EXIT_SUCCESS) {
                        printf("Falha ao executar Bucket Sort para %s.\n", entrada);
                    }
                } else {
                    executar(escolha, tamanho);
                }
                break;
            }
            case 0:
                    printf("Encerrando o programa.\n");
                    break;
                default:
                    printf("Opcao invalida. Escolha 0, 1, 2, 3 ou 4.\n");
            }
        } while (escolha != 0);

        return 0;
}