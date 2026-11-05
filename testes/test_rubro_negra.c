/*
 *  Testes dos casos extremos (edge cases) - Arvore Rubro-Negra
 *
 *  Este arquivo INCLUI o rubro_negra.c (sem modificar nada nele) e troca
 *  o main original por este aqui. Por isso os dois arquivos precisam
 *  estar na mesma pasta.
 *
 *  Compilar:  gcc -O2 -Wall -Wextra -std=c11 -o test_rubro_negra test_rubro_negra.c -lm
 *  Executar:  ./test_rubro_negra          (usa 15800 registros no teste de ordem crescente)
 *             ./test_rubro_negra 50000    (usa 50000 registros)
 *
 *  Ao terminar, grava 1 arquivo na pasta atual:
 *    resultados_testes_rubro_negra.csv    (uma linha por verificacao: caso, descricao, OK/FALHOU)
 */

#define _POSIX_C_SOURCE 199309L

#define main main_original      /* esconde o main do rubro_negra.c */
#define CAMINHO_RUBRO "../src/rubro_negra.c"
#include CAMINHO_RUBRO
#undef main
#include <math.h>

/* ---------------------------------------------------------------
 *  Auxiliares dos testes
 * --------------------------------------------------------------- */

/* ---------------------------------------------------------------
 *  Registro dos resultados (tela + CSV)
 *  Cada verificacao fica guardada em memoria e, no final,
 *  e gravada no arquivo resultados_testes_rubro_negra.csv.
 * --------------------------------------------------------------- */

#define T_ARVORE      "rubro_negra"
#define T_TITULO      "Rubro-Negra"
#define T_MAX_CHECKS  400

typedef struct { char caso[100]; char descricao[240]; int ok; } T_Check;

static T_Check   t_res[T_MAX_CHECKS];
static int  t_nres = 0;
static char t_caso_atual[100] = "";

static int t_falhas = 0;
static int t_checks = 0;

static void t_iniciar_caso(const char *titulo) {
    snprintf(t_caso_atual, sizeof(t_caso_atual), "%s", titulo);
    printf("\n[%s]\n", titulo);
}

static void checar(const char *descricao, int condicao) {
    t_checks++;
    printf("  [%s] %s\n", condicao ? "OK" : "FALHOU", descricao);
    if (!condicao) t_falhas++;
    if (t_nres < T_MAX_CHECKS) {
        snprintf(t_res[t_nres].caso, sizeof(t_res[t_nres].caso), "%s", t_caso_atual);
        snprintf(t_res[t_nres].descricao, sizeof(t_res[t_nres].descricao), "%s", descricao);
        t_res[t_nres].ok = condicao ? 1 : 0;
        t_nres++;
    }
}

/* texto entre aspas para CSV (aspas internas viram "") */
static void t_csv_campo(FILE *f, const char *s) {
    fputc('"', f);
    for (; *s; s++) {
        if (*s == '"') fputc('"', f);
        fputc(*s, f);
    }
    fputc('"', f);
}

static void t_gravar_relatorios(int n) {
    (void)n;
    const char *arq_testes = "resultados_testes_" T_ARVORE ".csv";
    FILE *f;

    /* --- CSV 1: uma linha por verificacao --- */
    f = fopen(arq_testes, "w");
    if (f == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s'.\n", arq_testes);
    } else {
        fprintf(f, "arvore,caso,verificacao,resultado\n");
        for (int i = 0; i < t_nres; i++) {
            fprintf(f, "%s,", T_ARVORE);
            t_csv_campo(f, t_res[i].caso);
            fputc(',', f);
            t_csv_campo(f, t_res[i].descricao);
            fprintf(f, ",%s\n", t_res[i].ok ? "OK" : "FALHOU");
        }
        fclose(f);
    }

    printf("\nResultados gravados em:\n  %s\n", arq_testes);
}

/* Jogador de numero i. Com zeros a esquerda, o nome de i fica sempre
 * antes do nome de i+1 (ordem alfabetica = ordem numerica). */
static Jogador t_jogador(int i) {
    Jogador j;
    memset(&j, 0, sizeof(j));
    snprintf(j.nome, TAM_NOME, "Jogador %06d", i);
    snprintf(j.data_nascimento, TAM_DATA, "Jan 1, 2000");
    j.altura_cm = 180 + (i % 40);
    snprintf(j.altura_imperial, TAM_ALTIMP, "5-11");
    snprintf(j.selecao, TAM_SIGLA, "BRA");
    snprintf(j.nacionalidade, TAM_SIGLA, "BRA");
    snprintf(j.url, TAM_URL, "http://fiba.test/jogador/%d", i);
    return j;
}

