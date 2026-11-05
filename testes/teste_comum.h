/*
 *  Infraestrutura comum aos testes (test_avl.c e test_rubro_negra.c)
 *
 *  NAO compile este arquivo sozinho: ele e incluido pelos testes, DEPOIS
 *  de incluir o .c da arvore e de definir T_ARVORE e T_TITULO.
 *
 *  Contem:
 *    - registro dos resultados (tela + 2 CSVs + relatorio HTML)
 *    - leitura do jogadores.dat para a memoria
 *    - ordenacao dos registros pela chave (nome + nacionalidade + nascimento)
 *    - visitantes de percurso e um gerador de numeros aleatorios fixo
 */
#ifndef TESTE_COMUM_H
#define TESTE_COMUM_H

#include <math.h>

#define T_MAX_CHECKS   400
#define T_MAX_METRICAS 200

/* ---------------------------------------------------------------
 *  Registro dos resultados
 * --------------------------------------------------------------- */

typedef struct { char caso[100]; char descricao[240]; int ok; } T_Check;
typedef struct { char caso[100]; char nome[100]; double valor; } T_Metrica;

static T_Check   t_res[T_MAX_CHECKS];
static T_Metrica t_met[T_MAX_METRICAS];
static int  t_nres = 0, t_nmet = 0;
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

/* igual ao checar, mas com um rotulo na frente: "[rotulo] descricao" */
static void checar_r(const char *rotulo, const char *descricao, int condicao) {
    char msg[240];
    snprintf(msg, sizeof(msg), "[%s] %s", rotulo, descricao);
    checar(msg, condicao);
}

/* guarda uma medida (altura, rotacoes, comparacoes...) para o CSV/HTML */
static void metrica(const char *nome, double valor) {
    if (t_nmet >= T_MAX_METRICAS) return;
    snprintf(t_met[t_nmet].caso, sizeof(t_met[t_nmet].caso), "%s", t_caso_atual);
    snprintf(t_met[t_nmet].nome, sizeof(t_met[t_nmet].nome), "%s", nome);
    t_met[t_nmet].valor = valor;
    t_nmet++;
}

/* ---------------------------------------------------------------
 *  Dataset em memoria
 * --------------------------------------------------------------- */

static const char *t_fonte = "";   /* caminho do arquivo lido */
static Jogador *t_lista = NULL;    /* todos os registros, na ordem do arquivo */
static int t_n_lista = 0;          /* quantos registros (linhas validas) */
static int t_ignoradas = 0;        /* linhas malformadas (menos de 7 campos) */
static int *t_ord = NULL;          /* indices de t_lista ordenados por (chave, posicao no arquivo) */
static int *t_uniq = NULL;         /* 1o registro de cada chave, em ordem crescente de chave */
static int t_unicas = 0;           /* quantidade de chaves distintas */

/* le o arquivo com a mesma regra de campos do avl.c / rubro_negra.c.
 * Devolve a quantidade de registros (-1 se nao abriu). */
static int t_carregar_dataset(const char *caminho) {
    t_fonte = caminho;
    FILE *arq = fopen(caminho, "r");
    if (arq == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s'.\n"
                        "      Rode o teste na mesma pasta do avl.c / rubro_negra.c "
                        "(o arquivo fica em ../dataset/) ou passe o caminho como argumento.\n",
                caminho);
        return -1;
    }

    int capacidade = 1024;
    t_lista = (Jogador*) malloc((size_t) capacidade * sizeof(Jogador));
    if (t_lista == NULL) { fclose(arq); return -1; }

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
        if (n < 7) { t_ignoradas++; continue; }

        if (t_n_lista == capacidade) {
            capacidade *= 2;
            Jogador *novo = (Jogador*) realloc(t_lista, (size_t) capacidade * sizeof(Jogador));
            if (novo == NULL) { fclose(arq); return -1; }
            t_lista = novo;
        }
        Jogador *j = &t_lista[t_n_lista++];
        copiar_campo(j->nome, TAM_NOME, campos[0]);
        copiar_campo(j->data_nascimento, TAM_DATA, campos[1]);
        j->altura_cm = atoi(campos[2]);
        copiar_campo(j->altura_imperial, TAM_ALTIMP, campos[3]);
        copiar_campo(j->selecao, TAM_SIGLA, campos[4]);
        copiar_campo(j->nacionalidade, TAM_SIGLA, campos[5]);
        copiar_campo(j->url, TAM_URL, campos[6]);
    }
    fclose(arq);
    return t_n_lista;
}

