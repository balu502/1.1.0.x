
#include "protocol.h"
#include "utility.h"


//-----------------------------------------------------------------------------
rt_Protocol::rt_Protocol()
{
    QString text;
    QStringList list;
    list << USE_CROSSTALK << USE_OPTICALCORRECTION << USE_DIGITFILTER << PARAM_SPLINECUBE;

    // preference (property default)
    rt_Preference *preference;

    foreach (text, list)
    {
       preference = new rt_Preference;
       preference->name = text.toStdString();
       if(text == PARAM_SPLINECUBE)
       {
            preference->value = "30.";
       }
       else preference->value = "yes";
       preference_Pro.push_back(preference);       
    }

    state = mWait;
}
//-----------------------------------------------------------------------------

//--- Clear Protocol ----------------------------------------------------------
void rt_Protocol::Clear_Protocol()
{
    uint i;

    name.clear();                       // name
    barcode.clear();                    // barcode
    time_created.clear();               // time_created
    regNumber.clear();                  // regNumber
    owned_by.clear();                   // owned_by
    program.clear(); 					// program of amplification
    enable_tube.clear();				// enable_tube
    color_tube.clear();					// color_tube
    //list_Channels.clear();				// list_Channels
    PCR_RawData.clear();                // clear PCR_RawData
    X_PCR.clear();                      // clear X_PCR
    PCR_Filtered.clear();               // clear PCR_Filtered
    PCR_Bace.clear();                   // clear PCR_Bace
    Threshold_Ct.clear();               // clear Threshold
    PCR_FinishFlash.clear();            // clear PCR_FinishFlash
    MC_RawData.clear();                 // clear MC_RawData
    X_MC.clear();                       // clear X_MC
    MC_Filtered.clear();                // clear MC_Filtered
    MC_dF_dT.clear();                   // clear MC_dF_dT
    MC_TPeaks.clear();                  // clear MC_TPeaks
    MC_TPeaks_Y.clear();                // clear MC_TPeaks_Y
    NormCp_Value.clear();               // clear NormCp_Value
    NormCt_Value.clear();               // clear NormCt_Value

    SerialName.clear();
    uC_Versions.clear();
    FluorDevice_MASK = 0;
    active_Channels = 0;
    Type_ThermoBlock.clear();
    DeviceParameters.clear();
    COEFF_Spectral.clear();             // clear COEFF_Spectral
    COEFF_Optic.clear();                // clear COEFF_Optic
    COEFF_UnequalCh.clear();            // clear COEFF_UnequalCh
    Exposure.clear();                   // clear Exposure

    volume = 0;                        // Volume (mcl)

    for(i=0; i<list_PointsOff.size(); i++)
    {
       POINT_TAKEOFF *pp_off = list_PointsOff.at(i);
       delete pp_off;
    }
    list_PointsOff.clear();                                         // clear list_PointsOff

    for(i=0; i<list_SplineCp.size(); i++)
    {
        SPLINE_CP *p_SplineCp = list_SplineCp.at(i);
        delete p_SplineCp;
    }
    list_SplineCp.clear();

    for(i=0; i<meas.size(); i++) delete meas[i];
    meas.clear();   												// clear measure

    Plate_Tests.clear();                                            // clear Plate_Tests
    Criterion_Test *criterion;
    for(i=0; i<Criterion.size(); i++)
    {
        criterion = (Criterion_Test*)Criterion.at(i);
        delete criterion;
    }
    Criterion.clear();

    for(i=0; i<preference_Pro.size(); i++) delete preference_Pro[i];// clear preference_Pro
    preference_Pro.clear();

    for(i=0; i<tests.size(); i++) delete tests[i];
    tests.clear();   												// clear tests

    for(i=0; i<Plate.groups.size(); i++) delete Plate.groups[i];
    Plate.groups.clear();                                           // clear Plate

    PrAmpl_value.clear();
    PrAmpl_time.clear();
    PrAmpl_color.clear();
    PrAmpl_name = "";
    PrAmpl_countLevelsCycles.clear();
    PrAmpl_timesInBlocks.clear();
    PrAmpl_minimumLevel = 0;

    hash_protocol = "";
    validity_hash = false;

    for(i=0; i<Map_Reserve.size(); i++)
    {
        QVector<QString> *vec = Map_Reserve.values().at(i);
        vec->clear();
        delete vec;
    }
    Map_Reserve.clear();
}
//-----------------------------------------------------------------------------

//--- Create Protocol ---------------------------------------------------------
rt_Protocol* Create_Protocol()
{
    return(new rt_Protocol());
}
//-----------------------------------------------------------------------------

//--- Delete Protocol ---------------------------------------------------------
void Delete(rt_Protocol *p)
{
    p->Clear_Protocol();
    delete p;
}

