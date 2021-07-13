#include "algorithm.h"
#include <gsl/gsl_fit.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>


#include "model_sigmoidfit.cpp"

//-----------------------------------------------------------------------------
//--- find mean value
//-----------------------------------------------------------------------------
double Find_MeanValue(QVector<double> &y)
{
    double value = 0.;
    double *buf = &y[0];

    value = gsl_stats_mean(buf, 1, y.count());

    return(value);
}
//-----------------------------------------------------------------------------
//--- find mean value
//-----------------------------------------------------------------------------
double Find_StDeviation(QVector<double> &y)
{
    double value = 0.;
    double *buf = &y[0];

    value = gsl_stats_sd(buf, 1, y.count());

    return(value);
}

//-----------------------------------------------------------------------------
//--- FilterData_ByMedian
//-----------------------------------------------------------------------------
void FilterData_ByMedian(QVector<double> &buf, int step)
{
    int i;
    double val;
    int count;
    int id;

    int size = buf.size();

    QVector<double> buf_avr(size,0.);
    QVector<double> tmp;

    for(i=0; i<size; i++)
    {
        if(i < step || i >= (size-step))
        {
            buf_avr.replace(i, buf.at(i));
        }
        else
        {
            tmp = buf.mid(i-step, 2*step + 1);
            qSort(tmp);
            count = tmp.size();
            id = div(count,2).quot;
            if(div(count,2).rem) val = tmp.at(id);
            else
            {
                val = (double)(tmp.at(id) + tmp.at(id-1))/2.;
            }

            buf_avr.replace(i, val);
        }
        tmp.clear();
    }

    for(i=0; i<size; i++) buf.replace(i, buf_avr.at(i));

    return;
}

//-----------------------------------------------------------------------------
//--- FilterData_ByAverage
//-----------------------------------------------------------------------------
void FilterData_ByAverage(QVector<double> &buf, int average_step)
{
    int i;
    double val;

    int size = buf.size();

    QVector<double> buf_avr(size,0.);
    QVector<double> sigma(size,0.);
    QVector<double> tmp;


    for(i=0; i<size; i++)
    {
        if(i < average_step || i >= (size-average_step))
        {
            buf_avr.replace(i, buf.at(i));
        }
        else
        {
            tmp = buf.mid(i-average_step, 2*average_step + 1);
            val = Find_MeanValue(tmp);
            buf_avr.replace(i, val);
        }
        tmp.clear();
    }

    for(i=0; i<size; i++) buf.replace(i, buf_avr.at(i));

    /*for(i=0; i<size; i++)
    {
            if(buf_avr.at(i) == 0.) sigma.replace(i, 0.);
            else sigma.replace(i, fabs((buf_avr.at(i) - buf.at(i)) / buf_avr.at(i)));
    }


    for(i=0; i<size; i++)
    {
        if(i == 0) continue;
        if(i >= size-1) break; //continue;
        if(i < 5) continue;
        //if(sigma.at(i) >= max_sigma) {max_sigma = sigma.at(i); p_max = i;}
        if(sigma.at(i) > 0.05 &&
        sigma.at(i) > sigma.at(i-1) &&
        sigma.at(i) > sigma.at(i+1))// &&
        //sigma[i-1]/sigma[i] < 0.75 &&
        //sigma[i*1]/sigma[i] < 0.75)
        {
            buf.replace(i, buf_avr.at(i));
            i++;
        }
    }*/

}
//-----------------------------------------------------------------------------
//--- Analysis of Finish Flash
//-----------------------------------------------------------------------------
short Analysis_FinishFlash(QVector<double> &In_buf)
{
    short res = -1;
    QVector<double> tmp;
    double start_flash, finish_flash;
    int count = In_buf.size();
    if(count < 15) return(res);

    tmp = In_buf.mid(10,3);
    start_flash = Find_MeanValue(tmp);

    tmp = In_buf.mid(count - 3,3);
    finish_flash = Find_MeanValue(tmp);

    if(/*start_flash >= 0 && */finish_flash > 0)
    {
        //res = (short)(qAbs(finish_flash - start_flash));
        res = (short)(finish_flash - start_flash);
    }    

    return(res);
}

