#ifndef ALGORITHM
#define ALGORITHM

#include "algorithm_global.h"

#include "ap.h"
#include "interpolation.h"
#include "point_takeoff.h"

#include <QVector>
#include <QtCore/qmath.h>
#include <QPointF>
#include <QDebug>

//-----------------------------------------------------------------------------
class ALGORITHMSHARED_EXPORT Sigmoid
{
public:
    Preference_INFO preference;
    QList<POINT_TAKEOFF *> list_pointOFF;
    int best_pointOFF;

public:
    Sigmoid()
    {
        preference.count_LF = 10;           //
        preference.count_depth = 6;         //
        preference.count_SF = 9;            //
        preference.point_start = 5;         //
        preference.count_eff = 8;           //

        best_pointOFF = -1;                 // no point_OFF

        list_pointOFF.clear();
    }
    ~Sigmoid()
    {
        POINT_TAKEOFF *pp_OFF;

        for(int i=0; i<list_pointOFF.count(); i++)
        {
            pp_OFF = list_pointOFF.at(i);
            delete pp_OFF;
        }
        list_pointOFF.clear();
    }
    Sigmoid(const Sigmoid &s)
    {
        int i;
        POINT_TAKEOFF *p_OFF;
        int count = s.list_pointOFF.size();

        preference = s.preference;
        best_pointOFF = s.best_pointOFF;

        for(int i=0; i<list_pointOFF.count(); i++)
        {
            p_OFF = list_pointOFF.at(i);
            delete p_OFF;
        }
        list_pointOFF.clear();

        for(i=0; i<count; i++)
        {
            POINT_TAKEOFF *p_off = new POINT_TAKEOFF;
            p_OFF = s.list_pointOFF.at(i);
            *p_off = *p_OFF;
            list_pointOFF.push_back(p_off);
        }
    }

};

//-----------------------------------------------------------------------------

ALGORITHMSHARED_EXPORT double Find_MeanValue(QVector<double> &y);                           // find Mean_value
ALGORITHMSHARED_EXPORT double Find_StDeviation(QVector<double> &y);                         // find Standart Deviation
ALGORITHMSHARED_EXPORT void FilterData_ByAverage(QVector<double> &buf, int average_step);   // filter by average
ALGORITHMSHARED_EXPORT void FilterData_ByMedian(QVector<double> &buf, int step);            // filter by median

ALGORITHMSHARED_EXPORT POINT_TAKEOFF Analysis_PCR(QVector<double> &Input_buf,               // PCR analysis
                                                  QVector<double> &Output_buf);
ALGORITHMSHARED_EXPORT POINT_TAKEOFF Analysis_PCR_Ex(QVector<double> &Input_buf,               // PCR analysis
                                                     QVector<double> &Output_buf,
                                                     Sigmoid *s = NULL,
                                                     int criterion_PosResult = 8);
ALGORITHMSHARED_EXPORT short Analysis_FinishFlash(QVector<double> &Input_buf);              // analysis of finish flash (percent)
ALGORITHMSHARED_EXPORT double Do_LinFit(QVector<double> &buf,                               // Linear fitting: f = c0 + c1*x
                                        double &coef_0,
                                        double &coef_1,
                                        double i_start);
ALGORITHMSHARED_EXPORT bool Fitting_Sigmoid(QVector<double> &buf, Sigmoid *s);              // Fitting Sigmoid
ALGORITHMSHARED_EXPORT bool Fitting_Sigmoid_Pro(QVector<double> &buf, Sigmoid *s);              // Fitting Sigmoid

ALGORITHMSHARED_EXPORT double Threshold_Curve(QVector<double> &buf, double val);            //


double Change_CpLevel(double A,         //
                      double x0,        //
                      double B,         //
                      double Cp,        //
                      double coef);     //

void Create_FittingCurve(QVector<double> &input_buf,    //
                         QVector<double> &output_buf,   //
                         Sigmoid *s);                   //


ALGORITHMSHARED_EXPORT int Find_BestPoint_OFF(Sigmoid *s, int count);
ALGORITHMSHARED_EXPORT  void SplineCube(QVector<double> &x,             // Spline approximation
                                        QVector<double> &y,             // x,y - input data
                                        QVector<double> &y_spline,      // output - spline
                                        QVector<double> &dy_spline,     // output - y'
                                        QVector<double> &d2y_spline,    // output - y"
                                        int &output_sts,                // output - status
                                        double rho = 0.00001);


ALGORITHMSHARED_EXPORT void Find_TemperaturePeaks(QVector<double> &two_peaks_x,
                                                  QVector<double> &two_peaks_y,
                                                  QVector<double> &dy,
                                                  QVector<double> &d2y,
                                                  QVector<double> &x,
                                                  QVector<double> *param = NULL);

bool FilterPeak_FWHM(QVector<double> &X,    // Filter FWHM for valid peaks
                     QVector<double> &Y,
                     int pos,
                     double border);

ALGORITHMSHARED_EXPORT double roundTo(double inpValue, int inpCount);


ALGORITHMSHARED_EXPORT QPointF CP_SplineFitting(QVector<double> &Y);

#endif // ALGORITHM

