#include <config.h>
#include <math.h>
#include <gsl_math.h>

#include "tests.h"

/* These are the test functions from table 4.1 of the QUADPACK book */

/* integ(book1,x,0,1) = 1/(alpha + 1)^2 */

double book1 (double x, void * params) {
  double alpha = *(double *) params ;
  return pow(x,alpha) * log(1/x) ;
}

/* integ(book2,x,0,1) = arctan((4-pi)4^(alpha-1)) + arctan(pi 4^(alpha-1)) */

double book2 (double x, void * params) {
  double alpha = *(double *) params ;
  return pow(4.0,-alpha) / (pow((x-M_PI/4.0),2.0) + pow(16.0,-alpha)) ;
}

/* integ(book3,x,0,pi) = pi J_0(2^alpha) */

double book3 (double x, void * params) {
  double alpha = *(double *) params ;
  return cos(pow(2.0,alpha) * sin(x)) ;
}

/* Functions 4, 5 and 6 are duplicates of functions  1, 2 and 3 */
/* ....                                                         */

/* integ(book7,x,0,1) = ((2/3)^(alpha+1) + (1/3)^(alpha+1))/(alpha + 1) */

double book7 (double x, void * params) {
  double alpha = *(double *) params ;
  return pow(fabs(x - (1.0/3.0)),alpha) ;
}

/* integ(book8,x,0,1) = 
   ((1 - pi/4)^(alpha+1) + (pi/4)^(alpha+1))/(alpha + 1) */

double book8 (double x, void * params) {
  double alpha = *(double *) params ;
  return pow(fabs(x - (M_PI/4.0)),alpha) ;
}

/* integ(book9,x,-1,1) = pi/sqrt((1+2^-alpha)^2-1) */

double book9 (double x, void * params) {
  double alpha = *(double *) params ;
  return 1 / ((x + 1 + pow(2.0,-alpha)) * sqrt(1-x*x)) ;
}

/* integ(book10,x,0,pi/2) = 2^(alpha-2) ((Gamma(alpha/2))^2)/Gamma(alpha) */

double book10 (double x, void * params) {
  double alpha = *(double *) params ;
  return pow(sin(x), alpha-1) ;
}

/* integ(book11,x,0,1) = Gamma(alpha) */

double book11 (double x, void * params) {
  double alpha = *(double *) params ;
  return pow(log(1/x), alpha-1) ;
}

/* integ(book12,x,0,1) = 
   (20 sin(2^alpha) - 2^alpha cos(2^alpha) + 2^alpha exp(-20))
   /(400 + 4^alpha) */

double book12 (double x, void * params) {
  double alpha = *(double *) params ;
  return exp(20*(x-1)) * sin(pow(2.0,alpha) * x) ;
}

/* integ(book13,x,0,1) = pi cos(2^(alpha-1)) J_0(2^(alpha-1))  */

double book13 (double x, void * params) {
  double alpha = *(double *) params ;
  return cos(pow(2.0,alpha)*x)/sqrt(x*(1-x)) ;
}

double book14 (double x, void * params) {
  double alpha = *(double *) params ;
  return exp(-pow(2.0,-alpha)*x)*cos(x)/sqrt(x) ;
}

double book15 (double x, void * params) {
  double alpha = *(double *) params ;
  return x*x * exp(-pow(2.0,-alpha)*x) ;
}

double book16 (double x, void * params) {
  double alpha = *(double *) params ;
  if (x==0 && alpha == 1) return 1 ;  /* make the function continuous in x */
  return pow(x,alpha-1)/pow((1+10*x),2.0) ;
}

double book17 (double x, void * params) {
  double alpha = *(double *) params ;
  return pow(2.0,-alpha)/(((x-1)*(x-1)+pow(4.0,-alpha))*(x-2)) ;
}


