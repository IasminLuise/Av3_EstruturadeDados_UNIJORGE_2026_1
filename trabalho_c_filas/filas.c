#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 

/* ============================================================
 *  TRABALHO PRATICO - ESTRUTURA DE DADOS
 *  Tema: Fila de Impressao (Filas Dinamicas)
 *
 *  Funcionalidades Principais:
 *    1. Adicionar trabalho     (malloc, validacoes)
 *    2. Processar impressao   (free, prioridade primeiro)
 *    3. Buscar trabalho por ID
 *    4. Cancelar trabalho     (free, com confirmacao)
 *    5. Listar filas
 *    6. Salvar CSV
 *    7. Carregar CSV
 *
 *  Funcionalidades Extras:
 *    8.  Estatisticas das filas
 *    9.  Buscar por nome do arquivo
 *    10. Mostrar proximo trabalho
 *    11. Limpar todas as filas (free em todos os nos)
 *
 *    0. Sair (libera filas)
 * ============================================================ */

#define TAM_ARQUIVO 100
#define ARQUIVO_CSV "dados_c.csv"

/* ============================================================
 *  Struct do No da Fila Dinamica (lista ligada)
 * ============================================================ */
typedef struct NoFila {
    int  id;
    char arquivo[TAM_ARQUIVO];
    int  paginas;
    char tipo;              /* 'N' = normal, 'P' = prioritario */
    struct NoFila *proximo;
} NoFila;

/* ============================================================
 *  Struct da Fila Dinamica: ponteiros para frente e tras + contador
 * ============================================================ */
typedef struct {
    NoFila *frente;   /* primeiro da fila (proximo a sair) */
    NoFila *tras;     /* ultimo da fila (ultimo a entrar)  */
    int     tamanho;
} Fila;

/* ============================================================
 *  Prototipos
 * ============================================================ */
/* I/O */
void     limpar_buffer(void);
void     limpar_tela(void);
void     pausar(void);
int      ler_inteiro(const char *msg, int min, int max);
void     ler_string(const char *msg, char *dest, int tamanho);

/* Operacoes primitivas da fila dinamica */
void     fila_inicializar(Fila *f);
int      fila_vazia(const Fila *f);
int      fila_tamanho(const Fila *f);
int      fila_enfileirar(Fila *f, NoFila *novo);
NoFila  *fila_desenfileirar(Fila *f);
NoFila  *fila_frente(const Fila *f);
void     fila_liberar(Fila *f);

/* Helpers */
NoFila  *criar_no_fila(void);
void     exibir_trabalho(const NoFila *no);
int      proximo_id(const Fila *normal, const Fila *prioridade);
void     to_lower_str(char *dest, const char *src);
int      total_paginas_fila(const Fila *f);

/* Funcoes principais */
void     adicionar_trabalho(Fila *normal, Fila *prioridade);
void     processar_trabalho(Fila *normal, Fila *prioridade);
void     buscar_trabalho(const Fila *normal, const Fila *prioridade);
void     cancelar_trabalho(Fila *normal, Fila *prioridade);
void     listar_filas(const Fila *normal, const Fila *prioridade);

/* Extras */
void     estatisticas_filas(const Fila *normal, const Fila *prioridade);
void     buscar_por_nome(const Fila *normal, const Fila *prioridade);
void     mostrar_proximo(const Fila *normal, const Fila *prioridade);
void     limpar_filas(Fila *normal, Fila *prioridade);

/* Persistencia */
void     salvar_csv(const Fila *normal, const Fila *prioridade);
void     carregar_csv(Fila *normal, Fila *prioridade, int silencioso);

/* Menu */
void     exibir_menu(const Fila *normal, const Fila *prioridade);

/* ============================================================
 *  I/O seguro
 * ============================================================ */
void limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void limpar_tela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar(void) {
    printf("\n  Pressione ENTER para continuar...");
    limpar_buffer();
}

