#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================
 *  TRABALHO PRATICO - ESTRUTURA DE DADOS
 *  Tema: Historico de Navegacao Web (Pilha Dinamica)
 *
 *  Funcionalidades Principais:
 *    1. Visitar nova pagina   (PUSH com malloc)
 *    2. Voltar                (POP back -> PUSH forward)
 *    3. Avancar               (POP forward -> PUSH back)
 *    4. Ver pagina atual      (PEEK)
 *    5. Buscar por ID
 *    6. Editar por ID
 *    7. Excluir por ID        (free, com confirmacao)
 *    8. Listar historico completo
 *    9. Salvar CSV
 *   10. Carregar CSV
 *
 *  Funcionalidades Extras:
 *   11. Buscar por titulo/URL/categoria
 *   12. Estatisticas do historico
 *   13. Listar favoritos
 *   14. Limpar historico inteiro (free em todos os nos)
 *
 *    0. Sair (libera pilhas)
 * ============================================================ */

#define TAM_URL        256
#define TAM_TITULO     128
#define TAM_DATA       20
#define TAM_CATEGORIA  64
#define ARQUIVO_CSV    "historico_pilha.csv"

/* ============================================================
 *  Struct do No da Pilha Dinamica (lista ligada)
 * ============================================================ */
typedef struct NoPilha {
    int  id;
    char url[TAM_URL];
    char titulo[TAM_TITULO];
    char data_hora[TAM_DATA];     /* DD/MM/AAAA HH:MM */
    char categoria[TAM_CATEGORIA];
    int  visitas;
    int  favorito;                /* 0=nao | 1=sim */
    struct NoPilha *abaixo;       /* ponteiro para o elemento abaixo na pilha */
} NoPilha;

/* ============================================================
 *  Struct da Pilha Dinamica: ponteiro para topo + contador
 * ============================================================ */
typedef struct {
    NoPilha *topo;
    int      tamanho;
} Pilha;

/* ============================================================
 *  Prototipos
 * ============================================================ */
/* I/O */
void     limpar_buffer(void);
void     limpar_tela(void);
void     pausar(void);
int      ler_inteiro(const char *msg, int min, int max);
void     ler_string(const char *msg, char *dest, int tamanho);

/* Operacoes primitivas da pilha dinamica */
void     pilha_inicializar(Pilha *p);
int      pilha_vazia(const Pilha *p);
int      pilha_tamanho(const Pilha *p);
int      pilha_push(Pilha *p, NoPilha *novo);
NoPilha *pilha_pop(Pilha *p);
NoPilha *pilha_peek(const Pilha *p);
void     pilha_liberar(Pilha *p);

/* Helpers */
NoPilha *criar_no(void);
int      proximo_id(const Pilha *back, const Pilha *fwd);
void     exibir_pagina(const NoPilha *no, const char *rotulo);
void     exibir_status(const Pilha *back, const Pilha *fwd);
int      validar_url(const char *url);
void     to_lower_str(char *dest, const char *src);

/* Funcoes do navegador */
void     visitar(Pilha *back, Pilha *fwd);
void     voltar(Pilha *back, Pilha *fwd);
void     avancar_pagina(Pilha *back, Pilha *fwd);
void     pagina_atual(const Pilha *back);
void     buscar_historico(const Pilha *back, const Pilha *fwd);
void     editar_por_id(Pilha *back, Pilha *fwd);
void     excluir_por_id(Pilha *back, Pilha *fwd);
void     listar_historico(const Pilha *back, const Pilha *fwd);

/* Extras */
void     buscar_por_campo(const Pilha *back, const Pilha *fwd);
void     estatisticas_historico(const Pilha *back, const Pilha *fwd);
void     listar_favoritos(const Pilha *back, const Pilha *fwd);
void     limpar_historico(Pilha *back, Pilha *fwd);

/* Persistencia */
void     salvar_csv(const Pilha *back, const Pilha *fwd);
void     carregar_csv(Pilha *back, Pilha *fwd, int silencioso);

/* Menu */
void     menu_principal(Pilha *back, Pilha *fwd);

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
 *  Operacoes primitivas da PILHA DINAMICA
 * ============================================================ */

/* Inicializa a pilha (vazia) */
void pilha_inicializar(Pilha *p) {
    p->topo    = NULL;
    p->tamanho = 0;
}

/* Verifica se a pilha esta vazia */
int pilha_vazia(const Pilha *p) {
    return p->topo == NULL;
}

/* Retorna o numero de elementos na pilha */
int pilha_tamanho(const Pilha *p) {
    return p->tamanho;
}

/* PUSH: insere no no topo da pilha. Retorna 1 se ok. */
int pilha_push(Pilha *p, NoPilha *novo) {
    if (novo == NULL) return 0;
    novo->abaixo = p->topo;
    p->topo      = novo;
    p->tamanho++;
    return 1;
}

/* POP: remove do topo e retorna o ponteiro (caller deve dar free).
 * Retorna NULL se a pilha estiver vazia. */
