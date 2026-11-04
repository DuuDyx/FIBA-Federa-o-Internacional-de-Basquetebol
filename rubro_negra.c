/*
 *  Arvore Rubro-Negra - jogadores FIBA
 *  Chave: nome + nacionalidade + data_nascimento
 *
 *  Compilar:  gcc -Wall -Wextra -std=c11 -o rubro_negra rubro_negra.c
 *  Executar:  ./rubro_negra          (usa todos os registros do arquivo)
 *             ./rubro_negra 5000     (usa so os 5000 primeiros registros)
 *             ./rubro_negra --bench  (mede o tempo de insercao, busca e remocao)
 *             (o arquivo "jogadores.dat" precisa estar na mesma pasta)
 */

#define _POSIX_C_SOURCE 199309L   /* necessario para clock_gettime no Linux/Mac */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#define ARQUIVO_DADOS "jogadores.dat"

#define TAM_NOME   120
#define TAM_DATA   30
#define TAM_ALTIMP 20
#define TAM_SIGLA  10
#define TAM_URL    200

/* ---------------------------------------------------------------
 *  Estruturas
 * --------------------------------------------------------------- */

/* Dados de um jogador (o "conteudo" guardado em cada no da arvore) */
typedef struct {
    char nome[TAM_NOME];
    char data_nascimento[TAM_DATA];
    int  altura_cm;
    char altura_imperial[TAM_ALTIMP];
    char selecao[TAM_SIGLA];
    char nacionalidade[TAM_SIGLA];
    char url[TAM_URL];
} Jogador;

/* Cor do no */
typedef enum {
    VERMELHO,
    PRETO
} Cor;

/* No da arvore Rubro-Negra (folhas sao NULL e contam como pretas) */
typedef struct NoRB {
    Jogador dado;
    Cor cor;
    struct NoRB *esq;
    struct NoRB *dir;
    struct NoRB *pai;
} NoRB;

/* Contadores para comparar com a AVL */
static int total_rotacoes = 0;
static int total_recoloracoes = 0;

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
 *  Auxiliares: cor e rotacoes
 * --------------------------------------------------------------- */

/* folha (NULL) conta como preta */
static Cor cor_de(NoRB *no) {
    return (no == NULL) ? PRETO : no->cor;
}

static NoRB* rotacao_esquerda(NoRB *raiz, NoRB *x) {
    NoRB *y = x->dir;

    x->dir = y->esq;
    if (y->esq != NULL) y->esq->pai = x;

    y->pai = x->pai;
    if (x->pai == NULL)          raiz = y;
    else if (x == x->pai->esq)   x->pai->esq = y;
    else                         x->pai->dir = y;

    y->esq = x;
    x->pai = y;
    total_rotacoes++;
    return raiz;
}

static NoRB* rotacao_direita(NoRB *raiz, NoRB *x) {
    NoRB *y = x->esq;

    x->esq = y->dir;
    if (y->dir != NULL) y->dir->pai = x;

    y->pai = x->pai;
    if (x->pai == NULL)          raiz = y;
    else if (x == x->pai->dir)   x->pai->dir = y;
    else                         x->pai->esq = y;

    y->dir = x;
    x->pai = y;
    total_rotacoes++;
    return raiz;
}

/* ---------------------------------------------------------------
 *  Insercao
 *  *duplicado vira 1 se a chave ja existia (nao insere e a arvore
 *  fica igual). O chamador deve iniciar *duplicado com 0.
 * --------------------------------------------------------------- */

static NoRB* criar_no(Jogador j) {
    NoRB *novo = (NoRB*) malloc(sizeof(NoRB));
    if (novo == NULL) {
        fprintf(stderr, "Erro: sem memoria para criar no.\n");
        exit(EXIT_FAILURE);
    }
    novo->dado = j;
    novo->cor = VERMELHO;      /* todo no novo nasce vermelho */
    novo->esq = NULL;
    novo->dir = NULL;
    novo->pai = NULL;
    return novo;
}

