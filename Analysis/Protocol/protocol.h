#ifndef PROTOCOL_H
#define PROTOCOL_H

//#include "protocol_global.h"
#if defined(PROTOCOL_LIBRARY)
#  define PROTOCOLSHARED_EXPORT __declspec(dllexport)
#else
#  define PROTOCOLSHARED_EXPORT __declspec(dllimport)
#endif

#include <string>
#include <vector>

#include <QMap>
#include <QVector>


#include "../Algorithm/point_takeoff.h"

using namespace std;

#define MAX_CHANNELS 5
enum Protocol_State{mNotExist,mWait,mDrop,mRun,mData,mAnalysis,mReAnalysis};

#define USE_CROSSTALK "use_CrossTalk"
#define USE_OPTICALCORRECTION "use_OpticalCorrection"
#define USE_DIGITFILTER "use_DigitFilter"
#define PARAM_SPLINECUBE "param_SplineCube"

//--- rt_HeaderTest -----------------------------------------------------------
typedef struct
{
    //int 					ID_Test;
    string                  ID_Test;
    string 					Name_Test;
    string                  version;            // user version
    int 					Type_analysis;
    string 					Hash;
    unsigned long 			CRC32;
    int 					Active_channel;
    int 					Volume_Tube;
    vector<string>			program;
    string					comment;
    string                  Catalog;
    vector<string>			Reserve;

}rt_HeaderTest;
//-----------------------------------------------------------------------------

//--- rt_Preference ---------------------------------------------------------
typedef struct
{
    string                  name;
    string                  value;
    string                  unit;

}rt_Preference;
//-----------------------------------------------------------------------------

//--- rt_ChannelTest ----------------------------------------------------------
class rt_ChannelTest
{
    public:

    int                         ID_Channel;
    int                         number;			// 0-Fam 1-Hex ...
    string                      name;
    vector<rt_Preference *>     preference_ChannelTest;

    ~rt_ChannelTest()
    {
        for(unsigned int i=0; i<preference_ChannelTest.size(); i++) delete preference_ChannelTest[i];
        preference_ChannelTest.clear();
    }
};
//-----------------------------------------------------------------------------

//--- rt_TubeTest -------------------------------------------------------------
class rt_TubeTest
{
    public:

    int 						ID_Tube;
    string						name;
    int                         color;
    vector<rt_ChannelTest *>    channels;
    vector<rt_Preference *>    	preference_TubeTest;

    ~rt_TubeTest()
    {
        for(unsigned int i=0; i<channels.size(); i++) delete channels[i];
        channels.clear();
        for(unsigned int i=0; i<preference_TubeTest.size(); i++) delete preference_TubeTest[i];
        preference_TubeTest.clear();
    }
};
//-----------------------------------------------------------------------------

//--- rt_TemplateTest ---------------------------------------------------------
class rt_TemplateTest
{
    public:

    int 						ID_Template;
    vector<rt_TubeTest *>    	tubes;

    ~rt_TemplateTest()
    {
        for(unsigned int i=0; i<tubes.size(); i++) delete tubes[i];
        tubes.clear();
    }
};
//-----------------------------------------------------------------------------


//--- rt_Test -----------------------------------------------------------------
class rt_Test
{
    public:

    rt_HeaderTest				header;
    //vector<rt_TemplateTest *> 	templates;
    vector<rt_TubeTest *>       tubes;
    vector<rt_Preference *>	    preference_Test;

    ~rt_Test()
    {
        for(unsigned int i=0; i<tubes.size(); i++) delete tubes[i];
        tubes.clear();
        for(unsigned int i=0; i<preference_Test.size(); i++) delete preference_Test[i];
        preference_Test.clear();
    }
};
//-----------------------------------------------------------------------------

//--- rt_Channel --------------------------------------------------------------
class rt_Channel
{
    public:

    int 					ID_Channel;
    string 					Unique_NameChannel;
    vector<string> 			result_Channel;
    vector<rt_Preference *>	preference_Channel;

    ~rt_Channel()
    {
        result_Channel.clear();
        for(unsigned int i=0; i<preference_Channel.size(); i++) delete preference_Channel[i];
        preference_Channel.clear();
    }
};
//-----------------------------------------------------------------------------

//--- rt_Tube -----------------------------------------------------------------
class rt_Tube
{
    public:

