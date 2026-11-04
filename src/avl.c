/*
 *  Arvore AVL - jogadores FIBA
 *  Chave: nome + nacionalidade + data_nascimento
 *
 *  Compilar:  gcc -O2 -Wall -Wextra -std=c11 -o avl avl.c
 *  Executar:  ./avl          (roda os testes de funcionamento)
 *             ./avl --bench  (mede o tempo de insercao, busca e remocao)
 *             (o arquivo "jogadores.dat" precisa estar na mesma pasta)
 */

#define _POSIX_C_SOURCE 199309L   /* necessario para clock_gettime no Linux/Mac */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#define ARQUIVO_DADOS "../dataset/jogadores.dat"

#define TAM_NOME   120
#define TAM_DATA   30
#define TAM_ALTIMP 20
#define TAM_SIGLA  10
#define TAM_URL    200

/* ---------------------------------------------------------------
 *  Estruturas
 * --------------------------------------------------------------- */

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

/* No da arvore AVL */
typedef struct NoAVL {
    Jogador dado;
    struct NoAVL *esq;
    struct NoAVL *dir;
    int altura;          /* folha = 0, arvore vazia = -1 */
} NoAVL;

/* Conta quantas rotacoes foram feitas (vai ser usado na comparacao
 * com a Rubro-Negra) */
static int total_rotacoes = 0;

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

/* ---------------------------------------------------------------
 *  Balanceamento: altura, fator de balanceamento e rotacoes
 * --------------------------------------------------------------- */

static int maximo(int a, int b) {
    return (a > b) ? a : b;
}

static int altura_no(NoAVL *no) {
    if (no == NULL) return -1;
    return no->altura;
}

static void atualizar_altura(NoAVL *no) {
    no->altura = 1 + maximo(altura_no(no->esq), altura_no(no->dir));
}

/* fator = altura da esquerda - altura da direita (deve ficar entre -1 e 1) */
static int fator_balanceamento(NoAVL *no) {
    if (no == NULL) return 0;
    return altura_no(no->esq) - altura_no(no->dir);
}

static NoAVL* rotacao_direita(NoAVL *y) {
    NoAVL *x  = y->esq;
    NoAVL *t2 = x->dir;

    x->dir = y;
    y->esq = t2;

    atualizar_altura(y);
    atualizar_altura(x);
    total_rotacoes++;
    return x;
}

static NoAVL* rotacao_esquerda(NoAVL *x) {
    NoAVL *y  = x->dir;
    NoAVL *t2 = y->esq;

    y->esq = x;
    x->dir = t2;

    atualizar_altura(x);
    atualizar_altura(y);
    total_rotacoes++;
    return y;
}

/* Atualiza a altura do no e, se estiver desbalanceado, aplica a
 * rotacao certa (esq-esq, esq-dir, dir-dir ou dir-esq). Devolve a
 * nova raiz da subarvore. */
static NoAVL* balancear(NoAVL *no) {
    atualizar_altura(no);
    int fb = fator_balanceamento(no);

    if (fb > 1) {
        if (fator_balanceamento(no->esq) < 0)      /* esquerda-direita */
            no->esq = rotacao_esquerda(no->esq);
        return rotacao_direita(no);                /* esquerda-esquerda */
    }
    if (fb < -1) {
        if (fator_balanceamento(no->dir) > 0)      /* direita-esquerda */
            no->dir = rotacao_direita(no->dir);
        return rotacao_esquerda(no);               /* direita-direita */
    }
    return no;
}

/* ---------------------------------------------------------------
 *  Insercao
 * --------------------------------------------------------------- */

static NoAVL* criar_no(Jogador j) {
    NoAVL *novo = (NoAVL*) malloc(sizeof(NoAVL));
    if (novo == NULL) {
        fprintf(stderr, "Erro: sem memoria para criar no.\n");
        exit(EXIT_FAILURE);
    }
    novo->dado = j;
    novo->esq = NULL;
    novo->dir = NULL;
    novo->altura = 0;
    return novo;
}