static int t_mesma_chave(const Jogador *a, const Jogador *b) {
    return chave_comparar(a->nome, a->nacionalidade, a->data_nascimento,
                          b->nome, b->nacionalidade, b->data_nascimento) == 0;
}

/* ordena por chave; empate (chave repetida) resolve pela posicao no arquivo,
 * entao o primeiro de cada grupo e sempre o que aparece antes no arquivo */
static int t_cmp_idx(const void *pa, const void *pb) {
    int ia = *(const int*) pa, ib = *(const int*) pb;
    const Jogador *a = &t_lista[ia], *b = &t_lista[ib];
    int c = chave_comparar(a->nome, a->nacionalidade, a->data_nascimento,
                           b->nome, b->nacionalidade, b->data_nascimento);
    if (c != 0) return c;
    return (ia > ib) - (ia < ib);
}

static void t_preparar_ordem(void) {
    t_ord  = (int*) malloc((size_t) t_n_lista * sizeof(int));
    t_uniq = (int*) malloc((size_t) t_n_lista * sizeof(int));
    for (int i = 0; i < t_n_lista; i++) t_ord[i] = i;
    qsort(t_ord, (size_t) t_n_lista, sizeof(int), t_cmp_idx);

    t_unicas = 0;
    for (int i = 0; i < t_n_lista; i++)
        if (i == 0 || !t_mesma_chave(&t_lista[t_ord[i]], &t_lista[t_ord[i - 1]]))
            t_uniq[t_unicas++] = t_ord[i];
}

static void t_liberar_dataset(void) {
    free(t_lista); free(t_ord); free(t_uniq);
    t_lista = NULL; t_ord = NULL; t_uniq = NULL;
}

/* ---------------------------------------------------------------
 *  Visitantes de percurso e aleatorio fixo
 * --------------------------------------------------------------- */

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

/* gerador proprio: mesma sequencia em qualquer computador */
static unsigned long long t_sem = 42;
static unsigned int t_rand(void) {
    t_sem = t_sem * 6364136223846793005ULL + 1442695040888963407ULL;
    return (unsigned int) (t_sem >> 33);
}
static void t_embaralhar(int *v, int n) {
    for (int i = n - 1; i > 0; i--) {
        int k = (int) (t_rand() % (unsigned int) (i + 1));
        int tmp = v[i]; v[i] = v[k]; v[k] = tmp;
    }
}

/* ---------------------------------------------------------------
 *  Gravacao dos arquivos de saida
 * --------------------------------------------------------------- */

/* texto entre aspas para CSV (aspas internas viram "") */
static void t_csv_campo(FILE *f, const char *s) {
    fputc('"', f);
    for (; *s; s++) {
        if (*s == '"') fputc('"', f);
        fputc(*s, f);
    }
    fputc('"', f);
}

/* texto seguro para HTML (<, > e & viram entidades) */
static void t_html_texto(FILE *f, const char *s) {
    for (; *s; s++) {
        if (*s == '<')      fputs("&lt;", f);
        else if (*s == '>') fputs("&gt;", f);
        else if (*s == '&') fputs("&amp;", f);
        else                fputc(*s, f);
    }
}