    int 					ID_Tube;
    string                  Unique_NameTube;
    int 					pos;                    // 0-95 0-385 ...
    int 					color;
    bool 					active;
    vector<rt_Channel *>    channels;
    vector<string>			result_Tube;
    vector<rt_Preference *>	preference_Tube;

    ~rt_Tube()
    {
        for(unsigned int i=0; i<channels.size(); i++) delete channels[i];
        channels.clear();
        result_Tube.clear();
        for(unsigned int i=0; i<preference_Tube.size(); i++) delete preference_Tube[i];
        preference_Tube.clear();
    }
};
//-----------------------------------------------------------------------------

//--- rt_Sample ---------------------------------------------------------------
class rt_Sample
{
    public:

    string 					ID_Sample;
    string					Unique_NameSample;
    string 					ID_Test;
    rt_Test                 *p_Test;
    vector<rt_Tube *>		tubes;
    vector<string>			result_Sample;
    vector<rt_Preference *>	preference_Sample;

    ~rt_Sample()
    {
        for(unsigned int i=0; i<tubes.size(); i++) delete tubes[i];
        tubes.clear();
        result_Sample.clear();
        for(unsigned int i=0; i<preference_Sample.size(); i++) delete preference_Sample[i];
        preference_Sample.clear();
    }
};
//-----------------------------------------------------------------------------

//--- rt_GroupSamples ---------------------------------------------------------
class rt_GroupSamples
{
    public:

    string 					ID_Group;
    string                  Unique_NameGroup;
    vector<rt_Sample *>		samples;
    vector<string>			result_Group;
    vector<rt_Preference *>	preference_Group;

    ~rt_GroupSamples()
    {
        for(unsigned int i=0; i<samples.size(); i++) delete samples[i];
        samples.clear();
        result_Group.clear();
        for(unsigned int i=0; i<preference_Group.size(); i++) delete preference_Group[i];
        preference_Group.clear();
    }
};
//-----------------------------------------------------------------------------

//--- rt_Plate ----------------------------------------------------------------
class rt_Plate
{
    public:

    int 						ID_Plate;
    string                  	Unique_NamePlate;

    vector<rt_GroupSamples *>   groups;

    void PlateSize(int count, int &row, int &col)
    {
       switch(count)
       {
        default:
        case 96:    row = 8;    col = 12;   break;
        case 384:   row = 16;   col = 24;   break;
        case 48:    row = 6;    col = 8;    break;
        case 32:    row = 4;    col = 8;    break;
       case 192:    row = 12;   col = 16;   break;
       }
    }

    ~rt_Plate()
    {
        for(unsigned int i=0; i<groups.size(); i++) delete groups[i];
        groups.clear();
    }
};
//-----------------------------------------------------------------------------

//--- rt_Measurement ----------------------------------------------------------
class rt_Measurement
{
    public:

    short fn;                       // number of measurement
    short type_meas;                // type measurement: 1-main 2-melting curve
    short block_number;             //
    short cycle_rep;                //
    short optical_channel;          // 0 - Fam, 1 - Hex, 2 - Rox, 3 - Cy5, 4 - Cy5.5 ...
    short num_exp;                  // number of current expo
    short exp_value;                // value of current expo
    short blk_exp;		    		//

    vector<short> measurements;

    ~rt_Measurement()
    {
        measurements.clear();
    }

};
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class Criterion_Test
{
public:
    rt_Test *ptest;

    int criterion_PosResult;
    int criterion_Validity;
    bool use_AFF;
    int criterion_AFF;
    int criterion_AFF_MIN;
    int criterion_AFF_MAX;
    int sigmoid_ValidityMin;
    int sigmoid_ValidityMax;

    bool use_threshold;
    int value_threshold;

    string exposure;

    int fwhm_border;
    int peaks_border;

public:
    Criterion_Test()
    {
        ptest = NULL;
        criterion_PosResult = 8;
        criterion_Validity = 5;
        use_AFF = true;
        criterion_AFF = 1000;
        criterion_AFF_MIN = 10;
        criterion_AFF_MAX = 90;
        sigmoid_ValidityMin = 7;
        sigmoid_ValidityMax = 20;
        use_threshold = false;
        value_threshold = 10;
        exposure = "1 1 1 1 1";
        fwhm_border = 30;
        peaks_border = 20;
    }
};
//-----------------------------------------------------------------------------