//-----------------------------------------------------------------------------
//--- Analysis_PCR
//-----------------------------------------------------------------------------
POINT_TAKEOFF Analysis_PCR(QVector<double> &In_buf, QVector<double> &Out_buf)
{
    int i,j;

    Sigmoid sigmoid;
    double sigma, c0, c1;
    int i_start, count_linfit;
    int id;
    bool sts;
    int count = In_buf.count();
    QVector<double> buf, buf_calc;

    POINT_TAKEOFF *pp_OFF;
    POINT_TAKEOFF res_pOFF;
    //--- init struct ---
    res_pOFF.cp_Sigmoid = 0;
    res_pOFF.valid = false;
    res_pOFF.real_cp = 0;
    res_pOFF.real_Fluor_cp = 0;
    res_pOFF.Fluor_Cp = 0;
    res_pOFF.Criteria = 0;
    res_pOFF.real_ct = 0.;
    //--------------------

    double eff = 1. + (double)sigmoid.preference.count_eff/10.;

    //--- 1. Find Point_OFF ---
    for(i=0; i<count; i++)
    {
        if(sigmoid.preference.point_start > i) continue;               //false start
        if(i > (count - sigmoid.preference.count_depth - 1)) break;    //exit

        //--- i_start ---
        if(i > sigmoid.preference.count_LF) i_start = i + 1 - sigmoid.preference.count_LF;
        else i_start = 0;

        //--- count_linfit ---
        count_linfit = i - i_start + 1;

        buf = In_buf.mid(i_start,count_linfit);
        sigma = Do_LinFit(buf, c0, c1, i_start);

        if(sigma < 0.5) sigma = 0.5;

        //--- check on exponential growing ---
        sts = true;
        i_start = i+1;
        buf.clear();
        buf_calc.clear();
        for(j=0; j<sigmoid.preference.count_depth; j++)
        {
            buf_calc.append(sigma * pow(eff,j+1));
            buf.append(In_buf[i_start+j] - (c0 + c1*(i_start+j)));
            if(buf_calc[j] > buf[j])
            {
                sts = false;
                break;
            }
            if(j)
            {
                if((buf[j] < buf[j-1]) || (In_buf[i_start+j] < In_buf[i_start+j-1]))
                {
                    sts = false;
                    break;
                }
            }
        }

        if(sts)
        {
            if((In_buf[i_start + sigmoid.preference.count_depth - 1] -
                In_buf[i_start])/In_buf[i_start] < 0.05)   sts = false;
        }

        //--- add point OFF ---
        if(sts)
        {
            pp_OFF = new POINT_TAKEOFF;
            pp_OFF->i_start = i_start;
            pp_OFF->sigma = sigma;
            pp_OFF->c0 = c0;
            pp_OFF->c1 = c1;
            sigmoid.list_pointOFF.append(pp_OFF);

            //--- Fitting_Sigmoid ---

            Fitting_Sigmoid(In_buf, &sigmoid);
        }
    }

    //--- 2. Find the best point_OFF
    id = Find_BestPoint_OFF(&sigmoid, 3);
    if(id < 0) id = Find_BestPoint_OFF(&sigmoid, 1);
    sigmoid.best_pointOFF = id;    

    if(id >= 0) res_pOFF = *sigmoid.list_pointOFF.at(id);

    //--- 4.
    Create_FittingCurve(In_buf, Out_buf, &sigmoid);


    return(res_pOFF);
}
//-----------------------------------------------------------------------------
//--- Analysis_PCR_Ex
//-----------------------------------------------------------------------------
POINT_TAKEOFF Analysis_PCR_Ex(QVector<double> &In_buf, QVector<double> &Out_buf,
                              Sigmoid *s, int criterion_PosRes)
{
    int i,j;

    Sigmoid *sigmoid;
    double sigma, c0, c1;
    int i_start, count_linfit;
    int id;
    bool sts;
    bool sts_pOFF = false;
    int count = In_buf.count();
    QVector<double> buf, buf_calc;

    sigmoid = new Sigmoid();

    //... s->preference ...
    if(s)
    {
        //sigmoid->preference = s->preference;
    }

    POINT_TAKEOFF *pp_OFF;
    POINT_TAKEOFF res_pOFF;
    //--- init struct ---
    res_pOFF.cp_Sigmoid = 0;
    res_pOFF.valid = false;
    res_pOFF.real_cp = 0;
    res_pOFF.real_Fluor_cp = 0;
    res_pOFF.Fluor_Cp = 0;
    res_pOFF.Criteria = 0;
    res_pOFF.real_ct = 0.;
    //--------------------

    //double eff = 1. + (double)sigmoid.preference.count_eff/10.;
    double eff = 1. + (double)criterion_PosRes/10.;

    //--- 1. Find Point_OFF ---
    //qDebug() << "Find Point_OFF: ";

    //while(!sts_pOFF && eff > 1.5)
    //{
        for(i=0; i<count; i++)
        {
            if(sigmoid->preference.point_start > i) continue;               //false start
            if(i > (count - sigmoid->preference.count_depth - 1)) break;    //exit

            //--- i_start ---
            if(i > sigmoid->preference.count_LF) i_start = i + 1 - sigmoid->preference.count_LF;
            else i_start = 0;

            //--- count_linfit ---
            count_linfit = i - i_start + 1;

            buf = In_buf.mid(i_start,count_linfit);
            sigma = Do_LinFit(buf, c0, c1, i_start);

            if(sigma < 0.5) sigma = 0.5;

            //--- check on exponential growing ---
            sts = true;
            i_start = i+1;
            buf.clear();
            buf_calc.clear();
            for(j=0; j<sigmoid->preference.count_depth; j++)
            {
                buf_calc.append(sigma * pow(eff,j+1));
                buf.append(In_buf[i_start+j] - (c0 + c1*(i_start+j)));
                if(buf_calc[j] > buf[j])
                {
                    sts = false;
                    break;
                }
                if(j)
                {
                    if((buf[j] < buf[j-1]) || (In_buf[i_start+j] < In_buf[i_start+j-1]))
                    {
                        sts = false;
                        break;
                    }
                }
            }

            if(sts)
            {
                if((In_buf[i_start + sigmoid->preference.count_depth - 1] -
                    In_buf[i_start])/In_buf[i_start] < 0.05)   sts = false;
            }

            //--- add point OFF ---
            if(sts)
            {
                pp_OFF = new POINT_TAKEOFF;
                pp_OFF->i_start = i_start;
                pp_OFF->sigma = sigma;
                pp_OFF->c0 = c0;
                pp_OFF->c1 = c1;
                sigmoid->list_pointOFF.append(pp_OFF);

                //qDebug() << "count pp_OFF: " << sigmoid.list_pointOFF.size();

                //--- Fitting_Sigmoid ---

                Fitting_Sigmoid_Pro(In_buf, sigmoid);
            }
            //if(sts) sts_pOFF = true;
        }

        //if(!sts_pOFF) eff -= 0.1;   // reduce the efficiency
    //}

    //--- 2. Find the best point_OFF
    //qDebug() << "Find the best point_OFF: ";

    id = Find_BestPoint_OFF(sigmoid, 3);
    if(id < 0) id = Find_BestPoint_OFF(sigmoid, 1);
    sigmoid->best_pointOFF = id;

    if(id >= 0) res_pOFF = *sigmoid->list_pointOFF.at(id);

    //--- 4.
    //qDebug() << "FittingCurve: ";

    Create_FittingCurve(In_buf, Out_buf, sigmoid);

    //--- 5.
    if(s)
    {
        qDeleteAll(s->list_pointOFF.begin(), s->list_pointOFF.end());
        s->list_pointOFF.clear();
        for(i=0; i<sigmoid->list_pointOFF.size(); i++)
        {
            pp_OFF = new POINT_TAKEOFF;
            *pp_OFF = *sigmoid->list_pointOFF.at(i);
            s->list_pointOFF.append(pp_OFF);
        }
        s->best_pointOFF = sigmoid->best_pointOFF;
    }

    delete sigmoid;

    //qDebug() << "return: " << res_pOFF.real_cp;

    return(res_pOFF);
}
//-----------------------------------------------------------------------------
//--- Linear fitting: f = c0 + c1*x
//-----------------------------------------------------------------------------
double Do_LinFit(QVector<double> &buf, double &coef_0, double &coef_1, double i_start)
{
    int i;
        int count = buf.count();
        QVector<double> fx, fy, fw;
        double c0,c1,cov00,cov01,cov11,chisq,val;

        fy.resize(count);
        fx.resize(count);
        fw.resize(count);

        for(i=0; i<count; i++)
        {
            fy[i] = buf[i];
            fx[i] = i_start + i;
            fw[i] = 1.0/exp(0.1*(count-1-i));
        }
        gsl_fit_wlinear(fx.data(),1,fw.data(),1,fy.data(),1,count,&c0,&c1,&cov00,&cov01,&cov11,&chisq);

        coef_0 = c0;
        coef_1 = c1;
        val = sqrt(fabs(chisq/count));

        fx.clear();
        fy.clear();
        fw.clear();

        return(val);
}

