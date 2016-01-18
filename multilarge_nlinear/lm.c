/* multilargenlin/lm.c
 * 
 * Copyright (C) 2015 Patrick Alken
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_multilarge_nlinear.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_blas.h>

#define DEBUG   0

/*
 * This module contains an implementation of the Levenberg-Marquardt
 * algorithm for nonlinear optimization problems. This implementation
 * closely follows the following work:
 *
 * [1] H. B. Nielsen, K. Madsen, Introduction to Optimization and
 *     Data Fitting, Informatics and Mathematical Modeling,
 *     Technical University of Denmark (DTU), 2010.
 */

typedef struct
{
  size_t n;                  /* number of residuals */
  size_t p;                  /* number of model parameters */
  gsl_matrix *A;             /* contains Cholesky decomposition of (J^T J + mu D^T D) */
  gsl_vector *x_trial;       /* trial parameter vector x + dx, size p */
  gsl_vector *f_trial;       /* trial residual vector f(x + dx), size n */
  gsl_vector *fvv;           /* D_v^2 f(x), size n */
  gsl_vector *JTfvv;         /* J^T fvv, size p */
  gsl_vector *vel;           /* geodesic velocity (standard LM step), size p */
  gsl_vector *acc;           /* geodesic acceleration, size p */
  gsl_vector *DTD;           /* diagonal elements of D^T D matrix added to normal equations, size p */
  gsl_vector *chol_D;        /* Cholesky scale factors, size p */
  gsl_vector *tau;           /* Householder scalars for QR decomposition, size p */
  gsl_vector *workp;         /* temporary workspace, size p */
  long nu;                   /* nu */
  double mu;                 /* LM damping parameter */
  double mu0;                /* initial scale factor for mu */
  int chol;                  /* 1 if Cholesky succeeds, 0 if not */

  const gsl_multilarge_nlinear_scale *scale;

  gsl_eigen_symm_workspace *eigen_p;

  double avratio;            /* current |a| / |v| */

  /* tunable parameters */

  int accel;                 /* use geodesic acceleration */
  double avmax;              /* max |a| / |v| */
  double h_fvv;              /* step size for finite difference fvv */
} lm_state_t;

#include "lmdiag.c"
#include "lmsolve.c"

#define GSL_LM_ONE_THIRD         (0.333333333333333)

static void *lm_alloc (const gsl_multilarge_nlinear_parameters *params,
                       const size_t n, const size_t p);
static void lm_free(void *vstate);
static int lm_init(gsl_multilarge_nlinear_fdf * fdf,
                   const gsl_vector * x, gsl_vector * f,
                   gsl_vector * g, gsl_matrix * JTJ, void * vstate);
static int lm_iterate(gsl_multilarge_nlinear_fdf * fdf,
                      gsl_vector * x, gsl_vector * f,
                      gsl_matrix * JTJ, gsl_vector * g,
                      gsl_vector * dx, void * vstate);
static int lm_rcond(const gsl_matrix * JTJ, double * rcond, void * vstate);
static int lm_init_mu(const gsl_matrix * JTJ, lm_state_t *state);
static void lm_trial_step(const gsl_vector * x, const gsl_vector * dx,
                          gsl_vector * x_trial);
static double lm_calc_rho(const double mu, const gsl_vector * v,
                          const gsl_vector * g, const gsl_vector * f,
                          const gsl_vector * f_trial, lm_state_t * state);
static int lm_check_step(const gsl_vector * v, const gsl_vector * g,
                         const gsl_vector * f, const gsl_vector * f_trial,
                         double * rho, lm_state_t * state);
static double lm_scaled_norm(const gsl_vector *DTD, const gsl_vector *a,
                             gsl_vector *work);

