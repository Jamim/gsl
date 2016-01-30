/* gsl_multilarge_nlin.h
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

#ifndef __GSL_MULTILARGE_NLINEAR_H__
#define __GSL_MULTILARGE_NLINEAR_H__

#include <gsl/gsl_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_types.h>
#include <gsl/gsl_multilarge.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

__BEGIN_DECLS

typedef struct
{
  int (* f) (const gsl_vector * x, void * params, gsl_vector * f);
  int (* df) (const gsl_vector * x, const gsl_vector * y, void * params,
              gsl_vector * JTy, gsl_matrix * JTJ);
  int (* fvv) (const gsl_vector * x, const gsl_vector * v,
               void * params, gsl_vector * fvv);
  size_t n;              /* number of residuals */
  size_t p;              /* number of independent variables */
  void * params;         /* user parameters */
  size_t nevalf;         /* number of evaluations of f */
  size_t nevaldf;        /* number of evaluations of J^T J */
  size_t nevaldff;       /* number of evaluations of J^T y */
  size_t nevalfvv;       /* number of evaluations of J^T y */
} gsl_multilarge_nlinear_fdf;

/* scaling matrix specification */
typedef struct
{
  const char *name;
  int (*init) (const gsl_matrix * J, gsl_vector * dtd);
  int (*update) (const gsl_matrix * J, gsl_vector * dtd);
} gsl_multilarge_nlinear_scale;

/* tunable parameters for Levenberg-Marquardt method */
typedef struct
{
  const gsl_multilarge_nlinear_scale *scale;  /* scaling method */
  int accel;                                  /* use geodesic acceleration */
  double avmax;                               /* maximum |a| / |v| */
  double h_fvv;                               /* step size for finite difference fvv */
} gsl_multilarge_nlinear_parameters;

typedef struct
{
  const char *name;
  void * (*alloc) (const gsl_multilarge_nlinear_parameters * params,
                   const size_t n, const size_t p);
  int (*init) (gsl_multilarge_nlinear_fdf * fdf,
               const gsl_vector * x, gsl_vector * f,
               gsl_vector * g, gsl_matrix * JTJ, void * vstate);
  int (*iterate) (gsl_multilarge_nlinear_fdf * fdf,
                  gsl_vector * x, gsl_vector * f,
                  gsl_matrix * JTJ, gsl_vector * g,
                  gsl_vector * dx, double * avratio, void * vstate);
  int (*rcond) (const gsl_matrix * JTJ, double * rcond, void *vstate);
  void (*free) (void * vstate);
} gsl_multilarge_nlinear_type;

typedef struct
{
  const gsl_multilarge_nlinear_type * type;
  gsl_multilarge_nlinear_fdf * fdf;
  gsl_vector * x;        /* parameter values x */
  gsl_vector * dx;       /* step dx */
  gsl_vector * f;        /* residual vector f(x) */
  gsl_vector * g;        /* gradient vector J^T f */
  gsl_matrix * JTJ;      /* J^T J */
  double avratio;        /* |a| / |v| */
  size_t n;              /* number of residuals */
  size_t p;              /* number of model parameters */
  size_t niter;          /* number of iterations performed */
  void *state;           /* solver workspace */
} gsl_multilarge_nlinear_workspace;

/* available solvers */
GSL_VAR const gsl_multilarge_nlinear_type * gsl_multilarge_nlinear_lm;

gsl_multilarge_nlinear_workspace *
gsl_multilarge_nlinear_alloc (const gsl_multilarge_nlinear_type * T,
                              const gsl_multilarge_nlinear_parameters * params,
                              const size_t n, const size_t p);

void gsl_multilarge_nlinear_free (gsl_multilarge_nlinear_workspace * w);

gsl_multilarge_nlinear_parameters gsl_multilarge_nlinear_default_parameters(void);

const char *gsl_multilarge_nlinear_name (const gsl_multilarge_nlinear_workspace * w);

size_t gsl_multilarge_nlinear_niter (const gsl_multilarge_nlinear_workspace * w);

int
gsl_multilarge_nlinear_init (const gsl_vector * x, gsl_multilarge_nlinear_fdf * fdf,
                             gsl_multilarge_nlinear_workspace * w);

int gsl_multilarge_nlinear_iterate (gsl_multilarge_nlinear_workspace * w);

int gsl_multilarge_nlinear_rcond (double * rcond, const gsl_multilarge_nlinear_workspace * w);

gsl_vector *gsl_multilarge_nlinear_position (const gsl_multilarge_nlinear_workspace * w);

gsl_vector *gsl_multilarge_nlinear_residual (const gsl_multilarge_nlinear_workspace * w);

gsl_vector *gsl_multilarge_nlinear_step (const gsl_multilarge_nlinear_workspace * w);

