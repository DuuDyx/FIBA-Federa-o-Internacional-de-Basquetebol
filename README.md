#  FIBA – Jogadores de Basquete Internacionais

# Integrantes do Grupo
* Eduardo Dylan Mendes — Rgm: 43541194 
* Erick Ronald Huanca Clemente — RGM 44261837
*	Marcos Antonio Ribeiro Lago Junior - RGM 46450769
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


### 2.4 Análise de Complexidade

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
### 3.4 Testes de Desempenho

Para comparar a Árvore AVL e a Árvore Rubro-Negra, o grupo mediu o **tempo de execução** de inserções, buscas e remoções, e registrou também a altura da árvore e o número de rotações e recolorações. O **uso de memória** não foi medido, apenas estimado (item 3.4.3). Os resultados obtidos estão na seção 3.5.

#### 3.4.1 Tamanhos de entrada

Foram usados **1.000, 10.000 e 90.000 registros**, retirados do dataset de 96.013 registros. Os registros são embaralhados com uma semente fixa (`42`), de modo que as duas árvores recebem exatamente a mesma sequência, e cada tamanho usa os `n` primeiros da lista embaralhada. Em cada tamanho, o teste faz:

| n | Inserções | Buscas | Remoções |
|---|---|---|---|
| 1.000 | 1.000 | 2.000 | 100 |
| 10.000 | 10.000 | 2.000 | 1.000 |
| 90.000 | 90.000 | 2.000 | 9.000 |

As buscas são 1.000 de jogadores que existem na árvore e 1.000 de jogadores que não existem. As remoções correspondem a 10% de `n`.

#### 3.4.2 Tempo de execução

Cada programa (`src/avl.c` e `src/rubro_negra.c`) é executado com o argumento `--bench` (o código procura o dataset em `../dataset/jogadores.dat`, ou seja, a partir da pasta `src/`) e grava os resultados em um arquivo CSV, guardado na pasta `resultados/`.

- **Relógio:** nos testes (Windows), `QueryPerformanceCounter`. O código usa `clock_gettime(CLOCK_MONOTONIC)` em Linux/macOS.
- **O que é cronometrado:** cada lote de operações (inserções, buscas e remoções) tem seu próprio cronômetro. A leitura do arquivo, a preparação dos dados e as verificações de consistência ficam fora da medição.
- **Repetições:** cada tamanho é executado 5 vezes, com a árvore recriada do zero, e o tempo registrado é a média das 5 execuções.
- **Tempo por operação:** `tempo_total / número de operações`, em microssegundos.
- **Contagens registradas:** altura da árvore (folha = 0), rotações (uma rotação dupla conta como 2) e recolorações (só na Rubro-Negra).

#### 3.4.3 Uso de memória

O uso de memória **não foi medido**, apenas estimado. Em um compilador de 64 bits, o nó das duas árvores ocupa 424 bytes (396 bytes do registro do jogador mais ponteiros e um campo extra), então o consumo é de `n × 424` bytes: cerca de **38 MB** para 90.000 registros, igual nas duas estruturas.

#### 3.4.4 Ambiente de teste

| Item | Valor |
|---|---|
| Processador | Intel Core i3-10100F (4 núcleos, 8 threads) |
| Memória RAM | 12 GB |
| Sistema operacional | Windows |
| Compilador | GCC (`gcc`) |
| Flags de compilação | `-Wall -Wextra -std=c11` |

#### 3.4.5 Observações

- Os testes usam apenas ordem **aleatória** de inserção; o caso extremo de ordem crescente (item 3.3) não está incluído no benchmark.
- Se o dataset tiver chaves repetidas (item 3.3), elas são rejeitadas na inserção e uma segunda remoção da mesma chave não remove nada. Nesse caso a coluna `operacoes` do CSV conta as operações **tentadas**, e o programa pode exibir `ATENCAO` no tamanho de 90.000.


### 3.5 Resultados obtidos

Esta seção resume os resultados do benchmark descrito na seção 3.4 e os compara com a complexidade assintótica teórica.

#### 3.5.1 Resultados do benchmark

Os dados completos estão em [`resultados/resultados_avl.csv`](resultados/resultados_avl.csv) e [`resultados/resultados_rubro_negra.csv`](resultados/resultados_rubro_negra.csv). O método de medição está descrito na seção 3.4 e o ambiente de teste, no item 3.4.4. As tabelas abaixo resumem os valores: tempo médio por operação, em microssegundos (média de 5 repetições).

**Inserção**

| n | AVL (µs/op) | Rubro-Negra (µs/op) | Altura AVL | Altura RB | Rotações AVL | Rotações RB | Recolorações RB |
|---|---|---|---|---|---|---|---|
| 1.000 | 0,898 | 0,526 | 11 | 11 | 711 | 579 | 515 |
| 10.000 | 1,290 | 0,825 | 15 | 15 | 6.945 | 5.770 | 5.074 |
| 90.000 | 1,836 | 1,370 | 19 | 19 | 62.718 | 52.276 | 46.119 |

**Busca** (2.000 buscas: 1.000 existentes + 1.000 inexistentes)

| n | AVL (µs/op) | Rubro-Negra (µs/op) | Altura AVL | Altura RB |
|---|---|---|---|---|
| 1.000 | 0,173 | 0,170 | 11 | 11 |
| 10.000 | 0,208 | 0,198 | 15 | 15 |
| 90.000 | 0,357 | 0,290 | 19 | 19 |