NoPilha *pilha_pop(Pilha *p) {
    if (pilha_vazia(p)) return NULL;
    NoPilha *removido = p->topo;
    p->topo = removido->abaixo;
    removido->abaixo = NULL;
    p->tamanho--;
    return removido;
}

/* PEEK: retorna ponteiro para o topo sem remover. NULL se vazia. */
NoPilha *pilha_peek(const Pilha *p) {
    return p->topo;
}

/* Libera todos os nos da pilha com free */
void pilha_liberar(Pilha *p) {
    NoPilha *atual = p->topo;
    while (atual != NULL) {
        NoPilha *temp = atual;
        atual = atual->abaixo;
        free(temp);
    }
    p->topo    = NULL;
    p->tamanho = 0;
}

/* ============================================================
 *  Helpers
 * ============================================================ */

/* Aloca um novo no com malloc e inicializa campos */
NoPilha *criar_no(void) {
    NoPilha *novo = (NoPilha *)malloc(sizeof(NoPilha));
    if (novo == NULL) {
        printf("  [ERRO] Falha ao alocar memoria.\n");
        return NULL;
    }
    memset(novo, 0, sizeof(NoPilha));
    return novo;
}

/* Converte string para minusculas (para busca case-insensitive) */
void to_lower_str(char *dest, const char *src) {
    while (*src) {
        *dest = (char)tolower((unsigned char)*src);
        dest++;
        src++;
    }
    *dest = '\0';
}

/* Valida se a URL comeca com http:// ou https:// */
int validar_url(const char *url) {
    if (strncmp(url, "http://", 7) == 0)  return 1;
    if (strncmp(url, "https://", 8) == 0) return 1;
    return 0;
}

/* ID unico: percorre as duas pilhas e retorna max+1 */
int proximo_id(const Pilha *back, const Pilha *fwd) {
    int max = 0;
    NoPilha *atual;

    atual = back->topo;
    while (atual != NULL) {
        if (atual->id > max) max = atual->id;
        atual = atual->abaixo;
    }

    atual = fwd->topo;
    while (atual != NULL) {
        if (atual->id > max) max = atual->id;
        atual = atual->abaixo;
    }

    return max + 1;
}

/* Exibe os dados de uma pagina formatados */
void exibir_pagina(const NoPilha *no, const char *rotulo) {
    if (rotulo && strlen(rotulo) > 0)
        printf("  [ %s ]\n", rotulo);
    printf("  +--------------------------------------------------+\n");
    printf("  | ID       : %-38d|\n", no->id);
    printf("  | Titulo   : %-38.38s|\n", no->titulo);
    printf("  | URL      : %-38.38s|\n", no->url);
    if (strlen(no->url) > 38)
        printf("  |            %-38.38s|\n", no->url + 38);
    printf("  | Data/Hora: %-38s|\n", no->data_hora);
    printf("  | Categoria: %-38s|\n", no->categoria);
    printf("  | Visitas  : %-38d|\n", no->visitas);
    if (no->favorito)
        printf("  | Favorito : %-38s|\n", "*** Sim *** (*)");
    else
        printf("  | Favorito : %-38s|\n", "Nao");
    printf("  +--------------------------------------------------+\n");
}

/* Mostra barra de status: paginas atras | atual | paginas a frente */
void exibir_status(const Pilha *back, const Pilha *fwd) {
    NoPilha *atual = pilha_peek(back);
    printf("\n  Estado do navegador:\n");
    printf("  [<< Voltar: %d]  [Atual: %s]  [Avancar: %d >>]\n",
           pilha_tamanho(back) > 1 ? pilha_tamanho(back) - 1 : 0,
           atual ? atual->titulo : "nenhuma",
           pilha_tamanho(fwd));
}

/* ============================================================
 *  Helper interno: busca um no pelo ID em uma pilha.
 *  Retorna o ponteiro para o no e, opcionalmente, o anterior.
 * ============================================================ */
static NoPilha *buscar_na_pilha(Pilha *p, int id, NoPilha **prev_out) {
    NoPilha *prev = NULL;
    NoPilha *atual = p->topo;
    while (atual != NULL) {
        if (atual->id == id) {
            if (prev_out) *prev_out = prev;
            return atual;
        }
        prev  = atual;
        atual = atual->abaixo;
    }
    return NULL;
}

/* Remove um no especifico da pilha (nao necessariamente o topo) e retorna-o.
 * O caller e responsavel por dar free. */
static NoPilha *remover_no_da_pilha(Pilha *p, NoPilha *no, NoPilha *prev) {
    if (no == NULL) return NULL;

    if (prev == NULL) {
        /* E o topo */
        p->topo = no->abaixo;
    } else {
        prev->abaixo = no->abaixo;
    }
    no->abaixo = NULL;
    p->tamanho--;
    return no;
}

/* ============================================================
 *  Helper interno: edita os campos de um no.
 * ============================================================ */
