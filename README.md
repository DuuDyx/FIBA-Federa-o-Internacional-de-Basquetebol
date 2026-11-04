#  FIBA – Jogadores de Basquete Internacionais

# Integrantes do Grupo
* Eduardo Dylan Mendes — RA: 43541194 
* Erick Ronald Huanca Clemente — RGM 44261837
*	Marcos Antonio Ribeiro Lago Junior 46450769
*	Vitor Boer Blanco — RGM 42402794
*	Vagner Ferreira José Maria — RGM 44054807


## 1. Dataset

### 1.1 Descrição
O dataset é composto por dados de jogadores(as) de basquete cadastrados na **FIBA** (Federação Internacional de Basquete), obtidos via *scraping* do site oficial `fiba.basketball`. Cada registro representa um(a) atleta e contém informações como nome, data de nascimento, altura, seleção pela qual atuou e nacionalidade.

* **Formato original:** JSON (`fiba_jogadores.json`), contendo uma lista de objetos (um por jogador) ao todo há mais de 90 mil jogadores.
* **Tratamento:** Foi utilizado um script em **Python** para transformar o JSON em um arquivo binário `.dat`, otimizando a leitura e manipulação direta na linguagem C.

### 1.2 Fonte
* **Site oficial da FIBA:** [https://fiba.basketball](https://fiba.basketball) — páginas individuais de atletas (ex: `https://fiba.basketballen/players/<id>-<nome>`), coletadas por script próprio de *web scraping*.

### 1.3 Estrutura dos Dados
Os seguintes campos são mapeados a partir do dataset:

* `nome` *(string)* — Nome completo do(a) jogador(a); base primária da chave.
* `nacionalidade` *(string, sigla de 3 letras)* — País de origem; 1º critério de desempate.
* `data_nascimento` *(string)* — Data de nascimento; 2º critério de desempate.
* `altura_cm` *(int)* — Altura em centímetros (pode ser nula em registros antigos).
* `altura_imperial` *(string)* — Altura no sistema imperial (pés/polegadas).
* `selecao` *(string, sigla)* — Seleção defendida (pode divergir da nacionalidade em casos de naturalização).
* `url` *(string)* — Link da página do(a) jogador(a) no site da FIBA.

>  **Chave Composta da Árvore:** `nome` + `nacionalidade` + `data_nascimento`. 
> nome + nacionalidade + data_nascimento. A comparação é realizada primeiro pelo nome; em caso de empate, pela nacionalidade; e, persistindo o empate, pela data de nascimento. Dessa forma, a data de nascimento funciona como terceiro critério de desempate para diferenciar jogadores que possuem o mesmo nome e nacionalidade.

#### Exemplo de Registro (JSON):
```json
{
  "nome": "Melisa Gretter",
  "data_nascimento": "Jan 24, 1993",
  "altura_cm": 167,
  "altura_imperial": "5'6\"",
  "selecao": "ARG",
  "nacionalidade": "ARG",
  "url": "https://fiba.basketballen/players/163689-melisa-gretter"
}
```
### 1.4 Justificativa da Escolha
A escolha desse conjunto de dados se justifica por conter múltiplos campos de tipos distintos (tipagem mista e dados opcionais), exigindo tratamento estruturado em C. 
Além disso, a chave composta baseada em texto valida cenários complexos de ordenação (comparação lexicográfica de strings). O volume total de dados gerados ao fim da coleta servirá como um excelente cenário para testes de estresse** de desempenho das árvores.

---

## 2. Estrutura(s) de Árvore Escolhida(s)

### 2.1 Estruturas Implementadas
1. Árvore AVL: Árvore binária de busca autobalanceada estritamente por altura.
2. Árvore Rubro-Negra: Árvore binária de busca autobalanceada por regras de coloração de nós.

### 2.2 Justificativa Técnica
Ambas garantem complexidade de tempo de pior caso em O(\log n), mas usam estratégias distintas:
* A AVL aplica regras rígidas de altura (fator de balanceamento em \{-1, 0, 1\}), tornando as buscas cirurgicamente rápidas, sacrificando desempenho com rotações frequentes na inserção e remoção.
* A Rubro-Negra é mais permissiva em seu balanceamento cromático, aceitando pequenas variações de altura para obter inserções e remoções muito mais baratas e performáticas.

### 2.3.1 Operações Implementadas
[X]  Inserção
[X]  Remoção
[X]  Busca
[X]  Percursos (pré-ordem, em ordem, pós-ordem)
[X]  Balanceamento (se aplicável)


### 2.3.2 Análise de Complexidade

#### Árvore AVL

| Operação | Melhor Caso | Caso Médio | Pior Caso |
| :--- | :---: | :---: | :---: |
| **Inserção** | O(log n) | O(log n) | O(log n) |
| **Remoção** | O(log n) | O(log n) | O(log n) |
| **Busca** | O(log n) | O(log n) | O(log n) |
| **Percurso (Pré/Em/Pós-ordem)** | O(n) | O(n) | O(n) |
| **Rebalanceamento (Rotação)** | O(1) | O(1) | O(1) |

#### Árvore Rubro-Negra

| Operação | Melhor Caso | Caso Médio | Pior Caso |
| :--- | :---: | :---: | :---: |
| **Inserção** | O(log n) | O(log n) | O(log n) |
| **Remoção** | O(log n) | O(log n) | O(log n) |
| **Busca** | O(log n) | O(log n) | O(log n) |
| **Percurso (Pré/Em/Pós-ordem)** | O(n) | O(n) | O(n) |
| **Rebalanceamento (Rotação)** | O(1) | O(1) | O(1) |

Como a AVL mantém o fator de balanceamento de cada nó entre -1 e 1, a altura da árvore é sempre O(log n), o que garante que inserção, remoção e busca nunca degradam para O(n).

## 3. Plano de Testes

### 3.1 Objetivo dos Testes
Avaliar o funcionamento das árvores AVL e Rubro-Negra, observando a implementação das operações, o comportamento do balanceamento e a quantidade de rotações e recolorações. Também será comparado o desempenho das duas estruturas conforme o volume de jogadores aumenta.

### 3.2 Cenários de Teste
● Inserção de todos os registros do dataset e verificação de que o número final de nós corresponde ao número de registros únicos (chave nome+nacionalidade+data de nas).
● Busca por jogadores existentes (chave presente) e inexistentes (chave ausente), validando o retorno correto em ambos os casos.
● Remoção de um jogador e verificação de que a contagem de nós e o balanceamento da árvore permanecem corretos após a operação.
● Comparação direta entre AVL e Rubro-Negra: inserir o mesmo dataset nas duas estruturas e comparar altura final da árvore, número de rotações (e recolorações, no caso da Rubro-Negra) e tempo de execução de cada operação.


### 3.3 Casos extremos (edge cases)

| Caso | O que vamos testar | Comportamento esperado |
|------|--------------------|------------------------|
| Árvore vazia | buscar, remover e percorrer sem nenhum nó | A busca retorna NULL, a remoção não faz nada e o percurso não imprime nada. Nada deve travar. |
| Inserção em ordem crescente | inserir todos os registros já ordenados pela chave | Nenhum registro se perde. Na AVL a altura fica em torno de ⌊log₂ n⌋ (com 15.800 registros, altura 13). Na Rubro-Negra fica no máximo 2·log(n+1).|
| Chave duplicada | inserir alguém com o mesmo nome, nacionalidade e data de nascimento de um jogador que já está na árvore | O novo registro é rejeitado e o original continua como estava (não atualizamos). A quantidade de nós e a altura não mudam. |
| Remoção com rebalanceamento em cascata | remover vários nós que desbalanceiam os ancestrais | Depois de cada remoção a árvore continua balanceada (|FB| ≤ 1 na AVL, regras rubro-negras válidas na Rubro-Negra), com um nó a menos e o percurso em ordem ainda crescente. |
| Busca do último registro em ordem alfabética | buscar o maior elemento da árvore | Ele é encontrado com no máximo "altura" comparações, ou seja, O(log n). |
3.5 Resultados Obtidos e Análise Comparativa
Nossos resultados foram obtidos executando a rotina automatizada de benchmark (--bench). Os dados
brutos foram salvos nos arquivos resultados_avl.csv e resultados_rubro_negra.csv na pasta resultados/.
Nos testes, avaliamos três volumes progressivos de dados (n = 1.000, 10.000 e 90.000 registros). Para
cada volume n, inserimos todos os n atletas, executamos 2.000 buscas por amostragem e realizamos a
remoção de 10% dos nós (100, 1.000 e 9.000 remoções, respectivamente).
3.5.1 Árvore AVL — Resultados Medidos
n Operação Nº ops Tempo
Total (s)

Tempo/op
(µs) Altura Rotações Recoloraçõ
es
1.000 Inserção 1.000 0,000839 0,839 11 711 0
1.000 Busca 2.000 0,000358 0,179 11 0 0
1.000 Remoção 100 0,000065 0,651 11 42 0
10.000 Inserção 10.000 0,012686 1,269 15 6.945 0
10.000 Busca 2.000 0,000469 0,234 15 0 0
10.000 Remoção 1.000 0,001042 1,042 15 429 0
90.000 Inserção 90.000 0,300311 3,337 19 62.718 0
90.000 Busca 2.000 0,000783 0,392 19 0 0
90.000 Remoção 9.000 0,027248 3,028 19 3.894 0

3.5.2 Árvore Rubro-Negra — Resultados Medidos
n Operação Nº ops Tempo
Total (s)

Tempo/op
(µs) Altura Rotações Recoloraçõ
es
1.000 Inserção 1.000 0,000579 0,579 11 579 515

1.000 Busca 2.000 0,000344 0,172 11 0 0
1.000 Remoção 100 0,000048 0,475 11 44 14
10.000 Inserção 10.000 0,007953 0,795 15 5.770 5.074
10.000 Busca 2.000 0,000450 0,225 15 0 0
10.000 Remoção 1.000 0,000577 0,577 15 400 86
90.000 Inserção 90.000 0,122236 1,358 19 52.276 46.119
90.000 Busca 2.000 0,000662 0,331 19 0 0
90.000 Remoção 9.000 0,013650 1,517 19 3.623 937

3.5.3 Comparação Direta (AVL × Rubro-Negra)
Tabela 1: Tempo por Operação (µs) e Razão de Desempenho (AVL / Rubro-Negra)
n Inserção

AVL/RN Razão Busca

AVL/RN Razão Remoção

AVL/RN Razão
1.000 0,839 / 0,579 1,45× 0,179 / 0,172 1,04× 0,651 / 0,475 1,37×
10.000 1,269 / 0,795 1,60× 0,234 / 0,225 1,04× 1,042 / 0,577 1,81×
90.000 3,337 / 1,358 2,46× 0,392 / 0,331 1,18× 3,028 / 1,517 2,00×

Tabela 2: Rotações e Recolorações Médias por Operação
n Rot/Ins (AVL) Rot/Ins (RN) Rec/Ins (RN) Rot/Rem
(AVL)

Rot/Rem
(RN)

Rec/Rem
(RN)
1.000 0,71 0,58 0,52 0,42 0,44 0,14
10.000 0,69 0,58 0,51 0,43 0,40 0,09
90.000 0,70 0,58 0,51 0,43 0,40 0,10

Tabela 3: Altura Final Medida vs. Limites Teóricos das Árvores
n log₂(n) Altura AVL Limite AVL Altura RN Limite RN (2·log₂

(≈1,44·log₂ n) n)
1.000 9,97 11 ≈ 14,4 11 ≈ 20,0
10.000 13,29 15 ≈ 19,1 15 ≈ 26,6
90.000 16,46 19 ≈ 23,7 19 ≈ 32,9

3.5.4 Análise Detalhada e Discussão dos Resultados
Ao analisarmos os dados obtidos, a primeira coisa que chamou a nossa atenção foi que ambas as
árvores terminaram com exatamente a mesma altura nos três volumes testados (11 para 1.000, 15 para
10.000 e 19 para 90.000 nós). Confessamos que esse resultado nos surpreendeu no primeiro momento,
pois esperávamos que a rigidez da AVL a deixasse ligeiramente mais baixa que a Rubro-Negra.
Entretanto, compreendemos o motivo: a distribuição dos dados de entrada (ordenados
lexicograficamente pelas chaves compostas) permitiu que a Rubro-Negra se rearranjasse de forma
quase perfeita, mantendo sua altura muito próxima do ideal e bem abaixo do limite teórico de worst-
case (2 · log₂(n+1)).
Em termos de crescimento, quando multiplicamos o volume n por 90 (passando de 1.000 para 90.000
elementos), a altura das árvores subiu apenas de 11 para 19 níveis. Esse comportamento acompanha de
forma perfeita a curva logarítmica log₂ n, confirmando na prática o limite assintótico O(log n) para
buscas, inserções e remoções.
No teste de busca, o tempo médio por consulta foi virtualmente idêntico em ambas as estruturas (com
razões de 1,04× a 1,18× favoráveis à Rubro-Negra). Como a altura final de ambas foi igual, a distância
percorrida da raiz até as folhas foi a mesma. Observamos também que o tempo de busca cresceu cerca
de 2,2× entre n=1.000 e n=90.000, enquanto log₂ n cresceu 1,65×. Trabalhamos com a hipótese de que
essa pequena diferença decorre de efeitos de cache da CPU — em volumes maiores, os nós da árvore
não cabem inteiramente na memória cache L1/L2, gerando mais cache misses durante a comparação de
strings.
Na inserção, a Rubro-Negra levou uma vantagem expressiva em todos os cenários, sendo 1,45× mais
rápida com 1.000 nós e atingindo 2,46× mais velocidade com 90.000 nós (0,122s contra 0,300s da AVL).
Essa diferença se explica pela mecânica de rebalanceamento: enquanto a AVL realiza ≈ 0,70 rotações
por inserção, a Rubro-Negra realiza apenas ≈ 0,58 rotações, compensando com ≈ 0,51 recolorações.
Como alterar a cor de um nó é uma operação simples de atribuição em memória, a Rubro-Negra evita a
sobrecarga de atualização de alturas e múltiplos ponteiros que a AVL exige.
Na remoção, o padrão se repetiu: a Rubro-Negra foi de 1,37× a 2,00× mais rápida. Curiosamente, a
média de rotações por remoção foi bastante parecida entre as duas (≈ 0,40 a 0,44). A vantagem
temporal da Rubro-Negra veio, portanto, da simplicidade em manter as propriedades de cores sem a
necessidade de recalcular dinamicamente os fatores de balanceamento dos nós ancestrais ao longo do
caminho de volta.

3.5.5 Conclusão da Comparação
Em síntese, os nossos testes práticos validaram integralmente as previsões da literatura teórica. Para
sistemas focados predominantemente em consultas e leituras (onde o dataset permanece
relativamente estático), ambas as estruturas oferecem desempenho equivalente. Por outro lado, em
sistemas dinâmicos que exigem constantes inserções e remoções de atletas (como a base de dados da
FIBA), a Árvore Rubro-Negra provou ser a melhor escolha arquitetural, apresentando ganho de até 2,5×
em velocidade de escrita sem sacrificar a eficiência das buscas.
3.5.6 Observações e Limitações do Estudo
1. Ruído de Medição nas Buscas: Como o tempo individual de cada busca é extremamente reduzido
(fração de microssegundos), as medições estão sujeitas a variações de clock do sistema operacional.
Para trabalhos futuros, sugerimos executar o benchmark em múltiplas passagens e utilizar a média
amostral com intervalo de confiança.
2. Teto do Benchmark: O teto dos nossos testes práticos foi fixado em n = 90.000 registros únicos. A
base de dados completa possui um volume ligeiramente superior, mas 90.000 nós foi a amostragem
perfeita para demonstrar a estabilidade das árvores em larga escala.
3. Alinhamento de Documentação: Notamos que a Seção 3.3 do planejamento inicial mencionava
hipoteticamente &#39;15.800 registros com altura 13&#39;. Esclarecemos que esse número era uma estimativa
preliminar baseada num recorte inicial do JSON; os testes oficiais finais foram consolidados com n =
90.000 (altura 19).
------------------------
### 4.1 Pré-requisitos (Para Entrega 2)

Para compilar e executar as árvores, você precisará de:

*   **Compilador C (GCC):** Necessário para compilar os arquivos `.c`. 
    *   *Windows:* Pode ser instalado através do MinGW-w64. Certifique-se de adicionar o caminho da pasta `bin` do GCC às variáveis de ambiente (PATH) do sistema.
*   **Arquivo de Dados:** O arquivo `jogadores.dat` deve estar na pasta dataset.
*   *Opcional:* Para visualizar os resultados gerados pelo benchmark, é recomendado ter o Excel ou outro leitor de planilhas (para abrir os arquivos `.csv`).

*   ### 4.2 Instruções (Para Entrega 2)

**1. Clonar o repositório e acessar a pasta:**
git clone https://github.com/DuuDyx/FIBA-Federa-o-Internacional-de-Basquetebol.git
cd FIBA-Federa-o-Internacional-de-Basquetebol
*   Para rodar as o teste de benchmark primeiro entre na pasta src e rode os seguintes comandos gcc -Wall -Wextra -std=c11 -o avl avl.c em seguida: .\avl.exe --bench para a rubro negra apenas troque de avl para rubro negra em todas as seções que mencione avl
------------------------

## 5. Referências
* **FIBA** — Federação Internacional de Basquete. Disponível em: [https://fiba.basketball](https://fiba.basketball) [1]
* **CORMEN, T. H. et al.** *Introduction to Algorithms*. 3ª ed. MIT Press, 2009. (Capítulos de árvores balanceadas AVL e Rubro-Negras).