NoAVL* avl_inserir(NoAVL *raiz, Jogador j, int *duplicado) {
    if (raiz == NULL) return criar_no(j);

    int cmp = chave_comparar(j.nome, j.nacionalidade, j.data_nascimento,
                              raiz->dado.nome, raiz->dado.nacionalidade,
                              raiz->dado.data_nascimento);

    if (cmp < 0) {
        raiz->esq = avl_inserir(raiz->esq, j, duplicado);
    } else if (cmp > 0) {
        raiz->dir = avl_inserir(raiz->dir, j, duplicado);
    } else {
        if (duplicado != NULL) *duplicado = 1;   /* chave repetida: rejeita */
        return raiz;
    }

    return balancear(raiz);
}

/* ---------------------------------------------------------------
 *  Busca (devolve o no ou NULL se nao achar)
 * --------------------------------------------------------------- */

NoAVL* avl_buscar(NoAVL *raiz, const char *nome, const char *nacionalidade,
                   const char *data_nascimento) {
    while (raiz != NULL) {
        int cmp = chave_comparar(nome, nacionalidade, data_nascimento,
                                  raiz->dado.nome, raiz->dado.nacionalidade,
                                  raiz->dado.data_nascimento);
        if (cmp == 0) return raiz;
        raiz = (cmp < 0) ? raiz->esq : raiz->dir;
    }
    return NULL;
}

/* ---------------------------------------------------------------
 *  Remocao
 * --------------------------------------------------------------- */

static NoAVL* no_minimo(NoAVL *no) {
    while (no->esq != NULL) no = no->esq;
    return no;
}

NoAVL* avl_remover(NoAVL *raiz, const char *nome, const char *nacionalidade,
                    const char *data_nascimento, int *removido) {
    if (raiz == NULL) return NULL;   /* nao achou (ou arvore vazia) */

    int cmp = chave_comparar(nome, nacionalidade, data_nascimento,
                              raiz->dado.nome, raiz->dado.nacionalidade,
                              raiz->dado.data_nascimento);

    if (cmp < 0) {
        raiz->esq = avl_remover(raiz->esq, nome, nacionalidade, data_nascimento, removido);
    } else if (cmp > 0) {
        raiz->dir = avl_remover(raiz->dir, nome, nacionalidade, data_nascimento, removido);
    } else {
        if (removido != NULL) *removido = 1;

        if (raiz->esq == NULL || raiz->dir == NULL) {
            /* zero ou um filho: o filho (ou NULL) ocupa o lugar */
            NoAVL *filho = (raiz->esq != NULL) ? raiz->esq : raiz->dir;
            free(raiz);
            return filho;
        }

        /* dois filhos: troca pelo sucessor (menor da subarvore direita)
         * e depois remove o sucessor la de baixo */
        NoAVL *sucessor = no_minimo(raiz->dir);
        raiz->dado = sucessor->dado;
        raiz->dir = avl_remover(raiz->dir, sucessor->dado.nome,
                                 sucessor->dado.nacionalidade,
                                 sucessor->dado.data_nascimento, NULL);
    }

    return balancear(raiz);
}

/* ---------------------------------------------------------------
 *  Percursos (a funcao "visitar" e chamada em cada jogador)
 * --------------------------------------------------------------- */

void avl_pre_ordem(NoAVL *raiz, void (*visitar)(const Jogador *)) {
    if (raiz == NULL) return;
    visitar(&raiz->dado);
    avl_pre_ordem(raiz->esq, visitar);
    avl_pre_ordem(raiz->dir, visitar);
}

void avl_em_ordem(NoAVL *raiz, void (*visitar)(const Jogador *)) {
    if (raiz == NULL) return;
    avl_em_ordem(raiz->esq, visitar);
    visitar(&raiz->dado);
    avl_em_ordem(raiz->dir, visitar);
}

void avl_pos_ordem(NoAVL *raiz, void (*visitar)(const Jogador *)) {
    if (raiz == NULL) return;
    avl_pos_ordem(raiz->esq, visitar);
    avl_pos_ordem(raiz->dir, visitar);
    visitar(&raiz->dado);
}

/* ---------------------------------------------------------------
 *  Utilidades: altura, contagem, conferencia de balanceamento, free
 * --------------------------------------------------------------- */

int avl_altura(NoAVL *raiz) {
    return altura_no(raiz);
}

int avl_contar_nos(NoAVL *raiz) {
    if (raiz == NULL) return 0;
    return 1 + avl_contar_nos(raiz->esq) + avl_contar_nos(raiz->dir);
}

