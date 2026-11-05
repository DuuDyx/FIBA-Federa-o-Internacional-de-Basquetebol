/*

Testes dos casos extremos (edge cases) - Arvore AVL

Este arquivo INCLUI o avl.c (sem modificar nada nele) e troca o main

original por este aqui. Por isso os dois arquivos precisam estar na

mesma pasta.

Compilar:  gcc -O2 -Wall -Wextra -std=c11 -o test_avl test_avl.c -lm

Executar:  ./test_avl          (usa 15800 registros no teste de ordem crescente)

        ./test_avl 50000    (usa 50000 registros)

Ao terminar, grava 1 arquivo na pasta atual:

resultados_testes_avl.csv    (uma linha por verificacao: caso, descricao, OK/FALHOU)
*/

#define _POSIX_C_SOURCE 199309L

#define main main_original      /* esconde o main do avl.c */
#define CAMINHO_AVL "../src/avl.c"
#include CAMINHO_AVL
#undef main
#include <math.h>

/* ---------------------------------------------------------------

Auxiliares dos testes

--------------------------------------------------------------- */

/* ---------------------------------------------------------------

Registro dos resultados (tela + CSV)

Cada verificacao fica guardada em memoria e, no final,

e gravada no arquivo resultados_testes_avl.csv.

--------------------------------------------------------------- */

#define T_ARVORE      "avl"
#define T_TITULO      "AVL"
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

antes do nome de i+1 (ordem alfabetica = ordem numerica). */
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

static NoAVL* t_buscar_j(NoAVL *raiz, Jogador j) {
return avl_buscar(raiz, j.nome, j.nacionalidade, j.data_nascimento);
}

/* Remove o jogador j; devolve a nova raiz e grava em *ok se removeu /
static NoAVL t_remover_j(NoAVL *raiz, Jogador j, int *ok) {
*ok = 0;
return avl_remover(raiz, j.nome, j.nacionalidade, j.data_nascimento, ok);
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

static int t_em_ordem_crescente(NoAVL *raiz) {
t_tem_ant = 0;
t_ordem_ok = 1;
avl_em_ordem(raiz, t_conferir_ordem);
return t_ordem_ok;
}

/* floor(log2(n)) usando so inteiros */
static int t_piso_log2(int n) {
int h = 0;
while ((1L << (h + 1)) <= n) h++;
return h;
}