static NoRB* t_buscar_j(NoRB *raiz, Jogador j) {
    return rb_buscar(raiz, j.nome, j.nacionalidade, j.data_nascimento);
}

/* Remove o jogador j; devolve a nova raiz e grava em *ok se removeu */
static NoRB* t_remover_j(NoRB *raiz, Jogador j, int *ok) {
    *ok = 0;
    return rb_remover(raiz, j.nome, j.nacionalidade, j.data_nascimento, ok);
}

/* visitante que so conta quantos nos foram visitados */
static long t_visitas = 0;
static void t_contar_visita(const Jogador *j) { (void) j; t_visitas++; }

/* visitante que confere se a ordem sai crescente */
static Jogador t_ant;
static int t_tem_ant = 0;
static int t_ordem_ok = 1;
static void t_conferir_ordem(const Jogador *j) {
    if (t_tem_ant &&
        chave_comparar(t_ant.nome, t_ant.nacionalidade, t_ant.data_nascimento,
                       j->nome, j->nacionalidade, j->data_nascimento) >= 0)
        t_ordem_ok = 0;
    t_ant = *j;
    t_tem_ant = 1;
}

static int t_em_ordem_crescente(NoRB *raiz) {
    t_tem_ant = 0;
    t_ordem_ok = 1;
    rb_em_ordem(raiz, t_conferir_ordem);
    return t_ordem_ok;
}

/* Numero de comparacoes que rb_buscar faz = nos visitados no caminho.
 * (mesmo laco do rb_buscar, so que contando) */
static int t_comparacoes_busca(NoRB *raiz, Jogador j) {
    int comps = 0;
    while (raiz != NULL) {
        comps++;
        int cmp = chave_comparar(j.nome, j.nacionalidade, j.data_nascimento,
                                 raiz->dado.nome, raiz->dado.nacionalidade,
                                 raiz->dado.data_nascimento);
        if (cmp == 0) break;
        raiz = (cmp < 0) ? raiz->esq : raiz->dir;
    }
    return comps;
}

/* Limite da Rubro-Negra: altura <= 2 * log2(n + 1) */
static int t_limite_altura(int n) {
    return (int) (2.0 * log2((double) n + 1.0));
}

/* Arvore perfeita com TODOS os nos pretos, montada na mao. E o pior caso
 * da remocao: tirar uma folha deixa "um preto a menos" e, como todos os
 * irmaos e sobrinhos sao pretos, o conserto sobe recolorindo nivel por
 * nivel (a cascata). Chaves em ordem crescente (em-ordem). */
static int t_prox_chave = 0;

static NoRB* t_perfeita_preta(int profundidade) {
    if (profundidade < 0) return NULL;
    NoRB *esq = t_perfeita_preta(profundidade - 1);
    NoRB *no = criar_no(t_jogador(t_prox_chave++));
    NoRB *dir = t_perfeita_preta(profundidade - 1);
    no->cor = PRETO;
    no->esq = esq;
    no->dir = dir;
    if (esq != NULL) esq->pai = no;
    if (dir != NULL) dir->pai = no;
    return no;
}

/* ---------------------------------------------------------------
 *  Caso 1: arvore vazia
 * --------------------------------------------------------------- */