//-----------------------------------------------------------------------------
//--- Fitting_Sigmoid :  Y = C + A / (1 + exp((x0-x)/b))
//-----------------------------------------------------------------------------
bool Fitting_Sigmoid_Pro(QVector<double> &buf, Sigmoid *sigmoid)
{
    int i;
    double A,x0,b,C;
    double chi, Cp;
    double f0, e0;
    double value_exp;
    bool res = true;
    unsigned int iter;
    int status, information;
    int num = sigmoid->list_pointOFF.count() - 1;
    if(num < 0) return(false);

    POINT_TAKEOFF *pp_off = sigmoid->list_pointOFF.at(num);
    pp_off->valid = true;

    double coef_reduceCp = 2.;

    double sigma = pp_off->sigma;
    int Ct = (int)pp_off->i_start;
    double c0 = pp_off->c0;
    double c1 = pp_off->c1;

    int count = buf.count();
    QVector<double> Y;
    QVector<double> Sigma;
    Info inf;
    double chi0, dev;

    int nd = sigmoid->preference.count_SF;
    if(nd > count - Ct) nd = count - Ct;        // count of fitting points

    for(i=0; i<nd; i++)
    {
        Y.append(buf.at(Ct+i) - (c0+c1*(Ct+i)));        // Base-Line
        Sigma.append((i<13)?sigma:(sigma*(i-12)));      //  Sigma
    }
    A = buf.at(count-1);
    x0 = Ct+nd-1;
    for(i=0; i<nd; i++) if(Y.at(i) > A/2) {x0 = Ct+i; break;}
    b = 1.4;
    C = 0.;

    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver *s;
    gsl_matrix *J = gsl_matrix_alloc(nd, ORDER);
    gsl_matrix *covar = gsl_matrix_alloc(ORDER, ORDER);    // allocation memory
    gsl_multifit_function_fdf f;
    gsl_vector *res_f;
    double x_init[3];

    const double xtol = 1e-8;
    const double gtol = 1e-8;
    const double ftol = 0.0;

    while(1)
    {
        inf.dd = Y.data();
        inf.sigma = Sigma.data();
        inf.nd = nd;
        inf.start = Ct;

        x_init[0] = A;
        x_init[1] = x0;
        x_init[2] = b;

        gsl_vector_view x = gsl_vector_view_array(x_init, ORDER);

        f.f = &expb_f_pro;
        f.df = NULL;
        f.n = nd;
        f.p = ORDER;
        f.params = &inf;

        iter = 0;

        s = gsl_multifit_fdfsolver_alloc(T, nd, ORDER);

        // initialize solver with starting point
        gsl_multifit_fdfsolver_set(s, &f, &x.vector);

        /* compute initial residual norm */
        res_f = gsl_multifit_fdfsolver_residual(s);
        chi0 = gsl_blas_dnrm2(res_f);

        /* solve the system with a maximum of 100 iterations */
        status = gsl_multifit_fdfsolver_driver(s, 100, xtol, gtol, ftol, &information);

        gsl_multifit_fdfsolver_jac(s, J);
        gsl_multifit_covar (J, 0.0, covar);

        /* compute final residual norm */
        chi = gsl_blas_dnrm2(res_f);
        dev = pow(chi, 2.0)/(nd - ORDER);

        //qDebug() << "chi, dev: " << chi << dev << gsl_blas_dnrm2(s->f);

        //--- get fit result ---
        A = gsl_vector_get(s->x, 0);
        x0 = gsl_vector_get(s->x, 1);
        b = gsl_vector_get(s->x, 2);

        //chi = gsl_blas_dnrm2(s->f);
        if((nd - ORDER) > 0 && chi < 10000) chi = pow(chi,2)/(nd-ORDER);
        else chi = -1;

        //try
        //{
            value_exp = exp(x0/b);
            //if(value_exp == HUGE_VAL)
            if(qIsInf(value_exp))
            {
                gsl_multifit_fdfsolver_free(s);
                gsl_matrix_free(covar);
                gsl_matrix_free(J);

                res = false;
                return(res);
                //break;
            }
            f0 = A/(1+value_exp);
            e0 = (1+exp((1+x0)/b))/(1+exp(x0/b)) - 1;
        //}
        /*catch(...)
        {
            res = false;
            break;
        }*/

        pp_off->e0_Sigmoid = e0;
        pp_off->f0_Sigmoid = f0;
        pp_off->A_Sigmoid = A;
        pp_off->x0_Sigmoid = x0;
        pp_off->b_Sigmoid = b;
        pp_off->chi_Sigmoid = chi;

#define ERR(i) sqrt(gsl_matrix_get(covar,i,i))

        pp_off->cpdev_Sigmoid = sqrt(pow(ERR(2)/b,2) + pow(ERR(1)/x0,2));

        /*if((pp_off->cpdev_Sigmoid > 0.01 || chi > 2) && nd > 8)
        {
            nd--;
            continue;
        }
        else break;*/

        break;

    }

    pp_off->point_fit = nd;                 // real count points of fitting
    pp_off->ct_Sigmoid = pp_off->i_start;   // Ct

    // Safety check
    if(pp_off->b_Sigmoid > 1000) pp_off->b_Sigmoid = 1000;

    Cp = (pp_off->x0_Sigmoid - 1.317 * pp_off->b_Sigmoid);

    if(Cp > buf.count())
    {
        Cp = buf.count();
        pp_off->valid = false;
    }


    //--- real Cp:
    pp_off->real_cp = Cp;
    pp_off->real_Fluor_cp = pp_off->A_Sigmoid /(1 + exp((pp_off->x0_Sigmoid - Cp)/pp_off->b_Sigmoid));

    Cp = Change_CpLevel(A,x0,b,Cp,coef_reduceCp);

    pp_off->cp_Sigmoid = Cp;
    pp_off->Fluor_Cp = pp_off->A_Sigmoid /(1 + exp((pp_off->x0_Sigmoid - Cp)/pp_off->b_Sigmoid));

    pp_off->cp_Sigmoid += 1.;

    //-----------------------------
    gsl_multifit_fdfsolver_free(s);
    gsl_matrix_free(covar);
    gsl_matrix_free(J);
    //-----------------------------

    return(res);
}

