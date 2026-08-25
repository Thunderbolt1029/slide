#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define USAGE_MESSAGE \
    "Usage: slide [OPTIONS]\n\n" \
    "The classic slide puzzle\n\n" \
    "Options\n" \
    "  -h, --help"        "\n          " "Show this help message\n" \
    "      --no-colour"   "\n          " "Disable colours\n" \
    "      --seed <seed>" "\n          " "Set the seed for generating the puzzle\n" \
    "      --size <size>" "\n          " "Set the size of the grid for this puzzle\n" 

#define TILE_WIDTH 5
#define TILE_HEIGHT 1

#define true (bool)1
#define false (bool)0

void drawGameWin(void);
bool trySlide(void);
bool gameWon(void);
void shuffleGrid(void);

int gridSize = 4;
int **grid;

WINDOW *gameWin;
int gameWinWidth, gameWinHeight;
int highlightX = 0, highlightY = 0;

unsigned int seed;
int moves = 0;

int main(int argc, char **argv) {	
    bool haveSeed = false, colourEnable = true;

    argc--; argv++;
    while (argc > 0) {
        if (!strcmp(*argv, "-h") || !strcmp(*argv, "--help")) {
            printf(USAGE_MESSAGE);
            return 0;
        }
        else if (!strcmp(*argv, "--no-colour")) {
            colourEnable = false;
        }
        else if (!strcmp(*argv, "--seed")) {
            argc--; argv++;
            seed = atoi(*argv);
            haveSeed = true;
        }
        else if (!strcmp(*argv, "--size")) {
            argc--; argv++;
            gridSize = atoi(*argv);
        }
        else {
            fprintf(stderr, "Unknown argument: %s\n", *argv);
            fprintf(stderr, USAGE_MESSAGE);
            return 1;
        }

        argc--; argv++;
    }

    if (!haveSeed)
        seed = time(NULL);
    srand(seed);

    grid = malloc(sizeof(int *) * gridSize);
    for (int i = 0; i < gridSize; i++)
        grid[i] = malloc(sizeof(int) * gridSize);

    for (int x = 0; x < gridSize; x++)
    for (int y = 0; y < gridSize; y++)
        grid[y][x] = y*gridSize + x + 1;
    grid[gridSize-1][gridSize-1] = -1;

    shuffleGrid();

	initscr();
	clear();
	noecho();
	cbreak();	/* Line buffering disabled. pass on everything */
    curs_set(0);

    if (colourEnable)
        start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    int maxX, maxY;
    getmaxyx(stdscr, maxY, maxX);

    gameWinWidth = gridSize * TILE_WIDTH + 2;
    gameWinHeight = gridSize * TILE_HEIGHT + 2;

	int gameWin_locX = (maxX - gameWinWidth) / 2;
	int gameWin_locY = (maxY - gameWinHeight) / 2;

	gameWin = newwin(gameWinHeight, gameWinWidth, gameWin_locY, gameWin_locX);
	keypad(gameWin, TRUE);

    drawGameWin();

    bool won = false;
	int c = 0;
	while(c != 'q' && c != 'Q') {	
        c = wgetch(gameWin);
		switch(c)
		{	
            case KEY_UP:
            case 'w':
            case 'k':
                if (highlightY > 0)
                    highlightY--;
                break;

			case KEY_DOWN:
            case 's':
            case 'j':
                if (highlightY < gridSize-1)
                    highlightY++;
				break;

            case KEY_LEFT:
            case 'a':
            case 'h':
                if (highlightX > 0)
                    highlightX--;
				break;

			case KEY_RIGHT:
            case 'd':
            case 'l':
                if (highlightX < gridSize-1)
                    highlightX++;
				break;

			case KEY_ENTER:
            case '\n':
            case ' ':
                if (trySlide())
                    moves++;

				break;
		}
		drawGameWin();

        if (won = gameWon()) {
            mvwprintw(gameWin, 0, 1, "WON!");
            wgetch(gameWin);
            break;
        }
	}	

	endwin();

    if (won) 
        printf("Size: %d, Seed: %u, Moves: %d\n", gridSize, seed, moves);
    else
        printf("Size: %d, Seed: %u, Moves: N/A\n", gridSize, seed);

    for (int i = 0; i < gridSize; i++)
        free(grid[i]);
    free(grid);

	return 0;
}


void drawGameWin() {
	box(gameWin, 0, 0);

    char seedStr[gridSize];
    int seedStrLen = snprintf(seedStr, gameWinWidth, "Seed:%d", seed);
    mvwprintw(gameWin, 0, gameWinWidth-1-seedStrLen, seedStr);
    mvwprintw(gameWin, gameWinHeight-1, 1, "Moves:%d", moves);

    for (int x = 0; x < gridSize; x++)
    for (int y = 0; y < gridSize; y++) {	
        if(x == highlightX && y == highlightY)
            wattron(gameWin, A_REVERSE); 
        if (grid[y][x] == y*gridSize + x + 1)
            wattron(gameWin, COLOR_PAIR(1));

        if (grid[y][x] < 0)
            mvwprintw(gameWin, 1+y*TILE_HEIGHT,1+x*TILE_WIDTH, "[   ]");
        else
            mvwprintw(gameWin, 1+y*TILE_HEIGHT,1+x*TILE_WIDTH, "[%3d]", grid[y][x]);

        if (grid[y][x] == y*gridSize + x + 1)
            wattroff(gameWin, COLOR_PAIR(1));
        if(x == highlightX && y == highlightY)
			wattroff(gameWin, A_REVERSE);
	}
	wrefresh(gameWin);
}

bool trySlide() {
    for (int i = 0; i < gridSize; i++) {
        int dx = 0, dy = 0;
        switch (i) {
            case 0:
                dx = 1;
                break;
            case 1:
                dx = -1;
                break;
            case 2:
                dy = 1;
                break;
            case 3:
                dy = -1;
                break;
        }
        int swapX = highlightX + dx, swapY = highlightY + dy;

        if (swapX < 0 || swapY < 0 || swapX >= gridSize || swapY >= gridSize)
            continue;
        if (grid[swapY][swapX] < 0) {
            grid[swapY][swapX] = grid[highlightY][highlightX];
            grid[highlightY][highlightX] = -1;
            return true;
        }
    }

    return false;
}

bool gameWon() {
    if (grid[gridSize-1][gridSize-1] >= 0)
        return false;

    for (int x = 0; x < gridSize; x++)
    for (int y = 0; y < gridSize; y++) {
        if (x == gridSize-1 && y == gridSize-1)
            continue;

        if (grid[y][x] != y*gridSize + x + 1)
            return false;
    }

    return true;
}

void shuffleGrid() {
    int x = 0, y = 0;
    for (x = 0; x < gridSize; x++)
    for (y = 0; y < gridSize; y++) {
        if (grid[y][x] < 0) goto endloop;
    }
endloop:

    for (int i = 0; i < 1000 * gridSize * gridSize; i++) {
        int dx = 0, dy = 0;
        switch (rand() % gridSize) {
            case 0:
                dx = 1;
                break;
            case 1:
                dx = -1;
                break;
            case 2:
                dy = 1;
                break;
            case 3:
                dy = -1;
                break;
        }
        int swapX = x + dx, swapY = y + dy;

        if (swapX < 0 || swapY < 0 || swapX >= gridSize || swapY >= gridSize) {
            i--; continue;
        }

        grid[y][x] = grid[swapY][swapX];
        grid[swapY][swapX] = -1;

        x += dx;
        y += dy;
        x %= gridSize;
        y %= gridSize;
    }
}
