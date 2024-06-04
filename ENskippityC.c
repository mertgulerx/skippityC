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
        printf("\nAI1                AI2\n");
    }
    else
    {
        printf("\nPlayer 1                  ");
        if (gameMode == 1)
        {
            printf("Player 2\n");
        }
        else
        {
            printf("Computer\n");
        }
    }

    printf("-------------------       -------------------\n");

    printf("%sA:%s%d %sB:%s%d %sC:%s%d %sD:%s%d %sE:%s%d       %sA:%s%d %sB:%s%d %sC:%s%d %sD:%s%d %sE:%s%d\n"
           "Score: %-10d          Score: %-10d\n"
           "Set: %-10d           Set: %-10d\n"
           "Extra Skipper: %-10dExtra Skipper: %-10d\n\n",
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
        printf("\nThe computer made a move.\n\nCurrent game board:\n\n");
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
        printf("\nEnter the name of the file you want to save the game to (It is recommended to enter numbers like 1,2,3 for easy remembering): ");
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
                printf("File name cannot be empty! Try again.\n");
            }
        }
    }

    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("File opening error!\n");
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
    printf("\nGame successfully saved to %s file.\n\n", filename);
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
        printf("\nEnter the name of the save file you want to load (Do not add .txt to the end of the file.), press enter only to cancel the loading process: ");
        if (fgets(input, sizeof(input), stdin) != NULL)
        {

            input[strcspn(input, "\n")] = 0;
            if (strlen(input) == 0)
            {
                clearScreen();
                printf("\nLoading process canceled.\n\n");
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
                printf("\nFile could not be opened or found. Please try again.\n");
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
    printf("\nLoaded Game Board:\n\n");
    printBoard(*board, *size, player1, player2, 1, 0, -1, -1, 0);

    bool validInput = true;
    while (!validInput)
    {
        printf("Press 'b' to select another file, or any key to continue with this save: ");
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
    printf("Game successfully loaded.\n\n");
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
        printf("Player %d's turn\n", *currentPlayer);
        printf("Use 'wasd' keys to select the source piece and press Enter.\n");

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
                    printf("\nYou cannot select an empty cell. Please select a valid piece.\n\n");
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
                    printf("\nThere are no directions where this piece can jump. Please select another piece.\n\n");

                    row = 0;
                    col = 0;
                    validInput = 1;
                }
                break;
            case 'k':
                if (*extraMove)
                {
                    printf("\nYou cannot save the game before confirming your move. Pass the turn to the next player and try again.\n\n");
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
                printf("\nIn this mode, you don't need to undo the move with the undo key. You can already undo your move with the movement keys!\n\n");
                validInput = 1;
                break;
            case 'r':
                clearScreen();
                printf("\nIn this mode, you don't need to redo the old move with the redo key. You can already redo your move with the movement keys!\n\n");
                validInput = 1;
                break;
            case 'q':
                clearScreen();
                printf("\nDo you want to save the game before exiting? (y/n): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    if (input[0] == 'y' || input[0] == 'Y')
                    {
                        saveGame(board, size, *player1, *player2, *currentPlayer);
                    }
                }
                clearScreen();
                printf("\nReturning to the main menu...");
                sleep_ms(1000);
                main();
                validInput = 1;
                break;
            default:
                printf("\nInvalid command! Please try again.\n");
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
    printf("Player %d's turn\n", *currentPlayer);
    printf("Use 'wasd' keys to select the destination piece and press Enter to confirm.\n\nIf you can make an extra move, you can continue. If you can't, press Enter to pass the turn to the other player.\n");

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
                    printf("\nThere is no cell to jump in this direction. Try another direction.\n\n");
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
                    printf("\nThere is no cell to jump in this direction. Try another direction.\n\n");
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
                    printf("\nThere is no cell to jump in this direction. Try another direction.\n\n");
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
                    printf("\nThere is no cell to jump in this direction. Try another direction.\n\n");
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
                printf("\nYou cannot save the game before confirming your move. Pass the turn to the next player and try again.\n\n");
                validInput = 1;
                break;
            case 'u':
                clearScreen();
                printf("\nIn this mode, you don't need to undo the move with the undo key. You can already undo your move with the movement keys!\n\n");
                validInput = 1;
                break;
            case 'r':
                clearScreen();
                printf("\nIn this mode, you don't need to redo the old move with the redo key. You can already redo your move with the movement keys!\n\n");
                validInput = 1;
                break;
            case 'l':
                clearScreen();
                printf("\nYou cannot load the game before confirming your move. Pass the turn to the next player and try again.\n\n");
                validInput = 1;
                break;
            case 'q':
                clearScreen();
                printf("\nDo you want to save the game before exiting? (y/n): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    if (input[0] == 'y' || input[0] == 'Y')
                    {
                        saveGame(board, size, *player1, *player2, *currentPlayer);
                    }
                }
                clearScreen();
                printf("\nReturning to the main menu...");
                sleep_ms(1000);
                main();
                validInput = 1;
                break;
            default:
                printf("\nInvalid command! Please try again.\n");
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
            printf("Player %d's turn\n", *currentPlayer);
            printf("Use 'wasd' keys to select the destination piece and press Enter to confirm.\n\nIf you can make an extra move, you can continue. If you can't, press Enter to pass the turn to the other player.\n");
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
                printf("\nPlayer %d's turn\nWhich piece to move (row / column): ", *currentPlayer);
                valid = false;
                while (!valid)
                {
                    fgets(input, sizeof(input), stdin);

                    input[strcspn(input, "\n")] = 0;

                    if (strcmp(input, "s") == 0)
                    {
                        saveGame(board, size, *player1, *player2, *currentPlayer);
                        printf("\nPlayer %d's turn\nWhich piece to move (row / column): ", *currentPlayer);
                    }
                    else if (strcmp(input, "l") == 0)
                    {
                        loadGame(&board, &size, player1, player2, currentPlayer);
                        printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                        printf("\nPlayer %d's turn\nWhich piece to move (row / column): ", *currentPlayer);
                        *gameLoaded = 1;
                    }
                    else if (strcmp(input, "q") == 0)
                    {
                        clearScreen();
                        printf("\nDo you want to save the game before exiting? (y/n): ");
                        if (fgets(input, sizeof(input), stdin) != NULL)
                        {
                            if (input[0] == 'y' || input[0] == 'Y')
                            {
                                saveGame(board, size, *player1, *player2, *currentPlayer);
                            }
                        }
                        clearScreen();
                        printf("\nReturning to the main menu...");
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
                        printf("\nInvalid source coordinates! Coordinates must be between 1 and %d, in the format 'a b'. Please try again.\n\n", size);
                        printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                        printf("\nPlayer %d's turn\nWhich piece to move (row / column): ", *currentPlayer);
                        valid = false;
                    }
                }

                piece = board[srcRow - 1][srcCol - 1];

                if (piece == ' ')
                {
                    printf("\nThe cell you selected is empty! You cannot select an empty cell! Try again.\n");
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
                printf("Where to place the piece (row / column): ");
            }
            else
            {
                if (currentPlayerPtr->undoUsed == 0)
                {
                    printf("Press 'u' to undo, 'g' to pass to the next player, or enter the destination coordinates to continue jumping: \n");
                }
                else if (currentPlayerPtr->redoUsed == 0)
                {
                    printf("Press 'r' to redo, 'g' to pass, or 't' to make another move. \n");
                }
                else
                {
                    printf("Press 'g' to pass to the next player, or enter the destination coordinates to continue jumping: \n");
                }
            }

            fgets(input, sizeof(input), stdin);

            input[strcspn(input, "\n")] = 0;

            if (strcmp(input, "s") == 0)
            {
                if (extraMove)
                {
                    printf("\nYou cannot save the game before confirming your move. Pass the turn to the next player and try again.\n\n");
                }
                else
                {
                    saveGame(board, size, *player1, *player2, *currentPlayer);
                }
            }
            else if (strcmp(input, "l") == 0)
            {
                printf("\nYou cannot load the game before confirming your move. Pass the turn to the next player and try again.\n\n");
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
                    printf("\nYou have used your undo right!\n\n");
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
                        printf("\nYou have used your redo right!\n\n");
                    }
                }
                else
                {
                    printf("\nYou need to undo first to redo!\n\n");
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
                printf("\nDo you want to save the game before exiting? (y/n): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    if (input[0] == 'y' || input[0] == 'Y')
                    {
                        saveGame(board, size, *player1, *player2, *currentPlayer);
                        printf("Your turn has been automatically passed to the other player because you did not confirm your move.");
                        sleep_ms(4000);
                    }
                }
                clearScreen();
                printf("\nReturning to the main menu...");
                sleep_ms(1000);
                main();
            }
            else if (sscanf(input, "%d %d", &destRow, &destCol) == 2)
            {

                if (destRow < 1 || destRow > size || destCol < 1 || destCol > size)
                {
                    clearScreen();
                    printf("\nInvalid destination coordinates! Coordinates must be between 1 and %d, in the format 'a b'. Please try again.\n\n", size);
                    printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                    return 0;
                }

                if (board[destRow - 1][destCol - 1] != ' ')
                {
                    printf("\nDestination cell is full! Please try again by entering the source coordinates from the beginning.\n");
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
                        printf("\nInvalid move! Please try again.\n");
                    }
                }
                else
                {
                    printf("\nInvalid move! Please try again.\n");
                }
            }
            else if (sscanf(input, "%d", &destRow) == 1)
            {
                clearScreen();
                printf("\nInvalid input! You have made too many invalid inputs! You are being penalized. Passing to the next player...\n\n");
                printBoard(board, size, player1, player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                return 1;
            }
            else
            {
                clearScreen();
                printf("\nInvalid input! Please do not enter random letters.\n\n");
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
        "Let's learn the rules and mechanics of the game together.",
        "",
        "The rules of the Skippity game are very simple:",
        "",
        "",
        "GAME BOARD",
        "",
        "It is played on a square game board of size 6-20. There are 5 pieces (Pieces can also be called 'Skippers') named A B C D E.",
        "",
        "Each square on the game board is randomly filled with one of the 5 different pieces.",
        "",
        "The middle 2x2 part is left empty at the beginning of the game to be able to make a move.",
        "",
        "",
        "MOVEMENT MECHANISM",
        "",
        "When it's your turn, you jump a piece on the board over another piece to an empty square, and you write the piece you jumped over to your score.",
        "",
        "Jumps can only be made vertically and horizontally. You can only jump over an adjacent piece.",
        "",
        "If you are not satisfied with your move, you can undo it with the 'u' key, and if you change your mind, you can redo the previous move with the 'r' key.",
        "",
        "You have 1 undo and 1 redo right per turn. Making a move does not reset your rights.",
        "",
        "If you can make another move from the square you jumped from, you can continue jumping. If you can't, you need to pass the turn to the other player.",
        "",
        "",
        "SCORE CALCULATION",
        "",
        "If you collect one of each of the A B C D E pieces, this is called a \"set\". Each collected set is worth 100 points.",
        "",
        "Each extra piece you have outside of the sets is worth 1 point, regardless of the type of piece.",
        "",
        "",
        "DETERMINING THE WINNER",
        "",
        "The game ends automatically when there are no more moves to be made at the end of the game.",
        "",
        "The player with the most points wins.",
        "",
        "",
        "GAMEPLAY MECHANICS AND KEYS",
        "",
        "At the beginning of the game, you need to choose the game mode. Press '1' for two-player game, '2' to play against the computer.",
        "",
        "Then you will be asked to enter the size of the game board. The game board must be entered between 6-20. Otherwise, the game will not start.",
        "",
        "You can press 's' to save the game before or after making a move, and 'l' to load any saved game.",
        "",
        "After making a move, you can press 'u' to undo and 'r' to redo.",
        "",
        "",
        "An example board is as follows:",
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
        "Player 1                 Player 2",
        "-------------------      -------------------",
        "A:0 B:0 C:0 D:0 E:0      A:0 B:0 C:0 D:0 E:0",
        "Score: 0                 Score: 0",
        "Set: 0                   Set: 0",
        "Extra Skipper: 0         Extra Skipper: 0",
        "",
        "",
        "After this stage, you need to enter coordinates to make a move. It will be asked as follows:",
        "",
        "Player 1's turn",
        "Which piece to move (row / column):",
        "Where to place the piece (row / column):",
        "",
        "Please enter 'x y', '1 3' when entering coordinates. Other entries are invalid.",
        "",
        "",
        "JUMPING TUTORIAL",
        "",
        "Examine the following example rows and columns.",
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
        "Move successful!",
        "",
        "A:0 B:1 C:0 D:0 E:0",
        "Score: 1",
        "Set: 0",
        "Extra Skipper: 1",
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
        "Move successful!",
        "",
        "A:0 B:0 C:1 D:0 E:0",
        "Score: 1",
        "Set: 0",
        "Extra Skipper: 1",
        "",
        "",
        "Now you are ready! Don't forget to play a game against the computer at extreme difficulty to test the intelligence of my AI!",
        "",
        "",
        "Game written by:",
        "MERT GULER - YILDIZ TEKNIK UNIVERSITY - COMPUTER ENGINEERING - 2024",
        "",
        "",
        "HAVE FUN :)",
        "",
        "⢠⣶⠀⠀⠈⢿⣧⠀⠀⠀⠀⣾⡶",
        "⠀⢿⣧⠀⠀⠘⣿⡆⠀⠀⢰⣿⠃",
        "⠀⠘⠟⠓⠀⠀⠀⠀⠀⢠⣿⠏⠀",
        "⠀⠀⠀⠀⠀⠀⠀⢀⣴⡿⠋⠀⠀",
        "⠀⠀⠀⠀⢀⣠⣴⣿⠟⠁⠀⠀⠀",
        "⠀⠀⠐⢾⠿⠟⠋⠀⠀⠀⠀⠀⠀",
        "",
        "",
        "Press any key to return to the main menu",
        "",
    };

    const char *ascii_art[] = {
        "",
        "",
        "",
        "Welcome to my Skippity game!",
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
    "   1- Two-player game\n"
    "   2- Game against the computer\n"
    "   3- AI vs AI\n"
    "   4- Introduction, rules and extras\n"
    "   5- Controls\n\n"
    "**************************************\n\n ";

char mainMenuControlsText[] =
    ""
    "\n***************************************\n"
    "     ** SkippityC by MERT GULER **\n"
    "***************************************\n"
    "              Controls\n\n"
    "  Undo the move: 'u\n"
    "  Redo the undone move: 'r'\n"
    "  Save the game: 's'\n"
    "  Save in arrow key mode: 'k'\n"
    "  Load the saved game: 'l'\n"
    "  Pass to the next player: 'g'\n"
    "  Make a move with coordinates: 'x y'\n"
    "  Move with arrow keys: 'w,a,s,d'\n"
    "  Return to the main menu: 'q'\n\n"
    "**************************************\n\n"
    "Press any key to return to the previous menu: ";

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
                    printf("\nInvalid game mode! Please try again.\n\n");
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
            printf("\nPress 'y' to continue with arrow key control, or any key to continue with coordinate control: ");
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
                    printf("\nContinuing with coordinate control... Please wait without pressing any keys...\n\n");
                    sleep_ms(1000);
                    clearScreen();
                }
                else
                {
                    printf("\nWARNING! THIS CONTROL MODE IS COMPLETELY EXPERIMENTAL. ALL GAME MECHANICS ARE DESIGNED ACCORDING TO COORDINATE MOVEMENT MODE. IT IS NOT RECOMMENDED TO PLAY IN THIS MODE!\n\n");
                    sleep_ms(2500);
                    clearScreen();
                    printf("\nContinuing with arrow key control... Please wait without pressing any keys...\n\nThe save key in this control mode is: 'k'");
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

            printf("\nPress 'a' to write the output of 100 games with random sizes to the \"aitest\" file.\n\nWARNING! The writing process may take a very long time!\n\nPress any key to simulate one game in the terminal:\n");
            if (fgets(input, sizeof(input), stdin) != NULL && input[0] == 'a')
            {
                printf("\nSimulations are being written to the file...\n");
                writeToFileMode = 1;

                FILE *file = fopen("aitest.txt", "w");
                if (file == NULL)
                {
                    printf("File could not be opened!\n");
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

                    fprintf(file, "Game %d ended!\n", i + 1);
                    fprintf(file, "Size: %d\n", size);
                    for (int r = 0; r < size; r++)
                    {
                        for (int c = 0; c < size; c++)
                        {
                            fprintf(file, "%c ", board[r][c]);
                        }
                        fprintf(file, "\n");
                    }
                    fprintf(file, "\n");
                    fprintf(file, "AI1 Score: %d\n", player1.score);
                    fprintf(file, "AI2 Score: %d\n", player2.score);

                    if (player1.score > player2.score)
                    {
                        fprintf(file, "\nAI1 won!\n\n");
                    }
                    else if (player2.score > player1.score)
                    {
                        fprintf(file, "\nAI2 won!\n\n");
                    }
                    else
                    {
                        fprintf(file, "\nThe game is a draw!\n\n");
                    }
                    fprintf(file, "\n");

                    freeBoard(board, size);
                    freeMoveStack(&undoStack1);
                    freeMoveStack(&redoStack1);
                    freeMoveStack(&undoStack2);
                    freeMoveStack(&redoStack2);
                }

                fclose(file);
                printf("\n100 game outputs were written to the \"aitest.txt\" file.\n");
                writeToFileMode = 0;

                printf("\nPress 'b' to exit the program, or any key to return to the main menu: ");
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
                printf("\nEnter the size of the game board (6-20): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {

                    input[strcspn(input, "\n")] = 0;
                    if (sscanf(input, "%d", &size) == 1 && size >= 6 && size <= 20)
                    {
                        valid = true;
                    }
                    else
                    {
                        printf("\nInvalid size! The game board size must be between 6 and 20.\n\n");
                    }
                }
            }
            clearScreen();
            printf("\nEnter the size of the game board (6-20): ");

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

            printf("\nGame ended!\n\n");
            printBoard(board, size, &player1, &player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
            printf("AI1 Score: %d\n", player1.score);
            printf("AI2 Score: %d\n\n", player2.score);

            if (player1.score > player2.score)
            {
                printf("AI1 won!\n\n");
            }
            else if (player2.score > player1.score)
            {
                printf("AI2 won!\n\n");
            }
            else
            {
                printf("The game is a draw!\n\n");
            }

            if (gameMode == 3)
            {
                printf("AI 1 evaluated a total of %d moves.\n", totalEvaluations1);
                printf("AI 2 evaluated a total of %d moves.\n\n", totalEvaluations2);
            }

            printf("\nPress 'b' to exit the program, or any key to return to the main menu: ");
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

                printf("\nChoose the AI difficulty: Press 'o' for medium difficulty, or 'e' for extreme difficulty: ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {
                    if (input[0] == 'e')
                    {
                        clearScreen();
                        printf("\nAre you sure? Extreme difficulty has been optimized after thousands of tests to make it almost impossible to beat. ");
                        printf("\nWhile you are thinking about your current move, it can see hundreds of thousands of moves ahead.\n\nIf you are sure, press 'e', or any key to continue with medium difficulty: ");
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
                    printf("\nContinuing with extreme difficulty.\n\n");
                }
                else
                {
                    printf("\nContinuing with medium difficulty.\n\n");
                }
            }

            valid = false;
            while (!valid)
            {
                printf("\nEnter the size of the game board (6-20): ");
                if (fgets(input, sizeof(input), stdin) != NULL)
                {

                    input[strcspn(input, "\n")] = 0;
                    if (sscanf(input, "%d", &size) == 1 && size >= 6 && size <= 20)
                    {
                        valid = true;
                    }
                    else
                    {
                        printf("\nInvalid size! The game board size must be between 6 and 20.\n");
                    }
                }
            }
            clearScreen();
            printf("\nBoard size: %d\n\n", size);

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
                        printf("\nGame ended!\n\n");
                        printBoard(board, size, &player1, &player2, gameMode, controlMode, selectedRow, selectedCol, highlight);
                        printf("Player 1 Score: %d\n", player1.score);
                        if (gameMode == 1)
                        {
                            printf("Player 2 Score: %d\n", player2.score);
                        }
                        else
                        {
                            printf("Computer Score: %d\n", player2.score);
                        }

                        if (player1.score > player2.score)
                        {
                            printf("\nPlayer 1 won!\n");
                        }
                        else if (player2.score > player1.score)
                        {
                            if (gameMode == 1)
                            {
                                printf("\nPlayer 2 won!\n");
                            }
                            else
                            {
                                printf("\nComputer won!\n");
                            }
                        }
                        else
                        {
                            printf("\nThe game is a draw!\n");
                        }

                        printf("\nPress 'b' to exit the program, or any key to return to the main menu: ");
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