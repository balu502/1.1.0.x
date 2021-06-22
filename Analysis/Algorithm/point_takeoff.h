#ifndef POINT_TAKEOFF_H
#define POINT_TAKEOFF_H

typedef struct
{
    double i_start;     // Ct-sigmoid
    double sigma;       // rms
    double c0;          //
    double c1;          //

//Y = C + A / (1 + exp((x0-x)/b))

    double A_Sigmoid;               //
    double x0_Sigmoid;              //
    double b_Sigmoid;               //
    double f0_Sigmoid;              //
    double e0_Sigmoid;              //
    double ct_Sigmoid;              //
    double chi_Sigmoid;             //
    double rms_Sigmoid;             //
    double cp_Sigmoid;              //
    double cpdev_Sigmoid;           //

    double real_cp;
    double real_Fluor_cp;
    double real_ct;

    double Fluor_Cp;                //
    int point_fit;                  //
    double Criteria;                //
    bool valid;                     //

} POINT_TAKEOFF;

typedef struct
{
    int count_LF;                   //
    int count_depth;                //
    int count_SF;                   //
    int count_eff;                  //
    int point_start;                //

} Preference_INFO;

typedef struct
{
    double Cp;
    double F_Cp;

}SPLINE_CP;


#endif // POINT_TAKEOFF_H
