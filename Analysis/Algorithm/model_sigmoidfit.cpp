#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>

#define ORDER 3
#define MAX_FIT_DATA 1024

typedef struct
{
    int nd;
    int start;
    double *dd;
    double *sigma;

}Info;
//-----------------------------------------------------------------------------
//--- expb_f_pro
//-----------------------------------------------------------------------------
int expb_f_pro(const gsl_vector * x, void *params, gsl_vector * f)
{
    size_t n = ((Info*)params)->nd;
    double *y = ((Info*)params)->dd;
    double *sigma = ((Info*)params)->sigma;
    int start = ((Info*)params)->start;

    double A = gsl_vector_get (x, 0);
    double lambda = gsl_vector_get (x, 1);
    double b = gsl_vector_get (x, 2);

    size_t i;
    for(i = 0; i < n; i++)
    {
        try
       {
        double t = i+start;
        double exparg =  (lambda-t)/b;
        if (exparg >= 100.) return GSL_ERANGE;
        double Yi = A/(1 + exp(exparg));
        gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
       }
        catch(...) {return GSL_EOVRFLW;}
    }
    return GSL_SUCCESS;
}
//-----------------------------------------------------------------------------
//--- expb_f
//-----------------------------------------------------------------------------
int expb_f (const gsl_vector * x, void *params, gsl_vector * f)
{
    size_t n = ((Info*)params)->nd;
    double *y = ((Info*)params)->dd;
    double *sigma = ((Info*)params)->sigma;
    int start = ((Info*)params)->start;
    double A = gsl_vector_get (x, 0);
    double lambda = gsl_vector_get (x, 1);
    double b = gsl_vector_get (x, 2);

    size_t i;
    for (i = 0; i < n; i++)
    {
        try
       {
        double t = i+start;
        double exparg =  (lambda-t)/b;
        if (exparg>=100.) return GSL_ERANGE;
        double Yi = A/(1 + exp(exparg));
        gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
       }
        catch(...) {return GSL_EOVRFLW;}
    }
    return GSL_SUCCESS;
}
//-----------------------------------------------------------------------------
//---   expb_df
//-----------------------------------------------------------------------------
int expb_df (const gsl_vector * x, void *params, gsl_matrix * J)
{

    size_t n = ((Info*)params)->nd;
    double *sigma = ((Info*)params)->sigma;
    int start = ((Info*)params)->start;

    size_t i;
    double A = gsl_vector_get (x, 0);
    double lambda = gsl_vector_get (x, 1);
    double b = gsl_vector_get (x, 2);

    for (i = 0; i < n; i++)
    {
        /* Jacobian matrix J(i,j) = dfi / dxj, */
        /* where fi = (Yi - yi)/sigma[i], */
        /* Yi = A/exp(lambda - i)/b */
        /* and the xj are the parameters (A,lambda,b) */

        try
       {
        double t = i+start;
        double s = sigma[i];
        double exparg=(lambda-t)/b;
        if (exparg>=100.) return GSL_ERANGE;
        double e = exp(exparg);

        gsl_matrix_set (J, i, 0, 1/((1+e)*s));
        gsl_matrix_set (J, i, 1, (-1*A*(e/b))/(s*pow(1+e,2)));
        gsl_matrix_set (J, i, 2, (A*e*(lambda-t))/(s*pow(1+e,2)*pow(b,2)));
       }
        catch(...) {return GSL_EOVRFLW;}
    }
    return GSL_SUCCESS;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int expb_fdf (const gsl_vector * x, void *params, gsl_vector * f, gsl_matrix * J)
{
    try
   {
    expb_f (x, params, f);
    expb_df (x, params, J);
   }
    catch(...) {return GSL_EOVRFLW;}
    return GSL_SUCCESS;
}