gsl_matrix *gsl_multilarge_nlinear_JTJ (const gsl_multilarge_nlinear_workspace * w);

double gsl_multilarge_nlinear_avratio (const gsl_multilarge_nlinear_workspace * w);

int
gsl_multilarge_nlinear_driver (const size_t maxiter,
                               const double xtol,
                               const double gtol,
                               const double ftol,
                               void (*callback)(const size_t iter, void *params,
                                                const gsl_multilarge_nlinear_workspace *w),
                               void *callback_params,
                               int *info,
                               gsl_multilarge_nlinear_workspace * w);

int gsl_multilarge_nlinear_applyW(const gsl_vector * w, gsl_matrix * J, gsl_vector * f);

int
gsl_multilarge_nlinear_eval_f(gsl_multilarge_nlinear_fdf *fdf,
                              const gsl_vector *x, gsl_vector *f);

int
gsl_multilarge_nlinear_eval_df(gsl_multilarge_nlinear_fdf *fdf,
                               const gsl_vector *x, const gsl_vector *y,
                               gsl_vector *JTy, gsl_matrix *JTJ);

int
gsl_multilarge_nlinear_eval_fvv(const double h, const gsl_vector *x, const gsl_vector *v,
                                const gsl_vector *g, const gsl_matrix *JTJ,
                                gsl_multilarge_nlinear_fdf *fdf,
                                gsl_vector *fvv, gsl_vector *JTfvv, gsl_vector *workp);

int gsl_multilarge_nlinear_test (const double xtol, const double gtol,
                                 const double ftol, int *info,
                                 const gsl_multilarge_nlinear_workspace * w);

/* fdfvv.c */
int
gsl_multilarge_nlinear_fdJTfvv(const double h, const gsl_vector *x, const gsl_vector *v,
                               const gsl_vector *g, const gsl_matrix *JTJ,
                               gsl_multilarge_nlinear_fdf *fdf,
                               gsl_vector *JTfvv, gsl_vector *workn, gsl_vector *workp);

/* scaling matrix strategies */
GSL_VAR const gsl_multilarge_nlinear_scale * gsl_multilarge_nlinear_scale_levenberg;
GSL_VAR const gsl_multilarge_nlinear_scale * gsl_multilarge_nlinear_scale_marquardt;
GSL_VAR const gsl_multilarge_nlinear_scale * gsl_multilarge_nlinear_scale_more;

/* regularized nonlinear least squares */

typedef struct
{
  size_t n;                          /* number of residuals of original problem */
  size_t p;                          /* number of model parameters */
  size_t m;                          /* number of rows in regularization matrix L */
  double lambda;                     /* regularization parameter */
  gsl_vector * diag;                 /* diagonal regularization matrix */

  gsl_multilarge_nlinear_fdf * fdf;  /* user defined fdf */
  gsl_multilarge_nlinear_fdf fdftik; /* Tikhonov modified fdf */

  gsl_multilarge_nlinear_workspace * multilarge_nlinear_p;
} gsl_multilarge_regnlinear_workspace;

gsl_multilarge_regnlinear_workspace *
gsl_multilarge_regnlinear_alloc (const gsl_multilarge_nlinear_type * T,
                                 const gsl_multilarge_nlinear_parameters * params,
                                 const size_t n, const size_t p, const size_t m);

void gsl_multilarge_regnlinear_free(gsl_multilarge_regnlinear_workspace *w);

const char * gsl_multilarge_regnlinear_name(const gsl_multilarge_regnlinear_workspace * w);

gsl_vector * gsl_multilarge_regnlinear_position (const gsl_multilarge_regnlinear_workspace * w);

gsl_vector * gsl_multilarge_regnlinear_residual (const gsl_multilarge_regnlinear_workspace * w);

gsl_matrix * gsl_multilarge_regnlinear_JTJ (const gsl_multilarge_regnlinear_workspace * w);

size_t gsl_multilarge_regnlinear_niter (const gsl_multilarge_regnlinear_workspace * w);

int
gsl_multilarge_regnlinear_init2 (const double lambda,
                                 const gsl_vector * L,
                                 const gsl_vector * x,
                                 gsl_multilarge_nlinear_fdf * f,
                                 gsl_multilarge_regnlinear_workspace * w);

int
gsl_multilarge_regnlinear_iterate (gsl_multilarge_regnlinear_workspace * w);

int
gsl_multilarge_regnlinear_driver (const size_t maxiter,
                                  const double xtol,
                                  const double gtol,
                                  const double ftol,
                                  void (*callback)(const size_t iter, void *params,
                                                   const gsl_multilarge_nlinear_workspace *w),
                                  void *callback_params,
                                  int *info,
                                  gsl_multilarge_regnlinear_workspace * w);

__END_DECLS

#endif /* __GSL_MULTILARGE_NLINEAR_H__ */