/* 1 se todos os nos tem fator entre -1 e 1 (e a altura guardada esta certa) */
int avl_esta_balanceada(NoAVL *raiz) {
    if (raiz == NULL) return 1;
    int fb = fator_balanceamento(raiz);
    if (fb < -1 || fb > 1) return 0;
    if (raiz->altura != 1 + maximo(altura_no(raiz->esq), altura_no(raiz->dir))) return 0;
    return avl_esta_balanceada(raiz->esq) && avl_esta_balanceada(raiz->dir);
}

void avl_liberar(NoAVL *raiz) {
    if (raiz == NULL) return;
    avl_liberar(raiz->esq);
    avl_liberar(raiz->dir);
    free(raiz);
}

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

/* Le o arquivo e insere cada jogador na arvore.
 * Devolve quantos foram inseridos (-1 se nao abriu o arquivo).
 * *duplicados recebe quantas linhas tinham chave repetida.
 * *primeiro_lido guarda o primeiro registro (usado nos testes). */
int carregar_arquivo(const char *caminho, NoAVL **raiz, int *duplicados,
                     Jogador *primeiro_lido) {
    FILE *arq = fopen(caminho, "r");
    if (arq == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'.\n", caminho);
        return -1;
    }

    char linha[1024];
    int inseridos = 0;
    int guardou_primeiro = 0;
    *duplicados = 0;

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

        Jogador j;
        copiar_campo(j.nome, TAM_NOME, campos[0]);
        copiar_campo(j.data_nascimento, TAM_DATA, campos[1]);
        j.altura_cm = atoi(campos[2]);
        copiar_campo(j.altura_imperial, TAM_ALTIMP, campos[3]);
        copiar_campo(j.selecao, TAM_SIGLA, campos[4]);
        copiar_campo(j.nacionalidade, TAM_SIGLA, campos[5]);
        copiar_campo(j.url, TAM_URL, campos[6]);

        if (!guardou_primeiro && primeiro_lido != NULL) {
            *primeiro_lido = j;
            guardou_primeiro = 1;
        }

        int dup = 0;
        *raiz = avl_inserir(*raiz, j, &dup);
        if (dup) (*duplicados)++;
        else inseridos++;
    }

    fclose(arq);
    return inseridos;
}

/* ---------------------------------------------------------------
 *  Funcoes auxiliares so para os testes do main
 * --------------------------------------------------------------- */

static int falhas = 0;

static void verificar(const char *descricao, int condicao) {
    printf("  [%s] %s\n", condicao ? "OK" : "FALHOU", descricao);
    if (!condicao) falhas++;
}

/* imprime so os primeiros "limite" jogadores visitados */
static int impressos = 0;
static int limite = 0;

static void imprimir_resumido(const Jogador *j) {
    if (impressos < limite) {
        printf("     %s | %s | %s\n", j->nome, j->nacionalidade, j->data_nascimento);
        impressos++;
    }
}

/* confere se o percurso em-ordem sai crescente pela chave */
static Jogador anterior;
static int tem_anterior = 0;
static int ordem_ok = 1;

static void conferir_ordem(const Jogador *j) {
    if (tem_anterior &&
        chave_comparar(anterior.nome, anterior.nacionalidade, anterior.data_nascimento,
                       j->nome, j->nacionalidade, j->data_nascimento) >= 0) {
        ordem_ok = 0;
    }
    anterior = *j;
    tem_anterior = 1;
}

static void mostrar_primeiros(const char *titulo, void (*percurso)(NoAVL *, void (*)(const Jogador *)),
                              NoAVL *raiz, int quantos) {
    printf("  %s (primeiros %d):\n", titulo, quantos);
    impressos = 0;
    limite = quantos;
    percurso(raiz, imprimir_resumido);
}

/* ---------------------------------------------------------------
 *  BENCHMARK (./avl --bench)
 *  Mede o tempo de lotes de insercao, busca e remocao para
 *  n = 1.000, 10.000 e 90.000 e grava tudo em um CSV.
 *  Mesmo formato/semente da Rubro-Negra, pra comparar as duas.
 * --------------------------------------------------------------- */

#define REPETICOES 5
#define ARQUIVO_CSV "resultados_avl.csv"

/* hora atual em segundos (precisao de microssegundos ou melhor) */
static double agora(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, cont;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&cont);
    return (double) cont.QuadPart / (double) freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double) ts.tv_sec + (double) ts.tv_nsec / 1e9;
