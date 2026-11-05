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

3.5 Resultados obtidos

Os números abaixo vêm do benchmark que rodamos com a flag --bench, e os arquivos completos estão na pasta /resultados (resultados_avl.csv e resultados_rubro_negra.csv). Testamos três volumes de jogadores: 1.000, 10.000 e 90.000. Em cada um deles, inserimos todos os registros, fizemos 2.000 buscas e removemos 10% dos nós. Em seguida, comparamos o que observamos com o que a teoria prevê.
3.5.1 Árvore AVL
Tabela 1 – Resultados da AVL
n	Operação	Nº de ops	Tempo total (s)	Tempo/op (µs)	Altura	Rotações
1.000	Inserção	1.000	0,000839	0,839	11	711
1.000	Busca	2.000	0,000358	0,179	11	0
1.000	Remoção	100	0,000065	0,651	11	42
10.000	Inserção	10.000	0,012686	1,269	15	6.945
10.000	Busca	2.000	0,000469	0,234	15	0
10.000	Remoção	1.000	0,001042	1,042	15	429
90.000	Inserção	90.000	0,300311	3,337	19	62.718
90.000	Busca	2.000	0,000783	0,392	19	0
90.000	Remoção	9.000	0,027248	3,028	19	3.894

3.5.2 Árvore Rubro-Negra
Tabela 2 – Resultados da Rubro-Negra
n	Operação	Nº de ops	Tempo total (s)	Tempo/op (µs)	Altura	Rotações	Recolor.
1.000	Inserção	1.000	0,000579	0,579	11	579	515
1.000	Busca	2.000	0,000344	0,172	11	0	0
1.000	Remoção	100	0,000048	0,475	11	44	14
10.000	Inserção	10.000	0,007953	0,795	15	5.770	5.074
10.000	Busca	2.000	0,000450	0,225	15	0	0
10.000	Remoção	1.000	0,000577	0,577	15	400	86
90.000	Inserção	90.000	0,122236	1,358	19	52.276	46.119
90.000	Busca	2.000	0,000662	0,331	19	0	0
90.000	Remoção	9.000	0,013650	1,517	19	3.623	937

3.5.3 Comparação entre as duas árvores
Para facilitar a leitura, juntamos os tempos por operação das duas estruturas e calculamos quantas vezes a AVL foi mais lenta que a Rubro-Negra (RN).
Tabela 3 – Tempo por operação (µs) e razão AVL/RN
n	Inserção AVL	Inserção RN	Razão	Busca AVL	Busca RN	Razão	Remoção AVL	Remoção RN	Razão
1.000	0,839	0,579	1,45×	0,179	0,172	1,04×	0,651	0,475	1,37×
10.000	1,269	0,795	1,60×	0,234	0,225	1,04×	1,042	0,577	1,81×
90.000	3,337	1,358	2,46×	0,392	0,331	1,18×	3,028	1,517	2,00×

Também olhamos quantas rotações e recolorações cada árvore precisou, em média, por operação:
Tabela 4 – Rotações e recolorações médias por operação
n	Rot./inserção AVL	Rot./inserção RN	Recol./inserção RN	Rot./remoção AVL	Rot./remoção RN	Recol./remoção RN
1.000	0,71	0,58	0,52	0,42	0,44	0,14
10.000	0,69	0,58	0,51	0,43	0,40	0,09
90.000	0,70	0,58	0,51	0,43	0,40	0,10

Por fim, comparamos a altura final de cada árvore com os limites teóricos de pior caso:
Tabela 5 – Altura final versus limites teóricos
n	log₂(n)	Altura AVL	Limite AVL (≈1,44·log₂(n+2))	Altura RN	Limite RN (2·log₂(n+1))
1.000	9,97	11	≈ 14,4	11	≈ 20,0
10.000	13,29	15	≈ 19,1	15	≈ 26,6
90.000	16,46	19	≈ 23,7	19	≈ 32,9

3.5.4 O que os resultados mostram
A altura cresce devagar, como esperado. De 1.000 para 90.000 jogadores, o número de registros foi multiplicado por 90, mas a altura subiu só de 11 para 19. Isso acompanha o crescimento de log₂ n e bate com o O(log n) que prevemos para busca, inserção e remoção. As duas árvores ficaram bem abaixo dos limites de pior caso (Tabela 5), o que também faz sentido, já que esses limites são garantias e não o que normalmente acontece.
AVL e Rubro-Negra terminaram com a mesma altura. Isso nos surpreendeu um pouco. Esperávamos que a AVL, por ser mais rígida, ficasse mais baixa, mas com o nosso dataset as duas chegaram a 11, 15 e 19 nos três volumes. Ou seja, a vantagem teórica da AVL em balanceamento não apareceu na prática.
Na busca, o empate é praticamente total. A diferença entre as duas ficou entre 4% e 18%, o que é coerente com alturas iguais. A busca também não faz nenhuma rotação nem recoloração. Um detalhe que notamos é que o tempo por busca cresceu cerca de 2,2 vezes de 1.000 para 90.000, enquanto o log₂ n cresceu só 1,65 vez. Nossa hipótese é que isso venha do cache do processador, já que uma árvore maior não cabe nos níveis mais rápidos de memória. Isso não contradiz o Big-O, que não considera esses fatores constantes de hardware.
Na inserção, a Rubro-Negra ganha com folga. Ela foi mais rápida em todos os volumes, e a diferença aumentou com o tamanho: 1,45 vez com 1.000 registros e 2,46 vezes com 90.000. Os contadores ajudam a explicar. A AVL faz cerca de 0,70 rotação por inserção contra 0,58 da RN, e a RN ainda compensa com recolorações (cerca de 0,51 por inserção), que são operações muito baratas. Além disso, a AVL precisa atualizar altura e fator de balanceamento em todo o caminho de volta até a raiz. Outro ponto importante é que o total de rotações cresce de forma linear com n (711, 6.945 e 62.718 na AVL), isto é, cerca de uma rotação por inserção em média. Isso confirma o O(1) de rebalanceamento que colocamos nas tabelas de complexidade.
Na remoção, a história é parecida. A Rubro-Negra foi de 1,37 a 2,00 vezes mais rápida. Aqui o número de rotações é quase igual nas duas (entre 0,40 e 0,44 por remoção), e a RN faz poucas recolorações. Então a diferença de tempo vem mais do trabalho de recalcular alturas na AVL do que das rotações em si. A remoção continua sendo O(log n), e a média de rotações por remoção quase não muda quando n cresce.
3.5.5 Conclusão
No geral, os experimentos confirmaram a teoria: todas as operações escalam em O(log n) e o rebalanceamento custa O(1) amortizado. Como as alturas finais foram iguais, o que realmente separou as duas estruturas foi o custo de manter o balanceamento. Para uma carga com muitas inserções e remoções, a Rubro-Negra se saiu melhor, chegando a ser 2,5 vezes mais rápida na inserção com 90.000 registros. Para uma carga dominada por buscas, as duas são equivalentes.
3.5.6 Limitações
Vale registrar alguns cuidados na hora de interpretar os números. Os tempos de busca são muito pequenos (menos de 1 ms no total para 2.000 buscas), então podem sofrer bastante com ruído de medição; o ideal seria repetir o benchmark várias vezes e tirar a média. Além disso, o maior volume testado foi de 90.000 registros, e não necessariamente o dataset completo. Por fim, os resultados dependem da máquina em que o teste foi executado, então os valores absolutos podem variar, mas a proporção entre as duas árvores tende a se manter.

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
