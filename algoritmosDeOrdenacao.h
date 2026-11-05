#ifndef ALGORITMOS_DE_ORDENACAO_H
#define ALGORITMOS_DE_ORDENACAO_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

/* ========================================================================== */
/*                       ESTRUTURAS E TIPOS DE DADOS                          */
/* ========================================================================== */

// Estrutura para métricas genéricas (Merge Sort, etc.)
typedef struct {
    unsigned long long qtde_comparacoes;
    unsigned long long qtde_movimentos;
} Metricas;

// Estrutura para métricas do Heap Sort
typedef struct {
    unsigned long long comparacoes;
    unsigned long long movimentos;
} HeapMetricas;

// Ponteiro no Heap Sort
typedef void (*HeapObservador)(int *vetor, size_t quantidade, size_t limite, void *contexto);

// Estrutura utilizada pelo Bucket Sort
typedef struct {
    unsigned int *valores;
    size_t quantidade;
    size_t capacidade;
} Balde;

/* ========================================================================== */
/*                                 MERGE SORT                                 */
/* ========================================================================== */

void merge(int v[], int aux[], int esq, int meio, int dir, Metricas *m) {
    for (int k = esq; k <= dir; k++) {
        aux[k] = v[k];
        m->qtde_movimentos++;
    }

    int i = esq;
    int j = meio + 1;

    for (int k = esq; k <= dir; k++) {
        if (i > meio) { 
            v[k] = aux[j++];
            m->qtde_movimentos++;
        } else if (j > dir) { 
            v[k] = aux[i++];
            m->qtde_movimentos++;
        } else {
            m->qtde_comparacoes++; 
            if (aux[j] < aux[i]) {
                v[k] = aux[j++];
                m->qtde_movimentos++;
            } else {
                v[k] = aux[i++];
                m->qtde_movimentos++;
            }
        }
    }
}

void mergeSort(int v[], int aux[], int esq, int dir, Metricas *m) {
    if (dir <= esq) return;
    
    int meio = esq + (dir - esq) / 2;
    
    mergeSort(v, aux, esq, meio, m);
    mergeSort(v, aux, meio + 1, dir, m);
    merge(v, aux, esq, meio, dir, m);
}

int carrega_txt(const char *nome_txt, int v[], int tamanho) {
    FILE *arquivo = fopen(nome_txt, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s\n", nome_txt);
        return 0; 
    }

    for (int i = 0; i < tamanho; i++) {
        if (fscanf(arquivo, "%d", &v[i]) != 1) {
            printf("Erro ao ler o item %d do arquivo %s\n", i, nome_txt);
            fclose(arquivo);
            return 0;
        }
    }

    fclose(arquivo);
    return 1; 
}

int Resultado_merge_sort() {
    int tamanhos[] = {500, 5000, 50000, 500000, 5000000};
    int num_arquivos = sizeof(tamanhos) / sizeof(tamanhos[0]);
    char nome_txt[50];

    printf("========================================================\n");
    printf(" RELATORIO DE DESEMPENHO - MERGE SORT \n");
    printf("========================================================\n\n");

    for (int i = 0; i < num_arquivos; i++) {
        int N = tamanhos[i];
        sprintf(nome_txt, "codigos_%d.txt", N);

        int *v = (int *)malloc(N * sizeof(int));
        int *aux = (int *)malloc(N * sizeof(int));

        if (v == NULL || aux == NULL) {
            printf("Erro de alocacao de memoria para N=%d\n", N);
            return 1;
        }

        if (!carrega_txt(nome_txt, v, N)) {
            free(v); free(aux);
            continue;
        }

        Metricas metricas = {0, 0};

        long memoria_extra_bytes = N * sizeof(int);
        double memoria_extra_mb = (double)memoria_extra_bytes / (1024 * 1024);

        clock_t inicio = clock();
        mergeSort(v, aux, 0, N - 1, &metricas);
        clock_t fim = clock();
        double tempo_gasto = (double)(fim - inicio) / CLOCKS_PER_SEC;

        printf("Arquivo: %s\n", nome_txt);
        printf("- Tamanho da Entrada        : %d codigos\n", N);
        printf("- Tempo de Execucao         : %f segundos\n", tempo_gasto);
        printf("- Quantidade de comparacoes : %llu\n", metricas.qtde_comparacoes);
        printf("- Quantidade de movimentos  : %llu\n", metricas.qtde_movimentos);
        if (memoria_extra_mb >= 1.0) {
            printf("- Memoria Extra Usada: %.2f MB\n", memoria_extra_mb);
        } else {
            printf("- Memoria Extra Usada: %ld Bytes (%.2f KB)\n", memoria_extra_bytes, (double)memoria_extra_bytes / 1024);
        }
        printf("--------------------------------------------------------\n");

        free(v);
        free(aux);
    }

    return 0;
}