static void editar_campos(NoPilha *no) {
    char buf[TAM_URL];

    printf("  [Deixe em branco para manter o valor atual]\n\n");

    printf("  Novo titulo   [%s]: ", no->titulo);
    fflush(stdout);
    if (fgets(buf, TAM_TITULO, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) > 0) {
            strncpy(no->titulo, buf, TAM_TITULO - 1);
            no->titulo[TAM_TITULO - 1] = '\0';
        }
    }

    printf("  Nova URL      [%s]: ", no->url);
    fflush(stdout);
    if (fgets(buf, TAM_URL, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) > 0) {
            if (!validar_url(buf)) {
                printf("  [!] URL invalida (deve iniciar com http:// ou https://). Mantida anterior.\n");
            } else {
                strncpy(no->url, buf, TAM_URL - 1);
                no->url[TAM_URL - 1] = '\0';
            }
        }
    }

    printf("  Nova data/hora[%s]: ", no->data_hora);
    fflush(stdout);
    if (fgets(buf, TAM_DATA, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) > 0) {
            strncpy(no->data_hora, buf, TAM_DATA - 1);
            no->data_hora[TAM_DATA - 1] = '\0';
        }
    }

    printf("  Nova categoria[%s]: ", no->categoria);
    fflush(stdout);
    if (fgets(buf, TAM_CATEGORIA, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) > 0) {
            strncpy(no->categoria, buf, TAM_CATEGORIA - 1);
            no->categoria[TAM_CATEGORIA - 1] = '\0';
        }
    }

    {
        char tmp[32];
        printf("  Novas visitas [%d] (ENTER=manter): ", no->visitas);
        fflush(stdout);
        if (fgets(tmp, sizeof(tmp), stdin)) {
            tmp[strcspn(tmp, "\n")] = '\0';
            if (strlen(tmp) > 0) {
                int v = atoi(tmp);
                if (v >= 1) no->visitas = v;
                else printf("  [!] Ignorado (deve ser >= 1).\n");
            }
        }
    }

    {
        char tmp[32];
        printf("  Favorito 0/1  [%d] (ENTER=manter): ", no->favorito);
        fflush(stdout);
        if (fgets(tmp, sizeof(tmp), stdin)) {
            tmp[strcspn(tmp, "\n")] = '\0';
            if (strlen(tmp) > 0) {
                int f = atoi(tmp);
                if (f == 0 || f == 1) no->favorito = f;
                else printf("  [!] Ignorado (use 0 ou 1).\n");
            }
        }
    }
}

/* ============================================================
 *  1. VISITAR nova pagina (PUSH na pilha BACK, limpa FORWARD)
 * ============================================================ */
void visitar(Pilha *back, Pilha *fwd) {
    limpar_tela();
    printf("=== VISITAR NOVA PAGINA ===\n");
    exibir_status(back, fwd);
    printf("\n");

    NoPilha *novo = criar_no();
    if (novo == NULL) { pausar(); return; }

    novo->id = proximo_id(back, fwd);
    printf("  ID gerado automaticamente: %d\n\n", novo->id);

    ler_string("  Titulo   : ", novo->titulo, TAM_TITULO);

    /* Leitura de URL com validacao */
    while (1) {
        ler_string("  URL      : ", novo->url, TAM_URL);
        if (validar_url(novo->url)) break;
        printf("  [!] URL invalida. Deve iniciar com http:// ou https://\n");
    }

    ler_string("  Data/Hora (DD/MM/AAAA HH:MM): ", novo->data_hora, TAM_DATA);
    ler_string("  Categoria: ", novo->categoria, TAM_CATEGORIA);
    novo->visitas  = ler_inteiro("  Visitas [1-99999]: ", 1, 99999);
    novo->favorito = ler_inteiro("  Favorito [0=Nao / 1=Sim]: ", 0, 1);

    /* Ao visitar nova pagina, o historico de "avancar" e descartado */
    if (!pilha_vazia(fwd)) {
        printf("\n  [Info] Historico de 'Avancar' descartado (%d pagina(s)).\n",
               pilha_tamanho(fwd));
        pilha_liberar(fwd);  /* free em todos os nos da pilha forward */
    }

    pilha_push(back, novo);
    printf("\n  [OK] Pagina '%s' adicionada ao historico!\n", novo->titulo);
    pausar();
}

/* ============================================================
 *  2. VOLTAR (POP de BACK -> PUSH em FORWARD)
 * ============================================================ */
void voltar(Pilha *back, Pilha *fwd) {
    limpar_tela();
    printf("=== BOTAO VOLTAR ===\n");
    exibir_status(back, fwd);
    printf("\n");

    /* Precisa de pelo menos 2 paginas em BACK (a atual + a anterior) */
    if (pilha_tamanho(back) < 2) {
        printf("  [!] Nao ha pagina anterior para voltar.\n");
        pausar();
        return;
    }

    NoPilha *atual = pilha_pop(back);  /* remove pagina atual de BACK */
    pilha_push(fwd, atual);            /* coloca em FORWARD (sem copia, move o no) */

    printf("  Saindo de : '%s'\n", atual->titulo);
    printf("  Voltando para:\n\n");
    exibir_pagina(pilha_peek(back), "PAGINA ATUAL");
    pausar();
}