static void *
lm_alloc (const gsl_multilarge_nlinear_parameters *params,
          const size_t n, const size_t p)
{
  lm_state_t *state;

  if (p == 0)
    {
      GSL_ERROR_NULL ("p must be a positive integer",
                      GSL_EINVAL);
    }
  
  state = calloc(1, sizeof(lm_state_t));
  if (state == 0)
    {
      GSL_ERROR_NULL ("failed to allocate lm state", GSL_ENOMEM);
    }

  state->A = gsl_matrix_alloc(p, p);
  if (state->A == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for temporary A workspace", GSL_ENOMEM);
    }

  state->vel = gsl_vector_alloc(p);
  if (state->vel == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for vel", GSL_ENOMEM);
    }

  state->acc = gsl_vector_alloc(p);
  if (state->acc == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for acc", GSL_ENOMEM);
    }

  state->x_trial = gsl_vector_alloc(p);
  if (state->x_trial == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for x_trial", GSL_ENOMEM);
    }

  state->f_trial = gsl_vector_alloc(n);
  if (state->f_trial == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for f_trial", GSL_ENOMEM);
    }

  state->fvv = gsl_vector_alloc(n);
  if (state->fvv == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for fvv", GSL_ENOMEM);
    }

  state->JTfvv = gsl_vector_alloc(p);
  if (state->JTfvv == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for JTfvv", GSL_ENOMEM);
    }

  state->DTD = gsl_vector_alloc(p);
  if (state->DTD == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for DTD", GSL_ENOMEM);
    }

  state->chol_D = gsl_vector_alloc(p);
  if (state->chol_D == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for Cholesky scale factors", GSL_ENOMEM);
    }

  state->tau = gsl_vector_alloc(p);
  if (state->tau == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for Householder vector", GSL_ENOMEM);
    }

  state->workp = gsl_vector_alloc(p);
  if (state->workp == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for temporary p workspace", GSL_ENOMEM);
    }

  state->eigen_p = gsl_eigen_symm_alloc(p);
  if (state->eigen_p == NULL)
    {
      GSL_ERROR_NULL ("failed to allocate space for eigen workspace", GSL_ENOMEM);
    }

  state->n = n;
  state->p = p;
  state->mu0 = 1.0e-3;
  state->mu = 0.0;
  state->nu = 2;
  state->scale = params->scale;
  state->accel = params->accel;
  state->avmax = params->avmax;
  state->h_fvv = params->h_fvv;

  return state;
}

static void
lm_free(void *vstate)
{
  lm_state_t *state = (lm_state_t *) vstate;

  if (state->A)
    gsl_matrix_free(state->A);

  if (state->vel)
    gsl_vector_free(state->vel);

  if (state->acc)
    gsl_vector_free(state->acc);

  if (state->x_trial)
    gsl_vector_free(state->x_trial);

  if (state->f_trial)
    gsl_vector_free(state->f_trial);

  if (state->fvv)
    gsl_vector_free(state->fvv);

  if (state->JTfvv)
    gsl_vector_free(state->JTfvv);

  if (state->DTD)
    gsl_vector_free(state->DTD);

  if (state->chol_D)
    gsl_vector_free(state->chol_D);

  if (state->tau)
    gsl_vector_free(state->tau);

  if (state->workp)
    gsl_vector_free(state->workp);

  if (state->eigen_p)
    gsl_eigen_symm_free(state->eigen_p);

  free(state);
}

/*
lm_init()
  Initialize LM solver

Inputs: fdf    - user-supplied callback function
        x      - initial parameter vector
        f      - (output) f(x)
        g      - (output) gradient J^T f
        JTJ    - (output) J^T J
        vstate - local workspace
*/

static int
lm_init(gsl_multilarge_nlinear_fdf * fdf,
        const gsl_vector * x, gsl_vector * f,
        gsl_vector * g, gsl_matrix * JTJ, void * vstate)
{
  int status;
  lm_state_t *state = (lm_state_t *) vstate;

  /* compute f(x) */
  status = gsl_multilarge_nlinear_eval_f(fdf, x, f);
  if (status)
    return status;

  /* compute J^T J and J^T f at x */
  status = gsl_multilarge_nlinear_eval_df(fdf, x, f, g, JTJ);
  if (status)
    return status;

  /* initialize scaling matrix D^T D */
  (state->scale->init)(JTJ, state->DTD);

  /* initialize LM parameter mu */
  lm_init_mu(JTJ, state);

  gsl_vector_set_zero(state->vel);
  gsl_vector_set_zero(state->acc);

  state->avratio = 0.0;

  return GSL_SUCCESS;
}

/*
lm_iterate()
  Performe one iteration of LM solver with Nielsen
updating criteria for damping parameter.

Inputs: fdf      - user-supplied function
        x        - (input/output)
                   on input, current parameter vector
                   on output, new vector x <- x + dx
        f        - on input, f(x)
                   on output, f(x + dx)
        JTJ      - on input, J^T J at x
                   on output, J^T J at x + dx
        g        - on input, J^T f at x
                   on output, J^T f at x + dx
        dx       - (output) new step size dx, size p
        vstate   - workspace
*/