/* Conserta as regras depois de inserir o no z (pai vermelho + filho vermelho) */
static NoRB* corrigir_insercao(NoRB *raiz, NoRB *z) {
    while (z != raiz && z->pai->cor == VERMELHO) {
        NoRB *pai = z->pai;
        NoRB *avo = pai->pai;      /* existe: pai e vermelho, entao nao e a raiz */

        if (pai == avo->esq) {
            NoRB *tio = avo->dir;
            if (cor_de(tio) == VERMELHO) {
                /* caso 1: tio vermelho -> so recolore e sobe */
                pai->cor = PRETO;
                tio->cor = PRETO;
                avo->cor = VERMELHO;
                total_recoloracoes++;
                z = avo;
            } else {
                if (z == pai->dir) {         /* caso 2: vira caso 3 */
                    z = pai;
                    raiz = rotacao_esquerda(raiz, z);
                    pai = z->pai;
                }
                pai->cor = PRETO;            /* caso 3 */
                avo->cor = VERMELHO;
                raiz = rotacao_direita(raiz, avo);
            }
        } else {
            NoRB *tio = avo->esq;
            if (cor_de(tio) == VERMELHO) {
                pai->cor = PRETO;
                tio->cor = PRETO;
                avo->cor = VERMELHO;
                total_recoloracoes++;
                z = avo;
            } else {
                if (z == pai->esq) {
                    z = pai;
                    raiz = rotacao_direita(raiz, z);
                    pai = z->pai;
                }
                pai->cor = PRETO;
                avo->cor = VERMELHO;
                raiz = rotacao_esquerda(raiz, avo);
            }
        }
    }
    raiz->cor = PRETO;             /* a raiz sempre e preta */
    return raiz;
}

NoRB* rb_inserir(NoRB *raiz, Jogador j, int *duplicado) {
    NoRB *pai = NULL;
    NoRB *atual = raiz;
    int cmp = 0;

    while (atual != NULL) {
        pai = atual;
        cmp = chave_comparar(j.nome, j.nacionalidade, j.data_nascimento,
                              atual->dado.nome, atual->dado.nacionalidade,
                              atual->dado.data_nascimento);
        if (cmp == 0) {
            if (duplicado != NULL) *duplicado = 1;   /* chave repetida: rejeita */
            return raiz;
        }
        atual = (cmp < 0) ? atual->esq : atual->dir;
    }

    NoRB *novo = criar_no(j);
    novo->pai = pai;
    if (pai == NULL)     raiz = novo;
    else if (cmp < 0)    pai->esq = novo;
    else                 pai->dir = novo;

    return corrigir_insercao(raiz, novo);
}

/* ---------------------------------------------------------------
 *  Busca (devolve o no ou NULL se nao achar)
 * --------------------------------------------------------------- */