/* ============================================================
 *  3. AVANCAR (POP de FORWARD -> PUSH em BACK)
 * ============================================================ */
void avancar_pagina(Pilha *back, Pilha *fwd) {
    limpar_tela();
    printf("=== BOTAO AVANCAR ===\n");
    exibir_status(back, fwd);
    printf("\n");

    if (pilha_vazia(fwd)) {
        printf("  [!] Nao ha pagina a frente para avancar.\n");
        pausar();
        return;
    }

    NoPilha *proxima = pilha_pop(fwd);   /* remove do topo de FORWARD */
    pilha_push(back, proxima);           /* coloca no topo de BACK */

    printf("  Avancando para:\n\n");
    exibir_pagina(proxima, "PAGINA ATUAL");
    pausar();
}

/* ============================================================
 *  4. PAGINA ATUAL (PEEK no topo de BACK)
 * ============================================================ */
void pagina_atual(const Pilha *back) {
    limpar_tela();
    printf("=== PAGINA ATUAL ===\n\n");

    NoPilha *p = pilha_peek(back);
    if (!p) {
        printf("  [!] Nenhuma pagina visitada ainda.\n");
    } else {
        exibir_pagina(p, "PAGINA ATUAL (topo da pilha)");
    }
    pausar();
}

/* ============================================================
 *  5. BUSCAR POR ID (percorre as duas pilhas)
 * ============================================================ */
void buscar_historico(const Pilha *back, const Pilha *fwd) {
    limpar_tela();
    printf("=== BUSCAR NO HISTORICO POR ID ===\n\n");

    if (pilha_vazia(back) && pilha_vazia(fwd)) {
        printf("  [!] Historico vazio.\n");
        pausar();
        return;
    }

    int id = ler_inteiro("  ID a buscar: ", 1, 999999);
    int pos;

    /* Busca na pilha BACK (do topo para a base) */
    pos = 0;
    NoPilha *atual = back->topo;
    while (atual != NULL) {
        if (atual->id == id) {
            printf("\n  Encontrado na pilha HISTORICO (posicao %d do topo):\n\n", pos);
            exibir_pagina(atual, pos == 0 ? "PAGINA ATUAL" : "");
            pausar();
            return;
        }
        pos++;
        atual = atual->abaixo;
    }

    /* Busca na pilha FORWARD (do topo para a base) */
    pos = 0;
    atual = fwd->topo;
    while (atual != NULL) {
        if (atual->id == id) {
            printf("\n  Encontrado na pilha AVANCAR (posicao %d do topo):\n\n", pos);
            exibir_pagina(atual, "");
            pausar();
            return;
        }
        pos++;
        atual = atual->abaixo;
    }

    printf("\n  [!] ID %d nao encontrado em nenhuma pilha.\n", id);
    pausar();
}

/* ============================================================
 *  6. EDITAR por ID (busca nas duas pilhas, edita campos)
 * ============================================================ */
void editar_por_id(Pilha *back, Pilha *fwd) {
    limpar_tela();
    printf("=== EDITAR PAGINA POR ID ===\n\n");

    if (pilha_vazia(back) && pilha_vazia(fwd)) {
        printf("  [!] Historico vazio.\n");
        pausar();
        return;
    }

    int id = ler_inteiro("  ID a editar: ", 1, 999999);

    /* Busca na pilha BACK */
    NoPilha *no = back->topo;
    int e_topo = 1;
    while (no != NULL) {
        if (no->id == id) {
            printf("\n  Registro encontrado na pilha HISTORICO");
            if (e_topo) printf(" [PAGINA ATUAL]");
            printf(":\n\n");
            exibir_pagina(no, "");
            printf("\n");
            editar_campos(no);
            printf("\n  [OK] Registro ID %d atualizado!\n", id);
            pausar();
            return;
        }
        e_topo = 0;
        no = no->abaixo;
    }

    /* Busca na pilha FORWARD */
    no = fwd->topo;
    while (no != NULL) {
        if (no->id == id) {
            printf("\n  Registro encontrado na pilha AVANCAR:\n\n");
            exibir_pagina(no, "");
            printf("\n");
            editar_campos(no);
            printf("\n  [OK] Registro ID %d atualizado!\n", id);
            pausar();
            return;
        }
        no = no->abaixo;
    }

    printf("\n  [!] ID %d nao encontrado em nenhuma pilha.\n", id);
    pausar();
}

/* ============================================================
 *  7. EXCLUIR por ID (com confirmacao e free)
 * ============================================================ */
