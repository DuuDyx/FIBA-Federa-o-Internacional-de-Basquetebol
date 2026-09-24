/*
 *  Compilar:  gcc -Wall -Wextra -std=c11 -o avl avl.c
 *  Executar:  ./avl
 *             (o arquivo "jogadores.dat" precisa estar na mesma pasta)
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO_DADOS "jogadores.dat"

#define TAM_NOME   120
#define TAM_DATA   30
#define TAM_ALTIMP 20
#define TAM_SIGLA  10
#define TAM_URL    200

/* Estruturas */

/* Dados de um jogador (o "conteudo" guardado em cada no da AVL) */
typedef struct {
    char nome[TAM_NOME];
    char data_nascimento[TAM_DATA];
    int  altura_cm;
    char altura_imperial[TAM_ALTIMP];
    char selecao[TAM_SIGLA];
    char nacionalidade[TAM_SIGLA];
    char url[TAM_URL];
} Jogador;

/* No da arvore AVL. */
typedef struct NoAVL {
    Jogador dado;
    struct NoAVL *esq;
    struct NoAVL *dir;
    int altura;
} NoAVL;

/* ---------------------------------------------------------------
 *  Chave composta (nome + nacionalidade + data_nascimento)
 * --------------------------------------------------------------- */
int chave_comparar(const char *nomeA, const char *nacA, const char *nascA,
                    const char *nomeB, const char *nacB, const char *nascB) {
    int cmp = strcmp(nomeA, nomeB);
    if (cmp != 0) return cmp;
    cmp = strcmp(nacA, nacB);
    if (cmp != 0) return cmp;
    return strcmp(nascA, nascB);
}

/* Protótipos das operações da AVL
NoAVL* avl_inserir(NoAVL *raiz, Jogador j, int *duplicado) {
    (void) j;
    if (duplicado != NULL) *duplicado = 0;
    printf("");
    return raiz;
}


NoAVL* avl_remover(NoAVL *raiz, const char *nome, const char *nacionalidade,
                    const char *data_nascimento, int *removido) {
    (void) nome; (void) nacionalidade; (void) data_nascimento;
    if (removido != NULL) *removido = 0;
    printf("");
    return raiz;
}

NoAVL* avl_buscar(NoAVL *raiz, const char *nome, const char *nacionalidade,
                   const char *data_nascimento) {
    (void) raiz; (void) nome; (void) nacionalidade; (void) data_nascimento;
    printf("[TODO Entrega 2] avl_buscar ainda nao implementado.\n");
    return NULL;
}

*/
/* ---------------------------------------------------------------
 *  Utilidades de exibicao / carga de arquivo 
 * --------------------------------------------------------------- */

void jogador_imprimir(const Jogador *j) {
    printf("  Nome............: %s\n", j->nome);
    printf("  Nascimento......: %s\n", j->data_nascimento);
    if (j->altura_cm > 0)
        printf("  Altura..........: %d cm (%s)\n", j->altura_cm, j->altura_imperial);
    else
        printf("  Altura..........: nao informada\n");
    printf("  Selecao (jogou).: %s\n", j->selecao);
    printf("  Nacionalidade...: %s\n", j->nacionalidade);
    printf("  URL.............: %s\n", j->url);
    printf("  ------------------------------------------------------------\n");
}

static void limpar_quebra_linha(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

static void copiar_campo(char *destino, size_t tam, const char *origem) {
    strncpy(destino, origem, tam - 1);
    destino[tam - 1] = '\0';
}

/* Le o arquivo de dados e conta/mostra quantos jogadores existem,
 * SEM inserir nada em uma arvore ainda  Retorna a quantidade de linhas lidas com sucesso. */
int carregar_arquivo_demo(const char *caminho, Jogador *primeiro_lido) {
    FILE *arq = fopen(caminho, "r");
    if (arq == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'.\n", caminho);
        return -1;
    }

    char linha[1024];
    int total = 0;
    int guardou_primeiro = 0;

    while (fgets(linha, sizeof(linha), arq) != NULL) {
        limpar_quebra_linha(linha);
        if (strlen(linha) == 0) continue;

        char *campos[7];
        int n = 0;
        char *tok = strtok(linha, "|");
        while (tok != NULL && n < 7) {
            campos[n++] = tok;
            tok = strtok(NULL, "|");
        }
        if (n < 7) continue;

        if (!guardou_primeiro && primeiro_lido != NULL) {
            copiar_campo(primeiro_lido->nome, TAM_NOME, campos[0]);
            copiar_campo(primeiro_lido->data_nascimento, TAM_DATA, campos[1]);
            primeiro_lido->altura_cm = atoi(campos[2]);
            copiar_campo(primeiro_lido->altura_imperial, TAM_ALTIMP, campos[3]);
            copiar_campo(primeiro_lido->selecao, TAM_SIGLA, campos[4]);
            copiar_campo(primeiro_lido->nacionalidade, TAM_SIGLA, campos[5]);
            copiar_campo(primeiro_lido->url, TAM_URL, campos[6]);
            guardou_primeiro = 1;
        }

        total++;
    }

    fclose(arq);
    return total;
}



int main(void) {
    printf("=====================================================\n");
    printf("  ARVORE AVL - JOGADORES FIBA \n");
    printf("=====================================================\n\n");

    printf("Lendo dataset de '%s'\n", ARQUIVO_DADOS);
    Jogador primeiro;
    int total = carregar_arquivo_demo(ARQUIVO_DADOS, &primeiro);

    if (total < 0) {
        fprintf(stderr, "Nao foi possivel ler o arquivo de dados.\n");
        return EXIT_FAILURE;
    }

    printf("%d linhas lidas com sucesso do dataset.\n\n", total);

    if (total > 0) {
        printf("Exemplo do primeiro registro lido:\n");
        jogador_imprimir(&primeiro);
        printf("\n");
    }

    return EXIT_SUCCESS;
}
