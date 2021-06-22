#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>

#define ORDER 3
#define MAX_FIT_DATA 1024



typedef struct
{
    size_t n;
    double *y;
    double *t;
    double K_coef;
}Data;

//-----------------------------------------------------------------------------
double weib(double a, double l, double s, double k, double x)
{
    double res,xm;

    xm = x/l - s/l + 1;
    res = a*((k/l) * pow((xm),(k-1)) * exp(-pow((xm),k)));

    return res;
}
//-----------------------------------------------------------------------------
int expb_f (const gsl_vector * x, void *data, gsl_vector * f)
{
    size_t n = ((Data*)data)->n;
    double *y = ((Data*)data)->y;
    double *t = ((Data*)data)->t;
    double K = ((Data*)data)->K_coef;

    double A = gsl_vector_get (x, 0);
    double L = gsl_vector_get (x, 1);
    double S = gsl_vector_get (x, 2);
    size_t i;

    for (i=0; i<n; i++)
    {
        /* Model Weibull */


        double Yi = weib(A,L,S,K,t[i]);
        gsl_vector_set (f, i, Yi - y[i]);
    }

    return GSL_SUCCESS;
}
//-----------------------------------------------------------------------------