static void caso_arvore_vazia(void) {
    t_iniciar_caso("Caso 1 - Arvore vazia");
    NoRB *raiz = NULL;
    int removido = 0;   /* o chamador inicia com 0 (contrato do rb_remover) */

    checar("buscar devolve NULL",
           rb_buscar(raiz, "Fulano", "BRA", "Jan 1, 2000") == NULL);

    raiz = rb_remover(raiz, "Fulano", "BRA", "Jan 1, 2000", &removido);
    checar("remover nao faz nada (removido = 0 e raiz continua NULL)",
           removido == 0 && raiz == NULL);

    t_visitas = 0;
    rb_pre_ordem(raiz, t_contar_visita);
    rb_em_ordem(raiz, t_contar_visita);
    rb_pos_ordem(raiz, t_contar_visita);
    checar("pre/em/pos-ordem nao visitam nenhum no", t_visitas == 0);

    checar("altura = -1", rb_altura(raiz) == -1);
    checar("contagem de nos = 0", rb_contar_nos(raiz) == 0);
    checar("arvore vazia respeita as propriedades", rb_propriedades_validas(raiz));
    rb_liberar(raiz);   /* liberar NULL nao pode travar */
    checar("rb_liberar(NULL) nao trava", 1);

    /* variacao: arvore que ESVAZIOU depois de ter 1 no */
    int dup = 0;
    raiz = rb_inserir(NULL, t_jogador(1), &dup);
    checar("apos inserir 1 no: a raiz e preta", raiz != NULL && raiz->cor == PRETO);
    raiz = t_remover_j(raiz, t_jogador(1), &removido);
    checar("inserir 1 e remover 1 deixa a arvore vazia (raiz NULL)",
           removido == 1 && raiz == NULL);
    t_visitas = 0;
    rb_em_ordem(raiz, t_contar_visita);
    checar("percurso apos esvaziar nao visita nada", t_visitas == 0);
}

/* ---------------------------------------------------------------
 *  Caso 2: insercao em ordem crescente  (devolve a arvore criada,
 *  que o caso 5 reaproveita)
 * --------------------------------------------------------------- */
static NoRB* caso_ordem_crescente(int n) {
    { char t[100]; snprintf(t, sizeof(t), "Caso 2 - Insercao em ordem crescente (n = %d)", n); t_iniciar_caso(t); }
    NoRB *raiz = NULL;
    int rejeitados = 0;

    total_rotacoes = 0;
    total_recoloracoes = 0;
    for (int i = 0; i < n; i++) {
        int dup = 0;
        raiz = rb_inserir(raiz, t_jogador(i), &dup);
        rejeitados += dup;
    }

    checar("nenhuma insercao foi rejeitada", rejeitados == 0);
    checar("contagem de nos = n (nenhum registro se perdeu)",
           rb_contar_nos(raiz) == n);

    int todos = 1;
    for (int i = 0; i < n && todos; i++)
        if (t_buscar_j(raiz, t_jogador(i)) == NULL) todos = 0;
    checar("todos os n registros sao encontrados pela busca", todos);

    checar("em-ordem sai em ordem crescente da chave", t_em_ordem_crescente(raiz));

    t_visitas = 0;
    rb_em_ordem(raiz, t_contar_visita);
    checar("em-ordem visita exatamente n nos", t_visitas == n);

    checar("propriedades da Rubro-Negra validas (raiz preta, sem vermelho-vermelho, "
           "mesma altura negra, ponteiros pai certos)",
           rb_propriedades_validas(raiz));

    int h = rb_altura(raiz);
    printf("     altura = %d | limite 2*log2(n+1) = %d | rotacoes = %d | recoloracoes = %d\n",
           h, t_limite_altura(n), total_rotacoes, total_recoloracoes);
    checar("altura <= 2 * log2(n + 1)", h <= t_limite_altura(n));
    return raiz;
}

/* ---------------------------------------------------------------
 *  Caso 3: chave duplicada
 * --------------------------------------------------------------- */