void excluir_por_id(Pilha *back, Pilha *fwd) {
    limpar_tela();
    printf("=== EXCLUIR PAGINA POR ID ===\n\n");

    if (pilha_vazia(back) && pilha_vazia(fwd)) {
        printf("  [!] Historico vazio.\n");
        pausar();
        return;
    }

    int id = ler_inteiro("  ID a excluir: ", 1, 999999);

    /* --- Busca na pilha BACK --- */
    NoPilha *prev = NULL;
    NoPilha *encontrado = buscar_na_pilha(back, id, &prev);
    if (encontrado != NULL) {
        printf("\n  Registro encontrado na pilha HISTORICO");
        if (encontrado == back->topo) printf(" [PAGINA ATUAL]");
        printf(":\n\n");
        exibir_pagina(encontrado, "");

        int conf = ler_inteiro("\n  Confirmar exclusao? [1=Sim / 0=Nao]: ", 0, 1);
        if (!conf) {
            printf("  Operacao cancelada.\n");
            pausar();
            return;
        }

        remover_no_da_pilha(back, encontrado, prev);
        free(encontrado);

        printf("\n  [OK] ID %d removido da pilha HISTORICO. "
               "Restam %d pagina(s).\n", id, pilha_tamanho(back));
        pausar();
        return;
    }

    /* --- Busca na pilha FORWARD --- */
    prev = NULL;
    encontrado = buscar_na_pilha(fwd, id, &prev);
    if (encontrado != NULL) {
        printf("\n  Registro encontrado na pilha AVANCAR:\n\n");
        exibir_pagina(encontrado, "");

        int conf = ler_inteiro("\n  Confirmar exclusao? [1=Sim / 0=Nao]: ", 0, 1);
        if (!conf) {
            printf("  Operacao cancelada.\n");
            pausar();
            return;
        }

        remover_no_da_pilha(fwd, encontrado, prev);
        free(encontrado);

        printf("\n  [OK] ID %d removido da pilha AVANCAR. "
               "Restam %d pagina(s).\n", id, pilha_tamanho(fwd));
        pausar();
        return;
    }

    printf("\n  [!] ID %d nao encontrado em nenhuma pilha.\n", id);
    pausar();
}

/* ============================================================
 *  8. LISTAR HISTORICO COMPLETO
 * ============================================================ */
void listar_historico(const Pilha *back, const Pilha *fwd) {
    limpar_tela();
    int total = pilha_tamanho(back) + pilha_tamanho(fwd);
    printf("=== HISTORICO COMPLETO - %d pagina(s) ===\n\n", total);

    if (pilha_vazia(back) && pilha_vazia(fwd)) {
        printf("  [!] Historico vazio.\n");
        pausar();
        return;
    }

    /* --- Pilha BACK: do topo (atual) para a base --- */
    printf("  ========== HISTORICO (pilha BACK) ==========\n");
    printf("  Ordem: atual -> mais antiga\n\n");
    if (pilha_vazia(back)) {
        printf("  (vazia)\n\n");
    } else {
        int pos = 1;
        NoPilha *atual = back->topo;
        while (atual != NULL) {
            if (pos == 1)
                printf("  [%d/%d] <<< PAGINA ATUAL\n", pos, pilha_tamanho(back));
            else
                printf("  [%d/%d]\n", pos, pilha_tamanho(back));
            exibir_pagina(atual, "");
            printf("\n");
            atual = atual->abaixo;
            pos++;
        }
    }

    /* --- Pilha FORWARD: do topo para a base --- */
    printf("  ========== AVANCAR (pilha FORWARD) ==========\n");
    printf("  Ordem: proxima -> mais distante\n\n");
    if (pilha_vazia(fwd)) {
        printf("  (vazia - nenhuma pagina a frente)\n\n");
    } else {
        int pos = 1;
        NoPilha *atual = fwd->topo;
        while (atual != NULL) {
            printf("  [%d/%d]\n", pos, pilha_tamanho(fwd));
            exibir_pagina(atual, "");
            printf("\n");
            atual = atual->abaixo;
            pos++;
        }
    }
    pausar();
}

/* ============================================================
 *  EXTRAS
 * ============================================================ */

/* ============================================================
 *  11. BUSCAR POR TITULO, URL OU CATEGORIA (parcial, case-insensitive)
 * ============================================================ */