static void t_gravar_relatorios(void) {
    const char *arq_testes   = "resultados_testes_"   T_ARVORE ".csv";
    const char *arq_metricas = "resultados_metricas_" T_ARVORE ".csv";
    const char *arq_html     = "relatorio_testes_"    T_ARVORE ".html";
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

    /* --- CSV 2: uma linha por medida --- */
    f = fopen(arq_metricas, "w");
    if (f == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s'.\n", arq_metricas);
    } else {
        fprintf(f, "arvore,caso,metrica,valor\n");
        for (int i = 0; i < t_nmet; i++) {
            fprintf(f, "%s,", T_ARVORE);
            t_csv_campo(f, t_met[i].caso);
            fputc(',', f);
            t_csv_campo(f, t_met[i].nome);
            fprintf(f, ",%.6g\n", t_met[i].valor);
        }
        fclose(f);
    }

    /* --- HTML: relatorio para abrir no navegador --- */
    f = fopen(arq_html, "w");
    if (f == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar '%s'.\n", arq_html);
    } else {
        fprintf(f,
            "<!DOCTYPE html>\n<html lang=\"pt-BR\"><head><meta charset=\"utf-8\">\n"
            "<title>Testes - %s</title>\n<style>\n"
            "body{font-family:system-ui,sans-serif;max-width:960px;margin:2rem auto;padding:0 1rem;color:#222}\n"
            "table{border-collapse:collapse;width:100%%;margin:.5rem 0 1.5rem}\n"
            "th,td{border:1px solid #ccc;padding:.35rem .6rem;text-align:left;font-size:.92rem}\n"
            "th{background:#f2f2f2}\n"
            ".ok{color:#0a7a2f;font-weight:600}\n.falhou{color:#b00020;font-weight:700}\n"
            "tr.linha-falhou td{background:#fde8ea}\n"
            ".resumo{padding:.8rem 1rem;border-radius:6px;font-weight:600}\n"
            ".resumo.ok{background:#e3f5e8}\n.resumo.falhou{background:#fde8ea}\n"
            "td.num{text-align:right;font-variant-numeric:tabular-nums}\n"
            "code{background:#f2f2f2;padding:.1rem .3rem;border-radius:3px}\n"
            "</style></head><body>\n", T_TITULO);

        fprintf(f, "<h1>Testes de casos extremos &mdash; %s</h1>\n", T_TITULO);
        fprintf(f, "<p>Arquivo de dados: <code>");
        t_html_texto(f, t_fonte);
        fprintf(f, "</code><br>Linhas lidas: <b>%d</b> &middot; chaves distintas: <b>%d</b>"
                   " &middot; linhas com chave repetida: <b>%d</b></p>\n",
                t_n_lista, t_unicas, t_n_lista - t_unicas);
        if (t_falhas == 0)
            fprintf(f, "<p class=\"resumo ok\">Todos os %d testes passaram.</p>\n", t_checks);
        else
            fprintf(f, "<p class=\"resumo falhou\">%d de %d teste(s) falharam.</p>\n", t_falhas, t_checks);

        fprintf(f, "<h2>Verifica&ccedil;&otilde;es</h2>\n");
        const char *caso_anterior = NULL;
        for (int i = 0; i < t_nres; i++) {
            if (caso_anterior == NULL || strcmp(caso_anterior, t_res[i].caso) != 0) {
                if (caso_anterior != NULL) fprintf(f, "</table>\n");
                fprintf(f, "<h3>");
                t_html_texto(f, t_res[i].caso);
                fprintf(f, "</h3>\n<table><tr><th>Verifica&ccedil;&atilde;o</th>"
                           "<th style=\"width:6rem\">Resultado</th></tr>\n");
                caso_anterior = t_res[i].caso;
            }
            fprintf(f, "<tr%s><td>", t_res[i].ok ? "" : " class=\"linha-falhou\"");
            t_html_texto(f, t_res[i].descricao);
            fprintf(f, "</td><td class=\"%s\">%s</td></tr>\n",
                    t_res[i].ok ? "ok" : "falhou", t_res[i].ok ? "OK" : "FALHOU");
        }
        if (caso_anterior != NULL) fprintf(f, "</table>\n");

        fprintf(f, "<h2>Medidas</h2>\n<table><tr><th>Caso</th><th>Medida</th><th>Valor</th></tr>\n");
        for (int i = 0; i < t_nmet; i++) {
            fprintf(f, "<tr><td>");
            t_html_texto(f, t_met[i].caso);
            fprintf(f, "</td><td>");
            t_html_texto(f, t_met[i].nome);
            fprintf(f, "</td><td class=\"num\">%.6g</td></tr>\n", t_met[i].valor);
        }
        fprintf(f, "</table>\n</body></html>\n");
        fclose(f);
    }

    printf("\nResultados gravados em:\n  %s\n  %s\n  %s\n", arq_testes, arq_metricas, arq_html);
}

#endif /* TESTE_COMUM_H */
