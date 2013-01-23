#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef _OPENMP
   #include <omp.h>
   #include "wallclock.h"
   #define outfile "redblue.txt"
   #define testing "initOMP.txt"
#else
   #define outfile "redblue-nomp.txt"
   #define testing "initNOMP.txt"
   #define omp_get_num_threads() 0
   #define omp_get_thread_num() 0
   #define omp_set_num_threads(procCount) 0
   #define StartTime() NULL;
   #define EndTime() -0.0;
#endif

#define WHITE 0
#define RED   1
#define BLUE  2

int **globalBoard;
int highestConc;

void generateBoard(int N) {
   globalBoard = (int**) malloc( N * sizeof(int*) );

   for ( int i = 0; i < N; i++ ) {
      globalBoard[i] = (int*) malloc( N * sizeof(int) );
      for ( int j = 0; j < N; j++ ) {
         int temp = rand() % 3;
         switch(temp) {
            case WHITE: globalBoard[i][j] = WHITE; break;
            case RED:   globalBoard[i][j] = RED; break;
            case BLUE:  globalBoard[i][j] = BLUE; break;
         }
      }
   }
}

void printBoardFile(int N, FILE *fp) {
   for ( int i = 0; i < N; i++ ) { 
      for ( int j = 0; j < N; j++ ) {
         switch( globalBoard[i][j] ) {
            case 0: fprintf(fp, "_"); break;
            case 1: fprintf(fp, ">"); break;
            case 2: fprintf(fp, "v"); break;
         }
      }
      fprintf(fp, "\n");
   }
}

void printBoard(int N) {
   for ( int i = 0; i < N; i++ ) { 
      for ( int j = 0; j < N; j++ ) {
         switch( globalBoard[i][j] ) {
            case 0: printf("_"); break;
            case 1: printf(">"); break;
            case 2: printf("v"); break;
         }
      }
      printf("\n");
   }
}

void freeBoard(int N) {
   for (int i = 0; i < N; ++i) {
      free(globalBoard[i]);
   }
   free(globalBoard);
}

int checkBoard(int x, int y, int N, int T, int C) {

   int maxColouredCells = (T * T * C) / 100;
   int colouredRed      = 0;
   int colouredBlue     = 0;

   int startX = x * T;
   int startY = y * T;

   for ( int i = startX; i < T + startX; i++ ) {
      for ( int j = startY; j < T + startY; j++ ) {
         if ( globalBoard[i][j] == BLUE )
            colouredBlue += 1;
         else if ( globalBoard[i][j] == RED )
            colouredRed += 1;
      }
   }

   double tileArea = T * T;

   int concRed  = (colouredRed  / tileArea) * 100;
   int concBlue = (colouredBlue / tileArea) * 100;

   #pragma omp critical
   if ( concBlue > highestConc ) {
      highestConc = concBlue;
   }
   if ( concRed > highestConc ) {
      highestConc = concRed;
   }

   if ( colouredRed >= maxColouredCells ) {
      return colouredRed;
   }
   else if ( colouredBlue >= maxColouredCells ) {
      return colouredBlue;
   }

   return 0;
}

void redStep(int startRow, int endRow, int N) {
#pragma omp parallel 
{
   int willRollover = 0;
   #pragma omp for
   for ( int i = startRow; i < endRow; i++ ) {
      if ( globalBoard[i][N-1] == RED && globalBoard[i][0] == WHITE )
         willRollover = 1;
      
      for ( int j = 0; j < (N - 1); j++ ) {
         if ( globalBoard[i][j] == RED && globalBoard[i][j+1] == WHITE ) {
            globalBoard[i][j]   = WHITE;
            globalBoard[i][j+1] = RED;
            j += 1;
         }
      }
      if ( willRollover == 1 ) {
         globalBoard[i][N-1] = WHITE;
         globalBoard[i][0]   = RED;
         willRollover = 0;
      }
   }
   #pragma omp barrier
}
}

void blueStep(int startCol, int endCol, int N) {
#pragma omp parallel 
{
   int willRollover = 0;
   #pragma omp for
   for ( int j = startCol; j < endCol; j++ ) {
      if ( globalBoard[N-1][j] == BLUE && globalBoard[0][j] == WHITE )
         willRollover = 1;
      for ( int i = 0; i < (N - 1); i++ ) {
         if ( globalBoard[i][j] == BLUE && globalBoard[i + 1][j] == WHITE ) {
            globalBoard[i][j]   = WHITE;
            globalBoard[i + 1][j] = BLUE;
            i += 1;
         }
      }
      if ( willRollover == 1 ) {
         globalBoard[N-1][j] = WHITE;
         globalBoard[0][j]   = BLUE;
         willRollover = 0;
      }
   }
   #pragma omp barrier
}
}