static void caso_chave_duplicada(void) {
    t_iniciar_caso("Caso 3 - Chave duplicada");
    const int n = 1000;
    NoRB *raiz = NULL;
    for (int i = 0; i < n; i++) {
        int dup = 0;
        raiz = rb_inserir(raiz, t_jogador(i), &dup);
    }

    int altura_antes = rb_altura(raiz);
    int rotacoes_antes = total_rotacoes;
    int recol_antes = total_recoloracoes;
    Jogador original = t_jogador(500);
    Cor cor_antes = t_buscar_j(raiz, original)->cor;

    /* mesmo nome + nacionalidade + nascimento, mas o RESTO diferente */
    Jogador copia = original;
    copia.altura_cm = 999;
    snprintf(copia.url, TAM_URL, "http://fiba.test/DUPLICADO");
    snprintf(copia.selecao, TAM_SIGLA, "ARG");

    int dup = 0;
    raiz = rb_inserir(raiz, copia, &dup);
    checar("insercao da chave repetida e rejeitada (duplicado = 1)", dup == 1);
    checar("quantidade de nos nao mudou", rb_contar_nos(raiz) == n);
    checar("altura nao mudou", rb_altura(raiz) == altura_antes);
    checar("nenhuma rotacao nem recoloracao foi feita",
           total_rotacoes == rotacoes_antes && total_recoloracoes == recol_antes);

    NoRB *achado = t_buscar_j(raiz, original);
    checar("o registro original continua na arvore", achado != NULL);
    checar("o registro original NAO foi atualizado (altura_cm, url e selecao iguais)",
           achado != NULL && achado->dado.altura_cm == original.altura_cm &&
           strcmp(achado->dado.url, original.url) == 0 &&
           strcmp(achado->dado.selecao, original.selecao) == 0);
    checar("a cor do no original nao mudou", achado != NULL && achado->cor == cor_antes);
    checar("propriedades da Rubro-Negra continuam validas", rb_propriedades_validas(raiz));

    /* contra-prova: a chave e COMPOSTA, mudar so um campo da chave nao e duplicata */
    Jogador outra_nac = original;
    snprintf(outra_nac.nacionalidade, TAM_SIGLA, "ARG");
    dup = 0;
    raiz = rb_inserir(raiz, outra_nac, &dup);
    checar("mesmo nome e nascimento, nacionalidade diferente: NAO e duplicata",
           dup == 0 && rb_contar_nos(raiz) == n + 1);

    Jogador outro_nasc = original;
    snprintf(outro_nasc.data_nascimento, TAM_DATA, "Feb 2, 2001");
    dup = 0;
    raiz = rb_inserir(raiz, outro_nasc, &dup);
    checar("mesmo nome e nacionalidade, nascimento diferente: NAO e duplicata",
           dup == 0 && rb_contar_nos(raiz) == n + 2);
    checar("propriedades validas apos as duas insercoes", rb_propriedades_validas(raiz));

    rb_liberar(raiz);
}

/* ---------------------------------------------------------------
 *  Caso 4: remocao com rebalanceamento em cascata
 * --------------------------------------------------------------- */
static void caso_remocao_cascata(void) {
    t_iniciar_caso("Caso 4 - Remocao com rebalanceamento em cascata");

    /* --- 4a: arvore perfeita toda preta (a cascata de recoloracao sobe
     *         ate a raiz quando se tira uma folha) --- */
    const int PROF = 5;
    t_prox_chave = 0;
    NoRB *modelo = t_perfeita_preta(PROF);
    int tamanho = rb_contar_nos(modelo);
    checar("arvore perfeita toda preta montada e valida", rb_propriedades_validas(modelo));
    rb_liberar(modelo);
    printf("     arvore perfeita: profundidade %d, %d nos, todos pretos\n", PROF, tamanho);

    int ruins = 0, max_recol = 0, max_rot = 0, chave_max = -1;
    for (int k = 0; k < tamanho; k++) {
        t_prox_chave = 0;
        NoRB *raiz = t_perfeita_preta(PROF);
        int recol_antes = total_recoloracoes, rot_antes = total_rotacoes, ok = 0;

        raiz = t_remover_j(raiz, t_jogador(k), &ok);

        int recol = total_recoloracoes - recol_antes;
        if (recol > max_recol) { max_recol = recol; chave_max = k; }
        if (total_rotacoes - rot_antes > max_rot) max_rot = total_rotacoes - rot_antes;
        if (!ok || rb_contar_nos(raiz) != tamanho - 1 ||
            t_buscar_j(raiz, t_jogador(k)) != NULL ||
            !rb_propriedades_validas(raiz) || !t_em_ordem_crescente(raiz))
            ruins++;
        rb_liberar(raiz);
    }
    printf("     maior cascata numa unica remocao: %d recoloracoes (chave %d) | max %d rotacoes\n",
           max_recol, chave_max, max_rot);
    checar("removendo cada um dos nos (uma vez por arvore nova): propriedades validas, contagem certa",
           ruins == 0);
    checar("alguma remocao disparou recoloracoes em mais de um nivel (cascata)",
           max_recol >= 2);

    /* --- 4b: arvore grande, remocoes em varias ordens, conferindo a cada passo --- */
    const int N = 2000;
    const char *nomes[] = {"crescente", "decrescente", "pseudo-aleatoria", "sempre a raiz"};
    for (int modo = 0; modo < 4; modo++) {
        NoRB *raiz = NULL;
        for (int i = 0; i < N; i++) {
            int dup = 0;
            raiz = rb_inserir(raiz, t_jogador(i), &dup);
        }

        /* ordem das chaves a remover */
        int *ordem = (int*) malloc((size_t) N * sizeof(int));
        for (int i = 0; i < N; i++) ordem[i] = (modo == 1) ? N - 1 - i : i;
        if (modo == 2) {
            semente = 7;
            for (int i = N - 1; i > 0; i--) {
                int r = (int) (proximo_aleatorio() % (unsigned) (i + 1));
                int tmp = ordem[i]; ordem[i] = ordem[r]; ordem[r] = tmp;
            }
        }

        int falhou = 0, restantes = N, max_recol_modo = 0, max_rot_modo = 0;
        for (int i = 0; i < N; i++) {
            int ok = 0, recol_antes = total_recoloracoes, rot_antes = total_rotacoes;
            if (modo == 3 && raiz != NULL) {           /* remove sempre a raiz atual */
                Jogador alvo = raiz->dado;
                raiz = t_remover_j(raiz, alvo, &ok);
            } else {
                raiz = t_remover_j(raiz, t_jogador(ordem[i]), &ok);
            }
            if (total_recoloracoes - recol_antes > max_recol_modo)
                max_recol_modo = total_recoloracoes - recol_antes;
            if (total_rotacoes - rot_antes > max_rot_modo)
                max_rot_modo = total_rotacoes - rot_antes;
            if (ok) restantes--;
            if (!ok || rb_contar_nos(raiz) != restantes || !rb_propriedades_validas(raiz)) {
                falhou = 1;
                break;
            }
        }
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "remover %d nos em ordem %s: valida depois de CADA remocao (max %d recol, %d rot por remocao)",
                 N, nomes[modo], max_recol_modo, max_rot_modo);
        checar(msg, !falhou);
        checar("   ...e a arvore terminou vazia", raiz == NULL && restantes == 0);
        free(ordem);
        rb_liberar(raiz);
    }
}