static int
lm_iterate(gsl_multilarge_nlinear_fdf * fdf,
           gsl_vector * x, gsl_vector * f,
           gsl_matrix * JTJ, gsl_vector * g,
           gsl_vector * dx, void * vstate)
{
  int status;
  lm_state_t *state = (lm_state_t *) vstate;
  const size_t p = state->p;
  gsl_vector *x_trial = state->x_trial; /* trial x + dx */
  gsl_vector *f_trial = state->f_trial; /* f(x + dx) */
  int foundstep = 0;                    /* found step dx */
  double rho;                           /* ratio [ F(x) - F(x + dx) ] / [ L(0) - L(dx) ] */
  int bad_steps = 0;                    /* number of consecutive rejected steps */
  size_t i;

  /* loop until we find an acceptable step dx */
  while (!foundstep)
    {
      /* compute Cholesky decomposition of (J^T J + mu D^T D) */
      status = lm_solve_decomp(state->mu, JTJ, state->DTD, state->A, state);
      if (status)
        return status;

      /* solve: (J^T J + mu D^T D) v = - J^T f */
      status = lm_solve(state->A, g, state->vel, state);
      if (status)
        return status;

      if (state->accel)
        {
          /* compute second directional derivative in velocity direction f_vv(x) and J^T f_vv(x) */
          status = gsl_multilarge_nlinear_eval_fvv(state->h_fvv, x, state->vel, g, JTJ,
                                                   fdf, state->fvv, state->JTfvv, state->workp);
          if (status)
            return status;

          /* solve: (J^T J + mu D^T D) a = - J^T fvv */
          status = lm_solve(state->A, state->JTfvv, state->acc, state);
          if (status)
            return status;
        }

      /* compute dx = v + 1/2 a */
      for (i = 0; i < p; ++i)
        {
          double vi = gsl_vector_get(state->vel, i);
          double ai = gsl_vector_get(state->acc, i);
          gsl_vector_set(dx, i, vi + 0.5 * ai);
        }

      /* compute x_trial = x + dx */
      lm_trial_step(x, dx, x_trial);

      /* compute f(x+dx) */
      status = gsl_multilarge_nlinear_eval_f(fdf, x_trial, f_trial);
      if (status)
        return status;

      status = lm_check_step(state->vel, g, f, f_trial, &rho, state);

      if (status == GSL_SUCCESS)
        {
          /* reduction in error, step acceptable */

          double b;

          /* update LM parameter */
          b = 2.0 * rho - 1.0;
          b = 1.0 - b*b*b;
          state->mu *= GSL_MAX(GSL_LM_ONE_THIRD, b);
          state->nu = 2;

          /* compute new JTJ(x+dx) and JTf(x+dx) */
          status = gsl_multilarge_nlinear_eval_df(fdf, x_trial, f_trial, g, JTJ);
          if (status)
            return status;

          /* update x <- x + dx */
          gsl_vector_memcpy(x, x_trial);

          /* update f <- f(x + dx) */
          gsl_vector_memcpy(f, f_trial);

          /* update diag(D^T D) if using scaling */
          (state->scale->update)(JTJ, state->DTD);

          foundstep = 1;
          bad_steps = 0;
        }
      else
        {
          long nu2;

          /* step did not reduce error, reject step */

          /* if more than 15 consecutive rejected steps, report no progress */
          if (++bad_steps > 15)
            return GSL_ENOPROG;

          state->mu *= (double) state->nu;

          nu2 = state->nu << 1; /* 2*nu */
          if (nu2 <= state->nu)
            {
              gsl_vector_view diag = gsl_matrix_diagonal(JTJ);

              /*
               * nu has wrapped around / overflown, reset mu and nu
               * to original values and break to force another iteration
               */
              state->nu = 2;

              if (state->scale)
                state->mu = state->mu0;
              else
                state->mu = state->mu0 * gsl_vector_max(&diag.vector);

              break;
            }

          state->nu = nu2;
        }
    }

  return GSL_SUCCESS;
}

static int
lm_rcond(const gsl_matrix * JTJ, double * rcond, void * vstate)
{
  lm_state_t *state = (lm_state_t *) vstate;
  int status;
  double eval_min, eval_max;
  gsl_vector *eval = state->workp; /* temporary workspace for eigenvalues */

  /* compute eigenvalues of A = J^T J */

  /* copy lower triangle of A to temporary workspace */
  gsl_matrix_tricpy('L', 1, state->A, JTJ);

  /* compute eigenvalues of A */
  status = gsl_eigen_symm(state->A, eval, state->eigen_p);
  if (status)
    return status;

  gsl_vector_minmax(eval, &eval_min, &eval_max);

  if (eval_max > 0.0 && eval_min > 0.0)
    {
      *rcond = sqrt(eval_min / eval_max);
    }
  else
    {
      /*
       * computed eigenvalues are not accurate, probably due
       * to rounding errors in forming J^T J
       */
      *rcond = 0.0;
    }

  return GSL_SUCCESS;
}