//-----------------------------------------------------------------------------
//--- Fitting_Sigmoid :  Y = C + A / (1 + exp((x0-x)/b))
//-----------------------------------------------------------------------------
bool Fitting_Sigmoid(QVector<double> &buf, Sigmoid *sigmoid)
{
    int i;
    double A,x0,b,C;
    double chi, Cp;
    double f0, e0;
    double value_exp;
    bool res = true;
    unsigned int iter;
    int status;
    int num = sigmoid->list_pointOFF.count() - 1;
    if(num < 0) return(false);

    POINT_TAKEOFF *pp_off = sigmoid->list_pointOFF.at(num);
    pp_off->valid = true;

    double coef_reduceCp = 2.;

    double sigma = pp_off->sigma;
    int Ct = (int)pp_off->i_start;
    double c0 = pp_off->c0;
    double c1 = pp_off->c1;

    int count = buf.count();
    QVector<double> Y;
    QVector<double> Sigma;
    Info inf;

    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    gsl_multifit_function_fdf f;
    double x_init[3];

    int nd = sigmoid->preference.count_SF;
    if(nd > count - Ct) nd = count - Ct;        // count of fitting points

    gsl_matrix *J = gsl_matrix_alloc(nd, ORDER);
    gsl_matrix *covar = gsl_matrix_alloc(ORDER, ORDER);    // allocation memory



    for(i=0; i<nd; i++)
    {
        Y.append(buf.at(Ct+i) - (c0+c1*(Ct+i)));           // Base-Line
        Sigma.append((i<13)?sigma:(sigma*(i-12)));      //  Sigma
    }
    A = buf.at(count-1);
    x0 = Ct+nd-1;
    for(i=0; i<nd; i++) if(Y.at(i) > A/2) {x0 = Ct+i; break;}
    b = 1.4;
    C = 0.;


    while(1)
    {
        //--- SetData ---
        inf.dd = Y.data();
        inf.sigma = Sigma.data();
        inf.nd = nd;
        inf.start = Ct;

        //--- Fitting(Test) ---

        x_init[0] = A;
        x_init[1] = x0;
        x_init[2] = b;

        gsl_vector_view x = gsl_vector_view_array (x_init, ORDER);

        f.f = &expb_f;
        f.df = &expb_df;
        f.fdf = &expb_fdf;
        f.n = inf.nd;
        f.p = ORDER;
        f.params = &inf;

        iter = 0;

        T = gsl_multifit_fdfsolver_lmsder;
        s = gsl_multifit_fdfsolver_alloc(T, nd, ORDER);
        gsl_multifit_fdfsolver_set(s, &f, &x.vector);

        do
        {
            iter++;
            status = gsl_multifit_fdfsolver_iterate(s);
            if(status) break;
            status = gsl_multifit_test_delta(s->dx, s->x, 0, 1e-5);
        }
        while(status == GSL_CONTINUE && iter < 1000);

        gsl_multifit_fdfsolver_jac(s, J);
        gsl_multifit_covar(J, 0.0, covar);

        //--- get fit result ---
        A = gsl_vector_get(s->x, 0);
        x0 = gsl_vector_get(s->x, 1);
        b = gsl_vector_get(s->x, 2);

        chi = gsl_blas_dnrm2(s->f);
        if((nd - ORDER) > 0 && chi < 10000) chi = pow(chi,2)/(nd-ORDER);
        else chi = -1;

        try
        {
            value_exp = exp(x0/b);
            if(value_exp == HUGE_VAL)
            {
                res = false;
                break;
            }
            f0 = A/(1+value_exp);
            e0 = (1+exp((1+x0)/b))/(1+exp(x0/b)) - 1;
        }
        catch(...)
        {
            res = false;
            break;
        }

        pp_off->e0_Sigmoid = e0;
        pp_off->f0_Sigmoid = f0;
        pp_off->A_Sigmoid = A;
        pp_off->x0_Sigmoid = x0;
        pp_off->b_Sigmoid = b;
        pp_off->chi_Sigmoid = chi;

#define ERR(i) sqrt(gsl_matrix_get(covar,i,i))

        pp_off->cpdev_Sigmoid = sqrt(pow(ERR(2)/b,2) + pow(ERR(1)/x0,2));

        if((pp_off->cpdev_Sigmoid > 0.01 || chi > 2) && nd > 8)
        {
            nd--;
            continue;
        }
        else break;
    }

    pp_off->point_fit = nd;                 // real count points of fitting
    pp_off->ct_Sigmoid = pp_off->i_start;   // Ct

    // Safety check
    if(pp_off->b_Sigmoid > 1000) pp_off->b_Sigmoid = 1000;

    Cp = (pp_off->x0_Sigmoid - 1.317 * pp_off->b_Sigmoid);

    if(Cp > buf.count())
    {
        Cp = buf.count();
        pp_off->valid = false;
    }


    //--- real Cp:
    pp_off->real_cp = Cp;
    pp_off->real_Fluor_cp = pp_off->A_Sigmoid /(1 + exp((pp_off->x0_Sigmoid - Cp)/pp_off->b_Sigmoid));

    Cp = Change_CpLevel(A,x0,b,Cp,coef_reduceCp);

    pp_off->cp_Sigmoid = Cp;
    pp_off->Fluor_Cp = pp_off->A_Sigmoid /(1 + exp((pp_off->x0_Sigmoid - Cp)/pp_off->b_Sigmoid));

    pp_off->cp_Sigmoid += 1.;


    //-----------------------------
    gsl_multifit_fdfsolver_free(s);
    gsl_matrix_free(covar);
    //-----------------------------

    return(res);

}

