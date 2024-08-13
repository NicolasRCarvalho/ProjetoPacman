/**************************************************************************************************
 *  PAC-MAN - IMPLEMENTAÇÃO DO JOGO USANDO RAYLIB
 *
 *  Descrição:
 *  Este é o clássico jogo do Pac-Man, desenvolvido em C utilizando a biblioteca
 *  Raylib. O jogo apresenta múltiplos níveis, a IA dos monstros foi feita usando o algoritmo A*, conta com
 *  sistema de vidas e pontuação do jogador, e um sistema de high score. Os jogadores navegam por um labirinto,
 *  coletando itens enquanto evitam os monstros. O jogo permite salvar o progresso, permitindo
 *  que os jogadores retomem de onde pararam.
 *
 *  Como Jogar:
 *  - Use as setas do teclado para mover o Pac-Man.
 *  - Colete todos os itens no labirinto para completar o nível.
 *  - Evite os monstros, ou perderá uma vida.
 *  - Pressione 'TAB' para pausar o jogo e acessar a função de salvar.
 *
 *  Notas:
 *  - O jogo fechará automaticamente após salvar.
 *  - O jogo avança por múltiplos níveis; complete todos os níveis para vencer.
 *
 *  Autores:
 *  Nicolas R. Carvalho, Lucas F. Canto.
 *
 *  Data:
 *  [13/08/2024]
 **************************************************************************************************/

#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LAR_TELA 800
#define ALT_TELA 640
#define COLUNAS_MAPA 40
#define LINHAS_MAPA 32
#define TAM_PIXEL 20
#define MAXIMO_MONSTROS 10 // Define numero maximo de monstros
#define TEMPO_DIFICULDADE 225 //configura quanto tempo leva para a dificuldade mudar
#define MAXSCORES 5 // Define o número máximo de scores que serão armazenados
#define NUM_MAPAS 3 // Define numero maximo de mapas



//Estrutura para scores
typedef struct tipo_score
{
    char nome[30];
    int score;
} TIPO_SCORE;

//Posicao/direcao do pacman
typedef struct pos_pacman
{
    int x, y;
    int dx, dy;
    int x_inicial, y_inicial;
} POS_PACMAN;

//Posicao/direcao dos monstros
typedef struct pos_monstro
{
    int x, y;
    int dx, dy;
    int x_inicial, y_inicial;
} POS_MONSTRO;

//Vida e pontuacao do jogador
typedef struct status_player
{
    int vida;
    int pontuacao;
    int fase;
    int pontuacao_alvo;
    int dificuldade;
} STATUS_PLAYER;

//Node para algoritmo A* (OBS: entenda Node como "Um ponto na matriz")
typedef struct node
{
    int x, y; //Cordenadas do nó no mapa
    int g, h, f; // F = soma de G e H (F = G + H),   G  = Custo acumulado desde o ponto inicial até este nó,    H  =  Heurística (estimativa do custo restante até o objetivo).
    struct node *parent; //Ponteiro que indica o nó anterior do caminho ("Node pai"), que permitira reconstruir o caminho após encontrar o objetivo.
} Node;

//DEFINDO VARIAVEIS GLOBAIS
POS_MONSTRO monstros[MAXIMO_MONSTROS]; //definindo vetor que guardara a posicao de cada monstro
int num_monstros = 0; //setando numero de monstros iniciais como 0
float VEL_PACMAN = 0.15; //VEL INVERSAMENTE PROPORCIONAL
float VEL_MONSTROS = 0.30; //VELOCIDADE INVERSAMENTE PROPORCIONAL
const char *mapas[NUM_MAPAS] = {"mapa1.txt","mapa2.txt","mapa3.txt"};

// Declaração das funções antes de serem usadas
void exibe_highscores(TIPO_SCORE* scores);
void insere_highscore_grafico(TIPO_SCORE* scores, int pontuacao);
void game_over(STATUS_PLAYER *jogador);
int le_arquivo(TIPO_SCORE* scores, char* nome_arq);
void escreve_arquivo(TIPO_SCORE* scores, char* nome_arq);
void atualiza_highscores(TIPO_SCORE scores[], int nelem, TIPO_SCORE novo_score);