static int
lm_init_mu(const gsl_matrix * JTJ, lm_state_t *state)
{
  state->mu = state->mu0;

  if (state->scale == gsl_multilarge_nlinear_scale_levenberg)
    {
      /* when D = I, set mu = mu0 * max(diag(J^T J)) */
      gsl_vector_const_view d = gsl_matrix_const_diagonal(JTJ);
      double max;

      max = gsl_vector_max(&d.vector);
      state->mu *= max;
    }

  return GSL_SUCCESS;
}

/* compute x_trial = x + dx */
static void
lm_trial_step(const gsl_vector * x, const gsl_vector * dx,
              gsl_vector * x_trial)
{
  size_t i, N = x->size;

  for (i = 0; i < N; i++)
    {
      double dxi = gsl_vector_get (dx, i);
      double xi = gsl_vector_get (x, i);
      gsl_vector_set (x_trial, i, xi + dxi);
    }
}

/*
lm_calc_rho()
  Calculate ratio of actual reduction to predicted
reduction, given by Eq 4.4 of More, 1978.

Inputs: mu      - LM parameter
        v       - velocity vector (p in Eq 4.4)
        g       - gradient J^T f
        f       - f(x)
        f_trial - f(x + dx)
        state   - workspace
*/

static double
lm_calc_rho(const double mu, const gsl_vector * v,
            const gsl_vector * g, const gsl_vector * f,
            const gsl_vector * f_trial, lm_state_t * state)
{
  const double normf = gsl_blas_dnrm2(f);
  const double normf_trial = gsl_blas_dnrm2(f_trial);
  double rho;
  double actual_reduction;
  double pred_reduction;
  double u;
  double norm_Dp; /* || D p || */

  /* if ||f(x+dx)|| > ||f(x)|| reject step immediately */
  if (normf_trial >= normf)
    return -1.0;

  /* compute numerator of rho */
  u = normf_trial / normf;
  actual_reduction = 1.0 - u*u;

  /* compute || D p || */
  norm_Dp = lm_scaled_norm(state->DTD, v, state->workp);

  /*
   * compute denominator of rho; instead of computing J*v,
   * we note that:
   *
   * ||Jv||^2 + 2*mu*||Dv||^2 = mu*||Dv||^2 - v^T g
   * and g = J^T f
   */
  u = norm_Dp / normf;
  pred_reduction = mu * u * u;

  gsl_blas_ddot(v, g, &u);
  pred_reduction -= u / (normf * normf);

  if (pred_reduction > 0.0)
    rho = actual_reduction / pred_reduction;
  else
    rho = -1.0;

  return rho;
}

/*
lm_check_step()
  Check if a proposed step should be accepted or
rejected

Inputs: v       - proposed step velocity
        g       - gradient J^T f
        f       - f(x)
        f_trial - f(x + dx)
        rho     - (output) ratio of actual to predicted reduction
        state   - workspace

Return: GSL_SUCCESS to accept
        GSL_FAILURE to reject

Notes:
1) If using geodesic acceleration, state->avratio
   is updated with |a| / |v|
*/

static int
lm_check_step(const gsl_vector * v, const gsl_vector * g,
              const gsl_vector * f, const gsl_vector * f_trial,
              double * rho, lm_state_t * state)
{
  /* if using geodesic acceleration, check that |a|/|v| < alpha */
  if (state->accel)
    {
      double anorm = lm_scaled_norm(state->DTD, state->acc, state->workp);
      double vnorm = lm_scaled_norm(state->DTD, state->vel, state->workp);

      /* store |a| / |v| */
      state->avratio = anorm / vnorm;

      /* reject step if acceleration is too large compared to velocity */
      if (state->avratio > state->avmax)
        return GSL_FAILURE;
    }

  *rho = lm_calc_rho(state->mu, v, g, f, f_trial, state);

  /* if rho <= 0, the step does not reduce the cost function, reject */
  if (*rho <= 0.0)
    return GSL_FAILURE;

  return GSL_SUCCESS;
}

/* compute || diag(D) a || */
static double
lm_scaled_norm(const gsl_vector *DTD, const gsl_vector *a,
               gsl_vector *work)
{
  const size_t n = a->size;
  size_t i;

  for (i = 0; i < n; ++i)
    {
      double Di = gsl_vector_get(DTD, i);
      double ai = gsl_vector_get(a, i);

      gsl_vector_set(work, i, sqrt(Di) * ai);
    }

  return gsl_blas_dnrm2(work);
}

static const gsl_multilarge_nlinear_type lm_type =
{
  "lm",
  lm_alloc,
  lm_init,
  lm_iterate,
  lm_rcond,
  lm_free
};

const gsl_multilarge_nlinear_type *gsl_multilarge_nlinear_lm = &lm_type;