//-----------------------------------------------------------------------------
//---   Change_CpLevel
//-----------------------------------------------------------------------------
double Change_CpLevel(double A,
                      double x0,
                      double B,
                      double Cp,
                      double coef)

{
    int i=0;
    double a,b,x;
    double Y,Y0;
    double Buf_Cp = A/(1 + exp((x0 - Cp)/B));

    a = 1.;
    b = Cp;
    Buf_Cp /= coef;
    Y0 = Buf_Cp;

    while(i<100)
    {
        x = qMin(a,b) + fabs(b-a)/2.;

        Y = A/(1 + exp((x0 - x)/B));
        if(fabs(Y - Y0) < Y0/1000.) break;
        if(Y >= Y0) b = x;
        else        a = x;
        i++;
    }
    return(x);
}

//-----------------------------------------------------------------------------
//---   Find_BestPoint_OFF
//-----------------------------------------------------------------------------
int Find_BestPoint_OFF(Sigmoid *s, int count_border)
{
    int i;
    int res = -1;
    double CHI;
    double min_res, min;
    bool first = true;
    int count = s->list_pointOFF.count();

    POINT_TAKEOFF *pp_off;

    for(i=0; i<count; i++)
    {
        pp_off = s->list_pointOFF.at(i);

        pp_off->Criteria = 0;
        if(!pp_off->valid) continue;

        if(pp_off->chi_Sigmoid < 1.) CHI = 1.;
        else CHI = sqrt(pp_off->chi_Sigmoid);

        min_res = CHI * pp_off->cpdev_Sigmoid*100.;

        if(min_res > 1000) {pp_off->valid = false; continue;}       // !!!! new

        pp_off->Criteria = min_res;




        //--- real_Cp äîëæíà íàõîäèòñÿ íå áëèæå ÷åì "count_border" îò êðàÿ -------------------
        if((pp_off->ct_Sigmoid + pp_off->point_fit - pp_off->real_cp) < count_border) continue;
        //------------------------------------------------------------------------------------

        if(first) min = min_res;
        first = false;
        if(min_res <= min)
        {
            min = min_res;
            res = i;
        }
    }

    return(res);

}