/* ========================================================================== */
/*                                BUCKET SORT                                 */
/* ========================================================================== */

static unsigned long long comparacoesBucket = 0;

unsigned long long bucketSortQuantidadeComparacoes(void)
{
    return comparacoesBucket;
}

size_t bucketSortMemoriaExtra(size_t quantidadeBaldes, const size_t quantidades[])
{
    size_t memoria = quantidadeBaldes * (sizeof(Balde) + sizeof(size_t));
    size_t i;

    for (i = 0; i < quantidadeBaldes; i++) {
        size_t capacidade = 0;
        if (quantidades[i] > 0) {
            capacidade = 8;
            while (capacidade < quantidades[i]) {
                if (capacidade > SIZE_MAX / 2) {
                    return SIZE_MAX;
                }
                capacidade *= 2;
            }
            if (capacidade > (SIZE_MAX - memoria) / sizeof(unsigned int)) {
                return SIZE_MAX;
            }
            memoria += capacidade * sizeof(unsigned int);
        }
    }

    return memoria;
}

static size_t calcularQuantidadeBaldes(size_t n)
{
    size_t b = 1;

    if (n <= 1) {
        return 1;
    }

    while (b < n / b || (b == n / b && n % b != 0)) {
        if (b == SIZE_MAX) {
            return n;
        }
        ++b;
    }

    return b;
}

static void ordenarBalde(
    unsigned int valores[],
    size_t quantidade,
    unsigned long long *movimentacoes,
    unsigned long long *movimentacoesInternas
)
{
    size_t i;

    for (i = 1; i < quantidade; i++) {
        unsigned int chave = valores[i];
        size_t j = i;

        while (j > 0) {
            comparacoesBucket++;
            if (valores[j - 1] <= chave) {
                break;
            }
            valores[j] = valores[j - 1];
            if (movimentacoes != NULL) {
                (*movimentacoes)++;
            }
            if (movimentacoesInternas != NULL) {
                (*movimentacoesInternas)++;
            }
            j--;
        }

        valores[j] = chave;
        if (movimentacoes != NULL) {
            (*movimentacoes)++;
        }
        if (movimentacoesInternas != NULL) {
            (*movimentacoesInternas)++;
        }
    }
}

static void liberarBaldes(Balde *baldes, size_t quantidadeBaldes)
{
    size_t i;

    if (baldes == NULL) {
        return;
    }

    for (i = 0; i < quantidadeBaldes; i++) {
        free(baldes[i].valores);
    }

    free(baldes);
}

static int adicionarAoBalde(
    Balde *balde,
    unsigned int valor,
    unsigned long long *movimentacoes
)
{
    if (balde->quantidade == balde->capacidade) {
        size_t novaCapacidade;
        unsigned int *temporario;

        if (balde->capacidade == 0) {
            novaCapacidade = 8;
        } else {
            if (balde->capacidade > SIZE_MAX / 2) {
                return 0;
            }
            novaCapacidade = balde->capacidade * 2;
        }

        if (novaCapacidade > SIZE_MAX / sizeof(unsigned int)) {
            return 0;
        }

        temporario = (unsigned int *)realloc(
            balde->valores,
            novaCapacidade * sizeof(unsigned int)
        );

        if (temporario == NULL) {
            return 0;
        }

        balde->valores = temporario;
        balde->capacidade = novaCapacidade;
    }

    balde->valores[balde->quantidade++] = valor;

    if (movimentacoes != NULL) {
        (*movimentacoes)++;
    }

    return 1;
}

