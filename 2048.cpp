#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

using namespace std;
using std::vector;
using std::string;

int dirRow[] = {1, 0, -1, 0};
int dirCol[] = {0, 1, 0, -1};
int a[] = {2, 2, 2, 4}; //Hien thi random 1 so trong mang a, 25% hien thi so 4

const int WINDOW_CELL_WIDTH     = 100;
const int WINDOW_CELL_HEIGHT    = 100;

const int DEFAULT_NUM_ROWS      = 4;
const int DEFAULT_NUM_COLS      = 4;
const int NUM_APPEAR_FIRST     = 2;

const string SCREEN_TITLE       = "2048";
const string musicPath = "2048_sunrise.mp3";
const string fontPath = "text3d.ttf";
const string Lose = "Game Over!";
const string Win = "You Win!";

const SDL_Color colorText = {0, 0, 0, 0}; // Black
const SDL_Rect Result = {WINDOW_CELL_WIDTH,WINDOW_CELL_HEIGHT, 2*WINDOW_CELL_WIDTH, WINDOW_CELL_HEIGHT}; // Vi tri hien thi ket qua Lose or Win

enum GameState {
    GAME_PLAYING,
    GAME_WON,
    GAME_LOSE
};

typedef vector<vector<int> > CellTable;

struct Game {
    int nRows;
    int nCols;
    int n;
    CellTable cells;
    GameState state;
};

struct Graphic {
    SDL_Window *window;
    SDL_Texture *spriteTexture[12];
    SDL_Texture *replayIMG;
    SDL_Renderer *renderer;
    Mix_Music* music;
    TTF_Font* font;
    SDL_Texture *textEnd;
};
bool initGraphic(Graphic &g, int nRows, int nCols); // Khoi tao SDL, SDL_Image, SDL_mixer ,  ... Tra ve false neu khoi tao khong thanh cong

void finalizeGraphic(Graphic &g); // Huy khoi tao nhung gi da dung

SDL_Texture* createTexture(SDL_Renderer *renderer, const string &path); // Load anh tu duong dan path, tra ve bieu dien duoi dang SDL_Texture

void initGame(Game &game, int nRows, int nCols, int n); // Khoi tao game

void random(int nRows, int nCols, int n ,Game &game); //  Tao so o vi tri ngau nhien

void displayGame(Game &game,Graphic &graphic); // Hien thi trang thai game len cua so

int log2(int n);

bool canDoMove(int row, int col, int nextRow, int nextCol); // Check su di chuyen

void updateGame(Game &game, const SDL_Event& event ,SDL_Rect , Graphic &g); // Cap nhat trang thai cua game sau khi co su kien event

void check(Game &game); // Kiem tra Win - Lose

void err(const string &msg); // Hien thi cua so thong bao loi

void drawEndGame(Game &game,Graphic &g ,SDL_Rect replay); // Hien thi trang thai game khi ket thuc

int main(int argc, char *argv[]) {
    srand(time(0));

    int nRows = DEFAULT_NUM_ROWS,
        nCols = DEFAULT_NUM_COLS,
        n = NUM_APPEAR_FIRST;

    Graphic graphic;
    SDL_Rect replay = {WINDOW_CELL_WIDTH, 2*WINDOW_CELL_HEIGHT, 2*WINDOW_CELL_WIDTH, WINDOW_CELL_HEIGHT};
    if (!initGraphic(graphic, nRows, nCols)) {
        finalizeGraphic(graphic);
        return EXIT_FAILURE;
    }

    Game game;
    initGame(game, nRows, nCols, n);

    Mix_PlayMusic(graphic.music, -1);

    bool quit = false;
    while (!quit) {
        displayGame(game, graphic);

        check(game);
        if ( game.state != GAME_PLAYING)
        {
            Mix_FreeMusic(graphic.music);
            graphic.music = NULL;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            }

            updateGame(game, event,replay ,graphic);
        }
        if (game.state != GAME_PLAYING) drawEndGame(game,graphic,replay);
        SDL_RenderPresent(graphic.renderer);
    }

    finalizeGraphic(graphic);
    return EXIT_SUCCESS;
}