void buscar_por_campo(const Pilha *back, const Pilha *fwd) {
    limpar_tela();
    printf("=== BUSCAR POR TITULO / URL / CATEGORIA ===\n\n");

    if (pilha_vazia(back) && pilha_vazia(fwd)) {
        printf("  [!] Historico vazio.\n");
        pausar();
        return;
    }

    printf("  Buscar por:\n");
    printf("    1. Titulo\n");
    printf("    2. URL\n");
    printf("    3. Categoria\n");
    int opcao = ler_inteiro("  Opcao: ", 1, 3);

    char termo[TAM_URL];
    ler_string("  Termo de busca: ", termo, sizeof(termo));

    char termoLower[TAM_URL];
    to_lower_str(termoLower, termo);

    int encontradas = 0;

    printf("\n  Resultados da busca:\n\n");

    /* Percorre BACK */
    NoPilha *atual = back->topo;
    while (atual != NULL) {
        char campoLower[TAM_URL];
        switch (opcao) {
            case 1: to_lower_str(campoLower, atual->titulo);    break;
            case 2: to_lower_str(campoLower, atual->url);       break;
            case 3: to_lower_str(campoLower, atual->categoria); break;
        }
        if (strstr(campoLower, termoLower) != NULL) {
            printf("  [HISTORICO]");
            if (atual == back->topo) printf(" <<< PAGINA ATUAL");
            printf("\n");
            exibir_pagina(atual, "");
            printf("\n");
            encontradas++;
        }
        atual = atual->abaixo;
    }

    /* Percorre FORWARD */
    atual = fwd->topo;
    while (atual != NULL) {
        char campoLower[TAM_URL];
        switch (opcao) {
            case 1: to_lower_str(campoLower, atual->titulo);    break;
            case 2: to_lower_str(campoLower, atual->url);       break;
            case 3: to_lower_str(campoLower, atual->categoria); break;
        }
        if (strstr(campoLower, termoLower) != NULL) {
            printf("  [AVANCAR]\n");
            exibir_pagina(atual, "");
            printf("\n");
            encontradas++;
        }
        atual = atual->abaixo;
    }

    if (encontradas == 0)
        printf("  Nenhuma pagina encontrada com o termo \"%s\".\n", termo);
    else
        printf("  Total encontrado: %d pagina(s).\n", encontradas);

    pausar();
}

/* ============================================================
 *  12. ESTATISTICAS DO HISTORICO
 * ============================================================ */
void estatisticas_historico(const Pilha *back, const Pilha *fwd) {
    limpar_tela();
    printf("=== ESTATISTICAS DO HISTORICO ===\n\n");

    int total = pilha_tamanho(back) + pilha_tamanho(fwd);
    if (total == 0) {
        printf("  [!] Historico vazio.\n");
        pausar();
        return;
    }

    int total_visitas    = 0;
    int total_favoritos  = 0;
    int max_visitas      = 0;
    NoPilha *mais_visitada = NULL;

    /* Contadores por pilha */
    NoPilha *atual;

    /* Percorre BACK */
    atual = back->topo;
    while (atual != NULL) {
        total_visitas += atual->visitas;
        if (atual->favorito) total_favoritos++;
        if (atual->visitas > max_visitas) {
            max_visitas    = atual->visitas;
            mais_visitada  = atual;
        }
        atual = atual->abaixo;
    }

    /* Percorre FORWARD */
    atual = fwd->topo;
    while (atual != NULL) {
        total_visitas += atual->visitas;
        if (atual->favorito) total_favoritos++;
        if (atual->visitas > max_visitas) {
            max_visitas    = atual->visitas;
            mais_visitada  = atual;
        }
        atual = atual->abaixo;
    }

    printf("  Total de paginas:       %d\n", total);
    printf("    - Pilha Historico:    %d\n", pilha_tamanho(back));
    printf("    - Pilha Avancar:      %d\n", pilha_tamanho(fwd));
    printf("  Total de visitas:       %d\n", total_visitas);
    printf("  Media de visitas:       %.1f\n", (double)total_visitas / total);
    printf("  Total de favoritos:     %d\n", total_favoritos);

    if (mais_visitada != NULL) {
        printf("\n  Pagina mais visitada (%d visitas):\n", max_visitas);
        exibir_pagina(mais_visitada, "");
    }

    pausar();
}

/* ============================================================
 *  13. LISTAR FAVORITOS (com destaque)
 * ============================================================ */
void listar_favoritos(const Pilha *back, const Pilha *fwd) {
    limpar_tela();
    printf("=== PAGINAS FAVORITAS ===\n\n");

    if (pilha_vazia(back) && pilha_vazia(fwd)) {
        printf("  [!] Historico vazio.\n");
        pausar();
        return;
    }

    int encontrados = 0;
    NoPilha *atual;

    /* Percorre BACK */
    atual = back->topo;
    while (atual != NULL) {
        if (atual->favorito) {
            encontrados++;
            printf("  *** FAVORITO #%d ***", encontrados);
            if (atual == back->topo) printf("  <<< PAGINA ATUAL");
            printf("\n");
            exibir_pagina(atual, "");
            printf("\n");
        }
        atual = atual->abaixo;
    }

    /* Percorre FORWARD */
    atual = fwd->topo;
    while (atual != NULL) {
        if (atual->favorito) {
            encontrados++;
            printf("  *** FAVORITO #%d ***  [Avancar]\n", encontrados);
            exibir_pagina(atual, "");
            printf("\n");
        }
        atual = atual->abaixo;
    }

    if (encontrados == 0)
        printf("  Nenhuma pagina marcada como favorita.\n");
    else
        printf("  Total de favoritos: %d\n", encontrados);

    pausar();
}

/* ============================================================
 *  14. LIMPAR HISTORICO INTEIRO (free em todos os nos)
 * ============================================================ */