//FUNCAO que ira exibir o menu principal do jogo
int chama_menu()
{
    // posicao do marcador
    int posMarcador = 0;

    //iniciando tela
    InitWindow(LAR_TELA, ALT_TELA, "MENU");
    SetTargetFPS(60);

    //looping da tela
    while (!WindowShouldClose())
    {
        //Interface grafica
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("PAC-MAN", 30, 20, 130, YELLOW);
        DrawText("NOVO-JOGO", 30, 160, 80, YELLOW);
        DrawText("CARREGAR-JOGO", 30, 260, 80, YELLOW);
        DrawText("HIGHSCORES", 30, 360, 80, YELLOW);
        DrawText("SAIR", 30, 460, 80, YELLOW);
        DrawText("|W| |S| |ENTER|", 580, 600, 30, GRAY);

        //Interacoes com o menu
        if (IsKeyPressed(KEY_W)) posMarcador--;
        if (IsKeyPressed(KEY_S)) posMarcador++;
        if (posMarcador == 0) DrawCircle(550, 195, 25, YELLOW);
        if (posMarcador == 1) DrawCircle(765, 295, 25, YELLOW);
        if (posMarcador == 2) DrawCircle(600, 398, 25, YELLOW);
        if (posMarcador == 3) DrawCircle(262, 498, 25, YELLOW);
        //"Vai e vem" do menu
        if (posMarcador == 4) posMarcador = 0;
        if (posMarcador == -1) posMarcador = 3;

        //Se pressionar enter retorna valor selecionado para funcao main
        if (IsKeyPressed(KEY_ENTER))
        {
            CloseWindow();
            return posMarcador;
        }
        EndDrawing();
    }
    return -1;
}

// Função para exibir os highscores
void exibe_highscores(TIPO_SCORE* scores)
{
    // Inicia a janela Raylib para a exibição dos highscores
    InitWindow(LAR_TELA, ALT_TELA, "HIGHSCORES");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("HIGHSCORES", LAR_TELA/2 - MeasureText("HIGHSCORES", 40)/2, ALT_TELA/8, 40, YELLOW);
        for (int i = 0; i < MAXSCORES; i++)
        {
            char buffer[64];
            sprintf(buffer, "%d. %s - %d", i + 1, scores[i].nome, scores[i].score);
            DrawText(buffer, LAR_TELA/4, ALT_TELA/4 + 40*i, 30, WHITE);
        }

        DrawText("Pressione TAB para voltar", LAR_TELA/2 - MeasureText("Pressione TAB para voltar", 20)/2, ALT_TELA - 60, 20, GRAY);

        EndDrawing();

        if (IsKeyPressed(KEY_TAB))    // Verifica se o TAB foi pressionado para voltar ao menu
        {
            break;
        }
        if (IsKeyPressed(KEY_ESCAPE))    // Verifica se o ESC foi pressionado para fechar o jogo
        {
            CloseWindow();  // Fecha a janela
            exit(0);  // Encerra o programa
        }
    }

    CloseWindow();
}

// Função para verificar se o jogador entra no ranking e inserir a pontuação
void insere_highscore_grafico(TIPO_SCORE* scores, int pontuacao)
{
    char nome[30] = "\0";
    int letra_atual = 0;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Novo Highscore!", LAR_TELA / 2 - MeasureText("Novo Highscore!", 40) / 2, ALT_TELA / 4, 40, YELLOW);
        DrawText("Digite seu nome:", LAR_TELA / 2 - MeasureText("Digite seu nome:", 30) / 2, ALT_TELA / 2 - 50, 30, WHITE);
        DrawText(nome, LAR_TELA / 2 - MeasureText(nome, 30) / 2, ALT_TELA / 2, 30, WHITE);

        EndDrawing();

        int key = GetCharPressed();

        if ((key >= 32) && (key <= 125) && (letra_atual < 29))  // Verifica se é uma letra válida e se há espaço no buffer
        {
            nome[letra_atual] = (char)key;
            letra_atual++;
            nome[letra_atual] = '\0';  // Atualiza a string com o novo caractere
        }

        if (IsKeyPressed(KEY_BACKSPACE) && letra_atual > 0)  // Verifica se o backspace foi pressionado
        {
            letra_atual--;
            nome[letra_atual] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER) && letra_atual > 0)  // Se o jogador pressionar ENTER e tiver digitado um nome
        {
            TIPO_SCORE novo_score;
            novo_score.score = pontuacao;
            strcpy(novo_score.nome, nome);
            atualiza_highscores(scores, MAXSCORES, novo_score);
            escreve_arquivo(scores, "highscores.bin");
            break;
        }
    }
}


// Função para ler o arquivo de highscores
int le_arquivo(TIPO_SCORE* scores, char* nome_arq)
{
    FILE *arquivo = fopen(nome_arq, "rb");
    if (arquivo == NULL) return 0;
    fread(scores, sizeof(TIPO_SCORE), MAXSCORES, arquivo);
    fclose(arquivo);
    return 1;
}

// Função para escrever no arquivo de highscores
void escreve_arquivo(TIPO_SCORE* scores, char* nome_arq)
{
    FILE *arquivo = fopen(nome_arq, "wb");
    if (arquivo != NULL)
    {
        fwrite(scores, sizeof(TIPO_SCORE), MAXSCORES, arquivo);
        fclose(arquivo);
    }
}

