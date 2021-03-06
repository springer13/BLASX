/*
 * -- BLASX (version 1.0.0) --
 * @precisions normal z -> s d c
 */

#include <LRU.h>
#include <cblas.h>
#include <flops.h>
#include <blasx.h>
#include <blasx_config.h>
#include <blasx_internal.h>
#include <blasx_tile_resource.h>
#include <blasx_globalpointers.h> //FOR CPU BLAS
#define ERROR_NAME "ZTRSM "

void cblas_ztrsm(const enum CBLAS_ORDER Order, const enum CBLAS_SIDE Side,
                 const enum CBLAS_UPLO Uplo, const enum CBLAS_TRANSPOSE TransA,
                 const enum CBLAS_DIAG Diag, const int M, const int N,
                 const blasxDoubleComplex *alpha, const blasxDoubleComplex *A, const int lda,
                 blasxDoubleComplex *B, const int ldb)
{
    cublasOperation_t transa; cublasFillMode_t uplo;
    cublasSideMode_t side; cublasDiagType_t diag;
    cublasStatus_t status;
    
    /*---error handler---*/
    int info = 0;
    int nrowa;
    if (Side == CblasLeft) nrowa = M;
    else nrowa = N;
    if (ldb < MAX(1,M))                                       info = 11;
    if (lda < MAX(1,nrowa))                                   info =  9;
    if (N < 0)                                                info =  6;
    if (M < 0)                                                info =  5;
    if (CBlasDiagModeToCuBlasDiagMode(Diag, &diag) < 0)       info =  4;
    if (CBLasTransToCuBlasTrans(TransA,&transa) < 0)          info =  3;
    if (CBlasFilledModeToCuBlasFilledMode(Uplo,&uplo) < 0)    info =  2;
    if (CBlasSideToCuBlasSide(Side,&side) < 0)                info =  1;
    if (info != 0) {
        xerbla_(ERROR_NAME, &info);
        return;
    }
    /*-------------------*/
    
    /*----dispatcher-----*/
    int type = 0; //1:cpu 2:cublasxt 3:blasx
    if (N <= 0 || M <= 0)                      type = 1;
    if (type == 0 && (N > 1000 || M > 1000))   type = 1;
    else                                       type = 1;
    /*-------------------*/
    
    switch (type) {
        case 1:
        CPU_BLAS:
            Blasx_Debug_Output("Calling cblas_ztrsm\n ");
            if (cpublas_handle == NULL) blasx_init(CPU);
            if (cblas_ztrsm_p == NULL) blasx_init_cblas_func(&cblas_ztrsm_p, "cblas_ztrsm");
            (*cblas_ztrsm_p)(Order,Side,Uplo,TransA,Diag,M,N,alpha,A,lda,B,ldb);
            break;
        case 2:
            if (cublasXt_handle == NULL) blasx_init(CUBLASXT);
            Blasx_Debug_Output("Calling cublasXtZtrsm\n");
            status = cublasXtZtrsm(cublasXt_handle,
                                   side,   uplo,
                                   transa, diag,
                                   M, N,
                                   (cuDoubleComplex*)alpha,
                                   (cuDoubleComplex*)A, lda,
                                   (cuDoubleComplex*)B, ldb);
            if( status != CUBLAS_STATUS_SUCCESS ) goto CPU_BLAS;
            break;
        default:
            break;
    }
}

/* f77 interface */
void ztrsm_(char *side, char *uplo, char *transa, char *diag,
            int *m, int *n,
            blasxDoubleComplexF77 *alpha, blasxDoubleComplexF77 *A, int *lda,
            blasxDoubleComplexF77 *B, int *ldb)
{
    Blasx_Debug_Output("Calling ztrsm_ interface\n");
    enum CBLAS_TRANSPOSE TransA; enum CBLAS_UPLO Uplo;
    enum CBLAS_DIAG Diag; enum CBLAS_SIDE Side;
    int info = 0;
    if (F77DiagToCBLASDiag(diag, &Diag) < 0)                info =  4;
    if (F77TransToCBLASTrans(transa,&TransA) < 0)           info =  3;
    if (F77UploToCBlasUplo(uplo,&Uplo) < 0)                 info =  2;
    if (F77SideToCBlasSide(side, &Side) < 0)                info =  1;
    if (info != 0) {
        xerbla_(ERROR_NAME, &info);
        return;
    }
    Blasx_Debug_Output("CBLASColMajor=%d Side=%d Uplo=%d TransA=%d Diag=%d m=%d n=%d alpha=%p A=%p lda=%d B=%p ldb=%d\n",CblasColMajor,Side,Uplo,TransA,Diag,*m,*n,alpha,A,*lda,B,*ldb);
    cblas_ztrsm(CblasColMajor, Side, Uplo,
                TransA, Diag, *m, *n,
                (blasxDoubleComplex *)alpha, (blasxDoubleComplex *)A, *lda,
                (blasxDoubleComplex *)B, *ldb);
}