void limpar_historico(Pilha *back, Pilha *fwd) {
    limpar_tela();
    printf("=== LIMPAR HISTORICO INTEIRO ===\n\n");

    int total = pilha_tamanho(back) + pilha_tamanho(fwd);
    if (total == 0) {
        printf("  [!] Historico ja esta vazio.\n");
        pausar();
        return;
    }

    printf("  ATENCAO: Isso ira remover TODAS as %d pagina(s) do historico.\n", total);
    int conf = ler_inteiro("  Confirmar? [1=Sim / 0=Nao]: ", 0, 1);
    if (!conf) {
        printf("  Operacao cancelada.\n");
        pausar();
        return;
    }

    /* Libera todos os nos com free */
    pilha_liberar(back);
    pilha_liberar(fwd);

    printf("\n  [OK] %d pagina(s) removida(s). Historico limpo!\n", total);
    pausar();
}

/* ============================================================
 *  SALVAR CSV
 *  Para salvar a pilha dinamica, precisamos inverter a ordem
 *  (base -> topo) para manter compatibilidade ao recarregar.
 *  Usamos uma pilha auxiliar temporaria.
 * ============================================================ */
static void salvar_pilha_csv(FILE *fp, const Pilha *p, char qual) {
    /* Conta a posicao da base ao topo.
     * Como a lista vai do topo para a base, precisamos numerar inversamente. */
    int total = pilha_tamanho(p);
    int pos;

    /* Primeiro, coleta todos os nos em um array temporario para salvar
     * da base (posicao 0) ao topo (posicao total-1) */
    NoPilha **arr = NULL;
    if (total > 0) {
        arr = (NoPilha **)malloc(sizeof(NoPilha *) * total);
        if (arr == NULL) return;

        NoPilha *atual = p->topo;
        for (int i = total - 1; i >= 0; i--) {
            arr[i] = atual;
            atual = atual->abaixo;
        }

        for (pos = 0; pos < total; pos++) {
            const NoPilha *no = arr[pos];
            fprintf(fp, "%c,%d,%d,\"%s\",\"%s\",\"%s\",\"%s\",%d,%d\n",
                    qual, pos, no->id, no->titulo, no->url,
                    no->data_hora, no->categoria,
                    no->visitas, no->favorito);
        }

        free(arr);
    }
}

void salvar_csv(const Pilha *back, const Pilha *fwd) {
    FILE *fp = fopen(ARQUIVO_CSV, "w");
    if (!fp) {
        printf("\n  [ERRO] Nao foi possivel gravar '%s'.\n", ARQUIVO_CSV);
        return;
    }

    fprintf(fp, "pilha,posicao,id,titulo,url,data_hora,categoria,visitas,favorito\n");

    salvar_pilha_csv(fp, back, 'B');
    salvar_pilha_csv(fp, fwd,  'F');

    fclose(fp);
    int total = pilha_tamanho(back) + pilha_tamanho(fwd);
    printf("\n  [OK] %d registro(s) salvos em '%s'.\n", total, ARQUIVO_CSV);
}

/* ============================================================
 *  CARREGAR CSV — reconstroi as duas pilhas
 * ============================================================ */
void carregar_csv(Pilha *back, Pilha *fwd, int silencioso) {
    FILE *fp = fopen(ARQUIVO_CSV, "r");
    if (!fp) {
        printf("  [Aviso] '%s' nao encontrado. Iniciando com pilhas vazias.\n",
               ARQUIVO_CSV);
        if (!silencioso) pausar();
        return;
    }

    /* Libera pilhas atuais antes de recarregar */
    pilha_liberar(back);
    pilha_liberar(fwd);

    char linha[TAM_URL + TAM_TITULO + TAM_DATA + TAM_CATEGORIA + 64];

    /* Pula cabecalho */
    if (!fgets(linha, sizeof(linha), fp)) { fclose(fp); return; }

    /* Armazena temporariamente para inserir na ordem correta (base -> topo).
     * Primeiro lemos tudo e depois empilhamos na ordem. */
    NoPilha *back_arr[10000];
    NoPilha *fwd_arr[10000];
    int back_count = 0;
    int fwd_count  = 0;

    int carregados = 0;
    while (fgets(linha, sizeof(linha), fp) != NULL) {
        linha[strcspn(linha, "\n")] = '\0';
        if (strlen(linha) == 0) continue;

        char qual;
        int  posicao;

        NoPilha *novo = criar_no();
        if (novo == NULL) continue;

        int lidos = sscanf(linha,
            "%c,%d,%d,\"%127[^\"]\",\"%255[^\"]\",\"%19[^\"]\",\"%63[^\"]\",%d,%d",
            &qual, &posicao, &novo->id,
            novo->titulo, novo->url,
            novo->data_hora, novo->categoria,
            &novo->visitas, &novo->favorito);

        if (lidos == 9) {
            if (qual == 'B' && back_count < 10000) {
                back_arr[back_count++] = novo;
                carregados++;
            } else if (qual == 'F' && fwd_count < 10000) {
                fwd_arr[fwd_count++] = novo;
                carregados++;
            } else {
                free(novo);
            }
        } else {
            free(novo);
        }
    }

    fclose(fp);

    /* Empilha na ordem correta: do indice 0 (base) ao ultimo (topo) */
    for (int i = 0; i < back_count; i++)
        pilha_push(back, back_arr[i]);

    /* Para a pilha back, precisamos inverter pois push coloca no topo.
     * Solucao: empilhar em ordem inversa. Vamos reconstruir. */
    /* Na verdade, os registros ja estao na ordem base->topo no CSV,
     * e pilha_push coloca no topo. Entao o ultimo push sera o topo.
     * Isso esta correto: push(base), push(...), push(topo) */

    for (int i = 0; i < fwd_count; i++)
        pilha_push(fwd, fwd_arr[i]);

    printf("  [OK] %d registro(s) carregados de '%s'.\n", carregados, ARQUIVO_CSV);
    if (!silencioso) pausar();
}