#endif
}

/* gerador proprio (mesma sequencia em qualquer computador/compilador),
 * pra AVL e Rubro-Negra receberem os jogadores na MESMA ordem */
static unsigned long long semente = 42;

static unsigned int proximo_aleatorio(void) {
    semente = semente * 6364136223846793005ULL + 1442695040888963407ULL;
    return (unsigned int) (semente >> 33);
}

static void embaralhar(Jogador *v, int n) {
    for (int i = n - 1; i > 0; i--) {
        int k = (int) (proximo_aleatorio() % (unsigned int) (i + 1));
        Jogador t = v[i];
        v[i] = v[k];
        v[k] = t;
    }
}

/* Le o arquivo inteiro para um vetor (devolve NULL se der erro) */
static Jogador* carregar_vetor(const char *caminho, int *quantidade) {
    FILE *arq = fopen(caminho, "r");
    if (arq == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'.\n", caminho);
        return NULL;
    }

    int capacidade = 1024, total = 0;
    Jogador *v = (Jogador*) malloc((size_t) capacidade * sizeof(Jogador));
    if (v == NULL) { fclose(arq); return NULL; }

    char linha[1024];
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

        if (total == capacidade) {
            capacidade *= 2;
            Jogador *novo = (Jogador*) realloc(v, (size_t) capacidade * sizeof(Jogador));
            if (novo == NULL) { free(v); fclose(arq); return NULL; }
            v = novo;
        }

        Jogador *j = &v[total++];
        copiar_campo(j->nome, TAM_NOME, campos[0]);
        copiar_campo(j->data_nascimento, TAM_DATA, campos[1]);
        j->altura_cm = atoi(campos[2]);
        copiar_campo(j->altura_imperial, TAM_ALTIMP, campos[3]);
        copiar_campo(j->selecao, TAM_SIGLA, campos[4]);
        copiar_campo(j->nacionalidade, TAM_SIGLA, campos[5]);
        copiar_campo(j->url, TAM_URL, campos[6]);
    }

    fclose(arq);
    *quantidade = total;
    return v;
}

/* A AVL nao recolore nos, entao a coluna "recoloracoes" fica sempre 0
 * (mantida so pro CSV ter o mesmo formato do da Rubro-Negra). */
static void registrar(FILE *csv, const char *operacao, int n, int ops, double tempo_medio,
                      int altura, int rotacoes, int recoloracoes) {
    double por_op_us = tempo_medio / (double) ops * 1e6;
    fprintf(csv, "avl,%s,%d,%d,%.6f,%.3f,%d,%d,%d\n",
            operacao, n, ops, tempo_medio, por_op_us, altura, rotacoes, recoloracoes);
    printf("  %-9s | n=%-6d | %6d ops | total %9.6f s | %8.3f us/op | altura %2d | rot %6d | recol %6d\n",
           operacao, n, ops, tempo_medio, por_op_us, altura, rotacoes, recoloracoes);
}