NoRB* rb_buscar(NoRB *raiz, const char *nome, const char *nacionalidade,
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
 *  *removido vira 1 se achou e removeu. O chamador deve iniciar
 *  *removido com 0. Se a chave nao existe, a arvore fica igual.
 * --------------------------------------------------------------- */

static NoRB* no_minimo(NoRB *no) {
    while (no->esq != NULL) no = no->esq;
    return no;
}

/* x = no que ficou no lugar do removido (pode ser NULL), xpai = pai dele.
 * Resolve o "preto a menos" que a remocao de um no preto deixa. */
static NoRB* corrigir_remocao(NoRB *raiz, NoRB *x, NoRB *xpai) {
    while (x != raiz && cor_de(x) == PRETO) {
        if (xpai == NULL) break;

        if (x == xpai->esq) {
            NoRB *w = xpai->dir;                     /* irmao */
            if (cor_de(w) == VERMELHO) {             /* caso 1: irmao vermelho */
                w->cor = PRETO;
                xpai->cor = VERMELHO;
                raiz = rotacao_esquerda(raiz, xpai);
                w = xpai->dir;
            }
            if (cor_de(w->esq) == PRETO && cor_de(w->dir) == PRETO) {
                w->cor = VERMELHO;                   /* caso 2: sobrinhos pretos */
                total_recoloracoes++;
                x = xpai;
                xpai = x->pai;
            } else {
                if (cor_de(w->dir) == PRETO) {       /* caso 3 */
                    w->esq->cor = PRETO;
                    w->cor = VERMELHO;
                    raiz = rotacao_direita(raiz, w);
                    w = xpai->dir;
                }
                w->cor = xpai->cor;                  /* caso 4 */
                xpai->cor = PRETO;
                w->dir->cor = PRETO;
                raiz = rotacao_esquerda(raiz, xpai);
                x = raiz;
                xpai = NULL;
            }
        } else {
            NoRB *w = xpai->esq;
            if (cor_de(w) == VERMELHO) {
                w->cor = PRETO;
                xpai->cor = VERMELHO;
                raiz = rotacao_direita(raiz, xpai);
                w = xpai->esq;
            }
            if (cor_de(w->esq) == PRETO && cor_de(w->dir) == PRETO) {
                w->cor = VERMELHO;
                total_recoloracoes++;
                x = xpai;
                xpai = x->pai;
            } else {
                if (cor_de(w->esq) == PRETO) {
                    w->dir->cor = PRETO;
                    w->cor = VERMELHO;
                    raiz = rotacao_esquerda(raiz, w);
                    w = xpai->esq;
                }
                w->cor = xpai->cor;
                xpai->cor = PRETO;
                w->esq->cor = PRETO;
                raiz = rotacao_direita(raiz, xpai);
                x = raiz;
                xpai = NULL;
            }
        }
    }
    if (x != NULL) x->cor = PRETO;
    return raiz;
}

NoRB* rb_remover(NoRB *raiz, const char *nome, const char *nacionalidade,
                  const char *data_nascimento, int *removido) {
    NoRB *z = rb_buscar(raiz, nome, nacionalidade, data_nascimento);
    if (z == NULL) return raiz;          /* nao achou (ou arvore vazia) */

    if (removido != NULL) *removido = 1;

    /* y = no que sai de verdade da arvore (tem no maximo 1 filho).
     * Com dois filhos, copia o sucessor para z e tira o sucessor. */
    NoRB *y = z;
    if (z->esq != NULL && z->dir != NULL) {
        y = no_minimo(z->dir);
        z->dado = y->dado;
    }

    NoRB *x = (y->esq != NULL) ? y->esq : y->dir;
    NoRB *xpai = y->pai;
    Cor cor_removida = y->cor;

    if (x != NULL) x->pai = y->pai;
    if (y->pai == NULL)           raiz = x;
    else if (y == y->pai->esq)    y->pai->esq = x;
    else                          y->pai->dir = x;
    free(y);

    if (cor_removida == PRETO)
        raiz = corrigir_remocao(raiz, x, xpai);

    return raiz;
}

/* ---------------------------------------------------------------
 *  Percursos (a funcao "visitar" e chamada em cada jogador)
 * --------------------------------------------------------------- */

void rb_pre_ordem(NoRB *raiz, void (*visitar)(const Jogador *)) {
    if (raiz == NULL) return;
    visitar(&raiz->dado);
    rb_pre_ordem(raiz->esq, visitar);
    rb_pre_ordem(raiz->dir, visitar);
}

void rb_em_ordem(NoRB *raiz, void (*visitar)(const Jogador *)) {
    if (raiz == NULL) return;
    rb_em_ordem(raiz->esq, visitar);
    visitar(&raiz->dado);
    rb_em_ordem(raiz->dir, visitar);
}

void rb_pos_ordem(NoRB *raiz, void (*visitar)(const Jogador *)) {
    if (raiz == NULL) return;
    rb_pos_ordem(raiz->esq, visitar);
    rb_pos_ordem(raiz->dir, visitar);
    visitar(&raiz->dado);
}

/* ---------------------------------------------------------------
 *  Utilidades: altura, contagem, validacao das regras, free
 * --------------------------------------------------------------- */

/* folha = 0, arvore vazia = -1 (mesmo criterio da AVL) */
int rb_altura(NoRB *raiz) {
    if (raiz == NULL) return -1;
    int he = rb_altura(raiz->esq);
    int hd = rb_altura(raiz->dir);
    return 1 + ((he > hd) ? he : hd);
}

int rb_contar_nos(NoRB *raiz) {
    if (raiz == NULL) return 0;
    return 1 + rb_contar_nos(raiz->esq) + rb_contar_nos(raiz->dir);
}

/* Devolve a altura negra da subarvore, ou -1 se alguma regra foi violada:
 * - vermelho nao pode ter filho vermelho
 * - todo caminho ate as folhas tem a mesma quantidade de nos pretos
 * - os ponteiros "pai" precisam estar certos */
static int validar_no(NoRB *no) {
    if (no == NULL) return 1;

    if (no->esq != NULL && no->esq->pai != no) return -1;
    if (no->dir != NULL && no->dir->pai != no) return -1;

    if (no->cor == VERMELHO &&
        (cor_de(no->esq) == VERMELHO || cor_de(no->dir) == VERMELHO))
        return -1;

    int he = validar_no(no->esq);
    int hd = validar_no(no->dir);
    if (he < 0 || hd < 0 || he != hd) return -1;

    return he + ((no->cor == PRETO) ? 1 : 0);
}

/* 1 se a arvore respeita todas as regras da Rubro-Negra */
int rb_propriedades_validas(NoRB *raiz) {
    if (raiz == NULL) return 1;
    if (raiz->cor != PRETO || raiz->pai != NULL) return 0;
    return validar_no(raiz) >= 0;
}

void rb_liberar(NoRB *raiz) {
    if (raiz == NULL) return;
    rb_liberar(raiz->esq);
    rb_liberar(raiz->dir);
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

/* Le uma linha do arquivo aceitando \n, \r ou \r\n como quebra de linha */
static int ler_linha(FILE *arq, char *buffer, int tamanho) {
    int i = 0;
    int c;

    while (i < tamanho - 1) {
        c = fgetc(arq);
        if (c == EOF) {
            if (i == 0) return 0;
            break;
        }
        if (c == '\r') {
            int proximo = fgetc(arq);
            if (proximo != '\n' && proximo != EOF) {
                ungetc(proximo, arq);
            }
            break;
        }
        if (c == '\n') {
            break;
        }
        buffer[i++] = (char)c;
    }
    buffer[i] = '\0';
    return 1;
}

/* Le o arquivo e insere cada jogador na arvore.
 * max = quantos registros ler no maximo (0 = le tudo).
 * Devolve quantos foram inseridos (-1 se nao abriu o arquivo).
 * *duplicados recebe quantas linhas tinham chave repetida.
 * *lidos recebe quantos registros validos foram lidos do arquivo.
 * *primeiro_lido guarda o primeiro registro (usado nos testes). */
int carregar_arquivo(const char *caminho, NoRB **raiz, int max, int *duplicados,
                     int *lidos, Jogador *primeiro_lido) {
    FILE *arq = fopen(caminho, "r");
    if (arq == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'.\n", caminho);
        return -1;
    }

    char linha[1024];
    int inseridos = 0;
    int guardou_primeiro = 0;
    *duplicados = 0;
    *lidos = 0;

    while (ler_linha(arq, linha, sizeof(linha))) {
        if (max > 0 && *lidos >= max) break;

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

        (*lidos)++;
        int dup = 0;
        *raiz = rb_inserir(*raiz, j, &dup);
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

static void mostrar_primeiros(const char *titulo,
                              void (*percurso)(NoRB *, void (*)(const Jogador *)),
                              NoRB *raiz, int quantos) {
    printf("  %s (primeiros %d):\n", titulo, quantos);
    impressos = 0;
    limite = quantos;
    percurso(raiz, imprimir_resumido);
}

/* A altura de uma Rubro-Negra com n nos e no maximo 2*log2(n+1).
 * Equivale a (n+1)^2 >= 2^altura (evita usar math.h). */
static int altura_dentro_do_limite(int n, int altura) {
    if (altura < 0) return n == 0;
    if (altura >= 62) return 0;
    long long nn = (long long) n + 1;
    return nn * nn >= (1LL << altura);
}


/* ---------------------------------------------------------------
 *  BENCHMARK (./rubro_negra --bench)
 *  Mede o tempo de lotes de insercao, busca e remocao para
 *  n = 1.000, 10.000 e 90.000 e grava tudo em um CSV.
 * --------------------------------------------------------------- */

#define REPETICOES 5
#define ARQUIVO_CSV "resultados_rubro_negra.csv"

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
    while (ler_linha(arq, linha, sizeof(linha))) {
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

static void registrar(FILE *csv, const char *operacao, int n, int ops, double tempo_medio,
                      int altura, int rotacoes, int recoloracoes) {
    double por_op_us = tempo_medio / (double) ops * 1e6;
    fprintf(csv, "rubro_negra,%s,%d,%d,%.6f,%.3f,%d,%d,%d\n",
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

    printf("Benchmark da Rubro-Negra (%d repeticoes por tamanho, %d registros no arquivo)\n\n",
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
        int rot_ins = 0, rec_ins = 0, rot_rem = 0, rec_rem = 0;
        int erros = 0;

        for (int r = 0; r < REPETICOES; r++) {
            NoRB *raiz = NULL;
            total_rotacoes = 0;
            total_recoloracoes = 0;

            /* --- insercao --- */
            double ini = agora();
            for (int i = 0; i < n; i++) {
                int dup = 0;
                raiz = rb_inserir(raiz, lista[i], &dup);
            }
            t_ins += agora() - ini;
            altura_cheia = rb_altura(raiz);
            rot_ins = total_rotacoes;
            rec_ins = total_recoloracoes;

            /* --- busca (conta os acertos pra o compilador nao cortar o loop) --- */
            int achados = 0, falsos_achados = 0;
            ini = agora();
            for (int i = 0; i < qtd_bus; i++)
                if (rb_buscar(raiz, lista[i].nome, lista[i].nacionalidade,
                              lista[i].data_nascimento) != NULL) achados++;
            for (int i = 0; i < qtd_bus; i++)
                if (rb_buscar(raiz, falsos[i], lista[i].nacionalidade,
                              lista[i].data_nascimento) != NULL) falsos_achados++;
            t_bus += agora() - ini;
            if (achados != qtd_bus || falsos_achados != 0) erros++;

            /* --- remocao --- */
            int apagados = 0;
            ini = agora();
            for (int i = 0; i < qtd_rem; i++) {
                int rem = 0;
                raiz = rb_remover(raiz, lista[i].nome, lista[i].nacionalidade,
                                  lista[i].data_nascimento, &rem);
                apagados += rem;
            }
            t_rem += agora() - ini;
            if (apagados != qtd_rem || !rb_propriedades_validas(raiz)) erros++;
            altura_final = rb_altura(raiz);
            rot_rem = total_rotacoes - rot_ins;
            rec_rem = total_recoloracoes - rec_ins;

            rb_liberar(raiz);
        }

        printf("n = %d\n", n);
        registrar(csv, "insercao", n, n, t_ins / REPETICOES, altura_cheia, rot_ins, rec_ins);
        registrar(csv, "busca", n, 2 * qtd_bus, t_bus / REPETICOES, altura_cheia, 0, 0);
        registrar(csv, "remocao", n, qtd_rem, t_rem / REPETICOES, altura_final, rot_rem, rec_rem);
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
    NoRB *raiz = NULL;
    Jogador primeiro;
    int duplicado, removido;
    int max_registros = 0;           /* 0 = todos */

    if (argc == 2 && strcmp(argv[1], "--bench") == 0)
        return executar_benchmark();

    /* validacao do argumento opcional (quantidade de registros) */
    if (argc > 2) {
        fprintf(stderr, "Uso: %s [quantidade de registros]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        char *fim = NULL;
        long v = strtol(argv[1], &fim, 10);
        if (fim == argv[1] || *fim != '\0' || v <= 0 || v > INT_MAX) {
            fprintf(stderr, "Quantidade invalida: '%s'.\n", argv[1]);
            fprintf(stderr, "Uso: %s [quantidade de registros]  (inteiro positivo)\n", argv[0]);
            return EXIT_FAILURE;
        }
        max_registros = (int) v;
    }

    printf("=====================================================\n");
    printf("  ARVORE RUBRO-NEGRA - JOGADORES FIBA\n");
    printf("=====================================================\n\n");

    /* --- arvore vazia --- */
    printf("Arvore vazia:\n");
    verificar("busca em arvore vazia devolve NULL",
              rb_buscar(raiz, "Fulano", "BRA", "Jan 1, 2000") == NULL);
    removido = 0;
    raiz = rb_remover(raiz, "Fulano", "BRA", "Jan 1, 2000", &removido);
    verificar("remocao em arvore vazia nao remove nada", removido == 0 && raiz == NULL);
    verificar("altura = -1 e contagem = 0",
              rb_altura(raiz) == -1 && rb_contar_nos(raiz) == 0);

    /* --- insercao (carregando o dataset) --- */
    if (max_registros > 0)
        printf("\nInserindo os %d primeiros registros de '%s':\n", max_registros, ARQUIVO_DADOS);
    else
        printf("\nInserindo o dataset ('%s'):\n", ARQUIVO_DADOS);

    int dups = 0, lidos = 0;
    int total = carregar_arquivo(ARQUIVO_DADOS, &raiz, max_registros, &dups, &lidos, &primeiro);
    if (total < 0) return EXIT_FAILURE;
    if (total == 0) {
        fprintf(stderr, "Nenhum registro valido foi lido do arquivo.\n");
        return EXIT_FAILURE;
    }
    if (max_registros > 0 && lidos < max_registros)
        printf("  (o arquivo so tem %d registros; usando todos)\n", lidos);

    printf("  inseridos: %d | chaves repetidas ignoradas: %d\n", total, dups);
    printf("  altura da arvore: %d | rotacoes: %d | recoloracoes: %d\n",
           rb_altura(raiz), total_rotacoes, total_recoloracoes);
    verificar("contagem de nos = registros inseridos", rb_contar_nos(raiz) == total);
    verificar("regras da Rubro-Negra respeitadas apos as insercoes",
              rb_propriedades_validas(raiz));
    verificar("altura dentro do limite 2*log2(n+1)",
              altura_dentro_do_limite(total, rb_altura(raiz)));

    /* --- elemento unico --- */
    printf("\nElemento unico:\n");
    NoRB *unico = NULL;
    duplicado = 0;
    unico = rb_inserir(unico, primeiro, &duplicado);
    verificar("raiz preta, altura 0 e contagem 1",
              unico != NULL && unico->cor == PRETO &&
              rb_altura(unico) == 0 && rb_contar_nos(unico) == 1);
    removido = 0;
    unico = rb_remover(unico, primeiro.nome, primeiro.nacionalidade,
                       primeiro.data_nascimento, &removido);
    verificar("remover o unico elemento deixa a arvore vazia", removido == 1 && unico == NULL);

    /* --- chave duplicada --- */
    printf("\nChave duplicada:\n");
    duplicado = 0;
    raiz = rb_inserir(raiz, primeiro, &duplicado);
    verificar("reinserir o primeiro registro e rejeitado", duplicado == 1);
    verificar("contagem nao mudou", rb_contar_nos(raiz) == total);

    /* --- busca --- */
    printf("\nBusca:\n");
    NoRB *achado = rb_buscar(raiz, primeiro.nome, primeiro.nacionalidade,
                             primeiro.data_nascimento);
    verificar("acha um jogador que existe", achado != NULL);
    if (achado != NULL) jogador_imprimir(&achado->dado);
    verificar("nao acha um jogador que nao existe",
              rb_buscar(raiz, "Fulano Inexistente", "XXX", "N/A") == NULL);

    /* --- percursos --- */
    printf("\nPercursos:\n");
    mostrar_primeiros("em-ordem", rb_em_ordem, raiz, 3);
    mostrar_primeiros("pre-ordem", rb_pre_ordem, raiz, 3);
    mostrar_primeiros("pos-ordem", rb_pos_ordem, raiz, 3);
    tem_anterior = 0;
    ordem_ok = 1;
    rb_em_ordem(raiz, conferir_ordem);
    verificar("em-ordem sai em ordem crescente da chave", ordem_ok);

    /* --- remocao --- */
    printf("\nRemocao:\n");
    removido = 0;
    raiz = rb_remover(raiz, primeiro.nome, primeiro.nacionalidade,
                      primeiro.data_nascimento, &removido);
    verificar("remove um jogador que existe", removido == 1);
    verificar("contagem diminuiu em 1", rb_contar_nos(raiz) == total - 1);
    verificar("jogador removido nao e mais encontrado",
              rb_buscar(raiz, primeiro.nome, primeiro.nacionalidade,
                        primeiro.data_nascimento) == NULL);
    verificar("regras da Rubro-Negra continuam respeitadas", rb_propriedades_validas(raiz));

    removido = 0;
    raiz = rb_remover(raiz, primeiro.nome, primeiro.nacionalidade,
                      primeiro.data_nascimento, &removido);
    verificar("remover chave que nao existe nao altera nada",
              removido == 0 && rb_contar_nos(raiz) == total - 1);

    /* remove varias raizes seguidas (forca rebalanceamentos) */
    int restantes = total - 1;
    int alvo_remocoes = (total / 2 < 2000) ? total / 2 : 2000;
    int apagados = 0;
    int regras_ok = 1;
    while (raiz != NULL && apagados < alvo_remocoes) {
        Jogador alvo = raiz->dado;
        int ok = 0;
        raiz = rb_remover(raiz, alvo.nome, alvo.nacionalidade, alvo.data_nascimento, &ok);
        if (ok) { apagados++; restantes--; }
        if (apagados % 100 == 0 && !rb_propriedades_validas(raiz)) regras_ok = 0;
    }
    printf("  (removidas %d raizes seguidas)\n", apagados);
    verificar("apos remover varias raizes, contagem confere", rb_contar_nos(raiz) == restantes);
    verificar("regras respeitadas durante e depois das remocoes",
              regras_ok && rb_propriedades_validas(raiz));

    rb_liberar(raiz);
    raiz = NULL;

    printf("\n=====================================================\n");
    if (falhas == 0) printf("  Todos os testes passaram.\n");
    else             printf("  %d teste(s) falharam.\n", falhas);
    printf("=====================================================\n");

    return (falhas == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