//--- Protocol ----------------------------------------------------------------
class PROTOCOLSHARED_EXPORT rt_Protocol
{
public:

    vector<rt_Measurement *> 		meas;			// Raw data
    vector<string>					program;		// Program of Amplification
    vector<rt_Test *>				tests;			// Tests
    rt_Plate						Plate;			// Plate
    vector<rt_Test *>               Plate_Tests;    // distribution tests on Plate 96,384,...
    vector<Criterion_Test*>         Criterion;      // Tests Criterion of Analysis
    vector<rt_Preference *>         preference_Pro; // Preference of the protocol

    //--- device parameters ---
    string  SerialName;
    string  uC_Versions;
    int     FluorDevice_MASK;
    string  Type_ThermoBlock;
    string  DeviceParameters;   // 512*n (n=1-4)

    vector<string> COEFF_Spectral;         // CrossTalk between channels
    vector<string> COEFF_Optic;            // Optical unevenness (plate)
    vector<string> COEFF_UnequalCh;        // Optical unevenness (channels)

    vector<short> Exposure;                // first exposure (for all channels)

    //--- public parameters ---

    int count_Tubes;                    // count of tubes on the plate: 96,384,48,...
    int active_Channels;                // active_channels: 0x11111 (4 bit on channel)

    string name;                        // Name of Protocol
    string barcode;                     // barcode
    string time_created;               	// time/date of created
    string regNumber;                  	// regNumber
    string owned_by;                   	// Owned_by(Operator)
    int    volume;                      // volume in tube (mcl)
    Protocol_State state;               // state of protocol(run,analysis,...)
    vector<short> enable_tube;          // status of drawing protocols tubes
    vector<int> color_tube;         	// color of protocols tubes
    string xml_filename;               	// XML protocol (input xml,r96,rt protocol)
    string hash_protocol;               // Hash of protocol
    bool validity_hash;                 // Validity of current protocol (data safety)

    //--- PCR measurements ---
    int count_PCR;                      // count PCR measurements
    vector<double> PCR_RawData;         // PCR raw data
    vector<double> X_PCR;               // X_PCR
    vector<double> PCR_Filtered;        // PCR filtered raw data
    vector<double> PCR_Bace;            // PCR Bace data
    vector<POINT_TAKEOFF*> list_PointsOff;   // list PointsOFF
    vector<double> Threshold_Ct;        // Threshold for calculate Ct
    vector<short>  PCR_FinishFlash;     // Analysis of Finish Flash (percent)
    vector<SPLINE_CP*> list_SplineCp;   // list Spline_Fitting Cp (alternative variant)    
    vector<double> NormCp_Value;        // coefficients normalization(Cp)
    vector<double> NormCt_Value;        // coefficients normalization(Ct)

    //--- MC measurements ---
    int count_MC;                       // count MC measurements
    vector<double> MC_RawData;          // MC raw data
    vector<double> X_MC;                // X_MC (temperature)
    vector<double> MC_Filtered;         // MC filtered raw data
    vector<double> MC_dF_dT;            // dF/dT MC analysis data
    vector<double> MC_TPeaks;           // Temperature peaks (2 per channel)
    vector<double> MC_TPeaks_Y;         // Temperature peaks (Y) (2 per channel)
    double T_initial;                   // Initial Temperatures in Melting Curve
    double dT_mc;                       // dT in Melting Curve

    //--- additional parameters of ProgramAmpl ---
    vector<double> PrAmpl_value;
    vector<double> PrAmpl_time;
    vector<int> PrAmpl_color;
    string      PrAmpl_name;
    vector<int> PrAmpl_countLevelsCycles;
    vector<string> PrAmpl_timesInBlocks;
    int PrAmpl_minimumLevel;

    //--- reserve ---
    QMap<QString, QVector<QString>*> Map_Reserve;

    //--- public function ---

    void Clear_Protocol();    

    rt_Protocol();
    ~rt_Protocol(){ Clear_Protocol();}

};
//-----------------------------------------------------------------------------

PROTOCOLSHARED_EXPORT   rt_Protocol* Create_Protocol();             // create protocol
PROTOCOLSHARED_EXPORT	void Delete(rt_Protocol*);                  // delete protocol
//PROTOCOLSHARED_EXPORT   int Read_r96(rt_Protocol *p, string fn);    // Read OLD(*.r96) protocol


#endif // PROTOCOL_H