/* ============================================================
 *  Menu principal com secao de Extras e status da pagina atual
 * ============================================================ */
void menu_principal(Pilha *back, Pilha *fwd) {
    int opcao;
    do {
        limpar_tela();
        NoPilha *atual = pilha_peek(back);
        int total = pilha_tamanho(back) + pilha_tamanho(fwd);

        printf("+====================================================+\n");
        printf("|     HISTORICO DE NAVEGACAO WEB - PILHA DINAMICA     |\n");
        printf("|  Total de paginas: %-5d                            |\n", total);
        printf("+====================================================+\n");
        printf("| Pagina atual : %-37.37s |\n",
               atual ? atual->titulo : "(nenhuma)");
        if (atual && atual->favorito)
            printf("| *** FAVORITA ***                                    |\n");
        printf("| << Voltar: %-3d          Avancar: %-3d >>              |\n",
               pilha_tamanho(back) > 1 ? pilha_tamanho(back)-1 : 0,
               pilha_tamanho(fwd));
        printf("+----------------------------------------------------+\n");
        printf("|  1. Visitar nova pagina  [PUSH]                     |\n");
        printf("|  2. Voltar               [POP back]                 |\n");
        printf("|  3. Avancar              [POP forward]              |\n");
        printf("|  4. Ver pagina atual     [PEEK]                     |\n");
        printf("|  5. Buscar por ID                                   |\n");
        printf("|  6. Editar por ID                                   |\n");
        printf("|  7. Excluir por ID                                  |\n");
        printf("|  8. Listar historico completo                       |\n");
        printf("|  9. Salvar CSV                                      |\n");
        printf("|  10. Carregar CSV                                   |\n");
        printf("+----------------------------------------------------+\n");
        printf("|  EXTRAS                                             |\n");
        printf("+----------------------------------------------------+\n");
        printf("|  11. Buscar por titulo/URL/categoria                |\n");
        printf("|  12. Estatisticas do historico                      |\n");
        printf("|  13. Listar favoritos                               |\n");
        printf("|  14. Limpar historico inteiro                       |\n");
        printf("+----------------------------------------------------+\n");
        printf("|  0. Sair                                            |\n");
        printf("+====================================================+\n\n");

        opcao = ler_inteiro("  Opcao: ", 0, 14);

        switch (opcao) {
            case 1:  visitar(back, fwd);                break;
            case 2:  voltar(back, fwd);                 break;
            case 3:  avancar_pagina(back, fwd);         break;
            case 4:  pagina_atual(back);                break;
            case 5:  buscar_historico(back, fwd);       break;
            case 6:  editar_por_id(back, fwd);          break;
            case 7:  excluir_por_id(back, fwd);         break;
            case 8:  listar_historico(back, fwd);       break;
            case 9:
                salvar_csv(back, fwd);
                pausar();
                break;
            case 10:
                carregar_csv(back, fwd, 0);
                break;
            case 11: buscar_por_campo(back, fwd);       break;
            case 12: estatisticas_historico(back, fwd); break;
            case 13: listar_favoritos(back, fwd);       break;
            case 14: limpar_historico(back, fwd);       break;
            case 0:
                printf("\n  Salvando antes de sair...\n");
                salvar_csv(back, fwd);
                /* Libera toda a memoria das pilhas */
                pilha_liberar(back);
                pilha_liberar(fwd);
                printf("  Memoria liberada. Ate logo!\n\n");
                break;
        }
    } while (opcao != 0);
}

/* ============================================================
 *  Main
 * ============================================================ */
int main(void) {
    Pilha back, fwd;
    pilha_inicializar(&back);
    pilha_inicializar(&fwd);

    printf("\n  === Historico de Navegacao Web - Pilha Dinamica ===\n");
    printf("  Carregando dados salvos...\n");
    carregar_csv(&back, &fwd, 1);
    printf("\n");

    menu_principal(&back, &fwd);

    return 0;
}