// Função para atualizar o ranking de highscores
void atualiza_highscores(TIPO_SCORE scores[], int nelem, TIPO_SCORE novo_score)
{
    scores[nelem - 1] = novo_score;
    for (int i = nelem - 1; i > 0; i--)
    {
        if (scores[i].score > scores[i - 1].score)
        {
            TIPO_SCORE temp = scores[i];
            scores[i] = scores[i - 1];
            scores[i - 1] = temp;
        }
        else
        {
            break;
        }
    }
}

//FUNCAO que ira exibir o menu de pause do jogo
int chama_menu_pause()
{
    int posMarcador = 0; // posicao do marcador
    while (!WindowShouldClose()) //loping do pause
    {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("PAUSE", 170, 20, 140, YELLOW);
        DrawText("CONTINUAR", 30, 160, 80, YELLOW);
        DrawText("SALVAR JOGO", 30, 260, 80, YELLOW);
        DrawText("SAIR", 30, 360, 80, YELLOW);
        DrawText("|W| |S| |ENTER|", 580, 600, 30, GRAY);

        //Interacoes com o menu
        if (IsKeyPressed(KEY_W)) posMarcador--;
        if (IsKeyPressed(KEY_S)) posMarcador++;
        if (posMarcador == 0) DrawCircle(550, 195, 25, YELLOW);
        if (posMarcador == 1) DrawCircle(640, 295, 25, YELLOW);
        if (posMarcador == 2) DrawCircle(255, 398, 25, YELLOW);
        if (posMarcador == 3) posMarcador = 0;
        if (posMarcador == -1) posMarcador = 2;
        //Se apertar enter a funcao retorna o valor da opcao selecionada
        if (IsKeyPressed(KEY_ENTER))
        {
            return posMarcador;
        }
        EndDrawing();
    }
    return -1;
}
void salvar_jogo(char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA], STATUS_PLAYER *status_player, POS_PACMAN *pos_player, POS_MONSTRO *monstros)
{
    FILE *file = fopen("savegame.txt", "w");
    if (file == NULL)
    {
        printf("Erro ao salvar jogo\n");
        return;
    }

    // Salva as informações do player
    fprintf(file, "%d %d %d %d %d\n", status_player->vida, status_player->pontuacao, status_player->fase, status_player->dificuldade, status_player->pontuacao_alvo);

    // Salva a posição inicial e atual do Pac-Man
    fprintf(file, "%d %d %d %d\n", pos_player->x_inicial, pos_player->y_inicial, pos_player->x, pos_player->y);

    // Salva a velocidade dos monstros
    fprintf(file, "%f\n", VEL_MONSTROS);

    // Salva o número de monstros e suas posições iniciais e atuais
    fprintf(file, "%d\n", num_monstros);
    for (int i = 0; i < num_monstros; i++)
    {
        fprintf(file, "%d %d %d %d ", monstros[i].x_inicial, monstros[i].y_inicial, monstros[i].x, monstros[i].y);
    }
    // Salva o estado do mapa
    for (int i = 0; i < LINHAS_MAPA; i++)
    {
        for (int j = 0; j < COLUNAS_MAPA; j++)
        {
            fputc(matriz_mapa[i][j], file);
        }
        fputc('\n', file);
    }

    fclose(file);
    CloseWindow();
    exit(0);
}


void carregar_jogo(char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA], STATUS_PLAYER *player, POS_PACMAN *pacman)
{
    FILE *file = fopen("savegame.txt", "r");
    if (file == NULL)
    {
        printf("Erro ao carregar jogo\n");
        return;
    }

    // Carrega as informações do player
    fscanf(file, "%d %d %d %d %d\n", &player->vida, &player->pontuacao, &player->fase, &player->dificuldade, &player->pontuacao_alvo);

    // Carrega a posição atual do Pac-Man
    fscanf(file, "%d %d %d %d\n", &pacman->x_inicial, &pacman->y_inicial, &pacman->x, &pacman->y);
    pacman->dx = 0;
    pacman->dy = 0;
    // Carrega a velocidade dos monstros
    fscanf(file, "%f\n", &VEL_MONSTROS);

    // Carrega o número de monstros e suas posições atuais
    fscanf(file, "%d\n", &num_monstros);
    for (int i = 0; i < num_monstros; i++)
    {
        fscanf(file, "%d %d %d %d", &monstros[i].x_inicial, &monstros[i].y_inicial, &monstros[i].x, &monstros[i].y);
        monstros[i].dx = 1;
        monstros[i].dy = 0;
    }


    for (int i = 0; i < LINHAS_MAPA; i++)
    {
        int j = 0;
        char c;
        while ((c = fgetc(file)) != '\n' && c != EOF)
        {
            if (j < COLUNAS_MAPA)
            {
                matriz_mapa[i][j] = c;
                j++;
            }
        }
        while (j < COLUNAS_MAPA)
        {
            matriz_mapa[i][j] = ' ';
            j++;
        }
    }

    fclose(file);
}