/* Numero de comparacoes que avl_buscar faz = nos visitados no caminho.

(mesmo laco do avl_buscar, so que contando) */
static int t_comparacoes_busca(NoAVL *raiz, Jogador j) {
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

/* Arvore de Fibonacci: a AVL "mais torta possivel" para a altura h

(esq tem altura h-1, dir tem altura h-2). Montada na mao, no a no,

para controlar o formato. Chaves em ordem crescente (em-ordem). */
static int t_prox_chave = 0;

static NoAVL* t_fibonacci(int h) {
if (h < 0) return NULL;
NoAVL *esq = t_fibonacci(h - 1);
NoAVL *no = criar_no(t_jogador(t_prox_chave++));
no->esq = esq;
no->dir = t_fibonacci(h - 2);
no->altura = h;
return no;
}

/* ---------------------------------------------------------------

Caso 1: arvore vazia

--------------------------------------------------------------- */
static void caso_arvore_vazia(void) {
t_iniciar_caso("Caso 1 - Arvore vazia");
NoAVL raiz = NULL;
int removido = 0;   / o chamador inicia com 0 (contrato do avl_remover) */

checar("buscar devolve NULL",
avl_buscar(raiz, "Fulano", "BRA", "Jan 1, 2000") == NULL);

raiz = avl_remover(raiz, "Fulano", "BRA", "Jan 1, 2000", &removido);
checar("remover nao faz nada (removido = 0 e raiz continua NULL)",
removido == 0 && raiz == NULL);

t_visitas = 0;
avl_pre_ordem(raiz, t_contar_visita);
avl_em_ordem(raiz, t_contar_visita);
avl_pos_ordem(raiz, t_contar_visita);
checar("pre/em/pos-ordem nao visitam nenhum no", t_visitas == 0);

checar("altura = -1", avl_altura(raiz) == -1);
checar("contagem de nos = 0", avl_contar_nos(raiz) == 0);
checar("arvore vazia conta como balanceada", avl_esta_balanceada(raiz));
avl_liberar(raiz);   /* liberar NULL nao pode travar */
checar("avl_liberar(NULL) nao trava", 1);

/* variacao: arvore que ESVAZIOU depois de ter 1 no */
int dup = 0;
raiz = avl_inserir(NULL, t_jogador(1), &dup);
raiz = t_remover_j(raiz, t_jogador(1), &removido);
checar("inserir 1 e remover 1 deixa a arvore vazia (raiz NULL)",
removido == 1 && raiz == NULL);
t_visitas = 0;
avl_em_ordem(raiz, t_contar_visita);
checar("percurso apos esvaziar nao visita nada", t_visitas == 0);
}

/* ---------------------------------------------------------------

Caso 2: insercao em ordem crescente  (devolve a arvore criada,

que o caso 5 reaproveita)

--------------------------------------------------------------- /
static NoAVL caso_ordem_crescente(int n) {
{ char t[100]; snprintf(t, sizeof(t), "Caso 2 - Insercao em ordem crescente (n = %d)", n); t_iniciar_caso(t); }
NoAVL *raiz = NULL;
int rejeitados = 0;

total_rotacoes = 0;
for (int i = 0; i < n; i++) {
int dup = 0;
raiz = avl_inserir(raiz, t_jogador(i), &dup);
rejeitados += dup;
}

checar("nenhuma insercao foi rejeitada", rejeitados == 0);
checar("contagem de nos = n (nenhum registro se perdeu)",
avl_contar_nos(raiz) == n);

int todos = 1;
for (int i = 0; i < n && todos; i++)
if (t_buscar_j(raiz, t_jogador(i)) == NULL) todos = 0;
checar("todos os n registros sao encontrados pela busca", todos);

checar("em-ordem sai em ordem crescente da chave", t_em_ordem_crescente(raiz));

t_visitas = 0;
avl_em_ordem(raiz, t_contar_visita);
checar("em-ordem visita exatamente n nos", t_visitas == n);

checar("arvore balanceada (fatores entre -1 e 1, alturas corretas)",
avl_esta_balanceada(raiz));

int h = avl_altura(raiz);
int esperado = t_piso_log2(n);
printf("     altura = %d | floor(log2 n) = %d | rotacoes = %d\n",
h, esperado, total_rotacoes);
checar("altura == floor(log2 n)  (insercao ordenada gera arvore quase perfeita)",
h == esperado);
checar("altura nunca passa do limite teorico da AVL (1.44 * log2(n+2))",
h <= (int) (1.4405 * log2((double) n + 2.0)));
return raiz;
}

/* ---------------------------------------------------------------

Caso 3: chave duplicada

--------------------------------------------------------------- */
static void caso_chave_duplicada(void) {
t_iniciar_caso("Caso 3 - Chave duplicada");
const int n = 1000;
NoAVL *raiz = NULL;
for (int i = 0; i < n; i++) {
int dup = 0;
raiz = avl_inserir(raiz, t_jogador(i), &dup);
}

int altura_antes = avl_altura(raiz);
int rotacoes_antes = total_rotacoes;
Jogador original = t_jogador(500);

/* mesmo nome + nacionalidade + nascimento, mas o RESTO diferente */
Jogador copia = original;
copia.altura_cm = 999;
snprintf(copia.url, TAM_URL, "http://fiba.test/DUPLICADO");
snprintf(copia.selecao, TAM_SIGLA, "ARG");

int dup = 0;
raiz = avl_inserir(raiz, copia, &dup);
checar("insercao da chave repetida e rejeitada (duplicado = 1)", dup == 1);
checar("quantidade de nos nao mudou", avl_contar_nos(raiz) == n);
checar("altura nao mudou", avl_altura(raiz) == altura_antes);
checar("nenhuma rotacao foi feita", total_rotacoes == rotacoes_antes);

NoAVL *achado = t_buscar_j(raiz, original);
checar("o registro original continua na arvore",
achado != NULL);
checar("o registro original NAO foi atualizado (altura_cm, url e selecao iguais)",
achado != NULL && achado->dado.altura_cm == original.altura_cm &&
strcmp(achado->dado.url, original.url) == 0 &&
strcmp(achado->dado.selecao, original.selecao) == 0);
checar("arvore continua balanceada", avl_esta_balanceada(raiz));

/* contra-prova: a chave e COMPOSTA, mudar so um campo da chave nao e duplicata */
Jogador outra_nac = original;
snprintf(outra_nac.nacionalidade, TAM_SIGLA, "ARG");
dup = 0;
raiz = avl_inserir(raiz, outra_nac, &dup);
checar("mesmo nome e nascimento, nacionalidade diferente: NAO e duplicata",
dup == 0 && avl_contar_nos(raiz) == n + 1);

Jogador outro_nasc = original;
snprintf(outro_nasc.data_nascimento, TAM_DATA, "Feb 2, 2001");
dup = 0;
raiz = avl_inserir(raiz, outro_nasc, &dup);
checar("mesmo nome e nacionalidade, nascimento diferente: NAO e duplicata",
dup == 0 && avl_contar_nos(raiz) == n + 2);
checar("arvore continua balanceada apos as duas insercoes", avl_esta_balanceada(raiz));

avl_liberar(raiz);
}

/* ---------------------------------------------------------------

Caso 4: remocao com rebalanceamento em cascata

--------------------------------------------------------------- */
static void caso_remocao_cascata(void) {
t_iniciar_caso("Caso 4 - Remocao com rebalanceamento em cascata");

/* --- 4a: arvore de Fibonacci (pior caso: cada remocao pode rebalancear

    varios ancestrais seguidos) --- */


const int H = 10;
t_prox_chave = 0;
NoAVL *modelo = t_fibonacci(H);
int tamanho = avl_contar_nos(modelo);
checar("arvore de Fibonacci montada e valida (balanceada)", avl_esta_balanceada(modelo));
avl_liberar(modelo);
printf("     arvore de Fibonacci: altura %d, %d nos\n", H, tamanho);

int ruins = 0, max_rot = 0, chave_max = -1;
for (int k = 0; k < tamanho; k++) {
t_prox_chave = 0;
NoAVL *raiz = t_fibonacci(H);
int rot_antes = total_rotacoes, ok = 0;

 raiz = t_remover_j(raiz, t_jogador(k), &ok);

 int rot = total_rotacoes - rot_antes;
 if (rot > max_rot) { max_rot = rot; chave_max = k; }
 if (!ok || avl_contar_nos(raiz) != tamanho - 1 ||
     t_buscar_j(raiz, t_jogador(k)) != NULL ||
     !avl_esta_balanceada(raiz) || !t_em_ordem_crescente(raiz))
     ruins++;
 avl_liberar(raiz);


}
printf("     maior numero de rotacoes numa unica remocao: %d (chave %d)\n",
max_rot, chave_max);
checar("removendo cada um dos nos (uma vez por arvore nova): sempre balanceada, contagem certa",
ruins == 0);
checar("alguma remocao disparou rotacoes em mais de um ancestral (cascata)",
max_rot >= 2);

/* --- 4b: arvore grande, remocoes em varias ordens, conferindo a cada passo --- */
const int N = 2000;
const char *nomes[] = {"crescente", "decrescente", "pseudo-aleatoria", "sempre a raiz"};
for (int modo = 0; modo < 4; modo++) {
NoAVL *raiz = NULL;
for (int i = 0; i < N; i++) {
int dup = 0;
raiz = avl_inserir(raiz, t_jogador(i), &dup);
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

 int falhou = 0, restantes = N, max_rot_modo = 0;
 for (int i = 0; i < N; i++) {
     int ok = 0, rot_antes = total_rotacoes;
     if (modo == 3 && raiz != NULL) {           /* remove sempre a raiz atual */
         Jogador alvo = raiz->dado;
         raiz = t_remover_j(raiz, alvo, &ok);
     } else {
         raiz = t_remover_j(raiz, t_jogador(ordem[i]), &ok);
     }
     if (total_rotacoes - rot_antes > max_rot_modo) max_rot_modo = total_rotacoes - rot_antes;
     if (ok) restantes--;
     if (!ok || avl_contar_nos(raiz) != restantes || !avl_esta_balanceada(raiz)) {
         falhou = 1;
         break;
     }
 }
 char msg[160];
 snprintf(msg, sizeof(msg),
          "remover %d nos em ordem %s: balanceada depois de CADA remocao (max %d rot/remocao)",
          N, nomes[modo], max_rot_modo);
 checar(msg, !falhou);
 checar("   ...e a arvore terminou vazia", raiz == NULL && restantes == 0);
 free(ordem);
 avl_liberar(raiz);


}
}

/* ---------------------------------------------------------------

Caso 5: busca do ultimo registro em ordem alfabetica

--------------------------------------------------------------- */
static void caso_busca_maior(NoAVL *raiz, int n) {
{ char t[100]; snprintf(t, sizeof(t), "Caso 5 - Busca do ultimo registro em ordem alfabetica (n = %d)", n); t_iniciar_caso(t); }

/* o maior elemento e o no mais a direita */
NoAVL *maior = raiz;
while (maior->dir != NULL) maior = maior->dir;

Jogador ultimo = t_jogador(n - 1);
NoAVL *achado = t_buscar_j(raiz, ultimo);
int h = avl_altura(raiz);
int comps = t_comparacoes_busca(raiz, ultimo);

checar("o maior elemento e encontrado", achado != NULL);
checar("e o mesmo no que esta na ponta direita da arvore", achado == maior);
checar("o maior elemento nao tem filho a direita", maior->dir == NULL);
printf("     comparacoes = %d | altura = %d | n = %d\n", comps, h, n);
checar("comparacoes <= altura + 1  (altura conta arestas; o caminho tem altura+1 nos)",
comps <= h + 1);
checar("comparacoes <= 1.44 * log2(n+2)  -> O(log n)",
comps <= (int) (1.4405 * log2((double) n + 2.0)) + 1);

/* um nome maior que todos (depois do ultimo) nao pode ser achado */
Jogador alem = t_jogador(n + 12345);
checar("chave maior que todas nao e encontrada (e nao trava)",
t_buscar_j(raiz, alem) == NULL);
checar("   ...e gasta no maximo altura + 1 comparacoes",
t_comparacoes_busca(raiz, alem) <= h + 1);
}

/* ---------------------------------------------------------------

main

--------------------------------------------------------------- */
int main(int argc, char *argv[]) {
int n = 15800;
if (argc > 1) n = atoi(argv[1]);
if (n < 2) {
fprintf(stderr, "Uso: %s [n >= 2]\n", argv[0]);
return EXIT_FAILURE;
}

printf("=\n");
printf("  TESTES DE CASOS EXTREMOS - ARVORE AVL\n");
printf("=\n");

caso_arvore_vazia();
NoAVL *grande = caso_ordem_crescente(n);
caso_chave_duplicada();
caso_remocao_cascata();
caso_busca_maior(grande, n);
avl_liberar(grande);

printf("\n=====================================================\n");
if (t_falhas == 0) printf("  Todos os %d testes passaram.\n", t_checks);
else               printf("  %d de %d teste(s) falharam.\n", t_falhas, t_checks);
printf("=====================================================\n");
t_gravar_relatorios(n);
return (t_falhas == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}