int ler_inteiro(const char *msg, int min, int max) {
    int valor;
    char linha[64];
    while (1) {
        printf("%s", msg);
        fflush(stdout);
        if (fgets(linha, sizeof(linha), stdin) == NULL) exit(0);
        if (sscanf(linha, "%d", &valor) == 1 && valor >= min && valor <= max)
            return valor;
        printf("  [!] Valor invalido. Digite entre %d e %d.\n", min, max);
    }
}

void ler_string(const char *msg, char *dest, int tamanho) {
    while (1) {
        printf("%s", msg);
        fflush(stdout);
        if (fgets(dest, tamanho, stdin) == NULL) exit(0);
        dest[strcspn(dest, "\n")] = '\0';
        if (strlen(dest) > 0) return;
        printf("  [!] Campo obrigatorio.\n");
    }
}

/* ============================================================
 *  Operacoes primitivas da FILA DINAMICA
 * ============================================================ */

/* Inicializa a fila (vazia) */
void fila_inicializar(Fila *f) {
    f->frente  = NULL;
    f->tras    = NULL;
    f->tamanho = 0;
}

/* Verifica se a fila esta vazia */
int fila_vazia(const Fila *f) {
    return f->frente == NULL;
}

/* Retorna o numero de elementos na fila */
int fila_tamanho(const Fila *f) {
    return f->tamanho;
}

/* ENFILEIRAR: insere no final da fila. Retorna 1 se ok. */
int fila_enfileirar(Fila *f, NoFila *novo) {
    if (novo == NULL) return 0;
    novo->proximo = NULL;

    if (f->tras != NULL) {
        f->tras->proximo = novo;
    } else {
        /* Fila estava vazia */
        f->frente = novo;
    }
    f->tras = novo;
    f->tamanho++;
    return 1;
}

/* DESENFILEIRAR: remove da frente da fila e retorna o ponteiro.
 * O caller e responsavel por dar free. Retorna NULL se vazia. */
NoFila *fila_desenfileirar(Fila *f) {
    if (fila_vazia(f)) return NULL;
    NoFila *removido = f->frente;
    f->frente = removido->proximo;
    if (f->frente == NULL) {
        /* Fila ficou vazia */
        f->tras = NULL;
    }
    removido->proximo = NULL;
    f->tamanho--;
    return removido;
}

/* FRENTE: retorna ponteiro para o primeiro sem remover. NULL se vazia. */
NoFila *fila_frente(const Fila *f) {
    return f->frente;
}