void carrega_mapa(const char *nome_mapa, char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA], POS_PACMAN *pacman, STATUS_PLAYER *player)
{
    FILE *mapa;
    int i, j;
    if (nome_mapa == "mapa1.txt") // se for o primeiro mapa a pontuacao alvo é iniciada em zero
    {
        player->pontuacao_alvo = 0;
    }
    num_monstros = 0;  // Reinicializa o número de monstros para evitar duplicações
    mapa = fopen(nome_mapa, "r");
    if (mapa == NULL)
    {
        printf("Erro ao abrir mapa");
        return;
    }

    for (i = 0; i < LINHAS_MAPA; i++)
    {
        fgets(matriz_mapa[i], COLUNAS_MAPA + 2, mapa);
        for (j = 0; j < COLUNAS_MAPA; j++)
        {
            switch (matriz_mapa[i][j])
            {
            case 'J':
                pacman->x = j;
                pacman->y = i;
                pacman->x_inicial = j;
                pacman->y_inicial = i;
                pacman->dx = 0;
                pacman->dy = 0;
                matriz_mapa[i][j] = ' ';
                break;
            case 'M':
                monstros[num_monstros].x = j;
                monstros[num_monstros].y = i;
                monstros[num_monstros].x_inicial = j;
                monstros[num_monstros].y_inicial = i;
                monstros[num_monstros].dx = 1;
                monstros[num_monstros].dy = 0;
                num_monstros++;
                matriz_mapa[i][j] = ' ';
                break;
            case '.':
                player->pontuacao_alvo += 10;
                break;
            case 'S':
                player->pontuacao_alvo += 20;
                break;
            case 'F':
                player->pontuacao_alvo += 30;
                break;
            default:
                break;
            }
        }
    }

    fclose(mapa);
}



//Funcao responsavel por graficar elementos da matriz
void desenha_mapa(char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA], POS_PACMAN pacman, STATUS_PLAYER player)
{
    //corre linhas e colunas da matriz e grafica seus respectivos elementos
    int i, j;
    for (i = 0; i < LINHAS_MAPA; i++)
    {
        for (j = 0; j < COLUNAS_MAPA; j++)
        {
            switch (matriz_mapa[i][j])
            {
            case 'W':
                DrawRectangle(j * TAM_PIXEL, i * TAM_PIXEL, TAM_PIXEL, TAM_PIXEL, BLUE);
                break;
            case 'F':
                DrawCircle(j * TAM_PIXEL + TAM_PIXEL / 2, i * TAM_PIXEL + TAM_PIXEL / 2, TAM_PIXEL / 2, RED);
                break;
            case 'M':
                DrawCircle(j * TAM_PIXEL + TAM_PIXEL / 2, i * TAM_PIXEL + TAM_PIXEL / 2, TAM_PIXEL / 2, PURPLE);
                break;
            case '.':
                DrawCircle(j * TAM_PIXEL + TAM_PIXEL / 2, i * TAM_PIXEL + TAM_PIXEL / 2, TAM_PIXEL / 4, WHITE);
                break;
            case 'S':
                DrawCircle(j * TAM_PIXEL + TAM_PIXEL / 2, i * TAM_PIXEL + TAM_PIXEL / 2, TAM_PIXEL / 2, ORANGE);
                break;
            default:
                break;
            }
        }
    }

    // Desenha o Pacman
    DrawCircle(pacman.x * TAM_PIXEL + TAM_PIXEL / 2, pacman.y * TAM_PIXEL + TAM_PIXEL / 2, TAM_PIXEL / 2, YELLOW);

    // Desenha os monstros
    for (i = 0; i < num_monstros; i++)
    {
        DrawCircle(monstros[i].x * TAM_PIXEL + TAM_PIXEL / 2, monstros[i].y * TAM_PIXEL + TAM_PIXEL / 2, TAM_PIXEL / 2, PURPLE);
    }

    // Desenha a pontuação e vidas
    DrawText(TextFormat("Vidas: %d", player.vida), 10, 5, 30, RED);
    DrawText(TextFormat("Pontos: %d", player.pontuacao), 180, 5, 30, BLUE);
    DrawText(TextFormat("Fase: %d", player.fase), 400, 5, 30, YELLOW);
    DrawText(TextFormat("Dificuldade: %d", player.dificuldade), 590, 5, 30, ORANGE);
}
//Funcao acionada caso player colida em um monstro
void trata_colisao(POS_PACMAN *pacman, STATUS_PLAYER *player)
{
    player->vida--;
    pacman->x = pacman->x_inicial;
    pacman->y = pacman->y_inicial;

    for (int j = 0; j < num_monstros; j++)
    {
        monstros[j].x = monstros[j].x_inicial;
        monstros[j].y = monstros[j].y_inicial;
    }
}
//Funcao para evitar que dois monstros ocupem o mesmo pixel
void trata_colisao_monstros(POS_MONSTRO *monstros)
{
    for (int i = 0; i < num_monstros; i++)
    {
        for (int j = i + 1; j < num_monstros; j++)
        {
            if (monstros[i].x == monstros[j].x && monstros[i].y == monstros[j].y)
            {
                if (monstros[i].dx != 0)
                {
                    monstros[i].dx = 0;
                    monstros[i].dy = (rand() % 2) ? 1 : -1; // sorteia uma direcao para o monstro seguir
                }
                else
                {
                    monstros[i].dy = 0;
                    monstros[i].dx = (rand() % 2) ? 1 : -1; // sorteia uma direcao para o monstro seguir
                }
            }
        }
    }
}

