/* blas/source_syrk_r.h
 * 
 * Copyright (C) 2001 Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

{
    size_t i, j, k;
    int uplo, trans;

    if (alpha == 0.0 && beta == 1.0)
	return;

    if (Order == CblasRowMajor)
      {
        uplo = Uplo;
        trans = Trans;
      }
    else
      {
        uplo = (Uplo == CblasUpper) ? CblasLower : CblasUpper;
        trans = (Trans == CblasNoTrans) ? CblasTrans : CblasNoTrans;
      }

    /* form  C := beta*C */
    if (beta == 0.0) {
      if (uplo == CblasUpper) {
        for (i = 0; i < N; i++) {
          for (j = i; j < N; j++) {
            C[ldc * i + j] = 0.0;
          }
        }
      } else {
        for (i = 0; i < N; i++) {
          for (j = 0; j <= i; j++) {
            C[ldc * i + j] = 0.0;
          }
        }
      }
    } else if (beta != 1.0) {
      if (uplo == CblasUpper) {
        for (i = 0; i < N; i++) {
          for (j = i; j < N; j++) {
            C[ldc * i + j] *= beta;
          }
        }
      } else {
        for (i = 0; i < N; i++) {
          for (j = 0; j <= i; j++) {
            C[ldc * i + j] *= beta;
          }
        }
      }
    }

    if (alpha == 0.0)
	return;

    if (uplo == CblasUpper && trans == CblasNoTrans) {
 
      for (i = 0; i < N; i++) {
        for (j = i; j < N; j++) {
          BASE temp = 0.0;
          for (k = 0 ; k < K; k++) {
            temp += (A[i * lda + k] * B[j * lda + k] 
                     + B[i * lda + k] * A[j * lda + k]);
          }
          C[i * ldc + j] += alpha * temp;
        }
      }
      
    } else if (uplo == CblasUpper && trans == CblasTrans) {

      for (k = 0 ; k < K; k++) {
        for (i = 0; i < N; i++) {
          BASE temp1 = alpha * A[k * lda + i];
          BASE temp2 = alpha * B[k * lda + i];
          for (j = i; j < N; j++) {
            C[i * lda + j] += temp1 * B[k * lda + j] + temp2 * A[k *lda + j];
          }
        }
      }

    } else if (uplo == CblasLower && trans == CblasNoTrans) {


      for (i = 0; i < N; i++) {
        for (j = 0; j <= i; j++) {
          BASE temp = 0.0;
          for (k = 0 ; k < K; k++) {
            temp += (A[i * lda + k] * B[j * lda + k] 
                     + B[i * lda + k] * A[j * lda + k]);
          }
          C[i * ldc + j] += alpha * temp;
        }
      }

    } else if (uplo == CblasLower && trans == CblasTrans) {

      for (k = 0 ; k < K; k++) {
        for (i = 0; i < N; i++) {
          BASE temp1 = alpha * A[k * lda + i];
          BASE temp2 = alpha * B[k * lda + i];
          for (j = 0; j <= i; j++) {
            C[i * lda + j] += temp1 * B[k * lda + j] + temp2 * A[k *lda + j];
          }
        }
      }


    } else {
      BLAS_ERROR("unrecognized operation");
    }
}


