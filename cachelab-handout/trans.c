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

//have to handle diagonal in a smarter way because both array have size of pow of 2, and the
// first row of B maps to the same set as the first row of A.
void transpose_32_32(int M, int N, int A[N][M], int B[M][N]){
    int i,j,ii,jj;
    int temp,size=8;
    for(i=0;i<N;i+=size){
        for(j=0;j<M;j+=size){
            for(ii=i;ii<i+size;ii++){
                for(jj=j;jj<j+size;jj++){
                    if(ii==jj)continue;
                    temp=A[ii][jj];
                    B[jj][ii]=temp;
                }
                if(i==j){
                    temp=A[ii][ii];
                    B[ii][ii]=temp;
                }
            }
        }
    }
}

void transpose_67_61(int M, int N, int A[N][M], int B[M][N]){
    int i,j,ii,jj;
    int rsize,csize,size=16;
    for(i=0;i<N;i+=size){
        rsize=((i+size)>N)?N-i:size;
        for(j=0;j<M;j+=size){
            csize=((j+size)>M)?M-j:size;
            for(ii=i;ii<i+rsize;ii++){
                for(jj=j;jj<j+csize;jj++){
                    B[jj][ii]=A[ii][jj];
                }
            }
        }
    }
}
// the idea is from https://blog.csdn.net/xbb224007/article/details/81103995
void transpose_64_64(int M, int N, int A[N][M], int B[M][N]){
    int i,j,x1,x2,x3,x4,x5,x6,x7,x8,x;
    for(i=0;i<N;i+=8){
        for(j=0;j<M;j+=8){
            for(x=i;x<i+4;x++){
                x1=A[x][j];
                x2=A[x][j+1];
                x3=A[x][j+2];
                x4=A[x][j+3];
                x5=A[x][j+4];
                x6=A[x][j+5];
                x7=A[x][j+6];
                x8=A[x][j+7];
                B[j][x]=x1;
                B[j+1][x]=x2;
                B[j+2][x]=x3;
                B[j+3][x]=x4;
                B[j][4+x]=x5;
                B[j+1][4+x]=x6;
                B[j+2][4+x]=x7;
                B[j+3][4+x]=x8;
            }
            for(x=j;x<j+4;x++){
                x1=A[i+4][x];
                x2=A[i+5][x];
                x3=A[i+6][x];
                x4=A[i+7][x];
                x5=B[x][i+4];
                x6=B[x][i+5];
                x7=B[x][i+6];
                x8=B[x][i+7];
                B[x][i+4]=x1;
                B[x][i+5]=x2;
                B[x][i+6]=x3;
                B[x][i+7]=x4;
                B[x+4][i]=x5;
                B[x+4][i+1]=x6;
                B[x+4][i+2]=x7;
                B[x+4][i+3]=x8;
            }
            for(x=i+4;x<i+8;x++){
                x1=A[x][j+4];
                x2=A[x][j+5];
                x3=A[x][j+6];
                x4=A[x][j+7];
                B[j+4][x]=x1;
                B[j+5][x]=x2;
                B[j+6][x]=x3;
                B[j+7][x]=x4;
            }
        }
    }
}
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
//starting address of matrix A:0x10d080, starting address of matrix B:0x14d080
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M==32)transpose_32_32(M,N,A,B);
    else if(M==64) transpose_64_64(M,N,A,B);
    else transpose_67_61(M,N,A,B);
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
    //registerTransFunction(trans, trans_desc); 

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