int verifica_fim_de_fase(STATUS_PLAYER *player)
{
    return player->pontuacao >= player->pontuacao_alvo;
}
//Verifica se o pacman nao esta ocupando uma posicao com coletavel, se sim faz a coleta e substitui por um espaco vazio.
void verifica_coleta(POS_PACMAN *pacman, char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA], STATUS_PLAYER *player)
{
    char item = matriz_mapa[pacman->y][pacman->x];
    switch (item)
    {
    case '.':
        player->pontuacao += 10;
        matriz_mapa[pacman->y][pacman->x] = ' ';
        break;
    case 'S':
        player->pontuacao += 20;
        matriz_mapa[pacman->y][pacman->x] = ' ';
        break;
    case 'F':
        player->pontuacao += 30;
        matriz_mapa[pacman->y][pacman->x] = ' ';
        break;
    default:
        break;
    }
}
void controla_dificuldade(STATUS_PLAYER *player)
{
    player->dificuldade++;
    VEL_MONSTROS = VEL_MONSTROS-0.05;
}

//funcao responsavel pela movimentacao do pac-man
void move_pacman(POS_PACMAN *pacman, char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA], STATUS_PLAYER *player, float deltaTime)
{
    static float timer = 0.0f;
    //Quando se usa o "static"o valor da variavel fica armazenado na memoria, mesmo que o programa saia dessa funcao o valor nao ira mudar (nao voltara pra zero)
    timer += deltaTime; //temporizador para cadenciar movimentos do pacman (controlar velocidade)
    if (timer >= VEL_PACMAN) //Se o valor chegar na velocidade de pacman entra no looping
    {
        int novoX = pacman->x + pacman->dx; //move nova posicao do pacman na direcao do vetor "acionado"
        int novoY = pacman->y + pacman->dy;

        if (matriz_mapa[novoY][novoX] != 'W')  // se essa posicao NAO for uma parede, o pacman vai para nova posicao
        {
            pacman->x = novoX;
            pacman->y = novoY;
            // verifica se essa posicao nao é a mesma de nem um dos monstro, se for, aciona a funcao trata_colisao
            for (int i = 0; i < num_monstros; i++)
            {
                if (pacman->x == monstros[i].x && pacman->y == monstros[i].y)
                {
                    trata_colisao(pacman, player);
                    return;
                }
            }
            //Aciona funcao para verificar se coleta foi feita
            verifica_coleta(pacman, matriz_mapa, player);
        }
        timer = 0.0f; //Zera timer
    }
}


//Funcao heuristica para apoio do algoritmo A*
int heuristica(int x1, int y1, int x2, int y2)
{
    return abs(x1 - x2) + abs(y1 - y2); //Retorna a soma das distancias entre as coordenadas x e y de dois pontos.
}

//Funcao para prencher os valores de um Node
//Cria e inicializa um novo Node com as coordenadas fornecidas, custo g, heurística h, e uma referência ao node original.
Node *cria_node(int x, int y, int g, int h, Node *parent)
{
    //Vetor node (*node)
    Node *node = (Node *)malloc(sizeof(Node));
    // é necessario alocar memória suficiente para armazenar um Node e retorna um ponteiro para esse espaço de memória.
    //(Node *) Faz um casting que indica que é um ponteiro do tipo Node.
    node->x = x;
    node->y = y;
    node->g = g;
    node->h = h;
    node->f = g + h;
    node->parent = parent;
    return node;
}