/* Libera todos os nos da fila com free */
void fila_liberar(Fila *f) {
    NoFila *atual = f->frente;
    while (atual != NULL) {
        NoFila *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
    f->frente  = NULL;
    f->tras    = NULL;
    f->tamanho = 0;
}

/* ============================================================
 *  Helpers
 * ============================================================ */

/* Aloca um novo no com malloc */
NoFila *criar_no_fila(void) {
    NoFila *novo = (NoFila *)malloc(sizeof(NoFila));
    if (novo == NULL) {
        printf("  [ERRO] Falha ao alocar memoria.\n");
        return NULL;
    }
    memset(novo, 0, sizeof(NoFila));
    return novo;
}

/* Converte string para minusculas (busca case-insensitive) */
void to_lower_str(char *dest, const char *src) {
    while (*src) {
        *dest = (char)tolower((unsigned char)*src);
        dest++;
        src++;
    }
    *dest = '\0';
}

/* Calcula total de paginas em uma fila */
int total_paginas_fila(const Fila *f) {
    int total = 0;
    NoFila *atual = f->frente;
    while (atual != NULL) {
        total += atual->paginas;
        atual = atual->proximo;
    }
    return total;
}

/* Verifica se um ID ja existe em alguma fila */
static int id_existe(const Fila *normal, const Fila *prioridade, int id) {
    NoFila *atual;

    atual = normal->frente;
    while (atual != NULL) {
        if (atual->id == id) return 1;
        atual = atual->proximo;
    }

    atual = prioridade->frente;
    while (atual != NULL) {
        if (atual->id == id) return 1;
        atual = atual->proximo;
    }

    return 0;
}

/* Gera proximo ID unico */
int proximo_id(const Fila *normal, const Fila *prioridade) {
    int max = 0;
    NoFila *atual;

    atual = normal->frente;
    while (atual != NULL) {
        if (atual->id > max) max = atual->id;
        atual = atual->proximo;
    }

    atual = prioridade->frente;
    while (atual != NULL) {
        if (atual->id > max) max = atual->id;
        atual = atual->proximo;
    }

    return max + 1;
}

/* Exibe os dados de um trabalho formatados */
void exibir_trabalho(const NoFila *no) {
    printf("  +------------------------------------------+\n");
    printf("  | ID:       %-30d|\n", no->id);
    printf("  | Arquivo:  %-30.30s|\n", no->arquivo);
    printf("  | Paginas:  %-30d|\n", no->paginas);
    printf("  | Tipo:     %-30s|\n",
           no->tipo == 'P' ? "PRIORITARIO" : "Normal");
    printf("  +------------------------------------------+\n");
}

/* Remove um no especifico da fila pelo ID.
 * Retorna o no removido (caller deve dar free) ou NULL. */
static NoFila *remover_por_id(Fila *f, int id) {
    NoFila *prev  = NULL;
    NoFila *atual = f->frente;

    while (atual != NULL) {
        if (atual->id == id) {
            if (prev == NULL) {
                /* E o primeiro (frente) */
                f->frente = atual->proximo;
            } else {
                prev->proximo = atual->proximo;
            }
            /* Se era o ultimo (tras) */
            if (atual == f->tras) {
                f->tras = prev;
            }
            atual->proximo = NULL;
            f->tamanho--;
            return atual;
        }
        prev  = atual;
        atual = atual->proximo;
    }
    return NULL;
}

/* ============================================================
 *  Menu com status no topo e secao extras
 * ============================================================ */
void exibir_menu(const Fila *normal, const Fila *prioridade) {
    limpar_tela();
    int total_trab   = fila_tamanho(normal) + fila_tamanho(prioridade);
    int total_pag    = total_paginas_fila(normal) + total_paginas_fila(prioridade);

    printf("+====================================================+\n");
    printf("|          FILA DE IMPRESSAO - FILA DINAMICA          |\n");
    printf("+====================================================+\n");
    printf("| Trabalhos: %-3d  (Normal: %-3d | Priorit.: %-3d)      |\n",
           total_trab, fila_tamanho(normal), fila_tamanho(prioridade));
    printf("| Paginas pendentes: %-5d                            |\n", total_pag);
    printf("+----------------------------------------------------+\n");
    printf("|  1. Adicionar trabalho                              |\n");
    printf("|  2. Processar impressao                             |\n");
    printf("|  3. Buscar trabalho por ID                          |\n");
    printf("|  4. Cancelar trabalho                               |\n");
    printf("|  5. Listar filas                                    |\n");
    printf("|  6. Salvar CSV                                      |\n");
    printf("|  7. Carregar CSV                                    |\n");
    printf("+----------------------------------------------------+\n");
    printf("|  EXTRAS                                             |\n");
    printf("+----------------------------------------------------+\n");
    printf("|  8.  Estatisticas das filas                         |\n");
    printf("|  9.  Buscar por nome do arquivo                     |\n");
    printf("|  10. Mostrar proximo trabalho                       |\n");
    printf("|  11. Limpar todas as filas                          |\n");
    printf("+----------------------------------------------------+\n");
    printf("|  0. Sair                                            |\n");
    printf("+====================================================+\n\n");
}

/* ============================================================
 *  1. ADICIONAR TRABALHO (malloc, validacoes)
 * ============================================================ */
void adicionar_trabalho(Fila *normal, Fila *prioridade) {
    limpar_tela();
    printf("=== ADICIONAR TRABALHO ===\n\n");

    /* ID: pode ser automatico ou manual */
    int id;
    printf("  ID (0 para gerar automaticamente): ");
    {
        char linha[64];
        if (fgets(linha, sizeof(linha), stdin) == NULL) return;
        if (sscanf(linha, "%d", &id) != 1 || id < 0) {
            printf("  [!] ID invalido.\n");
            pausar();
            return;
        }
    }

    if (id == 0) {
        id = proximo_id(normal, prioridade);
        printf("  ID gerado automaticamente: %d\n", id);
    } else {
        /* Valida ID positivo */
        if (id <= 0) {
            printf("  [!] ID deve ser positivo.\n");
            pausar();
            return;
        }
        /* Verifica duplicidade */
        if (id_existe(normal, prioridade, id)) {
            printf("  [!] Ja existe um trabalho com ID %d.\n", id);
            pausar();
            return;
        }
    }

    /* Nome do arquivo */
    char arquivo[TAM_ARQUIVO];
    ler_string("  Nome do arquivo: ", arquivo, sizeof(arquivo));

    /* Paginas (validacao: >= 1) */
    int paginas = ler_inteiro("  Quantidade de paginas [1-99999]: ", 1, 99999);

    /* Tipo (validacao: N ou P) */
    char tipo;
    while (1) {
        char buf[16];
        ler_string("  Tipo (N=Normal / P=Prioritario): ", buf, sizeof(buf));
        tipo = (char)toupper((unsigned char)buf[0]);
        if (tipo == 'N' || tipo == 'P') break;
        printf("  [!] Tipo invalido. Digite N ou P.\n");
    }

    /* Aloca o no com malloc */
    NoFila *novo = criar_no_fila();
    if (novo == NULL) { pausar(); return; }

    novo->id      = id;
    novo->paginas = paginas;
    novo->tipo    = tipo;
    strncpy(novo->arquivo, arquivo, TAM_ARQUIVO - 1);
    novo->arquivo[TAM_ARQUIVO - 1] = '\0';

    /* Enfileira na fila correta */
    if (tipo == 'P') {
        fila_enfileirar(prioridade, novo);
    } else {
        fila_enfileirar(normal, novo);
    }

    printf("\n  [OK] Trabalho adicionado com sucesso!\n");
    exibir_trabalho(novo);
    pausar();
}

/* ============================================================
 *  2. PROCESSAR IMPRESSAO (free, prioridade primeiro)
 * ============================================================ */
void processar_trabalho(Fila *normal, Fila *prioridade) {
    limpar_tela();
    printf("=== PROCESSAR IMPRESSAO ===\n\n");

    NoFila *processado = NULL;

    /* Prioridade primeiro */
    if (!fila_vazia(prioridade)) {
        processado = fila_desenfileirar(prioridade);
        printf("  Imprimindo trabalho PRIORITARIO:\n\n");
    } else if (!fila_vazia(normal)) {
        processado = fila_desenfileirar(normal);
        printf("  Imprimindo trabalho NORMAL:\n\n");
    }

    if (processado == NULL) {
        printf("  [!] Nenhum trabalho na fila.\n");
        pausar();
        return;
    }

    exibir_trabalho(processado);

    int restante = fila_tamanho(normal) + fila_tamanho(prioridade);
    printf("\n  [OK] Trabalho processado e removido da fila.\n");
    printf("  Trabalhos restantes: %d\n", restante);

    /* Libera memoria com free */
    free(processado);
    pausar();
}

/* ============================================================
 *  3. BUSCAR TRABALHO POR ID
 * ============================================================ */
void buscar_trabalho(const Fila *normal, const Fila *prioridade) {
    limpar_tela();
    printf("=== BUSCAR TRABALHO POR ID ===\n\n");

    if (fila_vazia(normal) && fila_vazia(prioridade)) {
        printf("  [!] Filas vazias.\n");
        pausar();
        return;
    }

    int id = ler_inteiro("  ID a buscar: ", 1, 999999);
    NoFila *atual;

    /* Busca na fila prioritaria */
    int pos = 1;
    atual = prioridade->frente;
    while (atual != NULL) {
        if (atual->id == id) {
            printf("\n  Encontrado na fila PRIORITARIA (posicao %d):\n\n", pos);
            exibir_trabalho(atual);
            pausar();
            return;
        }
        pos++;
        atual = atual->proximo;
    }

    /* Busca na fila normal */
    pos = 1;
    atual = normal->frente;
    while (atual != NULL) {
        if (atual->id == id) {
            printf("\n  Encontrado na fila NORMAL (posicao %d):\n\n", pos);
            exibir_trabalho(atual);
            pausar();
            return;
        }
        pos++;
        atual = atual->proximo;
    }

    printf("\n  [!] Trabalho com ID %d nao encontrado.\n", id);
    pausar();
}

/* ============================================================
 *  4. CANCELAR TRABALHO (com confirmacao e free)
 *     Corrigido: agora realmente remove da fila prioritaria
 * ============================================================ */
void cancelar_trabalho(Fila *normal, Fila *prioridade) {
    limpar_tela();
    printf("=== CANCELAR TRABALHO ===\n\n");

    if (fila_vazia(normal) && fila_vazia(prioridade)) {
        printf("  [!] Filas vazias.\n");
        pausar();
        return;
    }

    int id = ler_inteiro("  ID a cancelar: ", 1, 999999);

    /* Procura primeiro em qual fila esta */
    NoFila *atual;
    char em_qual = ' ';

    atual = prioridade->frente;
    while (atual != NULL) {
        if (atual->id == id) { em_qual = 'P'; break; }
        atual = atual->proximo;
    }

    if (em_qual == ' ') {
        atual = normal->frente;
        while (atual != NULL) {
            if (atual->id == id) { em_qual = 'N'; break; }
            atual = atual->proximo;
        }
    }

    if (em_qual == ' ') {
        printf("\n  [!] Trabalho com ID %d nao encontrado.\n", id);
        pausar();
        return;
    }

    /* Mostra o trabalho e pede confirmacao */
    printf("\n  Trabalho encontrado na fila %s:\n\n",
           em_qual == 'P' ? "PRIORITARIA" : "NORMAL");
    exibir_trabalho(atual);

    int conf = ler_inteiro("\n  Confirmar cancelamento? [1=Sim / 0=Nao]: ", 0, 1);
    if (!conf) {
        printf("  Operacao cancelada.\n");
        pausar();
        return;
    }

    /* Remove da fila correta e libera memoria */
    NoFila *removido = NULL;
    if (em_qual == 'P') {
        removido = remover_por_id(prioridade, id);
    } else {
        removido = remover_por_id(normal, id);
    }

    if (removido != NULL) {
        free(removido);
        int restante = fila_tamanho(normal) + fila_tamanho(prioridade);
        printf("\n  [OK] Trabalho ID %d cancelado e removido.\n", id);
        printf("  Trabalhos restantes: %d\n", restante);
    }

    pausar();
}

/* ============================================================
 *  5. LISTAR FILAS
 * ============================================================ */
void listar_filas(const Fila *normal, const Fila *prioridade) {
    limpar_tela();
    int total = fila_tamanho(normal) + fila_tamanho(prioridade);
    printf("=== FILAS DE IMPRESSAO - %d trabalho(s) ===\n\n", total);

    /* Fila prioritaria */
    printf("  ========== FILA PRIORITARIA (%d) ==========\n\n",
           fila_tamanho(prioridade));
    if (fila_vazia(prioridade)) {
        printf("  (vazia)\n\n");
    } else {
        int pos = 1;
        NoFila *atual = prioridade->frente;
        while (atual != NULL) {
            printf("  [%d/%d]\n", pos, fila_tamanho(prioridade));
            exibir_trabalho(atual);
            printf("\n");
            pos++;
            atual = atual->proximo;
        }
    }

    /* Fila normal */
    printf("  ========== FILA NORMAL (%d) ==========\n\n",
           fila_tamanho(normal));
    if (fila_vazia(normal)) {
        printf("  (vazia)\n\n");
    } else {
        int pos = 1;
        NoFila *atual = normal->frente;
        while (atual != NULL) {
            printf("  [%d/%d]\n", pos, fila_tamanho(normal));
            exibir_trabalho(atual);
            printf("\n");
            pos++;
            atual = atual->proximo;
        }
    }

    pausar();
}

/* ============================================================
 *  EXTRAS
 * ============================================================ */

/* ============================================================
 *  8. ESTATISTICAS DAS FILAS
 * ============================================================ */
void estatisticas_filas(const Fila *normal, const Fila *prioridade) {
    limpar_tela();
    printf("=== ESTATISTICAS DAS FILAS ===\n\n");

    int total_trab = fila_tamanho(normal) + fila_tamanho(prioridade);

    if (total_trab == 0) {
        printf("  [!] Filas vazias.\n");
        pausar();
        return;
    }

    int pag_normal   = total_paginas_fila(normal);
    int pag_prior    = total_paginas_fila(prioridade);
    int pag_total    = pag_normal + pag_prior;

    int max_pag      = 0;
    int min_pag      = 0;
    NoFila *maior    = NULL;
    NoFila *menor    = NULL;

    /* Encontra maior e menor */
    NoFila *atual;
    int primeiro = 1;

    atual = prioridade->frente;
    while (atual != NULL) {
        if (primeiro || atual->paginas > max_pag) {
            max_pag = atual->paginas;
            maior   = atual;
        }
        if (primeiro || atual->paginas < min_pag) {
            min_pag = atual->paginas;
            menor   = atual;
        }
        primeiro = 0;
        atual = atual->proximo;
    }

    atual = normal->frente;
    while (atual != NULL) {
        if (primeiro || atual->paginas > max_pag) {
            max_pag = atual->paginas;
            maior   = atual;
        }
        if (primeiro || atual->paginas < min_pag) {
            min_pag = atual->paginas;
            menor   = atual;
        }
        primeiro = 0;
        atual = atual->proximo;
    }

    printf("  Total de trabalhos:     %d\n", total_trab);
    printf("    - Normais:            %d\n", fila_tamanho(normal));
    printf("    - Prioritarios:       %d\n", fila_tamanho(prioridade));
    printf("  Total de paginas:       %d\n", pag_total);
    printf("    - Normal:             %d\n", pag_normal);
    printf("    - Prioritario:        %d\n", pag_prior);
    printf("  Media de paginas:       %.1f\n", (double)pag_total / total_trab);

    if (maior != NULL) {
        printf("\n  Maior trabalho (%d paginas):\n", max_pag);
        exibir_trabalho(maior);
    }
    if (menor != NULL) {
        printf("\n  Menor trabalho (%d paginas):\n", min_pag);
        exibir_trabalho(menor);
    }

    pausar();
}

/* ============================================================
 *  9. BUSCAR POR NOME DO ARQUIVO (parcial, case-insensitive)
 * ============================================================ */
void buscar_por_nome(const Fila *normal, const Fila *prioridade) {
    limpar_tela();
    printf("=== BUSCAR POR NOME DO ARQUIVO ===\n\n");

    if (fila_vazia(normal) && fila_vazia(prioridade)) {
        printf("  [!] Filas vazias.\n");
        pausar();
        return;
    }

    char termo[TAM_ARQUIVO];
    ler_string("  Termo de busca: ", termo, sizeof(termo));

    char termoLower[TAM_ARQUIVO];
    to_lower_str(termoLower, termo);

    int encontrados = 0;
    NoFila *atual;

    printf("\n  Resultados:\n\n");

    /* Busca na fila prioritaria */
    atual = prioridade->frente;
    while (atual != NULL) {
        char campoLower[TAM_ARQUIVO];
        to_lower_str(campoLower, atual->arquivo);
        if (strstr(campoLower, termoLower) != NULL) {
            printf("  [PRIORITARIO]\n");
            exibir_trabalho(atual);
            printf("\n");
            encontrados++;
        }
        atual = atual->proximo;
    }

    /* Busca na fila normal */
    atual = normal->frente;
    while (atual != NULL) {
        char campoLower[TAM_ARQUIVO];
        to_lower_str(campoLower, atual->arquivo);
        if (strstr(campoLower, termoLower) != NULL) {
            printf("  [NORMAL]\n");
            exibir_trabalho(atual);
            printf("\n");
            encontrados++;
        }
        atual = atual->proximo;
    }

    if (encontrados == 0)
        printf("  Nenhum trabalho encontrado com \"%s\".\n", termo);
    else
        printf("  Total encontrado: %d trabalho(s).\n", encontrados);

    pausar();
}

/* ============================================================
 *  10. MOSTRAR PROXIMO TRABALHO (sem remover)
 * ============================================================ */
void mostrar_proximo(const Fila *normal, const Fila *prioridade) {
    limpar_tela();
    printf("=== PROXIMO TRABALHO A SER PROCESSADO ===\n\n");

    NoFila *proximo = NULL;

    /* Prioridade primeiro */
    if (!fila_vazia(prioridade)) {
        proximo = fila_frente(prioridade);
        printf("  O proximo trabalho e PRIORITARIO:\n\n");
    } else if (!fila_vazia(normal)) {
        proximo = fila_frente(normal);
        printf("  O proximo trabalho e NORMAL:\n\n");
    }

    if (proximo == NULL) {
        printf("  [!] Nenhum trabalho na fila.\n");
    } else {
        exibir_trabalho(proximo);
        int restante = fila_tamanho(normal) + fila_tamanho(prioridade);
        printf("\n  Trabalhos na fila: %d (apos este: %d)\n", restante, restante - 1);
    }

    pausar();
}

/* ============================================================
 *  11. LIMPAR TODAS AS FILAS (free em todos os nos)
 * ============================================================ */
void limpar_filas(Fila *normal, Fila *prioridade) {
    limpar_tela();
    printf("=== LIMPAR TODAS AS FILAS ===\n\n");

    int total = fila_tamanho(normal) + fila_tamanho(prioridade);
    if (total == 0) {
        printf("  [!] As filas ja estao vazias.\n");
        pausar();
        return;
    }

    printf("  ATENCAO: Isso ira remover TODOS os %d trabalho(s) das filas.\n", total);
    int conf = ler_inteiro("  Confirmar? [1=Sim / 0=Nao]: ", 0, 1);
    if (!conf) {
        printf("  Operacao cancelada.\n");
        pausar();
        return;
    }

    /* Libera todos os nos com free */
    fila_liberar(normal);
    fila_liberar(prioridade);

    printf("\n  [OK] %d trabalho(s) removido(s). Filas limpas!\n", total);
    pausar();
}

/* ============================================================
 *  SALVAR CSV
 * ============================================================ */
void salvar_csv(const Fila *normal, const Fila *prioridade) {
    FILE *fp = fopen(ARQUIVO_CSV, "w");
    if (!fp) {
        printf("\n  [ERRO] Nao foi possivel gravar '%s'.\n", ARQUIVO_CSV);
        return;
    }

    fprintf(fp, "ID,Arquivo,Paginas,Tipo\n");

    /* Salva fila normal (na ordem) */
    NoFila *atual = normal->frente;
    while (atual != NULL) {
        fprintf(fp, "%d,%s,%d,N\n", atual->id, atual->arquivo, atual->paginas);
        atual = atual->proximo;
    }

    /* Salva fila prioritaria (na ordem) */
    atual = prioridade->frente;
    while (atual != NULL) {
        fprintf(fp, "%d,%s,%d,P\n", atual->id, atual->arquivo, atual->paginas);
        atual = atual->proximo;
    }

    fclose(fp);
    int total = fila_tamanho(normal) + fila_tamanho(prioridade);
    printf("\n  [OK] %d registro(s) salvos em '%s'.\n", total, ARQUIVO_CSV);
}

/* ============================================================
 *  CARREGAR CSV — reconstroi as duas filas
 * ============================================================ */
void carregar_csv(Fila *normal, Fila *prioridade, int silencioso) {
    FILE *fp = fopen(ARQUIVO_CSV, "r");
    if (!fp) {
        if (!silencioso) {
            printf("  [Aviso] '%s' nao encontrado. Iniciando com filas vazias.\n",
                   ARQUIVO_CSV);
            pausar();
        }
        return;
    }

    /* Libera filas atuais */
    fila_liberar(normal);
    fila_liberar(prioridade);

    char linha[300];

    /* Pula cabecalho */
    if (!fgets(linha, sizeof(linha), fp)) { fclose(fp); return; }

    int carregados = 0;
    while (fgets(linha, sizeof(linha), fp) != NULL) {
        linha[strcspn(linha, "\n")] = '\0';
        if (strlen(linha) == 0) continue;

        int  id, paginas;
        char arquivo[TAM_ARQUIVO];
        char tipo;

        int lidos = sscanf(linha, "%d,%99[^,],%d,%c",
                           &id, arquivo, &paginas, &tipo);

        if (lidos != 4) continue;
        if (id <= 0 || paginas <= 0) continue;

        tipo = (char)toupper((unsigned char)tipo);
        if (tipo != 'N' && tipo != 'P') continue;

        /* Verifica duplicidade */
        if (id_existe(normal, prioridade, id)) continue;

        NoFila *novo = criar_no_fila();
        if (novo == NULL) continue;

        novo->id      = id;
        novo->paginas = paginas;
        novo->tipo    = tipo;
        strncpy(novo->arquivo, arquivo, TAM_ARQUIVO - 1);
        novo->arquivo[TAM_ARQUIVO - 1] = '\0';

        if (tipo == 'P')
            fila_enfileirar(prioridade, novo);
        else
            fila_enfileirar(normal, novo);

        carregados++;
    }

    fclose(fp);
    printf("  [OK] %d registro(s) carregados de '%s'.\n", carregados, ARQUIVO_CSV);
    if (!silencioso) pausar();
}

/* ============================================================
 *  MAIN
 * ============================================================ */
int main(void) {
    Fila normal, prioridade;
    fila_inicializar(&normal);
    fila_inicializar(&prioridade);

    printf("\n  === Fila de Impressao - Fila Dinamica ===\n");
    printf("  Carregando dados salvos...\n");
    carregar_csv(&normal, &prioridade, 1);
    printf("\n");

    int opcao;

    do {
        exibir_menu(&normal, &prioridade);
        opcao = ler_inteiro("  Opcao: ", 0, 11);

        switch (opcao) {
            case 1:  adicionar_trabalho(&normal, &prioridade);    break;
            case 2:  processar_trabalho(&normal, &prioridade);    break;
            case 3:  buscar_trabalho(&normal, &prioridade);       break;
            case 4:  cancelar_trabalho(&normal, &prioridade);     break;
            case 5:  listar_filas(&normal, &prioridade);          break;
            case 6:
                salvar_csv(&normal, &prioridade);
                pausar();
                break;
            case 7:
                carregar_csv(&normal, &prioridade, 0);
                break;
            case 8:  estatisticas_filas(&normal, &prioridade);    break;
            case 9:  buscar_por_nome(&normal, &prioridade);       break;
            case 10: mostrar_proximo(&normal, &prioridade);       break;
            case 11: limpar_filas(&normal, &prioridade);          break;
            case 0:
                printf("\n  Salvando antes de sair...\n");
                salvar_csv(&normal, &prioridade);
                /* Libera toda a memoria das filas */
                fila_liberar(&normal);
                fila_liberar(&prioridade);
                printf("  Memoria liberada. Ate logo!\n\n");
                break;
        }

    } while (opcao != 0);

    return 0;
}