static int executar_benchmark(void) {
    int total = 0;
    Jogador *lista = carregar_vetor(ARQUIVO_DADOS, &total);
    if (lista == NULL || total == 0) {
        fprintf(stderr, "Nao foi possivel ler registros de '%s'.\n", ARQUIVO_DADOS);
        free(lista);
        return EXIT_FAILURE;
    }
    embaralhar(lista, total);        /* ordem aleatoria, igual nas duas arvores */

    FILE *csv = fopen(ARQUIVO_CSV, "w");
    if (csv == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s'.\n", ARQUIVO_CSV);
        free(lista);
        return EXIT_FAILURE;
    }
    fprintf(csv, "arvore,operacao,n,operacoes,tempo_total_s,tempo_por_op_us,altura,rotacoes,recoloracoes\n");

    printf("Benchmark da AVL (%d repeticoes por tamanho, %d registros no arquivo)\n\n",
           REPETICOES, total);

    int tamanhos[] = {1000, 10000, 90000};
    int qtd_tamanhos = (int) (sizeof(tamanhos) / sizeof(tamanhos[0]));

    for (int t = 0; t < qtd_tamanhos; t++) {
        int n = tamanhos[t];
        if (n > total) {
            printf("n=%d: pulado (o arquivo so tem %d registros)\n\n", n, total);
            continue;
        }

        int qtd_bus = (n < 1000) ? n : 1000;   /* 1000 que existem + 1000 que nao existem */
        int qtd_rem = n / 10;                  /* remove 10% */

        /* nomes inventados (nao existem na arvore), preparados ANTES de medir */
        char (*falsos)[TAM_NOME + 1] = (char (*)[TAM_NOME + 1]) malloc((size_t) qtd_bus * (TAM_NOME + 1));
        if (falsos == NULL) { fclose(csv); free(lista); return EXIT_FAILURE; }
        for (int i = 0; i < qtd_bus; i++)
            snprintf(falsos[i], TAM_NOME + 1, "#%s", lista[i].nome);

        double t_ins = 0, t_bus = 0, t_rem = 0;
        int altura_cheia = 0, altura_final = 0;
        int rot_ins = 0, rot_rem = 0;
        int erros = 0;

        for (int r = 0; r < REPETICOES; r++) {
            NoAVL *raiz = NULL;
            total_rotacoes = 0;

            /* --- insercao --- */
            double ini = agora();
            for (int i = 0; i < n; i++) {
                int dup = 0;
                raiz = avl_inserir(raiz, lista[i], &dup);
            }
            t_ins += agora() - ini;
            altura_cheia = avl_altura(raiz);
            rot_ins = total_rotacoes;

            /* --- busca (conta os acertos pra o compilador nao cortar o loop) --- */
            int achados = 0, falsos_achados = 0;
            ini = agora();
            for (int i = 0; i < qtd_bus; i++)
                if (avl_buscar(raiz, lista[i].nome, lista[i].nacionalidade,
                               lista[i].data_nascimento) != NULL) achados++;
            for (int i = 0; i < qtd_bus; i++)
                if (avl_buscar(raiz, falsos[i], lista[i].nacionalidade,
                               lista[i].data_nascimento) != NULL) falsos_achados++;
            t_bus += agora() - ini;
            if (achados != qtd_bus || falsos_achados != 0) erros++;

            /* --- remocao --- */
            int apagados = 0;
            ini = agora();
            for (int i = 0; i < qtd_rem; i++) {
                int rem = 0;
                raiz = avl_remover(raiz, lista[i].nome, lista[i].nacionalidade,
                                   lista[i].data_nascimento, &rem);
                apagados += rem;
            }
            t_rem += agora() - ini;
            if (apagados != qtd_rem || !avl_esta_balanceada(raiz)) erros++;
            altura_final = avl_altura(raiz);
            rot_rem = total_rotacoes - rot_ins;

            avl_liberar(raiz);
        }

        printf("n = %d\n", n);
        registrar(csv, "insercao", n, n, t_ins / REPETICOES, altura_cheia, rot_ins, 0);
        registrar(csv, "busca", n, 2 * qtd_bus, t_bus / REPETICOES, altura_cheia, 0, 0);
        registrar(csv, "remocao", n, qtd_rem, t_rem / REPETICOES, altura_final, rot_rem, 0);
        if (erros > 0) printf("  ATENCAO: %d verificacao(oes) falharam neste tamanho!\n", erros);
        printf("\n");

        free(falsos);
    }

    fclose(csv);
    free(lista);
    printf("Resultados gravados em '%s'.\n", ARQUIVO_CSV);
    return EXIT_SUCCESS;
}

/* ---------------------------------------------------------------
 *  main: carrega o dataset e testa cada operacao
 * --------------------------------------------------------------- */