// Função de comparação usada pela funcao qsort para ordenar nós na lista aberta.
int compara_nodes(const void *a, const void *b)
// declarados como constantes vazias, pois valores não serao alterados (valores serao apenas comparados nessa funcao).
{
//Compara os valores f de dois nós. Isso garante que os nós na lista aberta sejam processados na ordem de menor custo total f.
    Node *nodeA = *(Node **)a;
    Node *nodeB = *(Node **)b;
    //Sintaxe chata de entender, mas passo a passo:
    // "a" é um ponteiro genérico (void *), (Node **)a: É um casting (conversão de tipo) do ponteiro genérico "a" para um ponteiro de Node (tipo Node).
    // *(Node **)a - desreferencianmento do ponteiro resultante do casting. Ou seja, está acessando o valor para o qual o ponteiro (agora do tipo Node) está apontando.
    // Node *nodeA = *(Node **)b  - No final armazena esse valor apontado em NodeA
    return nodeA->f - nodeB->f;  //Se o f do A for maior que o f do B retornara um valor positivo, se os "f" forem iguais retornara 0, se f do A for menor que f do B retornara negativo.
}

//função para atualizar o movimento dos monstros em direção ao Pac-Man usando o algoritmo A* e fazendo alguns outros tratamentos
void move_monstros(POS_PACMAN *pacman, STATUS_PLAYER *player, char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA], float deltaTime)
{
    static float timer = 0.0f;
    static int dificuldade_contador=0;

    timer += deltaTime;
    if (dificuldade_contador >=TEMPO_DIFICULDADE)
    {
        dificuldade_contador=0;
        controla_dificuldade(player);
    }

    if (timer >= VEL_MONSTROS) //Mesma logica do move_pacman
    {
        dificuldade_contador++;
        for (int i = 0; i < num_monstros; i++) //A operacao para mover os monstros ira se repetir para todos os monstros
        {
            //                    dir      esq     baixo    cima
            int direcoes[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; //vetor das configuracoes de direcoes (direita, esquerda, baixo, cima)
            int melhor_dx = 0, melhor_dy = 0; //iniciando parado

            Node *abertos[LINHAS_MAPA * COLUNAS_MAPA]; //Vetor de nos abertos
            int num_abertos = 0; //Numero de nodes abertos
            Node *fechados[LINHAS_MAPA * COLUNAS_MAPA]; //Vetor de nos Fechados
            int num_fechados = 0; //Numero de nodes fechados

            Node *inicio = cria_node(monstros[i].x, monstros[i].y, 0, heuristica(monstros[i].x, monstros[i].y, pacman->x, pacman->y), NULL);
            //                                                        ^chama funcao heuristica
            abertos[num_abertos++] = inicio;//adiciona posicao inicial a lista aberta

            while (num_abertos > 0)
            {
                qsort(abertos, num_abertos, sizeof(Node *), compara_nodes);//Funcao responsavel pela ordenacao/organizacao da lista aberta
//parametros do qsort(vetor que sera organizado, numero de elementos no vetor, tamanho em bytes dos elementos do vetor, funcao para comparar elementos)
                Node *atual = abertos[0]; //Node *atual = o elemento com o valor de menor F da lista aberta

                for (int j = 0; j < num_abertos - 1; j++)
                {
                    abertos[j] = abertos[j + 1]; //move todos os nós na lista de "abertos" uma posição para a esquerda, removendo o primeiro nó (abertos[0]) que já está sendo explorado.
                }
                num_abertos--; // Diminui o numero de nodes na lista aberta

                fechados[num_fechados++] = atual;//Adiciona posicao atual a lista fechada
                //PASSOS CASO NODE ATUAL SEJA A POSICAO DO PACMAN
                if (atual->x == pacman->x && atual->y == pacman->y) // se a posicao atual for a do pacman:
                {
                    Node *caminho = atual;//Um ponteiro que aponta para o node atual do pacman
                    while (caminho->parent->parent)
                        //Essa condicao do loop while não é trivial. Ela vai seguir os nós anteriores enquanto o ultimo parent (node pai) não for vazio (null).
                        //Nesse caso, um parent vazio significa que encontramos a posicao inicial, (a posicao sem node pai) mas como queremos a proxima posicao (seguinte a posicao inicial)
                        //o loop ira rodar até encontrar uma posicao que tenha o "Node vo" vazio.
                    {
                        caminho = caminho->parent; //Avancamos, ou melhor, recuamos um no, como visto acima o objetivo e recuar ate encontrar o no que antecede a posicao com o parent vazio, ou seja a posicao inicial
                    }
                    melhor_dx = caminho->x - monstros[i].x;//proxima posicao menos posicao atual do monstro para x
                    melhor_dy = caminho->y - monstros[i].y;//proxima posicao menos posicao atual do monstro para y
                    //Legenda

                    //melhor_dx = 1 significa mover para a direita,
                    //melhor_dx = -1 significa mover para a esquerda,
                    //melhor_dx = 0 signidica que o monstro não precisa se mover no eixo x

                    //melhor_dy = 1 significa mover para baixo,
                    //melhor_dy = -1 significa mover para cima.
                    //melhor_dy = 0 signidica que o monstro não precisa se mover no eixo y

                    break; //Sai do loop
                }

                //Esse trecho do código é responsável por explorar as quatro direções possíveis a partir do nó atual(direita, esquerda, para cima, para baixo)
                //e avaliar cada um desses movimentos para determinar se devem ser adicionados à lista de "abertos" (nós que serão explorados em etapas futuras).
                for (int d = 0; d < 4; d++) //Esse loop ira iterar para cada uma das 4 possiveis direcoes
                {
                    //legenda - direcoes[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                    int nx = atual->x + direcoes[d][0]; //nova cordenada de X, resultante da posicao atual mais a direcao
                    int ny = atual->y + direcoes[d][1]; //nova cordenada de Y, resultante da posicao atual mais a direcao
                    int g = atual->g + 1; //custo para chegar no nó atual + 1
                    int h = heuristica(nx, ny, pacman->x, pacman->y); //Heuristica da distancia da nova posicao ate o pacman
                    int f = g + h; // F = Valor do custo do nó + Heuristica

                    //Verificação para ver se o novo Nó não Está na Lista dos Fechados
                    int fechado = 0;//flag para node fechado
                    for (int j = 0; j < num_fechados; j++)
                    {
                        if (fechados[j]->x == nx && fechados[j]->y == ny)
                        {
                            fechado = 1;
                            break;//Se for constatado que o nó já está na lista dos fechados o looping é interrompido
                        }
                    }
                    if (fechado) continue;

                    //Verificação para ver se o novo Nó Está na Lista dos Abertos, se não estiver, será adicionado, a menos que a posicao seja uma parede
                    int aberto = 0; //flag para node aberto
                    for (int j = 0; j < num_abertos; j++)
                    {
                        if (abertos[j]->x == nx && abertos[j]->y == ny && abertos[j]->f <= f)
                            //Verifica para ver se o novo Nó Está na Lista dos Abertos e Verifica se o valor f do nó na lista de "abertos" é menor ou igual ao f do nó anterior.
                            //Se não for menor, o looping também para, isso é uma questáo de otimizacao na escolha dos nos que irao entrar na lista aberta
                        {
                            aberto = 1;
                            break; //Se for constatado que o nó esta na lista dos abertos o looping para
                        }
                    }
                    if (aberto) continue;

                    //Verifica se nova posicao não é uma parede
                    if (matriz_mapa[ny][nx] != 'W')
                    {
                        Node *vizinho = cria_node(nx, ny, g, h, atual); //Cria node com a nova posicao
                        abertos[num_abertos++] = vizinho; //Adicionha node a lista dos abertos
                    }
                }
            }
            //Limpa todos os nodes apos a direcao ter sido determinada
            for (int j = 0; j < num_abertos; j++)
            {
                free(abertos[j]); //Libera a memoria alocada dinamicamente na funcao cria nodes
            }
            for (int j = 0; j < num_fechados; j++)
            {
                free(fechados[j]); //Libera a memoria alocada dinamicamente na funcao cria nodes
            }

            monstros[i].dx = melhor_dx; //atualiza direcao do monstro em x
            monstros[i].dy = melhor_dy; //atualiza direcao do monstro em y
            monstros[i].x += monstros[i].dx; //atualiza posicao do monstro em x
            monstros[i].y += monstros[i].dy; //atualiza posicao do monstro em y


            if (monstros[i].x == pacman->x && monstros[i].y == pacman->y) //se a posicao atual do monstro for igual a do pacman
            {
                trata_colisao(pacman, player); //chama a funcao  trata colisao
                return;
            }
        }

        trata_colisao_monstros(monstros); //Se monstros estiverem ocupando o mesmo espaco essa funcao fara eles se separarem
        timer = 0.0f; //Zera timer
    }
}


void gameplay(char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA], POS_PACMAN *pacman, STATUS_PLAYER *player, int carregar)
{
    if (!carregar)  // Se não for para carregar jogo salvo, inicializa tudo.
    {
        player->vida = 3;
        player->pontuacao = 0;
        player->fase = 1;
        player->dificuldade = 1;
        carrega_mapa(mapas[player->fase - 1], matriz_mapa, pacman, player);
    }

    // Inicia interface gráfica do jogo
    InitWindow(LAR_TELA, ALT_TELA, "PAC-MAN");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();  // variação de tempo, baseado no valor de variação de quadros na tela

        //FLUXO PRA QUANDO O JOGADOR MORRE
        if (player->vida == 0)
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Game Over!", LAR_TELA / 2 - 100, ALT_TELA / 2 - 50, 50, RED);
            EndDrawing();
            WaitTime(2.0); // Espera 2 segundos antes de continuar

            // Verifica se a pontuação do jogador entra no ranking dos highscores
            TIPO_SCORE scores[MAXSCORES];
            le_arquivo(scores, "highscores.bin");

            if (player->pontuacao > scores[MAXSCORES - 1].score)
            {
                insere_highscore_grafico(scores, player->pontuacao);
            }

            break;  // Sai do loop para finalizar o jogo
        }

        //FLUXO PRA QUANDO JOGADOR FINALIZA FASE
        if (verifica_fim_de_fase(player))
        {
            player->fase++;
            if (player->fase > NUM_MAPAS) //VERIFICA SE JOGADOR FINALIZOU O JOGO
            {
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("VOCE VENCEU!", 200, 200, 50, GREEN);
                EndDrawing();
                WaitTime(2.0);

            // Verifica se a pontuação do jogador entra no ranking dos highscores
            TIPO_SCORE scores[MAXSCORES];
            le_arquivo(scores, "highscores.bin");

            if (player->pontuacao > scores[MAXSCORES - 1].score)
            {
                insere_highscore_grafico(scores, player->pontuacao);
            }
                break;
            }
            else
            {
                player->dificuldade = 1;
                VEL_MONSTROS = 0.30;
                carrega_mapa(mapas[player->fase - 1], matriz_mapa, pacman, player);
            }
        }

        // Fluxo pra quando o jogador aperta o botão para pausar o jogo
        if (IsKeyPressed(KEY_TAB))
        {
            int opcao_pause = chama_menu_pause();
            if (opcao_pause == 0)
            {
                continue;  // continua o jogo
            }
            else if (opcao_pause == 1)
            {
                salvar_jogo(matriz_mapa, player, pacman, monstros);  // Inicia função salvar jogo
                CloseWindow();  // Fecha o jogo após salvar
                return;  // Sai da função gameplay
            }
            else if (opcao_pause == 2)
            {
                break; // Fecha o jogo
            }
        }

        // Interações de movimento do pacman
        if (IsKeyPressed(KEY_RIGHT))
        {
            pacman->dx = 1; // Mudando a direção de movimento do pacman
            pacman->dy = 0;
        }
        if (IsKeyPressed(KEY_LEFT))
        {
            pacman->dx = -1;
            pacman->dy = 0;
        }
        if (IsKeyPressed(KEY_UP))
        {
            pacman->dx = 0;
            pacman->dy = -1;
        }
        if (IsKeyPressed(KEY_DOWN))
        {
            pacman->dx = 0;
            pacman->dy = 1;
        }

        // Atualiza o movimento do Pac-Man e dos monstros
        move_pacman(pacman, matriz_mapa, player, deltaTime);
        move_monstros(pacman, player, matriz_mapa, deltaTime);

        // Interface gráfica
        BeginDrawing();
        ClearBackground(BLACK);
        desenha_mapa(matriz_mapa, *pacman, *player);
        EndDrawing();
    }

    // Fecha a janela do jogo
    CloseWindow();
}
int main(void)
{
    while (1)  // Loop principal para manter o menu ativo
    {
        int opcao = chama_menu();

        switch (opcao)
        {
        case 0: //Código para novo jogo
        {
            char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA];
            POS_PACMAN pacman;
            STATUS_PLAYER player;
            player.fase = 1;
            player.dificuldade = 1;
            gameplay(matriz_mapa, &pacman, &player, 0);
            break;
        }
        case 1: //Código para carregar jogo
        {
            char matriz_mapa[LINHAS_MAPA][COLUNAS_MAPA];
            POS_PACMAN pacman;
            STATUS_PLAYER player;
            carregar_jogo(matriz_mapa, &player, &pacman);
            gameplay(matriz_mapa, &pacman, &player, 1);
            break;
        }
        case 2: // Código para exibir ranking
        {
            TIPO_SCORE scores[MAXSCORES];
            le_arquivo(scores, "highscores.bin");
            exibe_highscores(scores);
            break;
        }
        case 3: // Código para sair do jogo
            CloseWindow();  // Fecha a janela aberta
            return 0;  // Encerra o programa
        }

        // Verifica se ESC foi pressionado para sair do loop principal
        if (IsKeyPressed(KEY_ESCAPE))
        {
            CloseWindow();  // Fecha qualquer janela aberta
            return 0;  // Encerra o programa
        }
    }

    return 0;
}
