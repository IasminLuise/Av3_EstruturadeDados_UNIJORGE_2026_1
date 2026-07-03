# Trabalho Final - Estrutura de Dados

Repositório para desenvolvimento dos trabalhos da disciplina Estrutura de Dados do 3º semestre da UNIJORGE 2026.1.

## Integrantes

- Albert Claudio Arruda Nascimento
- Giovana Gusmão Santos
- Iasmin Luise Santos da Silva
- Monique Amorim Mendes

## Turma

- 42CMP01M3A

## Curso

- Ciência da Computação

## Organização do repositório

```text
Trabalho-Estrutura-De-Dados/
+-- README.md
+-- trabalho_a_listas_simples.c
+-- trabalho_b_pilhas.c
+-- trabalho_c_filas_simples.c
+-- trabalho_a_listas_simples.exe
+-- trabalho_b_pilhas.exe
+-- pilhas.exe
+-- filas.exe
```

## Temas escolhidos

- Trabalho A: Playlist de Músicas, utilizando lista duplamente encadeada.
- Trabalho B: Histórico de Navegação Web, utilizando pilha dinâmica.
- Trabalho C: Fila de Impressão, utilizando filas dinâmicas.

# Contribução de cada um do Integrantes

- Trabalho A - Giovana
- Trabalho B - Monique
- Trabalho C - Iasmin
- Extras - Albert

## Explicação breve de cada programa

### Trabalho A - Playlist de Músicas

Programa que gerencia uma playlist de músicas por meio de uma lista duplamente encadeada. Permite inserir, buscar, editar, excluir, listar, salvar e carregar músicas em arquivo CSV. Possui também funcionalidades extras documentadas abaixo.

### Trabalho B - Histórico de Navegação Web

Programa que simula o funcionamento de um navegador web utilizando pilhas dinâmicas. Permite visitar páginas, voltar, avançar, visualizar a página atual, buscar, editar, excluir, listar histórico, salvar e carregar dados em CSV. Possui também funcionalidades extras documentadas abaixo.

### Trabalho C - Fila de Impressão

Programa que simula um sistema de fila de impressão utilizando filas dinâmicas. Permite adicionar trabalhos, processar impressões, buscar por ID, cancelar trabalhos, listar filas, salvar e carregar dados em CSV. Possui também funcionalidades extras documentadas abaixo.

## Funcionalidades extras

### Trabalho A — Playlist (opções 8–11)

- **8. Buscar por título/artista/gênero** — busca parcial (contém o termo), sem diferenciar maiúsculas/minúsculas.
- **9. Estatísticas da playlist** — total de músicas, duração total e média, música mais longa e mais curta.
- **10. Listar em ordem reversa** — exibe a playlist do último ao primeiro nó.
- **11. Limpar playlist inteira** — remove todas as músicas com `free`, após confirmação.

### Trabalho B — Histórico Web (opções 11–14)

- **11. Buscar por título/URL/categoria** — busca parcial nas pilhas de histórico e avançar.
- **12. Estatísticas do histórico** — total de páginas, visitas, favoritos, média de visitas e página mais visitada.
- **13. Listar favoritos** — mostra apenas páginas marcadas como favoritas.
- **14. Limpar histórico inteiro** — libera todas as páginas das duas pilhas, após confirmação.

### Trabalho C — Fila de Impressão (opções 8–11)

- **8. Estatísticas das filas** — quantidade de trabalhos (normal e prioritário), páginas totais/média, maior e menor trabalho.
- **9. Buscar por nome do arquivo** — busca parcial nas duas filas.
- **10. Mostrar próximo trabalho** — indica o próximo a ser processado (prioritário primeiro), sem remover da fila.
- **11. Limpar todas as filas** — remove todos os trabalhos com `free`, após confirmação.

## Instruções para compilar

Para compilar os programas, utilize o GCC no terminal dentro da pasta do projeto:

```bash
gcc trabalho_a_listas_simples.c -o trabalho_a_listas_simples
gcc trabalho_b_pilhas.c -o trabalho_b_pilhas
gcc trabalho_c_filas_simples.c -o trabalho_c_filas_simples
```

## Instruções para executar

Após a compilação, execute cada programa separadamente:

```bash
./trabalho_a_listas_simples
./trabalho_b_pilhas
./trabalho_c_filas_simples
```

No Windows, também é possível executar os arquivos `.exe` gerados:

```bash
trabalho_a_listas_simples.exe
trabalho_b_pilhas.exe
filas.exe
```

Se estiver utilizando o GDB Online:

1. Abrir um novo projeto.
2. Escolher a linguagem C.
3. Copiar o conteúdo do arquivo correspondente ao trabalho.
4. Clicar em **Run**.
5. Utilizar o menu exibido no terminal.

## Observações sobre o funcionamento

Os três programas funcionam de forma independente e devem ser avaliados separadamente.

Cada programa possui menu próprio no terminal e utiliza arquivos CSV para salvar e carregar os dados cadastrados durante a execução.

## Dificuldades encontradas

Não foram encontradas dificuldades durante o desenvolvimento dos trabalhos.

