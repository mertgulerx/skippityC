#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#undef max
#undef min
#else
#include <unistd.h>
#undef max
#undef min
#endif

int main();

void sleep_ms(int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

void print_with_delay(const char *str, int delay_ms)
{
    while (*str)
    {
        putchar(*str++);
        fflush(stdout);
        sleep_ms(delay_ms);
    }
    putchar('\n');
}

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

#define RED "\033[31m"
#define YELLOW "\033[33m"
#define ORANGE "\033[38;5;208m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define WHITE "\033[37m"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef struct
{
    int score;
    int pieces[5];
    int sets;
    int extraPieces;
    int undoUsed;
    int redoUsed;
} Player;

typedef struct
{
    int srcRow;
    int srcCol;
    int destRow;
    int destCol;
    char capturedPiece;
} Move;

typedef struct
{
    Move *moves;
    int top;
    int capacity;
} MoveStack;

void initMoveStack(MoveStack *stack, int capacity)
{
    stack->moves = (Move *)malloc(sizeof(Move) * capacity);
    stack->top = -1;
    stack->capacity = capacity;
}

void freeMoveStack(MoveStack *stack)
{
    free(stack->moves);
}

int isMoveStackEmpty(MoveStack *stack)
{
    return stack->top == -1;
}

int isMoveStackFull(MoveStack *stack)
{
    return stack->top == stack->capacity - 1;
}

void pushMove(MoveStack *stack, Move move)
{
    if (!isMoveStackFull(stack))
    {
        stack->moves[++stack->top] = move;
    }
}

Move popMove(MoveStack *stack)
{
    if (!isMoveStackEmpty(stack))
    {
        return stack->moves[stack->top--];
    }
    Move emptyMove = {-1, -1, -1, -1, ' '};
    return emptyMove;
}

Move peekMove(MoveStack *stack)
{
    if (!isMoveStackEmpty(stack))
    {
        return stack->moves[stack->top];
    }
    Move emptyMove = {-1, -1, -1, -1, ' '};
    return emptyMove;
}

void printColoredChar(char c)
{
    switch (c)
    {
    case 'A':
        printf(RED "%c " RESET, c);
        break;
    case 'B':
        printf(YELLOW "%c " RESET, c);
        break;
    case 'C':
        printf(ORANGE "%c " RESET, c);
        break;
    case 'D':
        printf(GREEN "%c " RESET, c);
        break;
    case 'E':
        printf(BLUE "%c " RESET, c);
        break;
    default:
        printf("%c ", c);
        break;
    }
}

char **createBoard(int size)
{

    char **board = (char **)malloc(size * sizeof(char *));
    for (int i = 0; i < size; i++)
    {
        board[i] = (char *)malloc(size * sizeof(char));

        for (int j = 0; j < size; j++)
        {
            board[i][j] = ' ';
        }
    }
    return board;
}

void printBoard(char **board, int size, Player *player1, Player *player2, int gameMode, int controlMode, int selectedRow, int selectedCol, int highlight)
{

    printf("    ");
    for (int j = 0; j < size; j++)
    {
        printf("%2d  ", j + 1);
    }
    printf("\n");

    for (int i = 0; i < size; i++)
    {

        printf("   ");
        for (int j = 0; j < size; j++)
        {
            printf("+---");
        }
        printf("+\n");

        printf("%2d ", i + 1);
        for (int j = 0; j < size; j++)
        {
            printf("| ");
            if (controlMode == 2)
            {
                if (highlight && i == selectedRow && j == selectedCol)
                {
                    printf(BOLD WHITE "%c " RESET, board[i][j]);
                }
                else
                {
                    printColoredChar(board[i][j]);
                }
            }
            else
            {
                printColoredChar(board[i][j]);
            }
        }
        printf("|\n");
    }

    printf("   ");
    for (int j = 0; j < size; j++)
    {
        printf("+---");
    }
    printf("+\n");

    if (gameMode == 3)
    {
        printf("\nYapayzeka1                Yapayzeka2\n");
    }
    else
    {
        printf("\nOyuncu 1                  ");
        if (gameMode == 1)
        {
            printf("Oyuncu 2\n");
        }
        else
        {
            printf("Bilgisayar\n");
        }
    }

    printf("-------------------       -------------------\n");

    printf("%sA:%s%d %sB:%s%d %sC:%s%d %sD:%s%d %sE:%s%d       %sA:%s%d %sB:%s%d %sC:%s%d %sD:%s%d %sE:%s%d\n"
           "Puan: %-10d          Puan: %-10d\n"
           "Set: %-10d           Set: %-10d\n"
           "Ekstra Skipper: %-10dEkstra Skipper: %-10d\n\n",
           RED, RESET, player1->pieces[0], YELLOW, RESET, player1->pieces[1], ORANGE, RESET, player1->pieces[2], GREEN, RESET, player1->pieces[3], BLUE, RESET, player1->pieces[4],
           RED, RESET, player2->pieces[0], YELLOW, RESET, player2->pieces[1], ORANGE, RESET, player2->pieces[2], GREEN, RESET, player2->pieces[3], BLUE, RESET, player2->pieces[4],
           player1->score, player2->score,
           player1->sets, player2->sets,
           player1->extraPieces, player2->extraPieces);
}

void fillBoard(char **board, int size)
{
    char pieces[] = {'A', 'B', 'C', 'D', 'E'};
    int numPieces = sizeof(pieces) / sizeof(pieces[0]);
    int totalCells = size * size;
    int totalEmptyCells = 4;
    int totalFilledCells = totalCells - totalEmptyCells;
    int piecesPerType = totalFilledCells / numPieces;
    int remainingPieces = totalFilledCells % numPieces;

    int *pieceCounts = (int *)malloc(numPieces * sizeof(int));
    for (int i = 0; i < numPieces; i++)
    {
        pieceCounts[i] = piecesPerType;
    }

    for (int i = 0; remainingPieces > 0; i = (i + 1) % numPieces)
    {
        pieceCounts[i]++;
        remainingPieces--;
    }

    char *allPieces = (char *)malloc(totalFilledCells * sizeof(char));
    int index = 0;
    for (int i = 0; i < numPieces; i++)
    {
        for (int j = 0; j < pieceCounts[i]; j++)
        {
            allPieces[index++] = pieces[i];
        }
    }

    srand(time(NULL));
    for (int i = 0; i < totalFilledCells; i++)
    {
        int j = rand() % totalFilledCells;
        char temp = allPieces[i];
        allPieces[i] = allPieces[j];
        allPieces[j] = temp;
    }

    index = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0;
             j < size; j++)
        {

            if (!((i == size / 2 || i == size / 2 - 1) && (j == size / 2 || j == size / 2 - 1)))
            {
                board[i][j] = allPieces[index++];
            }
        }
    }

    free(allPieces);
    free(pieceCounts);
}

void freeBoard(char **board, int size)
{
    for (int i = 0; i < size; i++)
    {
        free(board[i]);
    }
    free(board);
}

void calculateScore(Player *player)
{
    int minPieces = player->pieces[0];

    for (int i = 1; i < 5; i++)
    {
        if (player->pieces[i] < minPieces)
        {
            minPieces = player->pieces[i];
        }
    }

    player->sets = minPieces;

    player->extraPieces = 0;
    for (int i = 0; i < 5; i++)
    {
        player->extraPieces += player->pieces[i] - minPieces;
    }

    player->score = player->sets * 100 + player->extraPieces;
}

void undoMove(char **board, MoveStack *undoStack, MoveStack *redoStack, Player *currentPlayer, Player *opponentPlayer)
{
    if (!isMoveStackEmpty(undoStack))
    {
        Move move = popMove(undoStack);
        pushMove(redoStack, move);

        board[move.srcRow][move.srcCol] = board[move.destRow][move.destCol];
        board[move.destRow][move.destCol] = ' ';

        if (move.capturedPiece != ' ')
        {
            int midRow = (move.srcRow + move.destRow) / 2;
            int midCol = (move.srcCol + move.destCol) / 2;
            board[midRow][midCol] = move.capturedPiece;

            currentPlayer->pieces[move.capturedPiece - 'A']--;
        }

        calculateScore(currentPlayer);
        calculateScore(opponentPlayer);
    }
}

void redoMove(char **board, MoveStack *undoStack, MoveStack *redoStack, Player *currentPlayer, Player *opponentPlayer)
{
    if (!isMoveStackEmpty(redoStack))
    {
        Move move = popMove(redoStack);
        pushMove(undoStack, move);

        board[move.destRow][move.destCol] = board[move.srcRow][move.srcCol];
        board[move.srcRow][move.srcCol] = ' ';

        if (move.capturedPiece != ' ')
        {
            int midRow = (move.srcRow + move.destRow) / 2;
            int midCol = (move.srcCol + move.destCol) / 2;
            board[midRow][midCol] = ' ';

            currentPlayer->pieces[move.capturedPiece - 'A']++;
        }

        calculateScore(currentPlayer);
        calculateScore(opponentPlayer);
    }
}

int analysePieces(char **board, int size, Player *currentPlayer, Player *opponentPlayer)
{
    int score = currentPlayer->score - opponentPlayer->score;

    int setPotential = 0;
    int minPieces = currentPlayer->pieces[0];
    for (int i = 0; i < 5; i++)
    {
        setPotential += currentPlayer->pieces[i];
        if (currentPlayer->pieces[i] < minPieces)
        {
            minPieces = currentPlayer->pieces[i];
        }
    }
    score += setPotential * 200;
    score += minPieces * 400;

    int vulnerablePieces = 0;
    for (int srcRow = 1; srcRow <= size; srcRow++)
    {
        for (int srcCol = 1; srcCol <= size; srcCol++)
        {
            if (board[srcRow - 1][srcCol - 1] != ' ')
            {
                for (int direction = -2; direction <= 2; direction += 4)
                {
                    int destRow = srcRow + direction;
                    int destCol = srcCol;
                    if (destRow >= 1 && destRow <= size && destCol >= 1 && destCol <= size)
                    {
                        if (board[destRow - 1][destCol - 1] == ' ' && board[(srcRow + destRow) / 2 - 1][destCol - 1] != ' ')
                        {
                            vulnerablePieces++;
                        }
                    }
                    destRow = srcRow;
                    destCol = srcCol + direction;
                    if (destRow >= 1 && destRow <= size && destCol >= 1 && destCol <= size)
                    {
                        if (board[destRow - 1][destCol - 1] == ' ' && board[destRow - 1][(srcCol + destCol) / 2 - 1] != ' ')
                        {
                            vulnerablePieces++;
                        }
                    }
                }
            }
        }
    }
    score -= vulnerablePieces * 10;

    int opponentSetPotential = 0;
    int opponentMinPieces = opponentPlayer->pieces[0];
    for (int i = 0; i < 5; i++)
    {
        opponentSetPotential += opponentPlayer->pieces[i];
        if (opponentPlayer->pieces[i] < opponentMinPieces)
        {
            opponentMinPieces = opponentPlayer->pieces[i];
        }
    }
    score -= opponentSetPotential * 200;
    score -= opponentMinPieces * 400;

    score += currentPlayer->extraPieces * 10;

    int capturedPieces = 0;
    for (int i = 0; i < 5; i++)
    {
        capturedPieces += currentPlayer->pieces[i] - opponentPlayer->pieces[i];
    }
    score += capturedPieces * 100;

    int totalPieces = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (board[i][j] != ' ')
            {
                totalPieces++;
            }
        }
    }
    int remainingPieces = totalPieces;
    score += (totalPieces - remainingPieces) * 20;

    return score;
}