int bucketSort(
    unsigned int *vetor,
    size_t n,
    unsigned long long *movimentacoes,
    unsigned long long *movimentacoesInternas,
    size_t *quantidadeBaldes,
    size_t **quantidadesPorBalde
)
{
    Balde *baldes;
    unsigned int menor, maior;
    size_t i, indiceBalde, posicao = 0;

    if (movimentacoes != NULL) {
        *movimentacoes = 0;
    }
    if (movimentacoesInternas != NULL) {
        *movimentacoesInternas = 0;
    }
    comparacoesBucket = 0;

    if (quantidadeBaldes == NULL || quantidadesPorBalde == NULL) {
        return 0;
    }

    *quantidadeBaldes = 0;
    *quantidadesPorBalde = NULL;

    if (n == 0) {
        return 1;
    }

    if (vetor == NULL) {
        return 0;
    }

    menor = maior = vetor[0];

    for (i = 1; i < n; i++) {
        if (vetor[i] < menor) menor = vetor[i];
        if (vetor[i] > maior) maior = vetor[i];
    }

    *quantidadeBaldes = calcularQuantidadeBaldes(n);

    if (*quantidadeBaldes > SIZE_MAX / sizeof(Balde) ||
        *quantidadeBaldes > SIZE_MAX / sizeof(size_t)) {
        *quantidadeBaldes = 0;
        return 0;
    }

    baldes = (Balde *)calloc(*quantidadeBaldes, sizeof(Balde));
    if (baldes == NULL) {
        *quantidadeBaldes = 0;
        return 0;
    }

    *quantidadesPorBalde = (size_t *)calloc(
        *quantidadeBaldes,
        sizeof(size_t)
    );

    if (*quantidadesPorBalde == NULL) {
        free(baldes);
        *quantidadeBaldes = 0;
        return 0;
    }

    for (i = 0; i < n; i++) {
        indiceBalde = 0;

        if (maior != menor) {
            long double proporcao =
                ((long double)vetor[i] - (long double)menor) /
                ((long double)maior - (long double)menor);

            indiceBalde = (size_t)(
                proporcao * (long double)(*quantidadeBaldes - 1)
            );

            if (indiceBalde >= *quantidadeBaldes) {
                indiceBalde = *quantidadeBaldes - 1;
            }
        }

        if (!adicionarAoBalde(
                &baldes[indiceBalde],
                vetor[i],
                movimentacoes
            )) {
            liberarBaldes(baldes, *quantidadeBaldes);
            free(*quantidadesPorBalde);
            *quantidadesPorBalde = NULL;
            *quantidadeBaldes = 0;
            return 0;
        }
    }

    for (i = 0; i < *quantidadeBaldes; i++) {
        (*quantidadesPorBalde)[i] = baldes[i].quantidade;

        if (baldes[i].quantidade > 1) {
            ordenarBalde(baldes[i].valores, baldes[i].quantidade,
                         movimentacoes, movimentacoesInternas);
        }
    }

    for (i = 0; i < *quantidadeBaldes; i++) {
        size_t j;

        for (j = 0; j < baldes[i].quantidade; j++) {
            vetor[posicao++] = baldes[i].valores[j];

            if (movimentacoes != NULL) {
                (*movimentacoes)++;
            }
        }
    }

    liberarBaldes(baldes, *quantidadeBaldes);
    return 1;
}

