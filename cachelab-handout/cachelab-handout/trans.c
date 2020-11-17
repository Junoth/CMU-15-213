/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
  int i, j, ii, n1, n2, n3, n4, n5, n6, n7, n8;

  for (i = 0; i < N; i += 8) {
    for (j = 0; j < M; j += 8) {
      for (ii = i; ii < i + 8; ++ii) {
        n1 = A[ii][j];
        n2 = A[ii][j + 1];
        n3 = A[ii][j + 2];
        n4 = A[ii][j + 3];
        n5 = A[ii][j + 4];
        n6 = A[ii][j + 5];
        n7 = A[ii][j + 6];
        n8 = A[ii][j + 7];
        B[j][ii] = n1;
        B[j + 1][ii] = n2;
        B[j + 2][ii] = n3;
        B[j + 3][ii] = n4;
        B[j + 4][ii] = n5;
        B[j + 5][ii] = n6;
        B[j + 6][ii] = n7;
        B[j + 7][ii] = n8;
      }
    }
  }
}

char trans_two_desc[] = "Transpose two submission";
void trans_two(int M, int N, int A[N][M], int B[M][N]) {
  int i, j, ii, n1, n2, n3, n4, n5, n6, n7, n8;
  for (i = 0; i < N; i += 4) {
    for (j = 0; j < M; j += 4) {
      for (ii = i; ii < i + 4; ii += 2) {
        n1 = A[ii][j];
        n2 = A[ii][j + 1];
        n3 = A[ii][j + 2];
        n4 = A[ii][j + 3];
        n5 = A[ii + 1][j];
        n6 = A[ii + 1][j + 1];
        n7 = A[ii + 1][j + 2];
        n8 = A[ii + 1][j + 3];
        B[j][ii] = n1;
        B[j + 1][ii] = n2;
        B[j + 2][ii] = n3;
        B[j + 3][ii] = n4;
        B[j][ii + 1] = n5;
        B[j + 1][ii + 1] = n6;
        B[j + 2][ii + 2] = n7;
        B[j + 3][ii + 3] = n8;
      }
    }
  }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    registerTransFunction(trans_two, trans_two_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