int analysePiecesMedium(char **board, int size, Player *currentPlayer, Player *opponentPlayer)
{
    int score = currentPlayer->score - opponentPlayer->score;

    int setPotential = 0;
    for (int i = 0; i < 5; i++)
    {
        setPotential += currentPlayer->pieces[i];
    }
    score += setPotential * 100;

    int vulnerablePieces = 0;
    for (int srcRow = 1; srcRow <= size; srcRow++)
    {
        for (int srcCol = 1; srcCol <= size; srcCol++)
        {
            if (board[srcRow - 1][srcCol - 1] != ' ')
            {
                for (int direction = -2; direction <= 2; direction += 4)
                {
                    int destRow = srcRow + direction;
                    int destCol = srcCol;
                    if (destRow >= 1 && destRow <= size && destCol >= 1 && destCol <= size)
                    {
                        if (board[destRow - 1][destCol - 1] == ' ' && board[(srcRow + destRow) / 2 - 1][destCol - 1] != ' ')
                        {
                            vulnerablePieces++;
                        }
                    }
                    destRow = srcRow;
                    destCol = srcCol + direction;
                    if (destRow >= 1 && destRow <= size && destCol >= 1 && destCol <= size)
                    {
                        if (board[destRow - 1][destCol - 1] == ' ' && board[destRow - 1][(srcCol + destCol) / 2 - 1] != ' ')
                        {
                            vulnerablePieces++;
                        }
                    }
                }
            }
        }
    }
    score -= vulnerablePieces * 1;

    int opponentSetPotential = 0;
    for (int i = 0; i < 5; i++)
    {
        opponentSetPotential += opponentPlayer->pieces[i];
    }
    score -= opponentSetPotential * 100;

    score += currentPlayer->extraPieces * 1;

    return score;
}

int analysePiecesAi(char **board, int size, Player *currentPlayer, Player *opponentPlayer)
{
    int score = currentPlayer->score - opponentPlayer->score;

    int setPotential = 0;
    int minPieces = currentPlayer->pieces[0];
    for (int i = 0; i < 5; i++)
    {
        setPotential += currentPlayer->pieces[i];
        if (currentPlayer->pieces[i] < minPieces)
        {
            minPieces = currentPlayer->pieces[i];
        }
    }
    score += setPotential * 200;
    score += minPieces * 400;

    int vulnerablePieces = 0;
    for (int srcRow = 1; srcRow <= size; srcRow++)
    {
        for (int srcCol = 1; srcCol <= size; srcCol++)
        {
            if (board[srcRow - 1][srcCol - 1] != ' ')
            {
                for (int direction = -2; direction <= 2; direction += 4)
                {
                    int destRow = srcRow + direction;
                    int destCol = srcCol;
                    if (destRow >= 1 && destRow <= size && destCol >= 1 && destCol <= size)
                    {
                        if (board[destRow - 1][destCol - 1] == ' ' && board[(srcRow + destRow) / 2 - 1][destCol - 1] != ' ')
                        {
                            vulnerablePieces++;
                        }
                    }
                    destRow = srcRow;
                    destCol = srcCol + direction;
                    if (destRow >= 1 && destRow <= size && destCol >= 1 && destCol <= size)
                    {
                        if (board[destRow - 1][destCol - 1] == ' ' && board[destRow - 1][(srcCol + destCol) / 2 - 1] != ' ')
                        {
                            vulnerablePieces++;
                        }
                    }
                }
            }
        }
    }
    score -= vulnerablePieces * 10;

    int opponentSetPotential = 0;
    int opponentMinPieces = opponentPlayer->pieces[0];
    for (int i = 0; i < 5; i++)
    {
        opponentSetPotential += opponentPlayer->pieces[i];
        if (opponentPlayer->pieces[i] < opponentMinPieces)
        {
            opponentMinPieces = opponentPlayer->pieces[i];
        }
    }
    score -= opponentSetPotential * 200;
    score -= opponentMinPieces * 400;

    score += currentPlayer->extraPieces * 10;

    int capturedPieces = 0;
    for (int i = 0; i < 5; i++)
    {
        capturedPieces += currentPlayer->pieces[i] - opponentPlayer->pieces[i];
    }
    score += capturedPieces * 100;

    int totalPieces = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (board[i][j] != ' ')
            {
                totalPieces++;
            }
        }
    }
    int remainingPieces = totalPieces;
    score += (totalPieces - remainingPieces) * 20;

    return score;
}

int isValidMove(char **board, int size, int srcRow, int srcCol, int destRow, int destCol, int currentPlayer)
{
    if (srcRow < 1 || srcRow > size || srcCol < 1 || srcCol > size ||
        destRow < 1 || destRow > size || destCol < 1 || destCol > size)
    {
        return 0;
    }

    if (board[srcRow - 1][srcCol - 1] == ' ')
    {
        return 0;
    }

    if (board[destRow - 1][destCol - 1] != ' ')
    {
        return 0;
    }

    int deltaRow = abs(destRow - srcRow);
    int deltaCol = abs(destCol - srcCol);

    if ((deltaRow == 2 && deltaCol == 0) || (deltaRow == 0 && deltaCol == 2))
    {
        int midRow = (srcRow + destRow) / 2;
        int midCol = (srcCol + destCol) / 2;

        if (board[midRow - 1][midCol - 1] != ' ')
        {
            return 1;
        }
    }

    return 0;
}