int startSimulation(int procCount, int boardSize, int tileSize, int colorDen, int stepCount) {

   omp_set_num_threads(procCount);

   for ( int i = 0; i < stepCount; i++ ) {
      int done = 0;

      #pragma omp parallel for
      for ( int x = 0; x < boardSize/tileSize; x++ ) {
         for ( int y = 0 ; y < boardSize/tileSize; y++ ) {
            done = checkBoard(x, y, boardSize, tileSize, colorDen);
         }
      }
      #pragma omp barrier

      if ( done ) return i;

      redStep(0, boardSize, boardSize);
      blueStep(0, boardSize, boardSize);   
   }
   return -1;
}

void writeFile(int procCount, int boardSize, int tileSize, int colorDen, int stepCount, int seedNumber, int numSteps, float wallTime ) { 
   FILE *fp;
   fp = fopen(outfile, "w");
   if ( fp != NULL ) {
      for ( int i = 0; i < boardSize; i++ ) { 
         for ( int j = 0; j < boardSize; j++ ) {
            switch( globalBoard[i][j] ) {
               case 0: fprintf(fp, "_"); break;
               case 1: fprintf(fp, ">"); break;
               case 2: fprintf(fp, "v"); break;
            }
         }
         fprintf(fp, "\n");
      }
      fprintf(fp, "ARGS: p%d b%d t%d c%d m%d ", procCount, boardSize, tileSize, colorDen, stepCount);
      fprintf(stdout, "ARGS: p%d b%d t%d c%d m%d ", procCount, boardSize, tileSize, colorDen, stepCount);
      if ( seedNumber != -1 ) {
         fprintf(fp, "s%d ", seedNumber);
         fprintf(stdout, "s%d ", seedNumber);
      }

      fprintf(fp, "STEPS: %d   HIGHEST CONC: %d%%   TIME: %.2f \n", numSteps, highestConc, wallTime);
      fprintf(stdout, "STEPS: %d   HIGHEST CONC: %d%%    TIME: %.2f \n", numSteps, highestConc, wallTime);

   }
}

int main(int argc, char const *argv[]) {

   int argCheck[5]     = {0,0,0,0,0};
   int procCount       = -1;
   int boardSize       = -1;
   int tileSize        = -1;
   int colorDen        = -1;
   int stepCount       = -1;
   int seedNumber      = -1;
   int interactiveMode = -1;
   int boardSolved     = 0;

   for ( int i = 0; i < argc; i++ ) {
      char arg = argv[i][0];

      switch(arg) {
         case 'p': procCount   = atoi(&argv[i][1]); 
                   argCheck[0] = 1; 
                   break;
         case 'b': boardSize   = atoi(&argv[i][1]); 
                   argCheck[1] = 1; 
                   break;
         case 't': tileSize    = atoi(&argv[i][1]); 
                   argCheck[2] = 1; 
                   break;
         case 'c': colorDen    = atoi(&argv[i][1]);
                   argCheck[3] = 1; 
                   break;
         case 'm': stepCount   = atoi(&argv[i][1]);
                   argCheck[4] = 1; 
                   break;
         case 's': seedNumber  = atoi(&argv[i][1]); 
                   break;
      }
   }

   for ( int i = 0; i < 5; i++ ) {
      if ( argCheck[i] == 0 ) {
         printf("Missing required argument.\n");
         printf("Usage: rbs p[1..] b[2..] t[1..] c[1..100] m[1..] [s] [i]\n");
         exit(1);
      }
   }

   if ( procCount < 1 ) {
      printf("Number of processors must be >= 1.\n");
      printf("Usage: rbs p[1..] b[2..] t[1..] c[1..100] m[1..] [s] [i]\n");
      exit(1);
   } else if ( boardSize < 2 ) {
      printf("Board size must be >= 2.\n");
      printf("Usage: rbs p[1..] b[2..] t[1..] c[1..100] m[1..] [s] [i]\n");
      exit(1);
   } else if ( boardSize % tileSize != 0 || tileSize > boardSize ) {
      printf("Tile size must be <= board size and divisible into board size.\n");
      printf("Usage: rbs p[1..] b[2..] t[1..] c[1..100] m[1..] [s] [i]\n");
      exit(1);
   } else if ( colorDen > 33 ) {
      printf("Warning: color density very large. Chances of solving very low.\n");
   }

   if ( seedNumber == -1 ) {
      srand( time(NULL) );
   } else {
      srand( seedNumber );
   }

   int pc = procCount;
   if ( pc > boardSize )
      pc = boardSize;

   generateBoard(boardSize);
   FILE *fp = fopen(testing, "w");
   printBoardFile(boardSize, fp);
   fclose(fp);

   StartTime();

   boardSolved = startSimulation(pc, boardSize, tileSize, colorDen, stepCount);

   double wallTime = EndTime();

   if ( boardSolved >= 0 ) {
      writeFile(procCount, boardSize, tileSize, colorDen, stepCount, seedNumber, boardSolved, wallTime);
   } else { 
      writeFile(procCount, boardSize, tileSize, colorDen, stepCount, seedNumber, stepCount, wallTime);
   }

   freeBoard(boardSize);

   return 0;
}