/* ---------------------------------------------------------------
 *  Caso 5: busca do ultimo registro em ordem alfabetica
 * --------------------------------------------------------------- */
static void caso_busca_maior(NoRB *raiz, int n) {
    { char t[100]; snprintf(t, sizeof(t), "Caso 5 - Busca do ultimo registro em ordem alfabetica (n = %d)", n); t_iniciar_caso(t); }

    /* o maior elemento e o no mais a direita */
    NoRB *maior = raiz;
    while (maior->dir != NULL) maior = maior->dir;

    Jogador ultimo = t_jogador(n - 1);
    NoRB *achado = t_buscar_j(raiz, ultimo);
    int h = rb_altura(raiz);
    int comps = t_comparacoes_busca(raiz, ultimo);

    checar("o maior elemento e encontrado", achado != NULL);
    checar("e o mesmo no que esta na ponta direita da arvore", achado == maior);
    checar("o maior elemento nao tem filho a direita", maior->dir == NULL);
    printf("     comparacoes = %d | altura = %d | n = %d\n", comps, h, n);
    checar("comparacoes <= altura + 1  (altura conta arestas; o caminho tem altura+1 nos)",
           comps <= h + 1);
    checar("comparacoes <= 2 * log2(n+1) + 1  -> O(log n)",
           comps <= t_limite_altura(n) + 1);

    /* um nome maior que todos (depois do ultimo) nao pode ser achado */
    Jogador alem = t_jogador(n + 12345);
    checar("chave maior que todas nao e encontrada (e nao trava)",
           t_buscar_j(raiz, alem) == NULL);
    checar("   ...e gasta no maximo altura + 1 comparacoes",
           t_comparacoes_busca(raiz, alem) <= h + 1);
}

/* ---------------------------------------------------------------
 *  main
 * --------------------------------------------------------------- */
int main(int argc, char *argv[]) {
    int n = 15800;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 2) {
        fprintf(stderr, "Uso: %s [n >= 2]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("=====================================================\n");
    printf("  TESTES DE CASOS EXTREMOS - ARVORE RUBRO-NEGRA\n");
    printf("=====================================================\n");

    caso_arvore_vazia();
    NoRB *grande = caso_ordem_crescente(n);
    caso_chave_duplicada();
    caso_remocao_cascata();
    caso_busca_maior(grande, n);
    rb_liberar(grande);

    printf("\n=====================================================\n");
    if (t_falhas == 0) printf("  Todos os %d testes passaram.\n", t_checks);
    else               printf("  %d de %d teste(s) falharam.\n", t_falhas, t_checks);
    printf("=====================================================\n");
    t_gravar_relatorios(n);
    return (t_falhas == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}