//-----------------------------------------------------------------------------
//---   Create_FittingCurve
//-----------------------------------------------------------------------------
void Create_FittingCurve(QVector<double> &input_buf,
                         QVector<double> &output_buf,
                         Sigmoid *s)
{
    int i,j;
    int i_start, nd;
    int count = input_buf.count();
    QVector<double> x,y,y_spline,dy_spline,d2y_spline;
    int sts;
    double val_sigmoid;
    double weight;
    double val;

    //qDebug() << "fitting: start";

    output_buf.clear();

    //--- 1. Check on valid point_OFF ---
    if(s->best_pointOFF < 0 || s->best_pointOFF >= s->list_pointOFF.count())
    {
        output_buf.fill(0., count);
        //qDebug() << "fitting: Check on valid point_OFF";
        return;
    }

    //--- 2. Check on valid Ct ---
    POINT_TAKEOFF *pp_off = s->list_pointOFF.at(s->best_pointOFF);

    if(pp_off->ct_Sigmoid <= 0 || pp_off->real_cp <= 0)
    {
        output_buf.fill(0., count);
        //qDebug() << "fitting: Check on valid Ct";
        return;
    }

    //--- 3. Base Line ---
    //output_buf.fill(0., count);
    //qDebug() << "fitting: BaseLine";
    for(i=0; i<count; i++)
    {
        if(i < pp_off->ct_Sigmoid) output_buf.append(0.);
        else output_buf.append(input_buf.at(i) - (pp_off->c0 + pp_off->c1*i));     // Base_line
    }


    //--- 4. Spline after sigmoida ---
    //qDebug() << "fitting: Spline after sigmoida";
    i_start = pp_off->ct_Sigmoid + pp_off->point_fit;
    i_start -= 3;
    if((count - i_start) < 6) i_start = count - 6;

    y = output_buf.mid(i_start, count-i_start);
    for(i=0; i<count-i_start; i++) x.append(i);
    y_spline.fill(0,count-i_start);
    dy_spline.fill(0,count-i_start);
    d2y_spline.fill(0,count-i_start);

    //qDebug() << "fitting: SplineCube";
    //rho = 0.00002;
    //SplineCube(x, y, y_spline, dy_spline, d2y_spline, sts, 0.00001);   // !!!!!!!!!!!!!!! 22.05.2020
    y_spline.clear();
    foreach(val, y)
    {
        y_spline.append(val);
    }
                                                      // ..........................

    //--- 5. Sigmoida + Spline ---
    //qDebug() << "fitting: Sigmoida + Spline";
    nd = pp_off->ct_Sigmoid + pp_off->point_fit - i_start;
    j = 0;

    for(i=0; i<count; i++)
    {
         if(i < pp_off->ct_Sigmoid + pp_off->point_fit)
         {
            val_sigmoid = pp_off->A_Sigmoid /(1 + exp((pp_off->x0_Sigmoid - i)/pp_off->b_Sigmoid));

            if(i < i_start) output_buf.replace(i, val_sigmoid);
            else
            {
                weight = 1./(nd - 1.)  * j;
                output_buf.replace(i, y_spline.at(j) * weight  + val_sigmoid * (1.-weight));
            }
         }
         else
         {
            output_buf.replace(i, y_spline.at(j));
         }

         if(i >= i_start) j++;
    }
    //qDebug() << "fitting: stop";

    x.clear();
    y.clear();
    y_spline.clear();
    dy_spline.clear();
    d2y_spline.clear();
}
//-----------------------------------------------------------------------------
//--- SplineCube
//-----------------------------------------------------------------------------
void SplineCube(QVector<double> &x,         // Spline approximation
                QVector<double> &y,         // x,y - input data
                QVector<double> &y_spline,  // output - spline
                QVector<double> &dy_spline, // output - y'
                QVector<double> &d2y_spline,// output - y"
                int &output_sts,            // output - status
                double LambdaNS)
{
    using namespace alglib;

    int i;
    double d2s;

    real_1d_array X;        // X - points, array[0..N-1].
    real_1d_array Y;        // Y - function values, array[0..N-1].
    real_1d_array W;        // W - weights, array[0..N-1]
    ae_int_t info;          // info > 0 task is solved; info <= 0 an error occured
    spline1dinterpolant s;  // spline interpolant
    spline1dfitreport rep;  // report after calculation

    int N = y.count();      // count of nodes
    double M = N*1.1;       // the number of degrees of freedom (степень свободы)(2.5)
   //double rho = 2.0;       // coefficient of smoothing (сглаживание) [-2;+6] (2.0)
    //LambdaNS    -   LambdaNS>=0, regularization  constant  passed by user.
    //                It penalizes nonlinearity in the regression spline.
    //                Possible values to start from are 0.00001, 0.1, 1

    if(N<5)
    {
        output_sts = -1;
        return;
    }

    Y.setcontent(N,&y[0]);
    X.setcontent(N,&x[0]);    

    spline1dfit(X, Y, M, LambdaNS, s, rep);
    //spline1dfitpenalized(X, Y, M, rho, info, s, rep);   // spline approximation -> s (spline interpolant)

    for(i=0; i<N; i++)
    {
        spline1ddiff(s, x[i], y_spline[i], dy_spline[i], d2y_spline[i]);
    }
}
//-----------------------------------------------------------------------------
//--- roundTo
//-----------------------------------------------------------------------------
double roundTo(double inpValue, int inpCount)
{
    double outpValue;
    double tempVal;
    tempVal=inpValue*pow(10,inpCount);

    if(double(int(tempVal))+0.5==tempVal)
    {
        if(int(tempVal)%2==0) {outpValue=double(qFloor(tempVal))/pow(10,inpCount);}
        else {outpValue=double(qCeil(tempVal))/pow(10,inpCount);}
    }
    else
    {
        if(double(int(tempVal))+0.5>tempVal) {outpValue=double(qFloor(tempVal))/pow(10,inpCount);}
        else {outpValue=double(qCeil(tempVal))/pow(10,inpCount);}
    }
    return(outpValue);
}

//-----------------------------------------------------------------------------
//--- Threshold_Curve
//-----------------------------------------------------------------------------
double Threshold_Curve(QVector<double> &buf, double val)
{
    int i;
    int id = 0;
    double res = 0.;
    double y0,y1,x0,x1,tg_a;

    int count = buf.size();

    for(i=1; i<count; i++)
    {
        if(val < buf.at(i)) {id = i; break;}
    }
    if(id)
    {
        x1 = id+1;
        x0 = x1-1;
        y1 = buf.at(id);
        y0 = buf.at(id-1);
        tg_a = (y1-y0)/1.;

        res = x0 + (val - y0)/tg_a;
        res = roundTo(res,1);
    }

    return(res);
}