static void formatarNumeroLocal(unsigned long long valor, char *destino, size_t capacidade)
{
    char digitos[32];
    size_t quantidade = 0, posicao = 0;

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

int Resultado_buckt_sort(int argc, char *argv[])
{
    FILE *arquivoSaida = NULL;
    int *tempInt = NULL;
    unsigned int *numeros = NULL;
    size_t quantidade = 0;
    size_t i;
    size_t quantidadeBaldes = 0;
    size_t *quantidadesPorBalde = NULL;
    unsigned long long movimentacoes = 0;
    unsigned long long movimentacoesInternas = 0;
    clock_t inicio, fim;
    double tempo;
    size_t menorQuantidade = 0;
    size_t maiorQuantidade = 0;
    size_t baldesVazios = 0;
    size_t baldesOcupados = 0;
    size_t totalDistribuido = 0;
    double mediaPorBalde = 0.0;
    size_t memoriaExtraBytes = 0;
    char quantidadeTexto[32], movimentosTexto[32], movimentosForaTexto[32];
    char movimentosDentroTexto[32], comparacoesTexto[32];

    if (argc != 3) {
        fprintf(stderr, "Uso: %s arquivo_entrada arquivo_saida\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (sscanf(argv[1], "%*[^0-9]%zu", &quantidade) != 1) {
        const char *nome = strrchr(argv[1], '/');
        if (!nome) nome = strrchr(argv[1], '\\');
        nome = (nome != NULL) ? nome + 1 : argv[1];
        
        if (sscanf(nome, "codigos_%zu.txt", &quantidade) != 1) {
            fprintf(stderr, "Erro ao identificar a quantidade de numeros pelo nome do arquivo.\n");
            return EXIT_FAILURE;
        }
    }

    if (quantidade == 0) {
        fprintf(stderr, "Quantidade invalida de elementos.\n");
        return EXIT_FAILURE;
    }

    tempInt = (int *)malloc(quantidade * sizeof(int));
    numeros = (unsigned int *)malloc(quantidade * sizeof(unsigned int));

    if (tempInt == NULL || numeros == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        free(tempInt);
        free(numeros);
        return EXIT_FAILURE;
    }

    if (!carrega_txt(argv[1], tempInt, (int)quantidade)) {
        const char *nomeSimples = strrchr(argv[1], '/');
        if (!nomeSimples) nomeSimples = strrchr(argv[1], '\\');
        
        if (nomeSimples == NULL || !carrega_txt(nomeSimples + 1, tempInt, (int)quantidade)) {
            free(tempInt);
            free(numeros);
            return EXIT_FAILURE;
        }
    }

    /* Copia os dados convertendo int -> unsigned int para o Bucket Sort */
    for (i = 0; i < quantidade; i++) {
        numeros[i] = (unsigned int)tempInt[i];
    }
    free(tempInt); // Libera o vetor temporário

    printf("========================================\n");
    printf("        ORDENACAO POR BUCKET SORT\n");
    printf("========================================\n");
    printf("Arquivo de entrada: %s\n", argv[1]);
    printf("Quantidade de numeros: %zu\n", quantidade);

    inicio = clock();

    if (!bucketSort(
            numeros,
            quantidade,
            &movimentacoes,
            &movimentacoesInternas,
            &quantidadeBaldes,
            &quantidadesPorBalde)) {
        fprintf(stderr, "Erro durante a ordenacao.\n");
        free(quantidadesPorBalde);
        free(numeros);
        return EXIT_FAILURE;
    }

    fim = clock();
    tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
    memoriaExtraBytes = bucketSortMemoriaExtra(quantidadeBaldes, quantidadesPorBalde);

    if (quantidadeBaldes > 0) {
        menorQuantidade = quantidadesPorBalde[0];

        for (i = 0; i < quantidadeBaldes; i++) {
            size_t qtd = quantidadesPorBalde[i];

            totalDistribuido += qtd;

            if (qtd < menorQuantidade) menorQuantidade = qtd;
            if (qtd > maiorQuantidade) maiorQuantidade = qtd;
            if (qtd == 0) baldesVazios++;
            else baldesOcupados++;
        }

        mediaPorBalde = (double)totalDistribuido / (double)quantidadeBaldes;
    }

    printf("\n========== RESUMO DOS BALDES ==========\n");
    printf("Total de numeros distribuidos: %zu\n", totalDistribuido);
    printf("Total de baldes criados: %zu\n", quantidadeBaldes);
    printf("Media de numeros por balde: %d\n", (int)mediaPorBalde);
    printf("Menor quantidade em um balde: %zu\n", menorQuantidade);
    printf("Maior quantidade em um balde: %zu\n", maiorQuantidade);
    printf("Baldes vazios: %zu\n", baldesVazios);
    printf("Baldes ocupados: %zu\n", baldesOcupados);
    printf("=======================================\n");

    arquivoSaida = fopen(argv[2], "w");
    if (arquivoSaida == NULL) {
        perror("Erro ao criar o arquivo de saida");
        free(quantidadesPorBalde);
        free(numeros);
        return EXIT_FAILURE;
    }

    for (i = 0; i < quantidade; i++) {
        if (fprintf(arquivoSaida, "%u\n", numeros[i]) < 0) {
            fprintf(stderr, "Erro ao gravar os numeros no arquivo de saida.\n");
            fclose(arquivoSaida);
            free(quantidadesPorBalde);
            free(numeros);
            return EXIT_FAILURE;
        }
    }

    if (fclose(arquivoSaida) != 0) {
        perror("Erro ao fechar o arquivo de saida");
        free(quantidadesPorBalde);
        free(numeros);
        return EXIT_FAILURE;
    }

    formatarNumeroLocal((unsigned long long)quantidade, quantidadeTexto, sizeof(quantidadeTexto));
    formatarNumeroLocal(movimentacoes, movimentosTexto, sizeof(movimentosTexto));
    formatarNumeroLocal(movimentacoes - movimentacoesInternas, movimentosForaTexto, sizeof(movimentosForaTexto));
    formatarNumeroLocal(movimentacoesInternas, movimentosDentroTexto, sizeof(movimentosDentroTexto));
    formatarNumeroLocal(bucketSortQuantidadeComparacoes(), comparacoesTexto, sizeof(comparacoesTexto));

    printf("\n========== RESULTADO FINAL ==========\n");
    printf("Ordenacao concluida!\n");
    printf("Tempo da ordenacao: %.6f segundos\n", tempo);
    printf("Movimentos fora dos baldes (entrada e saida): %s\n", movimentosForaTexto);
    printf("Movimentos dentro dos baldes (ordenacao): %s\n", movimentosDentroTexto);
    printf("Total de movimentos: %s\n", movimentosTexto);
    printf("Comparacoes na ordenacao dos baldes: %s\n", comparacoesTexto);
    if (memoriaExtraBytes >= 1024 * 1024) {
        printf("Memoria Extra Usada: %.2f MB\n", (double)memoriaExtraBytes / (1024.0 * 1024.0));
    } else {
        printf("Memoria Extra Usada: %zu Bytes (%.2f KB)\n",
               memoriaExtraBytes, (double)memoriaExtraBytes / 1024.0);
    }
    printf("Quantidade de codigos: %s\n", quantidadeTexto);
    printf("Arquivo gerado: %s\n", argv[2]);
    printf("=====================================\n");

    free(quantidadesPorBalde);
    free(numeros);
    return EXIT_SUCCESS;
}

/* ========================================================================== */
/*                                 HEAP SORT                                  */
/* ========================================================================== */

static void ajustar_max_heap(int *vetor, size_t tamanho, size_t raiz,
                             HeapMetricas *metricas)
{
    int valor = vetor[raiz];
    size_t atual = raiz;

    while (atual < tamanho / 2) {
        size_t maior_filho = 2 * atual + 1;

        if (maior_filho + 1 < tamanho) {
            if (metricas != NULL) metricas->comparacoes++;
            if (vetor[maior_filho + 1] > vetor[maior_filho]) {
                ++maior_filho;
            }
        }

        if (metricas != NULL) metricas->comparacoes++;
        if (valor >= vetor[maior_filho]) {
            break;
        }

        vetor[atual] = vetor[maior_filho];
        if (metricas != NULL) metricas->movimentos++;
        atual = maior_filho;
    }

    vetor[atual] = valor;
    if (metricas != NULL) metricas->movimentos++;
}

static void heap_sort_interno(int *vetor, size_t quantidade,
                              HeapObservador observador, void *contexto,
                              HeapMetricas *metricas)
{
    size_t raiz;
    size_t fim;

    if (metricas != NULL) {
        metricas->comparacoes = 0;
        metricas->movimentos = 0;
    }
    if (vetor == NULL || quantidade < 2) {
        return;
    }

    for (raiz = quantidade / 2; raiz > 0; ) {
        --raiz;
        ajustar_max_heap(vetor, quantidade, raiz, metricas);
    }
    if (observador != NULL) {
        observador(vetor, quantidade, quantidade, contexto);
    }

    for (fim = quantidade - 1; fim > 0; --fim) {
        int temporario = vetor[0];
        vetor[0] = vetor[fim];
        vetor[fim] = temporario;
        if (metricas != NULL) metricas->movimentos += 3;

        ajustar_max_heap(vetor, fim, 0, metricas);
        if (observador != NULL) {
            observador(vetor, quantidade, fim, contexto);
        }
    }
}

void heap_sort(int *vetor, size_t quantidade)
{
    heap_sort_interno(vetor, quantidade, NULL, NULL, NULL);
}

void heap_sort_com_passos(int *vetor, size_t quantidade,
                          HeapObservador observador, void *contexto)
{
    heap_sort_interno(vetor, quantidade, observador, contexto, NULL);
}

void heap_sort_com_metricas(int *vetor, size_t quantidade,
                            HeapMetricas *metricas)
{
    heap_sort_interno(vetor, quantidade, NULL, NULL, metricas);
}

int heap_esta_ordenado(const int *vetor, size_t quantidade)
{
    size_t i;

    if (vetor == NULL) {
        return quantidade == 0;
    }

    for (i = 1; i < quantidade; ++i) {
        if (vetor[i - 1] > vetor[i]) {
            return 0;
        }
    }
    return 1;
}


#endif 