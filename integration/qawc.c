#include <config.h>
#include <math.h>
#include <float.h>
#include <gsl_math.h>
#include <gsl_errno.h>
#include <gsl_integration.h>

#include "integration.h"
#include "util.c"

#include "qc25c.c"

int
gsl_integration_qawc (gsl_function * f,
		      const double a, const double b, const double c,
		      const double epsabs, const double epsrel,
		      const size_t limit,
		      gsl_integration_workspace * workspace,
		      double *result, double *abserr)
{
  double area, errsum;
  double result0, abserr0;
  double tolerance;
  size_t iteration = 0;
  int roundoff_type1 = 0, roundoff_type2 = 0, error_type = 0;
  int err_reliable;

  initialise (workspace, a, b);

  if (epsabs <= 0 && (epsrel < 50 * GSL_DBL_EPSILON || epsrel < 0.5e-28))
    {
      *result = 0;
      *abserr = 0;

      GSL_ERROR ("tolerance cannot be acheived with given epsabs and epsrel",
		 GSL_EBADTOL);
    };

  if (c == a || c == b) 
    {
      *result = 0;
      *abserr = 0;

      GSL_ERROR ("cannot integrate with singularity on endpoint", GSL_EINVAL);
    }      

  /* perform the first integration */

  qc25c (f, a, b, c, &result0, &abserr0, &err_reliable);

  set_initial_result (workspace, result0, abserr0);

  /* Test on accuracy */

  tolerance = GSL_MAX_DBL (epsabs, epsrel * fabs (result0));

  /* FIXME: I don't know why they added the test abserr < 0.01 |result| */

  if (abserr0 < tolerance && abserr0 < 0.01 * fabs(result0))
    {
      *result = result0;
      *abserr = abserr0;

      return GSL_SUCCESS;
    }
  else if (limit == 1)
    {
      *result = result0;
      *abserr = abserr0;

      GSL_ERROR ("a maximum of one iteration was insufficient", GSL_EMAXITER);
    }

  area = result0;
  errsum = abserr0;

  iteration = 1;

  do
    {
      double a1, b1, a2, b2;
      double a_i, b_i, r_i, e_i;
      double area1 = 0, area2 = 0, area12 = 0;
      double error1 = 0, error2 = 0, error12 = 0;
      int err_reliable1, err_reliable2;

      /* Bisect the subinterval with the largest error estimate */

      retrieve (workspace, &a_i, &b_i, &r_i, &e_i);

      a1 = a_i; 
      b1 = 0.5 * (a_i + b_i);
      a2 = b1;
      b2 = b_i;

      if (c > a1 && c < b1) 
	{
	  b1 = 0.5 * (c + b2) ;
	  a2 = b1;
	}
      else if (c > b1 && c < b2)
	{
	  b1 = 0.5 * (a1 + c) ;
	  a2 = b1;
	}

      qc25c (f, a1, b1, c, &area1, &error1, &err_reliable1);
      qc25c (f, a2, b2, c, &area2, &error2, &err_reliable2);

      area12 = area1 + area2;
      error12 = error1 + error2;

      errsum += (error12 - e_i);
      area += area12 - r_i;

      if (err_reliable1 && err_reliable2)
	{
	  double delta = r_i - area12;

	  if (fabs (delta) <= 1.0e-5 * fabs (area12) && error12 >= 0.99 * e_i)
	    {
	      roundoff_type1++;
	    }
	  if (iteration >= 10 && error12 > e_i)
	    {
	      roundoff_type2++;
	    }
	}

      tolerance = GSL_MAX_DBL (epsabs, epsrel * fabs (area));

      if (errsum > tolerance)
	{
	  if (roundoff_type1 >= 6 || roundoff_type2 >= 20)
	    {
	      error_type = 2;	/* round off error */
	    }

	  /* set error flag in the case of bad integrand behaviour at
	     a point of the integration range */

	  if (subinterval_too_small (a1, a2, b2))
	    {
	      error_type = 3;
	    }
	}

      update (workspace, a1, b1, area1, error1, a2, b2, area2, error2);

      retrieve (workspace, &a_i, &b_i, &r_i, &e_i);

      iteration++;

    }
  while (iteration < limit && !error_type && errsum > tolerance);

  *result = sum_results (workspace);
  *abserr = errsum;

  if (errsum <= tolerance)
    {
      return GSL_SUCCESS;
    }
  else if (error_type == 2)
    {
      GSL_ERROR ("roundoff error prevents tolerance from being achieved",
		 GSL_EROUND);
    }
  else if (error_type == 3)
    {
      GSL_ERROR ("bad integrand behavior found in the integration interval",
		 GSL_ESING);
    }
  else if (iteration == limit)
    {
      GSL_ERROR ("maximum number of subdivisions reached", GSL_EMAXITER);
    }

  /* FIXME: we get here if there was a NAN in the function evaluations */

  GSL_ERROR ("shouldn't happen", GSL_ESANITY);

}