//-----------------------------------------------------------------------------
//--- Find_TemperaturePeaks
//-----------------------------------------------------------------------------
void Find_TemperaturePeaks(QVector<double> &two_peaks_x,    // result: two peaks for X
                           QVector<double> &two_peaks_y,    // result: two peaks for Y
                           QVector<double> &dy,             // the first derivative
                           QVector<double> &d2y,            // the second derivative
                           QVector<double> &x,              // x array
                           QVector<double> *param)          // parameters: percent_peaks, count_increase, border_offset, border_min ...
{
    int i,j;
    int N;
    int count = x.size();
    double value_Temp, value_dF;
    double value, z;
    double z1,z2;
    int id;
    int peaks_size;

    QVector<QPointF> Points_Peaks;
    QPointF peak;

    using namespace alglib;
    real_1d_array X;        // X - points, array[0..N-1].
    real_1d_array Y;        // Y - function values, array[0..N-1].
    spline1dinterpolant s;  // spline interpolant
    ae_int_t info;          // info > 0 task is solved; info <= 0 an error occured
    spline1dfitreport rep;  // report after calculation
    double M;
    double rho = 2.0;       // coefficient of smoothing (сглаживание) [-2;+6] (2.0)

    double dx;
    double Dx = 1.;
    if(count > 2) Dx = fabs(x.at(0)-x.at(1));

    double border_MinIncrease = 50;
    double percent_peaks = 15;
    double count_increase = 6;
    double border_offset = 7;       // offset: first ... points don't use
    double border_min = 1.;
    double border_FWHM = 30.;       // width in half height
    double border_OneDegree = 10.0;  // difference at 3 degree

    bool up_dfdt, down_dfdt;
    double max_dfdt;
    QVector<double> max_DfDt;

    max_DfDt.reserve((count_increase+1)*2);

    //... new algoritm ...

    // 1. index max element
    id = std::distance(dy.begin(), std::max_element(dy.begin()+2, dy.end()-2));
    if(d2y.at(id-1) > 0 && d2y.at(id+1) < 0 &&
       d2y.at(id-2) > 0 && d2y.at(id+2) < 0 &&
       dy.at(id) > border_min)
    {
        //... offset filter ...
        if(id < border_offset || id > (count-border_offset)) return;

        N = 5;
        X.setcontent(N,&x[id-2]);
        Y.setcontent(N,&dy[id-2]);
        spline1dbuildcubic(X, Y, s);                          // interpolation

        value = x.at(id-1);
        value_dF = 0.;
        value_Temp = value;
        while(value < x.at(id+1))
        {
            z = spline1dcalc(s,value);
            if(z > value_dF)
            {
                value_dF = z;
                value_Temp = value;
            }
            else break;
            value += 0.01;
        }
        if(value_dF > 0.)
        {
            two_peaks_y.replace(0, value_dF);
            two_peaks_x.replace(0, value_Temp);
        }
    }


    return;
    //...



    for(i=2; i<count-2; i++)
    {
        if(d2y.at(i-1) > 0 && d2y.at(i+1) < 0)
        {
            max_DfDt.clear();

            //... offset filter ...
            if(i < border_offset || i > (count-border_offset)) continue;

            //... first filter: value ...
            if(dy.at(i) < border_min) continue;

            //... Fitting (Not Interpolation) dy near x[i]: 5 points(i,i+1,i+2,i-1,i-2)
            N = 5;
            M = N*1.1;
            X.setcontent(N,&x[i-2]);
            Y.setcontent(N,&dy[i-2]);
            spline1dbuildcubic(X, Y, s);                          // interpolation
            //spline1dfitpenalized(X, Y, M, rho, info, s, rep);       // fitting


            value = x.at(i-1);
            value_dF = 0.;
            value_Temp = value;
            while(value < x.at(i+1))
            {
                z = spline1dcalc(s,value);
                if(z > value_dF)
                {
                    value_dF = z;
                    value_Temp = value;
                }
                else break;
                value += 0.01;
            }
            if(value_dF > 0.)
            {
                up_dfdt = true;
                down_dfdt = true;                

                // 1. increase...
                j = i-1;
                dx = Dx;
                while(j >= 0 && j >= (i-count_increase) && dx <= 3)
                {
                    if(d2y.at(j) <= 0) {up_dfdt = false; break;}
                    //max_DfDt.append(fabs(d2y.at(j)));
                    dx += Dx;
                    j--;
                }
                // 2. decrease...
                j = i+1;
                dx = Dx;
                while(j <= (count-1) && j <= (i+count_increase) && dx <= 3)
                {
                    if(d2y.at(j) >= 0) {down_dfdt = false; break;}
                    //max_DfDt.append(fabs(d2y.at(j)));
                    dx += Dx;
                    j++;
                }

                // 3.
                //max_dfdt = *std::max_element(max_DfDt.begin(),max_DfDt.end());
                //max_DfDt.clear();

                // second filter: increase,decrease peaks
                if((up_dfdt || down_dfdt)) // && max_dfdt > border_MinIncrease)
                {
                    peak.setX(value_Temp);
                    peak.setY(value_dF);
                    Points_Peaks.append(peak);
                }
            }
            i += 1;
        }
    }


    i=0;
    while(i<2 && i<Points_Peaks.size())
    {
        z = 0;
        id = -1;
        for(j=0; j<Points_Peaks.size(); j++)
        {
            if(Points_Peaks.at(j).y() > z)
            {
                z = Points_Peaks.at(j).y();
                id = j;                
            }
        }

        // third filter: second peak must be grater than 10% from first peak
        if(i == 1 && z > 0 && id >= 0)
        {
            if(z < (two_peaks_y.at(0)*percent_peaks/100.)) break;
        }
        //...

        if(z > 0 && id >= 0 /*&& i == 0*/) // !!! Only one peak... For two peaks
        {
            two_peaks_y.replace(i,Points_Peaks.at(id).y());
            peak.setX(Points_Peaks.at(id).x());
            peak.setY(0.);
            Points_Peaks.replace(id,peak);
            two_peaks_x.replace(i,Points_Peaks.at(id).x());
        }

        i++;
    }

    //qDebug() << Points_Peaks.size();

}
//-----------------------------------------------------------------------------
//--- Filter FWHM(Full Width at Half Maximum) for valid peaks
//-----------------------------------------------------------------------------
bool FilterPeak_FWHM(QVector<double> &X,    // X
                     QVector<double> &Y,    // Y
                     int pos,               // index
                     double border)
{
    int i;
    bool res = false;
    int count = X.size();
    double maximum = Y.at(pos);
    int left = 0;
    int right = count-1;

    //... left part ...
    i = pos;
    while(i>0)
    {
        if(Y.at(i) <= maximum/2.)
        {
            left = i;
            break;
        }
        i--;
    }

    //... right part ...
    i = pos;
    while(i<count)
    {
        if(Y.at(i) <= maximum/2.)
        {
            right = i;
            break;
        }
        i++;
    }

    if((X.at(right) - X.at(left)) < border) res = true;

    return(res);
}
//-----------------------------------------------------------------------------
//--- CP_SplineFitting(QVector<double> &Y)
//-----------------------------------------------------------------------------
QPointF CP_SplineFitting(QVector<double> &y)
{
    int i,id,index;
    int N;

    using namespace alglib;
    real_1d_array X;        // X - points, array[0..N-1].
    real_1d_array Y;        // Y - function values, array[0..N-1].
    spline1dinterpolant s;  // spline interpolant

    QPointF P;
    QVector<QPointF> vec_P;

    QVector<double> x,y_spline,dy_spline,d2y_spline;
    QVector<double> z_spline,dz_spline,d2z_spline;

    int sts;
    double value, value_F, value_Temp, z;
    int count = y.size();

    for(i=1; i<=count; i++) x.append(i);
    y_spline.fill(0,count);
    dy_spline.fill(0,count);
    d2y_spline.fill(0,count);
    z_spline.fill(0,count);
    dz_spline.fill(0,count);
    d2z_spline.fill(0,count);

    SplineCube(x,y,y_spline,dy_spline,d2y_spline,sts);

    // temporary ...
 /*
    P.setX(0);
    P.setY(0);
    //value = *std::max_element(d2y_spline.begin(), d2y_spline.end());
    id = std::distance(d2y_spline.begin(), std::max_element(d2y_spline.begin(), d2y_spline.end()));

    if(id < 2 || id > count-2) return(P);
    N = 5;
    X.setcontent(N,&x[id-2]);
    Y.setcontent(N,&d2y_spline[id-2]);
    spline1dbuildcubic(X, Y, s);
    for(i=0; i<20; i++)
    {
        value = x.at(id-1) + 0.1*i;
        z = spline1dcalc(s,value);
        y_spline_temp.append(z);
    }
    index = std::distance(y_spline_temp.begin(), std::max_element(y_spline_temp.begin(), y_spline_temp.end()));
    value = x.at(id-1) + 0.1*index;

    P.setX(value);
    return(P);
*/
    // ...

    SplineCube(x,dy_spline,z_spline,dz_spline,d2z_spline,sts);  //

    for(i=2; i<count-2; i++)
    {
        if(d2z_spline.at(i) > 0 && d2z_spline.at(i+1) < 0)
        {
            //qDebug() << "x: " << x.at(i);
            //... Interpolation dy near x[i]: 5 points(i,i+1,i+2,i-1,i-2)
            N = 5;
            X.setcontent(N,&x[i-2]);
            Y.setcontent(N,&dz_spline[i-2]);
            spline1dbuildcubic(X, Y, s);

            value = x.at(i-1);
            value_F = 0.;
            value_Temp = value;
            while(value < x.at(i+1))
            {
                z = spline1dcalc(s,value);
                if(z > value_F)
                {
                    value_F = z;
                    value_Temp = value;
                }
                else break;
                value += 0.1;
            }
            P.setX(value_Temp);
            P.setY(value_F);
            if(P.y() > 1.) vec_P.append(P);

            i += 2;
        }
    }

    //qDebug() << vec_P << y_spline;

    if(vec_P.isEmpty())
    {
        P.setX(0.);
        P.setY(0.);
        return(P);
    }

    value_F = 0.;
    id = 0;
    for(i=0; i<vec_P.size(); i++)
    {
        if(vec_P.at(i).y() > value_F)
        {
            value_F = vec_P.at(i).y();
            id = i;
        }
    }

    value = vec_P.at(id).x();
    id = 2;
    for(i=2; i<count-2; i++)
    {
        if(x.at(i) > value) {id = i; break;}
    }

    N = 5;
    X.setcontent(N,&x[id-2]);
    Y.setcontent(N,&y_spline[id-2]);
    spline1dbuildcubic(X, Y, s);
    value_F = spline1dcalc(s,value);
    z = value_F/2.;
    i = 100;
    while(z<value_F && i>0)
    {
        value -= 0.05;
        value_F = spline1dcalc(s,value);
        i--;
    }

    if(i>0)
    {
        P.setX(value);
        P.setY(value_F);
    }
    else
    {
        P.setX(0);
        P.setY(0);
    }

    return(P);

}

