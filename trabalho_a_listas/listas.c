/*
 * ============================================================
 *  TRABALHO PRATICO - ESTRUTURA DE DADOS
 *  Tema: Playlist de Musicas (Lista Duplamente Encadeada)
 *
 *  Funcionalidades Principais:
 *    1. Inserir musica          (malloc, ID unico, validacoes)
 *    2. Buscar musica por ID
 *    3. Editar musica
 *    4. Excluir musica          (free, com confirmacao)
 *    5. Listar todas as musicas
 *    6. Salvar em CSV
 *    7. Carregar do CSV
 *
 *  Funcionalidades Extras:
 *    8.  Buscar por titulo, artista ou genero
 *    9.  Estatisticas da playlist
 *    10. Listar em ordem reversa
 *    11. Limpar playlist inteira (free em todos os nos)
 *
 *    0. Sair
 * ============================================================
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 
 /* ----------------------------------------------
    CONSTANTES
    ---------------------------------------------- */
 #define ARQUIVO_CSV ".playlist.csv"
 
 /* ----------------------------------------------
    STRUCT - No da lista duplamente encadeada
    ---------------------------------------------- */
 typedef struct No {
     int  id;
     char titulo[50];
     char artista[50];
     int  duracao;       /* duracao em segundos */
     char genero[30];
     struct No *anterior;
     struct No *proximo;
 } No;
 
 /* ----------------------------------------------
    VARIAVEIS GLOBAIS
    ---------------------------------------------- */
 No  *inicio    = NULL;   /* ponteiro para o primeiro no */
 No  *fim       = NULL;   /* ponteiro para o ultimo no   */
 int  quantidade = 0;
 
 /* ----------------------------------------------
    PROTOTIPOS
    ---------------------------------------------- */
 void   limparBuffer(void);
 void   limparTela(void);
 void   pausar(int segundos);
 void   exibirMenu(void);
 No    *buscarNoPorId(int id);
 void   exibirMusica(No *no);
 void   inserirMusica(void);
 void   buscarMusica(void);
 void   editarMusica(void);
 void   excluirMusica(void);
 void   listarMusicas(void);
 void   salvarCSV(void);
 void   mostrarCSV(void);
 void   carregarCSV(void);
 static int lerCSV(void);
 
 /* Funcoes extras */
 void   buscarPorCampo(void);
 void   estatisticas(void);
 void   listarReverso(void);
 void   limparPlaylist(void);
 
 /* Funcoes auxiliares internas */
 static void   inserirNoFinal(No *novo);
 static void   removerNo(No *no);
 static int    stringVazia(const char *s);
 static void   toLowerStr(char *dest, const char *src);
 
 /* ----------------------------------------------
    FUNCOES AUXILIARES
    ---------------------------------------------- */
 
 void limparBuffer(void) {
     int c;
     while ((c = getchar()) != '\n' && c != EOF);
 }
 
 void limparTela(void) {
 #ifdef _WIN32
     system("cls");
 #else
     system("clear");
 #endif
 }
 
 void pausar(int segundos) {
 #ifdef _WIN32
     char cmd[32];
     sprintf(cmd, "timeout /t %d >nul", segundos);
     system(cmd);
 #else
     char cmd[32];
     sprintf(cmd, "sleep %d", segundos);
     system(cmd);
 #endif
 }
 
 static int stringVazia(const char *s) {
     if (s == NULL) return 1;
     while (*s) {
         if (!isspace((unsigned char)*s)) return 0;
         s++;
     }
     return 1;
 }
 
 static void toLowerStr(char *dest, const char *src) {
     while (*src) {
         *dest = (char)tolower((unsigned char)*src);
         dest++;
         src++;
     }
     *dest = '\0';
 }
 
 static void inserirNoFinal(No *novo) {
     novo->proximo  = NULL;
     novo->anterior = fim;
 
     if (fim != NULL) {
         fim->proximo = novo;
     } else {
         inicio = novo;
     }
     fim = novo;
     quantidade++;
 }
 
 static void removerNo(No *no) {
     if (no == NULL) return;
 
     if (no->anterior != NULL) {
         no->anterior->proximo = no->proximo;
     } else {
         inicio = no->proximo;
     }
 
     if (no->proximo != NULL) {
         no->proximo->anterior = no->anterior;
     } else {
         fim = no->anterior;
     }
 
     free(no);
     quantidade--;
 }
 
 void exibirMenu(void) {
     limparTela();
     printf("+==========================================+\n");
     printf("|        ** PLAYLIST MUSICAL **            |\n");
     printf("|   Total de músicas: %-5d               |\n", quantidade);
     printf("+==========================================+\n");
     printf("|  1. Inserir música                       |\n");
     printf("|  2. Buscar música por ID                 |\n");
     printf("|  3. Editar música                        |\n");
     printf("|  4. Excluir música                       |\n");
     printf("|  5. Listar todas as músicas              |\n");
     printf("|  6. Salvar em CSV                        |\n");
     printf("|  7. Carregar do CSV                      |\n");
     printf("+------------------------------------------+\n");
     printf("|  EXTRAS                                  |\n");
     printf("+------------------------------------------+\n");
     printf("|  8.  Buscar por titulo/artista/genero    |\n");
     printf("|  9.  Estatisticas da playlist            |\n");
     printf("|  10. Listar em ordem reversa             |\n");
     printf("|  11. Limpar playlist inteira             |\n");
     printf("+------------------------------------------+\n");
     printf("|  0. Sair                                 |\n");
     printf("+==========================================+\n");
     printf("Escolha uma opção: ");
 }
 
 No *buscarNoPorId(int id) {
     No *atual = inicio;
     while (atual != NULL) {
         if (atual->id == id) {
             return atual;
         }
         atual = atual->proximo;
     }
     return NULL;
 }
 
 void exibirMusica(No *no) {
     int min = no->duracao / 60;
     int seg = no->duracao % 60;
 
     printf("  +---------------------------------\n");
     printf("  | ID:      %d\n",        no->id);
     printf("  | Titulo:  %s\n",        no->titulo);
     printf("  | Artista: %s\n",        no->artista);
     printf("  | Duração: %d:%02d\n",   min, seg);
     printf("  | Gênero:  %s\n",        no->genero);
     printf("  +---------------------------------\n");
 }
 
 /* ----------------------------------------------
    FUNCIONALIDADE 1 - INSERIR
    ---------------------------------------------- */
 void inserirMusica(void) {
     limparTela();
     printf("-- INSERIR MÚSICA --\n\n");
 
     int id;
 
     printf("ID      : ");
     if (scanf("%d", &id) != 1) {
         printf("ERRO: Entrada inválida para ID.\n");
         limparBuffer();
         pausar(3);
         return;
     }
     limparBuffer();
 
     if (id <= 0) {
         printf("ERRO: O ID deve ser um número positivo.\n");
         pausar(3);
         return;
     }
 
     if (buscarNoPorId(id) != NULL) {
         printf("ERRO: Já existe uma música com o ID %d.\n", id);
         pausar(3);
         return;
     }
 
     char titulo[50];
     printf("Titulo  : ");
     fgets(titulo, sizeof(titulo), stdin);
     titulo[strcspn(titulo, "\n")] = '\0';
     if (stringVazia(titulo)) {
         printf("ERRO: O titulo não pode ser vazio.\n");
         pausar(3);
         return;
     }
 
     char artista[50];
     printf("Artista : ");
     fgets(artista, sizeof(artista), stdin);
     artista[strcspn(artista, "\n")] = '\0';
     if (stringVazia(artista)) {
         printf("ERRO: O artista não pode ser vazio.\n");
         pausar(3);
         return;
     }
 
     int duracao;
     printf("Duracao (segundos): ");
     if (scanf("%d", &duracao) != 1 || duracao <= 0) {
         printf("ERRO: Duração inválida. Deve ser um número positivo.\n");
         limparBuffer();
         pausar(3);
         return;
     }
     limparBuffer();
 
     char genero[30];
     printf("Genero  : ");
     fgets(genero, sizeof(genero), stdin);
     genero[strcspn(genero, "\n")] = '\0';
     if (stringVazia(genero)) {
         printf("ERRO: O gênero não pode ser vazio.\n");
         pausar(3);
         return;
     }
 
     No *novo = (No *)malloc(sizeof(No));
     if (novo == NULL) {
         printf("ERRO: Falha ao alocar memória.\n");
         pausar(3);
         return;
     }
 
     novo->id      = id;
     novo->duracao  = duracao;
     strncpy(novo->titulo,  titulo,  sizeof(novo->titulo)  - 1);
     novo->titulo[sizeof(novo->titulo) - 1] = '\0';
     strncpy(novo->artista, artista, sizeof(novo->artista) - 1);
     novo->artista[sizeof(novo->artista) - 1] = '\0';
     strncpy(novo->genero,  genero,  sizeof(novo->genero)  - 1);
     novo->genero[sizeof(novo->genero) - 1] = '\0';
 
     inserirNoFinal(novo);
 
     limparTela();
     printf("-- MÚSICA CADASTRADA COM SUCESSO! (Total: %d) --\n\n", quantidade);
     exibirMusica(novo);
     printf("\nVoltando ao menu em 6 segundos...\n");
     pausar(6);
 }
 
 /* ----------------------------------------------
    FUNCIONALIDADE 2 - BUSCAR POR ID
    ---------------------------------------------- */
 void buscarMusica(void) {
     limparTela();
     printf("-- BUSCAR MÚSICA POR ID --\n\n");
 
     if (quantidade == 0) {
         printf("AVISO: A playlist esta vazia.\n");
         pausar(3);
         return;
     }
 
     int id;
     printf("Digite o ID da musica: ");
     if (scanf("%d", &id) != 1) {
         printf("ERRO: Entrada inválida.\n");
         limparBuffer();
         pausar(3);
         return;
     }
     limparBuffer();
 
     No *encontrado = buscarNoPorId(id);
     if (encontrado == NULL) {
         printf("ERRO: Nenhuma música encontrada com ID %d.\n", id);
         pausar(3);
         return;
     }
 
     printf("\nMusica encontrada:\n\n");
     exibirMusica(encontrado);
     printf("\nPressione ENTER para voltar ao menu...");
     limparBuffer();
 }
 
 /* ----------------------------------------------
    FUNCIONALIDADE 3 - EDITAR
    ---------------------------------------------- */
 void editarMusica(void) {
     limparTela();
     printf("-- EDITAR MÚSICA --\n\n");
 
     if (quantidade == 0) {
         printf("AVISO: A playlist esta vazia.\n");
         pausar(3);
         return;
     }
 
     int id;
     printf("Digite o ID da música a editar: ");
     if (scanf("%d", &id) != 1) {
         printf("ERRO: Entrada inválida.\n");
         limparBuffer();
         pausar(3);
         return;
     }
     limparBuffer();
 
     No *no = buscarNoPorId(id);
     if (no == NULL) {
         printf("ERRO: Nenhuma música encontrada com ID %d.\n", id);
         pausar(3);
         return;
     }
 
     printf("\nMúsica atual:\n\n");
     exibirMusica(no);
     printf("\nDeixe em branco para manter o valor atual.\n\n");
 
     char buffer[50];
 
     printf("Novo titulo  [%s]: ", no->titulo);
     fgets(buffer, sizeof(buffer), stdin);
     buffer[strcspn(buffer, "\n")] = '\0';
     if (!stringVazia(buffer)) {
         strncpy(no->titulo, buffer, sizeof(no->titulo) - 1);
         no->titulo[sizeof(no->titulo) - 1] = '\0';
     }
 
     printf("Novo artista [%s]: ", no->artista);
     fgets(buffer, sizeof(buffer), stdin);
     buffer[strcspn(buffer, "\n")] = '\0';
     if (!stringVazia(buffer)) {
         strncpy(no->artista, buffer, sizeof(no->artista) - 1);
         no->artista[sizeof(no->artista) - 1] = '\0';
     }
 
     printf("Nova duração [%d seg]: ", no->duracao);
     fgets(buffer, sizeof(buffer), stdin);
     buffer[strcspn(buffer, "\n")] = '\0';
     if (!stringVazia(buffer)) {
         int nova_dur = atoi(buffer);
         if (nova_dur > 0) {
             no->duracao = nova_dur;
         } else {
             printf("AVISO: Duração inválida, mantido o valor anterior.\n");
         }
     }
 
     printf("Novo gênero  [%s]: ", no->genero);
     fgets(buffer, sizeof(buffer), stdin);
     buffer[strcspn(buffer, "\n")] = '\0';
     if (!stringVazia(buffer)) {
         strncpy(no->genero, buffer, sizeof(no->genero) - 1);
         no->genero[sizeof(no->genero) - 1] = '\0';
     }
 
     limparTela();
     printf("-- MÚSICA ATUALIZADA COM SUCESSO! --\n\n");
     exibirMusica(no);
     printf("\nVoltando ao menu em 6 segundos...\n");
     pausar(6);
 }
 
 /* ----------------------------------------------
    FUNCIONALIDADE 4 - EXCLUIR
    ---------------------------------------------- */
 void excluirMusica(void) {
     limparTela();
     printf("-- EXCLUIR MÚSICA --\n\n");
 
     if (quantidade == 0) {
         printf("AVISO: A playlist esta vazia.\n");
         pausar(3);
         return;
     }
 
     int id;
     printf("Digite o ID da música a excluir: ");
     if (scanf("%d", &id) != 1) {
         printf("ERRO: Entrada inválida.\n");
         limparBuffer();
         pausar(3);
         return;
     }
     limparBuffer();
 
     No *no = buscarNoPorId(id);
     if (no == NULL) {
         printf("ERRO: Nenhuma música encontrada com ID %d.\n", id);
         pausar(3);
         return;
     }
 
     printf("\nMúsica a excluir:\n\n");
     exibirMusica(no);
 
     printf("Tem certeza que deseja excluir? (s/n): ");
     char conf;
     scanf(" %c", &conf);
     limparBuffer();
     if (conf != 's' && conf != 'S') {
         printf("Operação cancelada.\n");
         pausar(3);
         return;
     }
 
     removerNo(no);
 
     printf("\nOK: Música excluida com sucesso! (Total restante: %d)\n", quantidade);
     printf("Voltando ao menu em 3 segundos...\n");
     pausar(3);
 }
 
 /* ----------------------------------------------
    FUNCIONALIDADE 5 - LISTAR
    ---------------------------------------------- */
 void listarMusicas(void) {
     limparTela();
     printf("-- PLAYLIST COMPLETA --\n\n");
 
     if (quantidade == 0) {
         printf("AVISO: A playlist esta vazia.\n");
         pausar(3);
         return;
     }
 
     printf("Total: %d música(s)\n\n", quantidade);
 
     int i = 1;
     No *atual = inicio;
     while (atual != NULL) {
         printf("#%d\n", i);
         exibirMusica(atual);
         atual = atual->proximo;
         i++;
     }
 
     printf("\nPressione ENTER para voltar ao menu...");
     limparBuffer();
 }
 
 /* ----------------------------------------------
    FUNCIONALIDADE 6 - SALVAR CSV
    ---------------------------------------------- */
 void salvarCSV(void) {
     FILE *arq = fopen(ARQUIVO_CSV, "w");
     if (arq == NULL) {
         printf("ERRO: Não foi possivel abrir o arquivo para escrita.\n");
         pausar(3);
         return;
     }
 
     fprintf(arq, "id;titulo;artista;duracao;genero\n");
 
     No *atual = inicio;
     while (atual != NULL) {
         fprintf(arq, "%d;%s;%s;%d;%s\n",
                 atual->id,
                 atual->titulo,
                 atual->artista,
                 atual->duracao,
                 atual->genero);
         atual = atual->proximo;
     }
 
     fclose(arq);
 }
 
 /* ----------------------------------------------
    MOSTRAR CONTEUDO DO CSV NO TERMINAL
    ---------------------------------------------- */
 void mostrarCSV(void) {
     FILE *arq = fopen(ARQUIVO_CSV, "r");
     if (arq == NULL) {
         printf("ERRO: Arquivo CSV não encontrado.\n");
         return;
     }
 
     printf("\nConteúdo do arquivo \"%s\":\n", ARQUIVO_CSV);
     printf("+------------------------------------------+\n");
 
     char linha[256];
     while (fgets(linha, sizeof(linha), arq) != NULL) {
         linha[strcspn(linha, "\n")] = '\0';
         printf("  %s\n", linha);
     }
 
     printf("+------------------------------------------+\n");
     fclose(arq);
 }
 
 /* ----------------------------------------------
    FUNCAO INTERNA - le o CSV e insere na lista
    ---------------------------------------------- */
 static int lerCSV(void) {
     FILE *arq = fopen(ARQUIVO_CSV, "r");
     if (arq == NULL) return -1;
 
     char linha[256];
 
     if (fgets(linha, sizeof(linha), arq) == NULL) {
         fclose(arq);
         return 0;
     }
 
     int carregados = 0;
 
     while (fgets(linha, sizeof(linha), arq) != NULL) {
         linha[strcspn(linha, "\n")] = '\0';
 
         char *token;
 
         token = strtok(linha, ";");
         if (token == NULL) continue;
         int id = atoi(token);
 
         token = strtok(NULL, ";");
         if (token == NULL) continue;
         char titulo[50];
         strncpy(titulo, token, sizeof(titulo) - 1);
         titulo[sizeof(titulo) - 1] = '\0';
 
         token = strtok(NULL, ";");
         if (token == NULL) continue;
         char artista[50];
         strncpy(artista, token, sizeof(artista) - 1);
         artista[sizeof(artista) - 1] = '\0';
 
         token = strtok(NULL, ";");
         if (token == NULL) continue;
         int duracao = atoi(token);
 
         token = strtok(NULL, ";");
         if (token == NULL) continue;
         char genero[30];
         strncpy(genero, token, sizeof(genero) - 1);
         genero[sizeof(genero) - 1] = '\0';
 
         if (buscarNoPorId(id) != NULL) continue;
         if (id <= 0 || duracao <= 0) continue;
         if (stringVazia(titulo) || stringVazia(artista) || stringVazia(genero)) continue;
 
         No *novo = (No *)malloc(sizeof(No));
         if (novo == NULL) continue;
 
         novo->id      = id;
         novo->duracao  = duracao;
         strncpy(novo->titulo,  titulo,  sizeof(novo->titulo)  - 1);
         novo->titulo[sizeof(novo->titulo) - 1] = '\0';
         strncpy(novo->artista, artista, sizeof(novo->artista) - 1);
         novo->artista[sizeof(novo->artista) - 1] = '\0';
         strncpy(novo->genero,  genero,  sizeof(novo->genero)  - 1);
         novo->genero[sizeof(novo->genero) - 1] = '\0';
 
         inserirNoFinal(novo);
         carregados++;
     }
 
     fclose(arq);
     return carregados;
 }
 
 /* ----------------------------------------------
    FUNCIONALIDADE 7 - CARREGAR CSV (manual)
    ---------------------------------------------- */
 void carregarCSV(void) {
     limparTela();
     printf("-- CARREGAR DO CSV --\n\n");
 
     while (inicio != NULL) {
         removerNo(inicio);
     }
 
     int result = lerCSV();
     if (result == -1) {
         printf("AVISO: Arquivo \"%s\" não encontrado.\n", ARQUIVO_CSV);
     } else {
         printf("OK: %d musica(s) carregada(s) de \"%s\".\n", result, ARQUIVO_CSV);
     }
 
     printf("\nVoltando ao menu em 3 segundos...\n");
     pausar(3);
 }
 
 /* ==============================================================
    FUNCIONALIDADES EXTRAS
    ============================================================== */
 
 /* EXTRA 8 - BUSCAR POR TITULO, ARTISTA OU GENERO */
 void buscarPorCampo(void) {
     limparTela();
     printf("-- BUSCAR POR TITULO / ARTISTA / GENERO --\n\n");
 
     if (quantidade == 0) {
         printf("AVISO: A playlist esta vazia.\n");
         pausar(3);
         return;
     }
 
     printf("Buscar por:\n");
     printf("  1. Titulo\n");
     printf("  2. Artista\n");
     printf("  3. Genero\n");
     printf("Opcao: ");
 
     int opcao;
     if (scanf("%d", &opcao) != 1 || opcao < 1 || opcao > 3) {
         printf("ERRO: Opção inválida.\n");
         limparBuffer();
         pausar(3);
         return;
     }
     limparBuffer();
 
     char termo[50];
     printf("Digite o termo de busca: ");
     fgets(termo, sizeof(termo), stdin);
     termo[strcspn(termo, "\n")] = '\0';
 
     if (stringVazia(termo)) {
         printf("ERRO: Termo de busca não pode ser vazio.\n");
         pausar(3);
         return;
     }
 
     char termoLower[50];
     toLowerStr(termoLower, termo);
 
     int encontradas = 0;
     No *atual = inicio;
 
     printf("\nResultados da busca:\n\n");
 
     while (atual != NULL) {
         char campoLower[50];
 
         switch (opcao) {
             case 1: toLowerStr(campoLower, atual->titulo);  break;
             case 2: toLowerStr(campoLower, atual->artista); break;
             case 3: toLowerStr(campoLower, atual->genero);  break;
         }
 
         if (strstr(campoLower, termoLower) != NULL) {
             exibirMusica(atual);
             encontradas++;
         }
 
         atual = atual->proximo;
     }
 
     if (encontradas == 0) {
         printf("Nenhuma música encontrada com o termo \"%s\".\n", termo);
     } else {
         printf("\nTotal encontrado: %d música(s).\n", encontradas);
     }
 
     printf("\nPressione ENTER para voltar ao menu...");
     limparBuffer();
 }
 
 /* EXTRA 9 - ESTATISTICAS DA PLAYLIST */
 void estatisticas(void) {
     limparTela();
     printf("-- ESTATISTICAS DA PLAYLIST --\n\n");
 
     if (quantidade == 0) {
         printf("AVISO: A playlist esta vazia.\n");
         pausar(3);
         return;
     }
 
     int duracaoTotal = 0;
     int maiorDuracao = 0;
     int menorDuracao = 0;
     No *maisCurta    = NULL;
     No *maisLonga    = NULL;
 
     No *atual = inicio;
 
     maiorDuracao = atual->duracao;
     menorDuracao = atual->duracao;
     maisLonga    = atual;
     maisCurta    = atual;
 
     while (atual != NULL) {
         duracaoTotal += atual->duracao;
 
         if (atual->duracao > maiorDuracao) {
             maiorDuracao = atual->duracao;
             maisLonga    = atual;
         }
         if (atual->duracao < menorDuracao) {
             menorDuracao = atual->duracao;
             maisCurta    = atual;
         }
 
         atual = atual->proximo;
     }
 
     int mediaSegundos = duracaoTotal / quantidade;
     int totalMin = duracaoTotal / 60;
     int totalSeg = duracaoTotal % 60;
     int mediaMin = mediaSegundos / 60;
     int mediaSeg = mediaSegundos % 60;
 
     printf("  Total de músicas:    %d\n", quantidade);
     printf("  Duração total:       %d:%02d (min:seg)\n", totalMin, totalSeg);
     printf("  Duração média:       %d:%02d (min:seg)\n", mediaMin, mediaSeg);
     printf("\n  Música mais longa:\n");
     exibirMusica(maisLonga);
     printf("\n  Música mais curta:\n");
     exibirMusica(maisCurta);
 
     printf("\nPressione ENTER para voltar ao menu...");
     limparBuffer();
 }
 
 /* EXTRA 10 - LISTAR EM ORDEM REVERSA */
 void listarReverso(void) {
     limparTela();
     printf("-- PLAYLIST EM ORDEM REVERSA --\n\n");
 
     if (quantidade == 0) {
         printf("AVISO: A playlist esta vazia.\n");
         pausar(3);
         return;
     }
 
     printf("Total: %d música(s)\n\n", quantidade);
 
     int i = quantidade;
     No *atual = fim;
     while (atual != NULL) {
         printf("#%d\n", i);
         exibirMusica(atual);
         atual = atual->anterior;
         i--;
     }
 
     printf("\nPressione ENTER para voltar ao menu...");
     limparBuffer();
 }
 
 /* EXTRA 11 - LIMPAR PLAYLIST INTEIRA */
 void limparPlaylist(void) {
     limparTela();
     printf("-- LIMPAR PLAYLIST INTEIRA --\n\n");
 
     if (quantidade == 0) {
         printf("AVISO: A playlist já esta vazia.\n");
         pausar(3);
         return;
     }
 
     printf("ATENÇÃO: Isso ira remover TODAS as %d musica(s) da playlist.\n", quantidade);
     printf("Tem certeza? (s/n): ");
 
     char conf;
     scanf(" %c", &conf);
     limparBuffer();
 
     if (conf != 's' && conf != 'S') {
         printf("Operação cancelada.\n");
         pausar(3);
         return;
     }
 
     int total = quantidade;
     while (inicio != NULL) {
         removerNo(inicio);
     }
 
     printf("\nOK: %d música(s) removida(s). Playlist limpa!\n", total);
     printf("Voltando ao menu em 3 segundos...\n");
     pausar(3);
 }
 
 /* ----------------------------------------------
    MAIN
    ---------------------------------------------- */
 int main(void) {
     lerCSV();
 
     int opcao;
 
     do {
         exibirMenu();
 
         if (scanf("%d", &opcao) != 1) {
             limparBuffer();
             opcao = -1;
             continue;
         }
         limparBuffer();
 
         switch (opcao) {
             case 1:  inserirMusica();    break;
             case 2:  buscarMusica();     break;
             case 3:  editarMusica();     break;
             case 4:  excluirMusica();    break;
             case 5:  listarMusicas();    break;
             case 6:
                 salvarCSV();
                 limparTela();
                 printf("OK: Dados salvos em \"%s\" (%d registro(s)).\n",
                        ARQUIVO_CSV, quantidade);
                 mostrarCSV();
                 printf("\nPressione ENTER para voltar ao menu...");
                 limparBuffer();
                 break;
             case 7:  carregarCSV();      break;
             case 8:  buscarPorCampo();   break;
             case 9:  estatisticas();     break;
             case 10: listarReverso();    break;
             case 11: limparPlaylist();   break;
             case 0:
                 salvarCSV();
                 while (inicio != NULL) {
                     removerNo(inicio);
                 }
                 limparTela();
                 printf("Dados salvos. Até logo!\n");
                 break;
             default:
                 break;
         }
 
     } while (opcao != 0);
 
     return 0;
 }
 