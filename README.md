#  FIBA – Jogadores de Basquete Internacionais

# Integrantes do Grupo
* Eduardo Dylan Mendes — RA: 43541194 
* Erick Ronald Huanca Clemente — RGM 43541194
*	Marcos Antonio Ribeiro Lago Junior 46450769
*	Vitor Boer Blanco — RGM 42402794
*	Vagner Ferreira José Maria — RGM 440554807


## 1. Dataset

### 1.1 Descrição
O dataset é composto por dados de jogadores(as) de basquete cadastrados na **FIBA** (Federação Internacional de Basquete), obtidos via *scraping* do site oficial `fiba.basketball`. Cada registro representa um(a) atleta e contém informações como nome, data de nascimento, altura, seleção pela qual atuou e nacionalidade.

* **Formato original:** JSON (`fiba_jogadores.json`), contendo uma lista de objetos (um por jogador).
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

### 2.3 Análise de Complexidade

#### Árvore AVL

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


```
### 3.3 Casos Extremos (Edge Cases)

| Operação | Caso Extremo | AVL | Rubro-Negra |
| :--- | :--- | :---: | :---: |
| **Inserção** | Registros inseridos em ordem estritamente crescente pela chave | O(log n) | O(log n) |
| **Remoção** | Elementos removidos de posições estratégicas que forçam rebalanceamentos em cascata | O(log n) | O(log n) |
| **Busca** | Localização do último registro teórico da ordem alfabética | O(log n) | O(log n) |
| **Inserção** | Tentativa de reinserir uma chave composta já existente (duplicada) | O(log n) | O(log n) |

---

## 5. Referências
* **FIBA** — Federação Internacional de Basquete. Disponível em: [https://fiba.basketball](https://fiba.basketball) [1]
* **CORMEN, T. H. et al.** *Introduction to Algorithms*. 3ª ed. MIT Press, 2009. (Capítulos de árvores balanceadas AVL e Rubro-Negras).