int main(int argc, char *argv[]) {
    NoAVL *raiz = NULL;
    Jogador primeiro;
    int duplicado, removido;

    if (argc == 2 && strcmp(argv[1], "--bench") == 0)
        return executar_benchmark();
    if (argc > 1) {
        fprintf(stderr, "Uso: %s [--bench]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("=====================================================\n");
    printf("  ARVORE AVL - JOGADORES FIBA\n");
    printf("=====================================================\n\n");

    /* --- arvore vazia --- */
    printf("Arvore vazia:\n");
    verificar("busca em arvore vazia devolve NULL",
              avl_buscar(raiz, "Fulano", "BRA", "Jan 1, 2000") == NULL);
    removido = 0;
    raiz = avl_remover(raiz, "Fulano", "BRA", "Jan 1, 2000", &removido);
    verificar("remocao em arvore vazia nao remove nada", removido == 0 && raiz == NULL);
    verificar("altura = -1 e contagem = 0",
              avl_altura(raiz) == -1 && avl_contar_nos(raiz) == 0);

    /* --- insercao (carregando o dataset) --- */
    printf("\nInserindo o dataset ('%s'):\n", ARQUIVO_DADOS);
    int dups = 0;
    int total = carregar_arquivo(ARQUIVO_DADOS, &raiz, &dups, &primeiro);
    if (total < 0) return EXIT_FAILURE;

    printf("  inseridos: %d | chaves repetidas ignoradas: %d\n", total, dups);
    printf("  altura da arvore: %d | rotacoes feitas: %d\n", avl_altura(raiz), total_rotacoes);
    verificar("contagem de nos = registros inseridos", avl_contar_nos(raiz) == total);
    verificar("arvore balanceada apos as insercoes", avl_esta_balanceada(raiz));

    /* --- chave duplicada --- */
    printf("\nChave duplicada:\n");
    duplicado = 0;
    raiz = avl_inserir(raiz, primeiro, &duplicado);
    verificar("reinserir o primeiro registro e rejeitado", duplicado == 1);
    verificar("contagem nao mudou", avl_contar_nos(raiz) == total);

    /* --- busca --- */
    printf("\nBusca:\n");
    NoAVL *achado = avl_buscar(raiz, primeiro.nome, primeiro.nacionalidade,
                               primeiro.data_nascimento);
    verificar("acha um jogador que existe", achado != NULL);
    if (achado != NULL) jogador_imprimir(&achado->dado);
    verificar("nao acha um jogador que nao existe",
              avl_buscar(raiz, "Fulano Inexistente", "XXX", "N/A") == NULL);

    /* --- percursos --- */
    printf("\nPercursos:\n");
    mostrar_primeiros("em-ordem", avl_em_ordem, raiz, 3);
    mostrar_primeiros("pre-ordem", avl_pre_ordem, raiz, 3);
    mostrar_primeiros("pos-ordem", avl_pos_ordem, raiz, 3);
    tem_anterior = 0;
    ordem_ok = 1;
    avl_em_ordem(raiz, conferir_ordem);
    verificar("em-ordem sai em ordem crescente da chave", ordem_ok);

    /* --- remocao --- */
    printf("\nRemocao:\n");
    removido = 0;
    raiz = avl_remover(raiz, primeiro.nome, primeiro.nacionalidade,
                       primeiro.data_nascimento, &removido);
    verificar("remove um jogador que existe", removido == 1);
    verificar("contagem diminuiu em 1", avl_contar_nos(raiz) == total - 1);
    verificar("jogador removido nao e mais encontrado",
              avl_buscar(raiz, primeiro.nome, primeiro.nacionalidade,
                         primeiro.data_nascimento) == NULL);
    verificar("arvore continua balanceada", avl_esta_balanceada(raiz));

    removido = 0;
    raiz = avl_remover(raiz, primeiro.nome, primeiro.nacionalidade,
                       primeiro.data_nascimento, &removido);
    verificar("remover chave que nao existe nao altera nada",
              removido == 0 && avl_contar_nos(raiz) == total - 1);

    /* remove varios jogadores seguidos (forca rebalanceamentos) */
    int restantes = total - 1;
    int apagados = 0;
    NoAVL *p = raiz;
    while (p != NULL && apagados < 2000) {      /* sempre remove a raiz atual */
        Jogador alvo = p->dado;
        int ok = 0;
        raiz = avl_remover(raiz, alvo.nome, alvo.nacionalidade, alvo.data_nascimento, &ok);
        if (ok) { apagados++; restantes--; }
        p = raiz;
    }
    verificar("apos remover 2000 raizes, contagem confere", avl_contar_nos(raiz) == restantes);
    verificar("e a arvore continua balanceada", avl_esta_balanceada(raiz));

    /* --- esvaziando --- */
    avl_liberar(raiz);
    raiz = NULL;

    printf("\n=====================================================\n");
    if (falhas == 0) printf("  Todos os testes passaram.\n");
    else             printf("  %d teste(s) falharam.\n", falhas);
    printf("=====================================================\n");

    return (falhas == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