int isGameOver(char **board, int size, Player *currentPlayer, Player *opponentPlayer)
{
    for (int srcRow = 1; srcRow <= size; srcRow++)
    {
        for (int srcCol = 1; srcCol <= size; srcCol++)
        {
            if (board[srcRow - 1][srcCol - 1] != ' ' && (board[srcRow - 1][srcCol - 1] == 'A' || board[srcRow - 1][srcCol - 1] == 'B' ||
                                                         board[srcRow - 1][srcCol - 1] == 'C' || board[srcRow - 1][srcCol - 1] == 'D' || board[srcRow - 1][srcCol - 1] == 'E'))
            {
                for (int destRow = 1; destRow <= size; destRow++)
                {
                    for (int destCol = 1; destCol <= size; destCol++)
                    {
                        if (isValidMove(board, size, srcRow, srcCol, destRow, destCol, 1))
                        {
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return 1;
}

int checkGameOver(char **board, int size, Player *player1, Player *player2)
{
    return isGameOver(board, size, player1, player2) && isGameOver(board, size, player2, player1);
}

void chooseAnalysePreset(int gameMode, int difficulty, int (**analysePiecesPtr)(char **, int, Player *, Player *))
{
    if (gameMode == 3)
    {

        *analysePiecesPtr = analysePiecesAi;
    }
    else if (difficulty == 2)
    {

        *analysePiecesPtr = analysePieces;
    }
    else
    {

        *analysePiecesPtr = analysePiecesMedium;
    }
}

int alphaBeta(char **board, int size, int depth, int alpha, int beta, int maximizingPlayer, Player *currentPlayer, Player *opponentPlayer, int difficulty, int gameMode)
{

    static int totalEvaluations = 0;
    totalEvaluations++;
    int (*analysePiecesPtr)(char **, int, Player *, Player *);
    bool exitPruningLoop;

    chooseAnalysePreset(gameMode, difficulty, &analysePiecesPtr);

    if (depth == 0 || (isGameOver(board, size, currentPlayer, opponentPlayer) && isGameOver(board, size, opponentPlayer, currentPlayer)))
    {
        return analysePiecesPtr(board, size, currentPlayer, opponentPlayer);
    }

    if (maximizingPlayer)
    {
        int maxEval = INT_MIN;
        bool exitPruningLoop = false;

        for (int srcRow = 1; srcRow <= size && !exitPruningLoop; srcRow++)
        {
            for (int srcCol = 1; srcCol <= size && !exitPruningLoop; srcCol++)
            {
                if (board[srcRow - 1][srcCol - 1] != ' ')
                {
                    for (int destRow = 1; destRow <= size && !exitPruningLoop; destRow++)
                    {
                        for (int destCol = 1; destCol <= size && !exitPruningLoop; destCol++)
                        {
                            if (isValidMove(board, size, srcRow, srcCol, destRow, destCol, 2))
                            {

                                int midRow = (srcRow + destRow) / 2;
                                int midCol = (srcCol + destCol) / 2;
                                char capturedPiece = board[midRow - 1][midCol - 1];
                                board[destRow - 1][destCol - 1] = board[srcRow - 1][srcCol - 1];
                                board[srcRow - 1][srcCol - 1] = ' ';
                                board[midRow - 1][midCol - 1] = ' ';
                                currentPlayer->pieces[capturedPiece - 'A']++;
                                calculateScore(currentPlayer);

                                int eval = alphaBeta(board, size, depth - 1, alpha, beta, 0, currentPlayer, opponentPlayer, difficulty, gameMode);

                                board[srcRow - 1][srcCol - 1] = board[destRow - 1][destCol - 1];
                                board[destRow - 1][destCol - 1] = ' ';
                                board[midRow - 1][midCol - 1] = capturedPiece;
                                currentPlayer->pieces[capturedPiece - 'A']--;
                                calculateScore(currentPlayer);

                                maxEval = max(maxEval, eval);
                                alpha = max(alpha, eval);
                                if (beta <= alpha)
                                {
                                    exitPruningLoop = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return maxEval;
    }
    else
    {
        int minEval = INT_MAX;
        bool exitPruningLoop = false;

        for (int srcRow = 1; srcRow <= size && !exitPruningLoop; srcRow++)
        {
            for (int srcCol = 1; srcCol <= size && !exitPruningLoop; srcCol++)
            {
                if (board[srcRow - 1][srcCol - 1] != ' ')
                {
                    for (int destRow = 1; destRow <= size && !exitPruningLoop; destRow++)
                    {
                        for (int destCol = 1; destCol <= size && !exitPruningLoop; destCol++)
                        {
                            if (isValidMove(board, size, srcRow, srcCol, destRow, destCol, 1))
                            {

                                int midRow = (srcRow + destRow) / 2;
                                int midCol = (srcCol + destCol) / 2;
                                char capturedPiece = board[midRow - 1][midCol - 1];
                                board[destRow - 1][destCol - 1] = board[srcRow - 1][srcCol - 1];
                                board[srcRow - 1][srcCol - 1] = ' ';
                                board[midRow - 1][midCol - 1] = ' ';
                                currentPlayer->pieces[capturedPiece - 'A']++;
                                calculateScore(currentPlayer);

                                int eval = alphaBeta(board, size, depth - 1, alpha, beta, 1, currentPlayer, opponentPlayer, difficulty, gameMode);

                                board[srcRow - 1][srcCol - 1] = board[destRow - 1][destCol - 1];
                                board[destRow - 1][destCol - 1] = ' ';
                                board[midRow - 1][midCol - 1] = capturedPiece;
                                currentPlayer->pieces[capturedPiece - 'A']--;
                                calculateScore(currentPlayer);

                                minEval = min(minEval, eval);
                                beta = min(beta, eval);
                                if (beta <= alpha)
                                {
                                    exitPruningLoop = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        return minEval;
    }
}

void makeComputerMove(char **board, int size, int *currentPlayer, Player *player1, Player *player2, MoveStack *undoStack, MoveStack *redoStack, int difficulty, int gameMode, int writeToFileMode, int *totalEvaluations, int controlMode, int selectedRow, int selectedCol, int highlight)
{
    Player *currentPlayerPtr = player2;
    Player *opponentPlayerPtr = player1;
    int bestSrcRow, bestSrcCol, bestDestRow, bestDestCol;
    int maxEval;
    int localEvaluations = 0;
    int depth;
    bool validMove;
    bool validExtraMove;

    if (difficulty == 2)
    {
        depth = 4;
    }
    else if (difficulty == 1)
    {
        depth = 2;
    }
    else if (gameMode == 3 && writeToFileMode == 0)
    {
        depth = 3;
    }
    else
    {
        depth = 1;
    }

    validMove = true;
    while (validMove)
    {
        bestSrcRow = bestSrcCol = bestDestRow = bestDestCol = -1;
        maxEval = INT_MIN;

        for (int srcRow = 1; srcRow <= size; srcRow++)
        {
            for (int srcCol = 1; srcCol <= size; srcCol++)
            {
                if (board[srcRow - 1][srcCol - 1] != ' ')
                {
                    for (int destRow = 1; destRow <= size; destRow++)
                    {
                        for (int destCol = 1; destCol <= size; destCol++)
                        {
                            if (isValidMove(board, size, srcRow, srcCol, destRow, destCol, 2))
                            {

                                int midRow = (srcRow + destRow) / 2;
                                int midCol = (srcCol + destCol) / 2;
                                char capturedPiece = board[midRow - 1][midCol - 1];
                                board[destRow - 1][destCol - 1] = board[srcRow - 1][srcCol - 1];
                                board[srcRow - 1][srcCol - 1] = ' ';
                                board[midRow - 1][midCol - 1] = ' ';
                                currentPlayerPtr->pieces[capturedPiece - 'A']++;
                                currentPlayerPtr->extraPieces++;
                                calculateScore(currentPlayerPtr);

                                int eval = alphaBeta(board, size, depth, INT_MIN, INT_MAX, 0, currentPlayerPtr, opponentPlayerPtr, difficulty, gameMode);
                                localEvaluations++;

                                board[srcRow - 1][srcCol - 1] = board[destRow - 1][destCol - 1];
                                board[destRow - 1][destCol - 1] = ' ';
                                board[midRow - 1][midCol - 1] = capturedPiece;
                                currentPlayerPtr->pieces[capturedPiece - 'A']--;
                                currentPlayerPtr->extraPieces--;
                                calculateScore(currentPlayerPtr);

                                if (eval > maxEval)
                                {
                                    bestSrcRow = srcRow;
                                    bestSrcCol = srcCol;
                                    bestDestRow = destRow;
                                    bestDestCol = destCol;
                                    maxEval = eval;
                                }
                            }
                        }
                    }
                }
            }
        }

        validExtraMove = true;
        while (validExtraMove)
        {
            if (bestSrcRow != -1 && bestSrcCol != -1 && bestDestRow != -1 && bestDestCol != -1)
            {
                int midRow = (bestSrcRow + bestDestRow) / 2;
                int midCol = (bestSrcCol + bestDestCol) / 2;
                char capturedPiece = board[midRow - 1][midCol - 1];
                board[bestDestRow - 1][bestDestCol - 1] = board[bestSrcRow - 1][bestSrcCol - 1];
                board[bestSrcRow - 1][bestSrcCol - 1] = ' ';
                board[midRow - 1][midCol - 1] = ' ';
                currentPlayerPtr->pieces[capturedPiece - 'A']++;
                currentPlayerPtr->extraPieces++;
                calculateScore(currentPlayerPtr);

                Move move = {bestSrcRow - 1, bestSrcCol - 1, bestDestRow - 1, bestDestCol - 1, capturedPiece};
                pushMove(undoStack, move);

                redoStack->top = -1;

                int extraSrcRow = bestDestRow;
                int extraSrcCol = bestDestCol;
                int extraDestRow = extraSrcRow + (bestDestRow - bestSrcRow);
                int extraDestCol = extraSrcCol + (bestDestCol - bestSrcCol);
                if (isValidMove(board, size, extraSrcRow, extraSrcCol, extraDestRow, extraDestCol, 2))
                {
                    bestSrcRow = extraSrcRow;
                    bestSrcCol = extraSrcCol;
                    validExtraMove = true;
                }
                else
                {
                    validExtraMove = false;
                }
            }
        }

        validMove = false;
    }

    *totalEvaluations += localEvaluations;

    if (!writeToFileMode)
    {
        clearScreen();
        printf("\nBilgisayar hamle yapti.\n\nGuncel oyun tahtasi:\n\n");
        printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
        printf("\n");
    }

    checkGameOver(board, size, player1, player2);
}

void saveGame(char **board, int size, Player player1, Player player2, int currentPlayer)
{
    char input[100];
    char filename[105];
    bool validInput = false;

    while (!validInput)
    {
        printf("\nOyunu kaydetmek istediginiz dosyanin adini girin (Unutmamaniz icin 1,2,3 gibi rakamlar girmeniz tavsiye edilir): ");
        if (fgets(input, sizeof(input), stdin) != NULL)
        {

            input[strcspn(input, "\n")] = 0;
            if (strlen(input) > 0)
            {
                snprintf(filename, sizeof(filename), "%s.txt", input);
                validInput = true;
            }
            else
            {
                printf("Dosya ismi bos olamaz! Tekrar deneyin.\n");
            }
        }
    }

    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Dosya acma hatasi!\n");
        return;
    }

    fprintf(file, "%d\n", size);
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            fprintf(file, "%c ", board[i][j] == ' ' ? '*' : board[i][j]);
        }
        fprintf(file, "\n");
    }

    fprintf(file, "%d %d %d %d %d %d %d %d\n", player1.score, player1.pieces[0], player1.pieces[1], player1.pieces[2], player1.pieces[3], player1.pieces[4], player1.sets, player1.extraPieces);

    fprintf(file, "%d %d %d %d %d %d %d %d\n", player2.score, player2.pieces[0], player2.pieces[1], player2.pieces[2], player2.pieces[3], player2.pieces[4], player2.sets, player2.extraPieces);

    fprintf(file, "%d\n", currentPlayer);

    fclose(file);
    printf("\nOyun basariyla %s dosyasina kaydedildi.\n\n", filename);
}

void loadGame(char ***board, int *size, Player *player1, Player *player2, int *currentPlayer)
{
    char input[100];
    char filename[105];
    FILE *file = NULL;
    bool validFile = false;

    clearScreen();

    while (!validFile)
    {
        printf("\nYuklemek istediginiz kayit dosyasinin adini girin (Dosyanin sonuna .txt eklemeyeniz.), yukleme islemini iptal etmek icin sadece enter tusuna basin: ");
        if (fgets(input, sizeof(input), stdin) != NULL)
        {

            input[strcspn(input, "\n")] = 0;
            if (strlen(input) == 0)
            {
                clearScreen();
                printf("\nYukleme islemi iptal edildi.\n\n");
                return;
            }
            snprintf(filename, sizeof(filename), "%s.txt", input);

            file = fopen(filename, "r");
            if (file != NULL)
            {
                validFile = true;
            }
            else
            {
                printf("\nDosya acilamadi veya bulunamadi. Lutfen tekrar deneyin.\n");
            }
        }
    }

    fscanf(file, "%d", size);

    if (*board != NULL)
    {
        for (int i = 0; i < *size; i++)
        {
            free((*board)[i]);
        }
        free(*board);
    }

    *board = createBoard(*size);

    for (int i = 0; i < *size; i++)
    {
        for (int j = 0; j < *size; j++)
        {
            char temp;
            fscanf(file, " %c", &temp);
            (*board)[i][j] = (temp == '*') ? ' ' : temp;
        }
    }

    fscanf(file, "%d %d %d %d %d %d %d %d", &player1->score, &player1->pieces[0], &player1->pieces[1], &player1->pieces[2], &player1->pieces[3], &player1->pieces[4], &player1->sets, &player1->extraPieces);

    fscanf(file, "%d %d %d %d %d %d %d %d", &player2->score, &player2->pieces[0], &player2->pieces[1], &player2->pieces[2], &player2->pieces[3], &player2->pieces[4], &player2->sets, &player2->extraPieces);

    fscanf(file, "%d", currentPlayer);

    fclose(file);

    clearScreen();
    printf("\nYuklenen Oyun Tahtasi:\n\n");
    printBoard(*board, *size, player1, player2, 1, 0, -1, -1, 0);

    bool validInput = true;
    while (!validInput)
    {
        printf("Baska bir dosya secmek icin 'b', bu kayitla devam etmek icin ise herhangi bir tusa basin: ");
        if (fgets(input, sizeof(input), stdin) != NULL)
        {

            input[strcspn(input, "\n")] = 0;
            if (input[0] == 'b' || input[0] == 'B')
            {

                loadGame(board, size, player1, player2, currentPlayer);
                return;
            }
            else
            {
                validInput = false;
            }
        }
    }
    clearScreen();
    printf("Oyun basariyla yuklendi.\n\n");
}

char getChar()
{
#ifdef _WIN32
    return _getch();
#else
    system("stty raw");
    system("stty -echo");
    char ch = getchar();
    system("stty cooked");
    system("stty echo");
    return ch;
#endif
}

void selectSourceWithWASD(char **board, int size, int *srcRow, int *srcCol, int *destRow, int *destCol, Player *player1, Player *player2, int gameMode, int *currentPlayer, int *extraMove, MoveStack *undoStack, MoveStack *redoStack, int *gameLoaded, int controlMode)
{
    int row = 0, col = 0;
    if (*extraMove)
    {
        row = *destRow - 1;
        col = *destCol - 1;
    }
    else if (*destRow != 0 && *destCol != 0)
    {
        row = *destRow - 1;
        col = *destCol - 1;
    }

    char tempChar = ' ';
    char input[100];

    while (1)
    {
        printBoard(board, size, player1, player2, gameMode, controlMode, row, col, 1);
        printf("Oyuncu %d hamlesi\n", *currentPlayer);
        printf("Kaynak tasi secmek icin 'wasd' tuslarini kullanin ve Enter'a basin.\n");

        int newRow = row, newCol = col;

        int validInput = 0;
        while (!validInput)
        {
            char ch = getChar();
            switch (ch)
            {
            case 'w':
                if (row > 0)
                    newRow -= 1;
                validInput = 1;
                clearScreen();
                break;
            case 'a':
                if (col > 0)
                    newCol -= 1;
                validInput = 1;
                clearScreen();
                break;
            case 's':
                if (row < size - 1)
                    newRow += 1;
                validInput = 1;
                clearScreen();
                break;
            case 'd':
                if (col < size - 1)
                    newCol += 1;
                validInput = 1;
                clearScreen();
                break;
            case '\r':

                if (board[row][col] == '*')
                {
                    printf("\nBos hucre secilemez. Lutfen gecerli bir tas seciniz.\n\n");
                }
                else if ((row > 1 && board[row - 1][col] != ' ' && board[row - 2][col] == ' ') ||
                         (col > 1 && board[row][col - 1] != ' ' && board[row][col - 2] == ' ') ||
                         (row < size - 2 && board[row + 1][col] != ' ' && board[row + 2][col] == ' ') ||
                         (col < size - 2 && board[row][col + 1] != ' ' && board[row][col + 2] == ' '))
                {

                    *srcRow = row + 1;
                    *srcCol = col + 1;
                    clearScreen();
                    return;
                }
                else
                {
                    clearScreen();
                    printf("\nBu tasin atlayabilecegi hicbir yon bulunmuyor. Lutfen baska bir tas secin.\n\n");

                    row = 0;
                    col = 0;
                    validInput = 1;
                }
                break;
            case 'k':
                if (*extraMove)
                {
                    printf("\nHamlenizi kesinlestirmeden oyunu kaydedemezsiniz. Siradaki oyuncuya gecip tekrar deneyiniz.\n\n");
                }
                else
                {
                    clearScreen();
                    saveGame(board, size, *player1, *player2, *currentPlayer);
                    validInput = 1;
                }
                break;
            case 'l':
                clearScreen();
                loadGame(&board, &size, player1, player2, currentPlayer);
                validInput = 1;
                *gameLoaded = 1;
                break;
            case 'u':
                clearScreen();
                printf("\nBu modda undo tusu ile hamleyi geri almaniza gerek yoktur. Zaten hareket tuslari ile hamlenizi geriye alabiliyorsunuz!\n\n");
                validInput = 1;
                break;
            case 'r':
                clearScreen();
                printf("\nBu modda redo tusu ile eski hamleyi geri almaniza gerek yoktur. Zaten hareket tuslari ile hamlenizi ileriye alabiliyorsunuz!\n\n");
                validInput = 1;
                break;
            case 'q':
                clearScreen();
                printf("\nOyundan cikmadan once oyunu kaydetmek ister misiniz? (e/h): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    if (input[0] == 'e' || input[0] == 'E')
                    {
                        saveGame(board, size, *player1, *player2, *currentPlayer);
                    }
                }
                clearScreen();
                printf("\nAna menuye donus yapiliyor...");
                sleep_ms(1000);
                main();
                validInput = 1;
                break;
            default:
                printf("\nGecersiz komut! Lutfen tekrar deneyin.\n");
                break;
            }
        }

        if (board[newRow][newCol] == ' ')
        {
            if (board[row][col] == '*')
            {
                board[row][col] = tempChar;
            }
            tempChar = board[newRow][newCol];
            board[newRow][newCol] = '*';
        }
        else
        {
            if (board[row][col] == '*')
            {
                board[row][col] = ' ';
            }
        }

        row = newRow;
        col = newCol;
    }
}

void selectDestinationWithWASD(char **board, int size, int srcRow, int srcCol, int *destRow, int *destCol, Player *player1, Player *player2, int gameMode, int *currentPlayer, int *extraMove, int controlMode)
{
    int row = srcRow - 1, col = srcCol - 1;
    int tempChar = board[row][col];
    char input[100];

    char **tempBoard = createBoard(size);
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            tempBoard[i][j] = board[i][j];
        }
    }

    printBoard(board, size, player1, player2, gameMode, controlMode, row, col, 1);
    printf("Oyuncu %d hamlesi\n", *currentPlayer);
    printf("Hedef tasi secmek icin 'wasd' tuslarini kullanin ve Enter'a basarak onaylayin.\n\nEkstra hamle yapabiliyorsaniz devam edebilirsiniz. Yapamiyorsaniz Enter'a basarak diger oyuncuya gecin.\n");

    bool exitLoop = false;
    while (!exitLoop)
    {
        int newRow = row, newCol = col;
        bool validMove = false;
        int validInput = 0;

        while (!validInput)
        {
            char ch = getChar();
            switch (ch)
            {
            case 'w':
                if (row > 1 && tempBoard[row - 1][col] != ' ' && tempBoard[row - 2][col] == ' ')
                {
                    newRow -= 2;
                    clearScreen();
                    *extraMove = 1;
                }
                else
                {
                    printf("\nBu yonde atlanabilecek bir hucre bulunmuyor. Baska bir yon ile tekrar deneyin.\n\n");
                    sleep_ms(700);
                }
                clearScreen();
                validInput = 1;
                break;
            case 'a':
                if (col > 1 && tempBoard[row][col - 1] != ' ' && tempBoard[row][col - 2] == ' ')
                {
                    newCol -= 2;
                    clearScreen();
                    *extraMove = 1;
                }
                else
                {
                    printf("\nBu yonde atlanabilecek bir hucre bulunmuyor. Baska bir yon ile tekrar deneyin.\n\n");
                    sleep_ms(700);
                }
                clearScreen();
                validInput = 1;
                break;
            case 's':
                if (row < size - 2 && tempBoard[row + 1][col] != ' ' && tempBoard[row + 2][col] == ' ')
                {
                    newRow += 2;
                    clearScreen();
                    *extraMove = 1;
                }
                else
                {
                    printf("\nBu yonde atlanabilecek bir hucre bulunmuyor. Baska bir yon ile tekrar deneyin.\n\n");
                    sleep_ms(700);
                }
                clearScreen();
                validInput = 1;
                break;
            case 'd':
                if (col < size - 2 && tempBoard[row][col + 1] != ' ' && tempBoard[row][col + 2] == ' ')
                {
                    newCol += 2;
                    clearScreen();
                    *extraMove = 1;
                }
                else
                {
                    printf("\nBu yonde atlanabilecek bir hucre bulunmuyor. Baska bir yon ile tekrar deneyin.\n\n");
                    sleep_ms(700);
                }
                clearScreen();
                validInput = 1;
                break;
            case '\r':
                *destRow = row + 1;
                *destCol = col + 1;
                exitLoop = true;
                clearScreen();
                validInput = 1;
                break;
            case 'k':
                clearScreen();
                printf("\nHamlenizi kesinlestirmeden oyunu kaydedemezsiniz. Siradaki oyuncuya gecip tekrar deneyiniz.\n\n");
                validInput = 1;
                break;
            case 'u':
                clearScreen();
                printf("\nBu modda undo tusu ile hamleyi geri almaniza gerek yoktur. Zaten hareket tuslari ile hamlenizi geriye alabiliyorsunuz!\n\n");
                validInput = 1;
                break;
            case 'r':
                clearScreen();
                printf("\nBu modda redo tusu ile eski hamleyi geri almaniza gerek yoktur. Zaten hareket tuslari ile hamlenizi ileriye alabiliyorsunuz!\n\n");
                validInput = 1;
                break;
            case 'l':
                clearScreen();
                printf("\nHamlenizi kesinlestirmeden oyun yukleyemezsiniz. Siradaki oyuncuya gecip tekrar deneyiniz.\n\n");
                validInput = 1;
                break;
            case 'q':
                clearScreen();
                printf("\nOyundan cikmadan once oyunu kaydetmek ister misiniz? (e/h): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    if (input[0] == 'e' || input[0] == 'E')
                    {
                        saveGame(board, size, *player1, *player2, *currentPlayer);
                    }
                }
                clearScreen();
                printf("\nAna menuye donus yapiliyor...");
                sleep_ms(1000);
                main();
                validInput = 1;
                break;
            default:
                printf("\nGecersiz komut! Lutfen tekrar deneyin.\n");
                break;
            }
        }

        if (!validMove)
        {
            tempBoard[row][col] = ' ';
            tempBoard[newRow][newCol] = tempChar;
            row = newRow;
            col = newCol;

            printBoard(tempBoard, size, player1, player2, gameMode, controlMode, row, col, 1);
            printf("Oyuncu %d hamlesi\n", *currentPlayer);
            printf("Hedef tasi secmek icin 'wasd' tuslarini kullanin ve Enter'a basarak onaylayin.\n\nEkstra hamle yapabiliyorsaniz devam edebilirsiniz. Yapamiyorsaniz Enter'a basarak diger oyuncuya gecin.\n");
        }
    }
    clearScreen();
    freeBoard(tempBoard, size);
}

int makeMoveWASD(char **board, int size, int *currentPlayer, Player *player1, Player *player2, MoveStack *undoStack1, MoveStack *redoStack1, MoveStack *undoStack2, MoveStack *redoStack2, int gameMode, int *gameLoaded, int difficulty, int writeToFileMode, int *totalEvaluations1, int *totalEvaluations2, int controlMode, int selectedRow, int selectedCol, int highlight)
{
    int srcRow, srcCol, destRow, destCol;
    char piece;
    Player *currentPlayerPtr = (*currentPlayer == 1) ? player1 : player2;
    Player *opponentPlayer = (*currentPlayer == 1) ? player2 : player1;
    MoveStack *undoStack = (*currentPlayer == 1) ? undoStack1 : undoStack2;
    MoveStack *redoStack = (*currentPlayer == 1) ? redoStack1 : redoStack2;
    int extraMove = 0;

    if (gameMode == 1 || *currentPlayer == 1)
    {
        while (1)
        {
            if (!extraMove)
            {
                selectSourceWithWASD(board, size, &srcRow, &srcCol, &destRow, &destCol, player1, player2, gameMode, currentPlayer, &extraMove, undoStack, redoStack, gameLoaded, controlMode);
            }
            else
            {
                srcRow = destRow;
                srcCol = destCol;
                piece = board[srcRow - 1][srcCol - 1];
            }

            selectDestinationWithWASD(board, size, srcRow, srcCol, &destRow, &destCol, player1, player2, gameMode, currentPlayer, &extraMove, controlMode);

            if (gameLoaded)
            {
                *currentPlayer = (*currentPlayer == 1) ? 2 : 1;
            }

            piece = board[srcRow - 1][srcCol - 1];

            int deltaRow = abs(destRow - srcRow);
            int deltaCol = abs(destCol - srcCol);

            if ((deltaRow == 2 && deltaCol == 0) || (deltaRow == 0 && deltaCol == 2))
            {
                int midRow = (srcRow + destRow) / 2;
                int midCol = (srcCol + destCol) / 2;

                if (board[midRow - 1][midCol - 1] != ' ')
                {
                    board[destRow - 1][destCol - 1] = piece;
                    board[srcRow - 1][srcCol - 1] = ' ';
                    char capturedPiece = board[midRow - 1][midCol - 1];
                    board[midRow - 1][midCol - 1] = ' ';

                    if (*gameLoaded)
                    {
                        currentPlayerPtr = (*currentPlayer == 1) ? player2 : player1;
                    }

                    currentPlayerPtr->pieces[capturedPiece - 'A']++;
                    calculateScore(currentPlayerPtr);

                    Move move = {srcRow - 1, srcCol - 1, destRow - 1, destCol - 1, capturedPiece};
                    pushMove(undoStack, move);

                    redoStack->top = -1;

                    if (extraMove)
                    {
                        extraMove = 1;
                        *currentPlayer = (*currentPlayer == 1) ? 2 : 1;
                    }
                    else
                    {
                        extraMove = 0;
                        *currentPlayer = (*currentPlayer == 1) ? 2 : 1;
                    }

                    player1->undoUsed = 0;
                    player1->redoUsed = 0;
                    player2->undoUsed = 0;
                    player2->redoUsed = 0;

                    int gameOverStatus = checkGameOver(board, size, player1, player2);
                    if (gameOverStatus == 1 || gameOverStatus == 2)
                    {
                        return gameOverStatus;
                    }
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
    }
    else
    {
        makeComputerMove(board, size, currentPlayer, player1, player2, undoStack, redoStack, difficulty, gameMode, writeToFileMode, totalEvaluations2, controlMode, selectedRow, selectedCol, highlight);
        int gameOverStatus = checkGameOver(board, size, player1, player2);
        if (gameOverStatus == 1 || gameOverStatus == 2)
        {
            return gameOverStatus;
        }
        return 1;
    }

    return 1;
}

int makeMoveXY(char **board, int size, int *currentPlayer, Player *player1, Player *player2, MoveStack *undoStack1, MoveStack *redoStack1, MoveStack *undoStack2, MoveStack *redoStack2, int gameMode, int *gameLoaded, int difficulty, int writeToFileMode, int *totalEvaluations1, int *totalEvaluations2, int controlMode, int selectedRow, int selectedCol, int highlight)
{
    int srcRow, srcCol, destRow, destCol;
    char piece;
    char input[100];
    Player *currentPlayerPtr = (*currentPlayer == 1) ? player1 : player2;
    Player *opponentPlayer = (*currentPlayer == 1) ? player2 : player1;
    MoveStack *undoStack = (*currentPlayer == 1) ? undoStack1 : undoStack2;
    MoveStack *redoStack = (*currentPlayer == 1) ? redoStack1 : redoStack2;
    int extraMove = 0;
    bool valid;

    if (gameMode == 1 || *currentPlayer == 1)
    {
        while (1)
        {
            if (!extraMove)
            {
                printf("\nOyuncu %d hamlesi\nHangi tas hareket ettirilecek (satir / sutun): ", *currentPlayer);
                valid = false;
                while (!valid)
                {
                    fgets(input, sizeof(input), stdin);

                    input[strcspn(input, "\n")] = 0;

                    if (strcmp(input, "s") == 0)
                    {
                        saveGame(board, size, *player1, *player2, *currentPlayer);
                        printf("\nOyuncu %d hamlesi\nHangi tas hareket ettirilecek (satir / sutun): ", *currentPlayer);
                    }
                    else if (strcmp(input, "l") == 0)
                    {
                        loadGame(&board, &size, player1, player2, currentPlayer);
                        printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                        printf("\nOyuncu %d hamlesi\nHangi tas hareket ettirilecek (satir / sutun): ", *currentPlayer);
                        *gameLoaded = 1;
                    }
                    else if (strcmp(input, "q") == 0)
                    {
                        clearScreen();
                        printf("\nOyundan cikmadan once oyunu kaydetmek ister misiniz? (e/h): ");
                        if (fgets(input, sizeof(input), stdin) != NULL)
                        {
                            if (input[0] == 'e' || input[0] == 'E')
                            {
                                saveGame(board, size, *player1, *player2, *currentPlayer);
                            }
                        }
                        clearScreen();
                        printf("\nAna menuye donus yapiliyor...");
                        sleep_ms(1000);
                        main();
                    }
                    else if (sscanf(input, "%d %d", &srcRow, &srcCol) == 2 && srcRow >= 1 && srcCol >= 1 && srcRow <= size && srcCol <= size)
                    {
                        valid = true;
                    }
                    else
                    {
                        clearScreen();
                        printf("\nGecersiz kaynak kordinatlari! Koordinatlar, 1 ve %d arasinda, 'a b' formatinda olmalidir. Lutfen tekrar deneyin.\n\n", size);
                        printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                        printf("\nOyuncu %d hamlesi\nHangi tas hareket ettirilecek (satir / sutun): ", *currentPlayer);
                        valid = false;
                    }
                }

                piece = board[srcRow - 1][srcCol - 1];

                if (piece == ' ')
                {
                    printf("\nSectiginiz hucre bos! Bos hucre secemezsiniz! Tekrar deneyin.\n");
                    return 0;
                }
            }
            else
            {

                srcRow = destRow;
                srcCol = destCol;
                piece = board[srcRow - 1][srcCol - 1];
            }

            if (!extraMove)
            {
                printf("Tas nereye yerlestirilecek (satir / sutun): ");
            }
            else
            {
                if (currentPlayerPtr->undoUsed == 0)
                {
                    printf("Undo icin 'u', sonraki oyuncuya gecmek icin 'g', atlayisa devam etmek icin ise hedef koordinati girin: \n");
                }
                else if (currentPlayerPtr->redoUsed == 0)
                {
                    printf("Redo icin 'r', pas gecmek icin 'g', tekrar hamle yapabilmek icin ise 't' tusuna basin. \n");
                }
                else
                {
                    printf("Sonraki oyuncuya gecmek icin 'g', atlayisa devam etmek icin ise hedef koordinati girin: \n");
                }
            }

            fgets(input, sizeof(input), stdin);

            input[strcspn(input, "\n")] = 0;

            if (strcmp(input, "s") == 0)
            {
                if (extraMove)
                {
                    printf("\nHamlenizi kesinlestirmeden oyunu kaydedemezsiniz. Siradaki oyuncuya gecip tekrar deneyiniz.\n\n");
                }
                else
                {
                    saveGame(board, size, *player1, *player2, *currentPlayer);
                }
            }
            else if (strcmp(input, "l") == 0)
            {
                printf("\nHamlenizi kesinlestirmeden oyun yukleyemezsiniz. Siradaki oyuncuya gecip tekrar deneyiniz.\n\n");
            }
            else if (strcmp(input, "u") == 0)
            {
                if (currentPlayerPtr->undoUsed == 0)
                {
                    undoMove(board, undoStack, redoStack, currentPlayerPtr, opponentPlayer);
                    currentPlayerPtr->undoUsed = 1;
                    clearScreen();
                    printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                }
                else
                {
                    printf("\nUndo hakkiniz tukendi!\n\n");
                }
            }
            else if (strcmp(input, "t") == 0)
            {
                if (currentPlayerPtr->undoUsed == 1 && currentPlayerPtr->redoUsed == 0)
                {
                    return 0;
                }
            }
            else if (strcmp(input, "r") == 0)
            {
                if (currentPlayerPtr->undoUsed == 1)
                {
                    if (currentPlayerPtr->redoUsed == 0)
                    {
                        redoMove(board, undoStack, redoStack, currentPlayerPtr, opponentPlayer);
                        currentPlayerPtr->redoUsed = 1;
                        clearScreen();
                        printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                    }
                    else
                    {
                        printf("\nRedo hakkiniz bitti!\n\n");
                    }
                }
                else
                {
                    printf("\nRedo yapabilmek icin once undo yapmaniz gerekir!\n\n");
                }
            }
            else if (strcmp(input, "g") == 0 && extraMove)
            {

                int gameOverStatus = checkGameOver(board, size, player1, player2);
                if (gameOverStatus == 1 || gameOverStatus == 2)
                {
                    return gameOverStatus;
                }
                clearScreen();
                printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                return 1;
            }
            else if (strcmp(input, "q") == 0)
            {
                *currentPlayer = (*currentPlayer == 1) ? 2 : 1;
                clearScreen();
                printf("\nOyundan cikmadan once oyunu kaydetmek ister misiniz? (e/h): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    if (input[0] == 'e' || input[0] == 'E')
                    {
                        saveGame(board, size, *player1, *player2, *currentPlayer);
                        printf("Hamlenizi kesinlestirmediginiz icin siraniz otomatik olarak diger oyuncuya gecirilmistir.");
                        sleep_ms(4000);
                    }
                }
                clearScreen();
                printf("\nAna menuye donus yapiliyor...");
                sleep_ms(1000);
                main();
            }
            else if (sscanf(input, "%d %d", &destRow, &destCol) == 2)
            {

                if (destRow < 1 || destRow > size || destCol < 1 || destCol > size)
                {
                    clearScreen();
                    printf("\nGecersiz hedef kordinatlari! Koordinatlar, 1 ve %d arasinda, 'a b' formatinda olmalidir. Lutfen kaynak koordinatlarini da bastan girip tekrar deneyin.\n\n", size);
                    printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                    return 0;
                }

                if (board[destRow - 1][destCol - 1] != ' ')
                {
                    printf("\nHedef hucre dolu! Lutfen kaynak koordinatlarini da bastan girip tekrar deneyin.\n");
                    return 0;
                }

                int deltaRow = abs(destRow - srcRow);
                int deltaCol = abs(destCol - srcCol);

                if ((deltaRow == 2 && deltaCol == 0) || (deltaRow == 0 && deltaCol == 2))
                {
                    int midRow = (srcRow + destRow) / 2;
                    int midCol = (srcCol + destCol) / 2;

                    if (board[midRow - 1][midCol - 1] != ' ')
                    {
                        board[destRow - 1][destCol - 1] = piece;
                        board[srcRow - 1][srcCol - 1] = ' ';
                        char capturedPiece = board[midRow - 1][midCol - 1];
                        board[midRow - 1][midCol - 1] = ' ';

                        if (*gameLoaded)
                        {
                            currentPlayerPtr = (*currentPlayer == 1) ? player1 : player2;
                        }
                        currentPlayerPtr->pieces[capturedPiece - 'A']++;
                        calculateScore(currentPlayerPtr);

                        Move move = {srcRow - 1, srcCol - 1, destRow - 1, destCol - 1, capturedPiece};
                        pushMove(undoStack, move);

                        redoStack->top = -1;

                        extraMove = 1;

                        player1->undoUsed = 0;
                        player1->redoUsed = 0;
                        player2->undoUsed = 0;
                        player2->redoUsed = 0;

                        clearScreen();
                        printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                    }
                    else
                    {
                        printf("\nGecersiz hamle! Lutfen tekrar deneyin.\n");
                    }
                }
                else
                {
                    printf("\nGecersiz hamle! Lutfen tekrar deneyin.\n");
                }
            }
            else if (sscanf(input, "%d", &destRow) == 1)
            {
                clearScreen();
                printf("\nGecersiz giris! Cok fazla gecersiz giris yaptiniz! Bu yuzden cezalandirildiniz. Siradaki oyuncuya geciliyor...\n\n");
                printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                return 1;
            }
            else
            {
                clearScreen();
                printf("\nGecersiz giris! Lutfen random harfler girmeyin.\n\n");
                printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
            }
        }

        int gameOverStatus = checkGameOver(board, size, player1, player2);
        if (gameOverStatus == 1 || gameOverStatus == 2)
        {
            return gameOverStatus;
        }
    }
    else
    {
        makeComputerMove(board, size, currentPlayer, player1, player2, undoStack, redoStack, difficulty, gameMode, writeToFileMode, totalEvaluations2, controlMode, selectedRow, selectedCol, highlight);
        int gameOverStatus = checkGameOver(board, size, player1, player2);
        if (gameOverStatus == 1 || gameOverStatus == 2)
        {
            return gameOverStatus;
        }
        return 1;
    }
    *gameLoaded = 0;

    return 1;
}

void tutorial(char *input)
{
    const char *tutorial[] = {
        "",
        "Birlikte oyunun kurallarini ve calisma mantigini ogrenelim.",
        "",
        "Skippity oyunu kurallari cok basittir:",
        "",
        "",
        "OYUN TAHTASI",
        "",
        "6-20 boyutundaki bir kare oyun tahtasinda oynanir. A B C D E olmak uzere 5 adet tas (Taslar 'Skipper' olarak da adlandirilabilir.) bulunmaktadir.",
        "",
        "Oyun tahtasindaki her kareye 5 farkli tas arasindan rastgele bir tas konulur.",
        "",
        "Ortadaki 2x2'lik kisim ise oyun baslangicinda hamle yapabilmek icin bos birakilmistir.",
        "",
        "",
        "HAREKET MEKANIZMASI",
        "",
        "Siraniz geldiginde tahtadaki bir tasi diger bir tas uzerinden bos bir kareye atlatarak uzerinden atladiginiz tasi hanenize yazmis olursunuz.",
        "",
        "Atlamalar sadece dikey ve yatay olarak yapilabilir. Sadece bitisik bir tasin uzerinden atlayabilirsiniz.",
        "",
        "Hamlenizden memnun degilseniz 'u' tusu ile geri alabilir, eger fikrinizi degistirirseniz 'r' tusu ile tekrar onceki hamleyi gerceklestirebilirsiniz.",
        "",
        "\"undo\" ve \"redo\" haklariniz tur basina 1 adettir. Hamle yapmak haklarinizi sifirlamaz.",
        "",
        "Eger atladiginiz kareden yine ayni sekilde bir hamle yapabiliyorsaniz atlamaya devam edebilirsiniz. Yapamiyorsaniz sirayi diger oyuncuya devretmeniz gerekir.",
        "",
        "",
        "PUAN HESAPLANMASI",
        "",
        "Eger A B C D E taslarinin her birinden bir adet toplarsaniz bu bir \"set\" sayilir. Her toplanan set 100 puan degerindedir.",
        "",
        "Setlerin disinda kalan fazladan taslarinizin her biri tas cinsi farketmeksizin 1 puandir.",
        "",
        "",
        "KAZANANIN BELIRLENMESI",
        "",
        "Oyun sonunda yapilabilecek hamle kalmazsa oyun otomatik olarak biter.",
        "",
        "En fazla puana sahip olan oyuncu kazanir.",
        "",
        "",
        "OYNANIS MANTIGI VE TUSLAR",
        "",
        "Oyun baslangicinda oyun modu secimi yapmaniz gerekir. Iki kisilik oyun icin '1', bilgisayara karsi oynamak icin '2' tusuna basilir.",
        "",
        "Sonrasinda sizden oyun tahtasi boyutu girmeniz istenir. Oyun tahtasi 6-20 arasinda girilmelidir. Aksi taktirde oyun baslamaz.",
        "",
        "Hamle yapmadan once veya yaptiktan sonra oyunu kaydetmek icin 's' kaydettiginiz herhangi bir oyunu yuklemek icin 'l' tusuna basabilirsiniz.",
        "",
        "Hamle yaptiktan sonra \"undo\" yapmak icin 'u', \"redo\" yapmak icin 'r' tusuna basabilirsiniz.",
        "",
        "",
        "Ornek bir tahta su sekildedir:",
        "",
        "     1   2   3   4   5   6  ",
        "   +---+---+---+---+---+---+",
        " 1 | C | A | B | A | C | C |",
        "   +---+---+---+---+---+---+",
        " 2 | D | E | C | B | C | A |",
        "   +---+---+---+---+---+---+",
        " 3 | D | D |   |   | A | A |",
        "   +---+---+---+---+---+---+",
        " 4 | D | B |   |   | C | A |",
        "   +---+---+---+---+---+---+",
        " 5 | B | B | E | E | B | A |",
        "   +---+---+---+---+---+---+",
        " 6 | D | E | B | D | E | E |",
        "   +---+---+---+---+---+---+",
        "",
        "Oyuncu 1                 Oyuncu 2",
        "-------------------      -------------------",
        "A:0 B:0 C:0 D:0 E:0      A:0 B:0 C:0 D:0 E:0",
        "Puan: 0                  Puan: 0",
        "Set: 0                   Set: 0",
        "Ekstra Skipper: 0        Ekstra Skipper: 0",
        "",
        "",
        "Bu asamadan sonra hamle yapmak icin koordinat girmeniz gereklidir. Asagidaki gibi sorulacaktir:",
        "",
        "Oyuncu 1 hamlesi",
        "Hangi tas hareket ettirilecek (satir / sutun):",
        "Tas nereye yerlestirilecek (satir / sutun):",
        "",
        "Lutfen koordinat girisi yaparken 'x y', '1 3' gibi giris yapiniz. Baska turlu yapilan girisler gecersizdir.",
        "",
        "",
        "ATLAMA OGRETICISI",
        "",
        "Asagidaki ornek satir ve sutunlari inceleyiniz.",
        "",
        "     1   2   3   4   5   6  ",
        "   +---+---+---+---+---+---+",
        " 1 | A | B |   |   |   |   |",
        "   +---+---+---+---+---+---+",
        "",
        "1 1 -----> 1 3",
        "",
        "     1   2   3   4   5   6  ",
        "   +---+---+---+---+---+---+",
        " 1 |   |   | A |   |   |   |",
        "   +---+---+---+---+---+---+",
        "",
        "Hamle basarili!",
        "",
        "A:0 B:1 C:0 D:0 E:0",
        "Puan: 1",
        "Set: 0",
        "Ekstra Skipper: 1",
        "",
        "",
        "",
        "     1   2   3   4   5   6  ",
        "   +---+---+---+---+---+---+",
        " 1 | A |   |   |   |   |   |",
        "   +---+---+---+---+---+---+",
        " 2 | C |   |   |   |   |   |",
        "   +---+---+---+---+---+---+",
        " 3 |   |   |   |   |   |   |",
        "   +---+---+---+---+---+---+",
        "",
        "1 1 -----> 3 1",
        "",
        "     1   2   3   4   5   6  ",
        "   +---+---+---+---+---+---+",
        " 1 |   |   |   |   |   |   |",
        "   +---+---+---+---+---+---+",
        " 2 |   |   |   |   |   |   |",
        "   +---+---+---+---+---+---+",
        " 3 | A |   |   |   |   |   |",
        "   +---+---+---+---+---+---+",
        "",
        "Hamle basarili!",
        "",
        "A:0 B:0 C:1 D:0 E:0",
        "Puan: 1",
        "Set: 0",
        "Ekstra Skipper: 1",
        "",
        "",
        "Artik hazirsiniz! Yapay zekamin zekasini test etmek icin ekstrem zorlukta bir bilgisayara karsi oyun oynamayi unutmayin!",
        "",
        "",
        "Oyunu yazan:",
        "MERT GULER - YILDIZ TEKNIK UNIVERSITESI - BILGISAYAR MUHENDISLIGI BOLUMU - 2024",
        "",
        "",
        "IYI EGLENCELER :)",
        "",
        "⢠⣶⠀⠀⠈⢿⣧⠀⠀⠀⠀⣾⡶",
        "⠀⢿⣧⠀⠀⠘⣿⡆⠀⠀⢰⣿⠃",
        "⠀⠘⠟⠓⠀⠀⠀⠀⠀⢠⣿⠏⠀",
        "⠀⠀⠀⠀⠀⠀⠀⢀⣴⡿⠋⠀⠀",
        "⠀⠀⠀⠀⢀⣠⣴⣿⠟⠁⠀⠀⠀",
        "⠀⠀⠐⢾⠿⠟⠋⠀⠀⠀⠀⠀⠀",
        "",
        "",
        "Ana menuye donmek icin herhangi bir tusa basiniz",
        "",
    };

    const char *ascii_art[] = {
        "",
        "",
        "",
        "Merhaba! Skippity oyunuma hosgeldiniz!",
        "",
        "",
        "",
        "",
        "   ▄████████    ▄█   ▄█▄  ▄█     ▄███████▄    ▄███████▄  ▄█      ███     ▄██   ▄   ",
        "  ███    ███   ███ ▄███▀ ███    ███    ███   ███    ███ ███  ▀█████████▄ ███   ██▄ ",
        "  ███    █▀    ███▐██▀   ███▌   ███    ███   ███    ███ ███▌    ▀███▀▀██ ███▄▄▄███ ",
        "  ███         ▄█████▀    ███▌   ███    ███   ███    ███ ███▌     ███   ▀ ▀▀▀▀▀▀███ ",
        "▀███████████ ▀▀█████▄    ███▌ ▀█████████▀  ▀█████████▀  ███▌     ███     ▄██   ███ ",
        "         ███   ███▐██▄   ███    ███          ███        ███      ███     ███   ███ ",
        "   ▄█    ███   ███ ▀███▄ ███    ███          ███        ███      ███     ███   ███ ",
        " ▄████████▀    ███   ▀█▀ █▀    ▄████▀       ▄████▀      █▀      ▄████▀    ▀█████▀  ",
        "",
        "",
        "",
    };

    for (int i = 0; i < sizeof(ascii_art) / sizeof(ascii_art[0]); i++)
    {
        print_with_delay(ascii_art[i], 0);
    }

    for (int i = 0; i < sizeof(tutorial) / sizeof(tutorial[0]); i++)
    {
        print_with_delay(tutorial[i], 20);
    }

    while (1)
    {
        if (fgets(input, sizeof(input), stdin) != NULL)
        {
            clearScreen();
            return;
        }
    }
}

char mainMenuText[] =
    ""
    "\n***************************************\n"
    "     ** SkippityC by MERT GULER **\n"
    "***************************************\n"
    "                MENU                 \n\n"
    "   1- Iki kisilik oyun\n"
    "   2- Bilgisayara karsi oyun\n"
    "   3- Yapay zeka vs yapay zeka\n"
    "   4- Tanitim, kurallar ve ek notlar\n"
    "   5- Kontroller\n\n"
    "**************************************\n\n ";

char mainMenuControlsText[] =
    ""
    "\n***************************************\n"
    "     ** SkippityC by MERT GULER **\n"
    "***************************************\n"
    "              Kontroller\n\n"
    "  Hamleyi geri alma: 'u\n"
    "  Geri alinan hamleyi ileri alma: 'r'\n"
    "  Oyunu kaydetme: 's'\n"
    "  Yon tuslari modunda kaydetme: 'k'\n"
    "  Kayitli oyunu yukleme: 'l'\n"
    "  Siradaki oyuncuya gecme: 'g'\n"
    "  Koordinat ile hamle yapma: 'x y'\n"
    "  Yon tuslari ile ilerleme: 'w,a,s,d'\n"
    "  Ana menuye donme: 'q'\n\n"
    "**************************************\n\n"
    "Onceki menuye donmek icin herhangi bir tusa basin: ";

int main()
{
    clearScreen();
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    int size;
    int currentPlayer = 1;
    char command;
    int gameMode;
    int difficulty = 1;
    char input[100];
    int gameLoaded = 0;
    int gameOverStatus = 0;
    int writeToFileMode = 0;
    int totalEvaluations1 = 0;
    int totalEvaluations2 = 0;
    int controlMode = 0;
    int selectedRow = 0;
    int selectedCol = 0;
    int highlight = 0;
    bool valid;

    MoveStack undoStack1, redoStack1, undoStack2, redoStack2;

    while (1)
    {

        valid = false;
        while (!valid)
        {
            printf("%s", mainMenuText);
            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                if (sscanf(input, "%d", &gameMode) == 1 && (gameMode == 1 || gameMode == 2 || gameMode == 3 || gameMode == 4 || gameMode == 5))
                {
                    clearScreen();
                    valid = true;
                }
                else
                {
                    clearScreen();
                    printf("\nGecersiz oyun modu! Lutfen tekrar deneyin.\n\n");
                }
                if (gameMode == 5)
                {
                    clearScreen();
                    printf("%s", mainMenuControlsText);
                    if (fgets(input, sizeof(input), stdin) != NULL)
                    {
                        clearScreen();
                        main();
                    }
                }
            }
        }

        if (gameMode == 1 || gameMode == 2)
        {
            printf("\nYon tuslariyla kontrol ile devam etmek icin 'y', koordinatlar ile kontrole devam etmek icin ise herhangi bir tusuna basiniz: ");
            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                clearScreen();
                if (input[0] == 'y')
                {
                    controlMode = 2;
                }
                else
                {
                    controlMode = 1;
                }
                clearScreen();
                if (controlMode == 1)
                {
                    printf("\nKoordinatlarla kontrol ile devam ediliyor... Herhangi bir tusa basmadan bekleyiniz...\n\n");
                    sleep_ms(1000);
                    clearScreen();
                }
                else
                {   
                    printf("\nDIKKAT! BU KONTROL MODU TAMAMEN DENEYSELDIR. OYUNUN TUM MEKANIKLERI KOORDINAT ILE HAREKET MODUNA GORE TASARLANMISTIR. BU MODDA OYNAMANIZ TAVSIYE EDILMEZ!\n\n");
                    sleep_ms(2500);
                    clearScreen();
                    printf("\nYon tuslariyla kontrol ile devam ediliyor... Herhangi bir tusa basmadan bekleyiniz...\n\nBu kontrol modunda oyunu kaydetme tusu: 'k'");
                    sleep_ms(1000);
                    clearScreen();
                }
            }
        }
        if (gameMode == 4)
        {
            tutorial(input);
            main();
        }

        if (gameMode == 3)
        {

            printf("\n100 adet rastgele boyutta oyunun ciktisini \"aitest\" dosyasina yazdirmak icin 'a' tusuna basin.\n\nDIKKAT! Yazma islemi asiri uzun surebilir!\n\nBir adet oyunu terminalde simule etmek ise herhangi bir tusa basin:\n");
            if (fgets(input, sizeof(input), stdin) != NULL && input[0] == 'a')
            {
                printf("\nSimulasyonlar dosyaya yaziliyor...\n");
                writeToFileMode = 1;

                FILE *file = fopen("aitest.txt", "w");
                if (file == NULL)
                {
                    printf("Dosya acilamadi!\n");
                }

                for (int i = 0; i < 100; i++)
                {
                    size = (rand() % 10) + 6;
                    currentPlayer = 1;

                    char **board = createBoard(size);

                    fillBoard(board, size);

                    Player player1 = {0, {0, 0, 0, 0, 0}, 0, 0, 0, 0};
                    Player player2 = {0, {0, 0, 0, 0, 0}, 0, 0, 0, 0};

                    initMoveStack(&undoStack1, size * size);
                    initMoveStack(&redoStack1, size * size);
                    initMoveStack(&undoStack2, size * size);
                    initMoveStack(&redoStack2, size * size);

                    while (!checkGameOver(board, size, &player1, &player2))
                    {
                        if (currentPlayer == 1)
                        {

                            makeComputerMove(board, size, &currentPlayer, &player1, &player2, &undoStack1, &redoStack1, 3, gameMode, writeToFileMode, &totalEvaluations1, controlMode, selectedRow, selectedCol, highlight);
                        }
                        else
                        {

                            makeComputerMove(board, size, &currentPlayer, &player2, &player1, &undoStack2, &redoStack2, 3, gameMode, writeToFileMode, &totalEvaluations2, controlMode, selectedRow, selectedCol, highlight);
                        }
                        currentPlayer = (currentPlayer == 1) ? 2 : 1;
                    }

                    fprintf(file, "Oyun %d bitti!\n", i + 1);
                    fprintf(file, "Boyut: %d\n", size);
                    for (int r = 0; r < size; r++)
                    {
                        for (int c = 0; c < size; c++)
                        {
                            fprintf(file, "%c ", board[r][c]);
                        }
                        fprintf(file, "\n");
                    }
                    fprintf(file, "\n");
                    fprintf(file, "Yapayzeka1 Puan: %d\n", player1.score);
                    fprintf(file, "Yapayzeka2 Puan: %d\n", player2.score);

                    if (player1.score > player2.score)
                    {
                        fprintf(file, "\nYapayzeka1 kazandi!\n\n");
                    }
                    else if (player2.score > player1.score)
                    {
                        fprintf(file, "\nYapayzeka2 kazandi!\n\n");
                    }
                    else
                    {
                        fprintf(file, "\nOyun berabere!\n\n");
                    }
                    fprintf(file, "\n");

                    freeBoard(board, size);
                    freeMoveStack(&undoStack1);
                    freeMoveStack(&redoStack1);
                    freeMoveStack(&undoStack2);
                    freeMoveStack(&redoStack2);
                }

                fclose(file);
                printf("\n100 adet oyun ciktisi \"aitest.txt\" dosyasina yazildi.\n");
                writeToFileMode = 0;

                printf("\nProgrami sonlandirmak icin 'b', ana menuye donmek icin ise herhangi bir tusa basin: ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {

                    input[strcspn(input, "\n")] = 0;
                    if (input[0] == 'b' || input[0] == 'B')
                    {
                        return 0;
                    }
                    else
                    {
                        clearScreen();
                        main();
                    }
                }
            }

            valid = false;
            while (!valid)
            {
                printf("\nOyun tahtasi boyutunu girin (6-20): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {

                    input[strcspn(input, "\n")] = 0;
                    if (sscanf(input, "%d", &size) == 1 && size >= 6 && size <= 20)
                    {
                        valid = true;
                    }
                    else
                    {
                        printf("\nGecersiz boyut! Oyun tahtasi boyutu 6 ile 20 arasinda olmalidir.\n\n");
                    }
                }
            }
            clearScreen();
            printf("\nOyun tahtasi boyutunu girin (6-20): ");

            char **board = createBoard(size);

            fillBoard(board, size);

            Player player1 = {0, {0, 0, 0, 0, 0}, 0, 0, 0, 0};
            Player player2 = {0, {0, 0, 0, 0, 0}, 0, 0, 0, 0};

            initMoveStack(&undoStack1, size * size);
            initMoveStack(&redoStack1, size * size);
            initMoveStack(&undoStack2, size * size);
            initMoveStack(&redoStack2, size * size);

            while (!checkGameOver(board, size, &player1, &player2))
            {
                if (currentPlayer == 1)
                {

                    makeComputerMove(board, size, &currentPlayer, &player1, &player2, &undoStack1, &redoStack1, 3, gameMode, writeToFileMode, &totalEvaluations1, controlMode, selectedRow, selectedCol, highlight);
                }
                else
                {

                    makeComputerMove(board, size, &currentPlayer, &player2, &player1, &undoStack2, &redoStack2, 3, gameMode, writeToFileMode, &totalEvaluations2, controlMode, selectedRow, selectedCol, highlight);
                }
                currentPlayer = (currentPlayer == 1) ? 2 : 1;
            }

            printf("\nOyun bitti!\n\n");
            printBoard(board, size, &player1, &player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
            printf("Yapayzeka1 Puan: %d\n", player1.score);
            printf("Yapayzeka2 Puan: %d\n\n", player2.score);

            if (player1.score > player2.score)
            {
                printf("Yapayzeka1 kazandi!\n\n");
            }
            else if (player2.score > player1.score)
            {
                printf("Yapayzeka2 kazandi!\n\n");
            }
            else
            {
                printf("Oyun berabere!\n\n");
            }

            if (gameMode == 3)
            {
                printf("Yapayzeka 1 toplam %d hamleyi degerlendirdi.\n", totalEvaluations1);
                printf("Yapayzeka 2 toplam %d hamleyi degerlendirdi.\n\n", totalEvaluations2);
            }

            printf("\nProgrami sonlandirmak icin 'b', ana menuye donmek icin ise herhangi bir tusa basin: ");
            if (fgets(input, sizeof(input), stdin) != NULL)
            {

                input[strcspn(input, "\n")] = 0;
                if (input[0] == 'b' || input[0] == 'B')
                {

                    freeBoard(board, size);
                    freeMoveStack(&undoStack1);
                    freeMoveStack(&redoStack1);
                    freeMoveStack(&undoStack2);
                    freeMoveStack(&redoStack2);
                    return 0;
                }
                else
                {

                    freeBoard(board, size);
                    freeMoveStack(&undoStack1);
                    freeMoveStack(&redoStack1);
                    freeMoveStack(&undoStack2);
                    freeMoveStack(&redoStack2);
                    clearScreen();
                    main();
                }
            }
        }
        else
        {
            if (gameMode == 2)
            {

                printf("\nYapay zeka zorlugu seciniz: Orta zorluk icin 'o', Ekstrem zorluk icin ise 'e' tusuna basin: ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    if (input[0] == 'e')
                    {
                        clearScreen();
                        printf("\nEmin misiniz? Ekstrem zorluk binlerce test sonrasi optimize edilerek yenilmesi imkansiza yakin olacak sekilde ayarlanmistir. ");
                        printf("\nSiz mevcut hamlenizi dusunurken o yuzbinlerce hamle ilerisini gorebilir.\n\nEminseniz 'e', orta zorlukta devam etmek icin ise herhangi bir tusa basabilirsiniz: ");
                        if (fgets(input, sizeof(input), stdin) != NULL)
                        {
                            difficulty = (input[0] == 'e') ? 2 : 1;
                        }
                    }
                    else
                    {
                        difficulty = 1;
                    }
                }

                clearScreen();

                if (difficulty == 2)
                {
                    printf("\nEkstrem zorluk ile devam ediliyor.\n\n");
                }
                else
                {
                    printf("\nOrta zorluk ile devam ediliyor.\n\n");
                }
            }

            valid = false;
            while (!valid)
            {
                printf("\nOyun tahtasi boyutunu girin (6-20): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {

                    input[strcspn(input, "\n")] = 0;
                    if (sscanf(input, "%d", &size) == 1 && size >= 6 && size <= 20)
                    {
                        valid = true;
                    }
                    else
                    {
                        printf("\nGecersiz boyut! Oyun tahtasi boyutu 6 ile 20 arasinda olmalidir.\n");
                    }
                }
            }
            clearScreen();
            printf("\nTahta boyutu: %d\n\n", size);

            char **board = createBoard(size);

            fillBoard(board, size);

            Player player1 = {0, {0, 0, 0, 0, 0}, 0, 0, 0, 0};
            Player player2 = {0, {0, 0, 0, 0, 0}, 0, 0, 0, 0};

            if (controlMode == 2)
            {
            }
            else
            {
                printBoard(board, size, &player1, &player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
            }

            initMoveStack(&undoStack1, size * size);
            initMoveStack(&redoStack1, size * size);
            initMoveStack(&undoStack2, size * size);
            initMoveStack(&redoStack2, size * size);

            while (1)
            {

                int moveMade = 0;
                if (gameMode == 1)
                {

                    if (controlMode == 1)
                    {
                        moveMade = makeMoveXY(board, size, &currentPlayer, &player1, &player2, &undoStack1, &redoStack1, &undoStack2, &redoStack2, gameMode, &gameLoaded, difficulty, writeToFileMode, &totalEvaluations1, &totalEvaluations2, controlMode, selectedRow, selectedCol, highlight);
                    }
                    else if (controlMode == 2)
                    {
                        moveMade = makeMoveWASD(board, size, &currentPlayer, &player1, &player2, &undoStack1, &redoStack1, &undoStack2, &redoStack2, gameMode, &gameLoaded, difficulty, writeToFileMode, &totalEvaluations1, &totalEvaluations2, controlMode, selectedRow, selectedCol, highlight);
                    }
                }
                else if (gameMode == 2)
                {

                    if (currentPlayer == 1)
                    {

                        if (controlMode == 1)
                        {
                            moveMade = makeMoveXY(board, size, &currentPlayer, &player1, &player2, &undoStack1, &redoStack1, &undoStack2, &redoStack2, gameMode, &gameLoaded, difficulty, writeToFileMode, &totalEvaluations1, &totalEvaluations2, controlMode, selectedRow, selectedCol, highlight);
                        }
                        else if (controlMode == 2)
                        {
                            moveMade = makeMoveWASD(board, size, &currentPlayer, &player1, &player2, &undoStack1, &redoStack1, &undoStack2, &redoStack2, gameMode, &gameLoaded, difficulty, writeToFileMode, &totalEvaluations1, &totalEvaluations2, controlMode, selectedRow, selectedCol, highlight);
                        }
                    }
                    else
                    {

                        makeComputerMove(board, size, &currentPlayer, &player1, &player2, &undoStack1, &redoStack1, difficulty, gameMode, writeToFileMode, &totalEvaluations2, controlMode, selectedRow, selectedCol, highlight);
                        moveMade = 1;
                    }
                }

                if (moveMade)
                {

                    if (checkGameOver(board, size, &player1, &player2))
                    {
                        printf("\nOyun bitti!\n\n");
                        printBoard(board, size, &player1, &player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                        printf("Oyuncu 1 Puan: %d\n", player1.score);
                        if (gameMode == 1)
                        {
                            printf("Oyuncu 2 Puan: %d\n", player2.score);
                        }
                        else
                        {
                            printf("Bilgisayar Puan: %d\n", player2.score);
                        }

                        if (player1.score > player2.score)
                        {
                            printf("\nOyuncu 1 kazandi!\n");
                        }
                        else if (player2.score > player1.score)
                        {
                            if (gameMode == 1)
                            {
                                printf("\nOyuncu 2 kazandi!\n");
                            }
                            else
                            {
                                printf("\nBilgisayar kazandi!\n");
                            }
                        }
                        else
                        {
                            printf("\nOyun berabere!\n");
                        }

                        printf("\nProgrami sonlandirmak icin 'b', ana menuye donmek icin ise herhangi bir tusa basin: ");
                        if (fgets(input, sizeof(input), stdin) != NULL)
                        {

                            input[strcspn(input, "\n")] = 0;
                            if (input[0] == 'b' || input[0] == 'B')
                            {

                                freeBoard(board, size);
                                freeMoveStack(&undoStack1);
                                freeMoveStack(&redoStack1);
                                freeMoveStack(&undoStack2);
                                freeMoveStack(&redoStack2);
                                return 0;
                            }
                            else
                            {

                                freeBoard(board, size);
                                freeMoveStack(&undoStack1);
                                freeMoveStack(&redoStack1);
                                freeMoveStack(&undoStack2);
                                freeMoveStack(&redoStack2);
                                clearScreen();
                                main();
                            }
                        }
                    }

                    if (!gameLoaded)
                    {
                        currentPlayer = (currentPlayer == 1) ? 2 : 1;
                    }
                    else
                    {
                        currentPlayer = (currentPlayer == 1) ? 2 : 1;
                        gameLoaded = 0;
                    }
                }
            }

            clearScreen();
        }
    }

    return 0;
}