**Remoção** (10% de n)

| n | AVL (µs/op) | Rubro-Negra (µs/op) | Rotações AVL | Rotações RB | Recolorações RB |
|---|---|---|---|---|---|
| 1.000 | 0,704 | 0,322 | 42 | 44 | 14 |
| 10.000 | 1,126 | 0,653 | 429 | 400 | 86 |
| 90.000 | 2,144 | 1,148 | 3.894 | 3.623 | 937 |

#### 3.5.2 Análise por operação

**Inserção.** A Rubro-Negra foi mais rápida nos três tamanhos: 0,526 contra 0,898 µs/op com 1.000 registros (a AVL levou cerca de 1,7× mais tempo), 0,825 contra 1,290 com 10.000 (cerca de 1,6×) e 1,370 contra 1,836 com 90.000 (cerca de 1,3×). A Rubro-Negra também fez entre 17% e 19% menos rotações (52.276 contra 62.718 com 90.000), ou seja, cerca de 0,58 rotação por inserção contra 0,70 na AVL. Além disso, parte das correções dela são apenas recolorações (46.119 com 90.000), que não alteram a estrutura da árvore. Isso confirma o que foi dito no item 2.2: a Rubro-Negra tem inserção mais barata. A vantagem diminui conforme `n` cresce.

**Busca.** As duas árvores ficaram praticamente empatadas (0,173 contra 0,170; 0,208 contra 0,198; 0,357 contra 0,290 µs/op). A altura final foi **idêntica** nas duas árvores em todos os tamanhos (11, 15 e 19), então a vantagem esperada da AVL na busca, por ser mais baixa, não apareceu neste dataset com inserção em ordem aleatória. A diferença com 90.000 registros (Rubro-Negra cerca de 19% mais rápida) não pode ser explicada pela altura. Possíveis causas são a disposição dos nós na memória (cache) e o ruído de medição, já que o lote de 2.000 buscas dura menos de 1 ms. Essas causas são hipóteses e não foram testadas.

**Remoção.** A Rubro-Negra foi mais rápida nos três tamanhos: 0,322 contra 0,704 µs/op (cerca de 2,2×), 0,653 contra 1,126 (cerca de 1,7×) e 1,148 contra 2,144 (cerca de 1,9×). O número de rotações foi parecido nas duas (3.894 contra 3.623 com 90.000), então as rotações não explicam a diferença. Uma explicação coerente com o código é que a AVL, por ser recursiva, recalcula a altura e o fator de balanceamento de todos os nós do caminho de volta até a raiz depois de cada remoção, enquanto a Rubro-Negra só faz correções quando remove um nó preto. Essa explicação também é uma hipótese, sem medição com profiler.

#### 3.5.3 Comparação com a complexidade teórica (Big-O)

A análise de complexidade apresentada na seção 2 prevê O(log n) para inserção, busca e remoção nas duas árvores. Ao passar de 1.000 para 90.000 registros (90× mais dados), o tempo por operação cresceu cerca de 2,0× (AVL) e 2,6× (Rubro-Negra) na inserção, 2,1× e 1,7× na busca, e 3,0× e 3,6× na remoção. Se as operações fossem O(n), o crescimento seria de 90×; para O(log n) seria de cerca de 1,65× (log₂ 90.000 ÷ log₂ 1.000). Os resultados ficam, portanto, muito longe de O(n) e um pouco acima de O(log n), o que é compatível com o custo de acesso à memória em árvores maiores (com 90.000 nós de 424 bytes, a árvore ocupa cerca de 38 MB, possivelmente mais do que cabe na cache do processador). As alturas medidas (11, 15 e 19) acompanham log₂ n e ficam dentro dos limites teóricos de cada estrutura (cerca de 1,44·log₂ n para a AVL e 2·log₂(n+1) para a Rubro-Negra).
### 4.1 Pré-requisitos 

Para compilar e executar as árvores, você precisará de:

*   **Compilador C (GCC):** Necessário para compilar os arquivos `.c`. 
    *   *Windows:* Pode ser instalado através do MinGW-w64. Certifique-se de adicionar o caminho da pasta `bin` do GCC às variáveis de ambiente (PATH) do sistema.
*   **Arquivo de Dados:** O arquivo `jogadores.dat` deve estar na pasta dataset.
*   *Opcional:* Para visualizar os resultados gerados pelo benchmark, é recomendado ter o Excel ou outro leitor de planilhas (para abrir os arquivos `.csv`).

*   ### 4.2 Instruções 

**1. Clonar o repositório e acessar a pasta:**
git clone https://github.com/DuuDyx/FIBA-Federa-o-Internacional-de-Basquetebol.git
cd FIBA-Federa-o-Internacional-de-Basquetebol
*   Para rodar as o teste de benchmark primeiro entre na pasta src e rode os seguintes comandos gcc -Wall -Wextra -std=c11 -o avl avl.c em seguida: .\avl.exe --bench para a rubro negra apenas troque de avl para rubro negra em todas as seções que mencione avl
------------------------

## 5. Referências
* **FIBA** — Federação Internacional de Basquete. Disponível em: [https://fiba.basketball](https://fiba.basketball) [1]
* **CORMEN, T. H. et al.** *Introduction to Algorithms*. 3ª ed. MIT Press, 2009. (Capítulos de árvores balanceadas AVL e Rubro-Negras).