bool initGraphic(Graphic &g, int nRows, int nCols) {
    g.window = NULL;
    g.renderer = NULL;
    g.music = NULL;
    g.font = NULL;

    for (int  i = 0 ; i < 12; i++)
        g.spriteTexture[i] = NULL;

    int sdlFlags = SDL_INIT_VIDEO;
    if (SDL_Init(sdlFlags) != 0) {
        err("SDL could not initialize!");
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        err("SDL_Image could not initialize!");
        return false;
    }

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) != 0)
    {
        err("SDL_Mixer could not initialize!");
        return false;
    }

    if (TTF_Init() != 0)
    {
        err("SDL_ttf could not initialize!");
        return false;
    }

    g.window = SDL_CreateWindow(SCREEN_TITLE.c_str(),
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                nCols * WINDOW_CELL_WIDTH,
                                nRows * WINDOW_CELL_HEIGHT,
                                SDL_WINDOW_SHOWN);

    if (g.window == NULL) {
        err("Window could not be created!");
        return false;
    }

    g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED);
    if (g.renderer == NULL) {
        err("Renderer could not be created!");
        return false;
    }

    g.spriteTexture[0] = createTexture(g.renderer, "blank.png");
    if (g.spriteTexture[0] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[1] = createTexture(g.renderer, "2.png");
    if (g.spriteTexture[1] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[2] = createTexture(g.renderer, "4.png");
    if (g.spriteTexture[2] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[3] = createTexture(g.renderer, "8.png");
    if (g.spriteTexture[3] == NULL) {
       err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[4] = createTexture(g.renderer, "16.png");
    if (g.spriteTexture[4] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[5] = createTexture(g.renderer, "32.png");
    if (g.spriteTexture[5] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[6] = createTexture(g.renderer, "64.png");
    if (g.spriteTexture[6] == NULL) {
        err("Unable to create texture ");
    }
    g.spriteTexture[7] = createTexture(g.renderer, "128.png");
    if (g.spriteTexture[7] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[8] = createTexture(g.renderer, "256.png");
    if (g.spriteTexture[8] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[9] = createTexture(g.renderer, "512.png");
    if (g.spriteTexture[9] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[10] = createTexture(g.renderer, "1024.png");
    if (g.spriteTexture[10] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.spriteTexture[11] = createTexture(g.renderer, "2048.png");
    if (g.spriteTexture[11] == NULL) {
        err("Unable to create texture ");
        return false;
    }
    g.replayIMG = createTexture(g.renderer,"reset.png");
    if(g.replayIMG == NULL) {
        err("Unable to create texture ");
        return false;
    }

    g.music = Mix_LoadMUS(musicPath.c_str());
    if (g.music == NULL)
    {
        err("Unable to create music ");
        return false;
    }

    g.font = TTF_OpenFont(fontPath.c_str(), 30);
    if(g.font == NULL)
    {
         err("Unable to create font ");
        return false;
    }
    return true;


}

void finalizeGraphic(Graphic &g) {
     for (int  i = 0 ; i < 12; i++) {
        SDL_DestroyTexture(g.spriteTexture[i]);
     }
    SDL_DestroyRenderer(g.renderer);
    SDL_DestroyWindow(g.window);
    Mix_FreeMusic(g.music);
    TTF_CloseFont(g.font);
    IMG_Quit();
    SDL_Quit();
    Mix_Quit();
    TTF_Quit();

}

SDL_Texture* createTexture(SDL_Renderer *renderer, const string &path) {
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (surface == NULL) {
        err("Unable to load image " + path + "!");
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void random(int nRows, int nCols, int n,Game &game) {
        int maxVal = nRows * nCols;
        int val = rand() % maxVal;
        int row = val / nCols;
        int col = val % nCols;
        if (game.cells[row][col] == 0) {
            game.cells[row][col] = n ;
        }
        else random(nRows, nCols, n , game);
}

void initGame(Game &game, int nRows, int nCols, int n) {
    game.cells = CellTable(nRows, vector<int>(nCols));
    for (int i = 0; i < nRows; i ++) {
        for (int j = 0; j < nCols; j ++) {
            game.cells[i][j] = 0;
        }
    }
    random(nRows, nCols, n, game);

    game.nRows = nRows;
    game.nCols = nCols;
    game.n = NUM_APPEAR_FIRST;
    game.state = GAME_PLAYING;
}

void displayGame(Game &game, Graphic &graphic) {
    SDL_RenderClear(graphic.renderer);

    for (int i = 0; i < game.nRows; i ++) {
        for (int j = 0; j < game.nCols; j ++){
            SDL_Rect destRect = {
                j * WINDOW_CELL_WIDTH,
                i * WINDOW_CELL_HEIGHT,
                WINDOW_CELL_WIDTH,
                WINDOW_CELL_HEIGHT
            };

            SDL_RenderCopy(graphic.renderer, graphic.spriteTexture[log2(game.cells[i][j])] , NULL,
                           &destRect);
        }
    }
}

int log2(int n) {
    if(n <=0) return 0;
    int a=n;
    int b=0;
    while(a%2==0)
    {
        b++;
        a=a/2;
    }
    return b;
}

bool canDoMove(Game &game, int row, int col, int nextRow, int nextCol)
{
    if (nextRow < 0 || nextCol < 0 || nextRow > 3 || nextCol > 3 || (game.cells[row][col] != game.cells[nextRow][nextCol] && game.cells[nextRow][nextCol] != 0))
        return false;
    return true;
}

void updateGame(Game &game, const SDL_Event &event, SDL_Rect replay , Graphic &g) {
    int startRow = 0, startCol = 0, rowStep = 1, colStep = 1;
    int direction = -1;
    if(event.type==SDL_KEYDOWN)
    {
        switch( event.key.keysym.sym ){
                    case SDLK_DOWN:
                        startRow = 3;
                        rowStep = -1;
                        direction = 0;
                        break;
                    case SDLK_RIGHT:
                        startCol = 3;
                        colStep = -1;
                        direction = 1;
                        break;
                    case SDLK_UP:
                        direction = 2;
                        break;
                    case SDLK_LEFT:
                        direction = 3;
                        break;
                        // De hien thi trang thai Win bam so 1 ( Test )
                    case SDLK_KP_1:
                        game.cells[0][0] = 2048;
                        break;
                    default:
                        break;
                }
    }
    if(event.type == SDL_MOUSEBUTTONDOWN && game.state != GAME_PLAYING)
        {
                int x,y;
                SDL_GetMouseState(&x,&y); // Lay vi tri con tro chuot click vao
                bool checkX=false;
                bool checkY=false;
                if(x >= replay.x && x <= replay.x + replay.w) checkX=true;
                if(y >= replay.y && y <= replay.y + replay.h) checkY=true;
                if(checkX && checkY)
                {
                    initGame(game,DEFAULT_NUM_ROWS,DEFAULT_NUM_COLS,NUM_APPEAR_FIRST);
                    g.music = Mix_LoadMUS(musicPath.c_str());
                    Mix_PlayMusic(g.music, -1);
                }
            }

    bool movePossible, canAddNumber = 0;
    do {
        movePossible = 0;
        for (int i = startRow; i >= 0 && i < 4; i += rowStep)
        {
            for (int j = startCol; j >= 0 && j < 4; j += colStep)
            {
                int nextI = i + dirRow[direction], nextJ = j + dirCol[direction];
                if (game.cells[i][j] != 0 && canDoMove(game, i, j, nextI, nextJ)) {
                    game.cells[nextI][nextJ] += game.cells[i][j];
                    game.cells[i][j] = 0;
                    movePossible =  canAddNumber = 1;
                }
            }
        }
    } while (movePossible && direction >=0);
    if (canAddNumber)
        random(DEFAULT_NUM_ROWS, DEFAULT_NUM_COLS, a[rand() % 4], game);

}

void drawEndGame(Game &game,Graphic &g , SDL_Rect replay)
{
    if(game.state!=GAME_PLAYING)
    {
        SDL_Surface *textEndBefore = NULL;
        if(game.state==GAME_WON)
        {
            textEndBefore=TTF_RenderText_Solid(g.font, Win.c_str(), colorText);
            g.textEnd=SDL_CreateTextureFromSurface(g.renderer, textEndBefore);
        }
        if(game.state==GAME_LOSE)
        {
            textEndBefore=TTF_RenderText_Solid(g.font,Lose.c_str(),colorText);
            g.textEnd=SDL_CreateTextureFromSurface(g.renderer, textEndBefore);
        }
        SDL_FreeSurface(textEndBefore);
        SDL_RenderCopy(g.renderer,g.textEnd,NULL,&Result);
        SDL_RenderCopy(g.renderer,g.replayIMG,NULL,&replay);
    }

}

void check(Game &game)
{
    bool check=false; // Check win
    bool checkEquals=false; // Check cac so bang nhau canh nhau
    bool checkZero=false; // Check con o blank nao khong
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (game.cells[i][j] == 2048) {game.state = GAME_WON; check=true;} // Neu xuat hien 2048 la Win
            else
            {
                if(game.cells[i][j]==0) checkZero=true;
                if(j < 3 && (game.cells[i][j]==game.cells[i][j+1] || game.cells[j][i]==game.cells[j+1][i]) && !check) checkEquals=true;
            }
            if(check || checkZero || checkEquals) break;
        }
        if(check || checkZero || checkEquals) break;
    }
    if(!checkZero && !checkEquals && !check) game.state = GAME_LOSE;
}

void err(const string &m) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", m.c_str(), NULL);
}
