#include "melt_curve.h"

#include <gsl/gsl_fit.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>


double kCoef,lCoef,sCoef;
double chi,terr,aerr;
int npf;



//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Melt_Curve::Melt_Curve(QWidget *parent) : QGroupBox(parent)
{
    setObjectName("Transparent");
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(2);
    setLayout(main_layout);

    Control_Group = new QGroupBox(this);
    Control_Group->setObjectName("Transparent");
    QHBoxLayout *control_layout = new QHBoxLayout();
    control_layout->setMargin(0);
    Control_Group->setLayout(control_layout);

    Chart_Group = new QGroupBox(this);
    Chart_Group->setObjectName("Transparent");
    QVBoxLayout *layout_chart = new QVBoxLayout;
    Chart_Group->setLayout(layout_chart);
    mc_chart = new MeltCurvePlot(Chart_Group);
    QHBoxLayout *fluor_layout = new QHBoxLayout();
    type_Box = new QComboBox(this);
    Fluor_Box = new QComboBox(this);
    fluor_delegate = new FluorMC_ItemDelegate;
    Fluor_Box->setItemDelegate(fluor_delegate);
    col_Box = new QComboBox(this);
    row_Box = new QComboBox(this);
    delegate = new RowColMC_ItemDelegate;
    col_Box->setItemDelegate(delegate);
    row_Box->setItemDelegate(delegate);
    type_Box->setItemDelegate(delegate);
    fluor_layout->setMargin(0);
    fluor_layout->addWidget(type_Box, 1, Qt::AlignLeft);
    fluor_layout->addWidget(Fluor_Box, 0, Qt::AlignRight);
    fluor_layout->addWidget(col_Box, 0, Qt::AlignRight);
    fluor_layout->addWidget(row_Box, 0, Qt::AlignRight);
    layout_chart->addLayout(fluor_layout);
    layout_chart->addWidget(mc_chart);

    Result_Group = new QGroupBox(this);
    Result_Group->setObjectName("Transparent");
    QVBoxLayout *layout_result = new QVBoxLayout;
    Result_Group->setLayout(layout_result);
    Result_Tab = new QTabWidget(this);
    layout_result->addWidget(Result_Tab, 1);
    Table_Result = new QTableWidget(0,0, this);
    Table_Result->setWordWrap(true);
    results_Delegate = new ResultsMCItemDelegate();
    Table_Result->setItemDelegate(results_Delegate);
    Table_ResultSpectrum = new QTableWidget(0,0, this);
    Table_ResultSpectrum->setWordWrap(true);
    spectrum_Delegate = new ResultsSpectrumItemDelegate();
    Table_ResultSpectrum->setItemDelegate(spectrum_Delegate);
    Table_Color = new QTableWidget(0,0, this);
    color_Delegate = new TableColorItemDelegate();
    color_Delegate->view_value = false;
    color_Delegate->type_view = 0;
    color_Delegate->mode_view = 0;
    Table_Color->setItemDelegate(color_Delegate);
    Table_Color->setSelectionMode(QAbstractItemView::NoSelection);
    Table_Color->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Table_Color->setFocusPolicy(Qt::NoFocus);
    QFont f = qApp->font();
    f.setPointSize(f.pointSize() - 2);
    Table_Color->verticalHeader()->setFont(f);
    Table_Color->horizontalHeader()->setFont(f);
    Table_Color->setFont(f);

    Color_Group = new QGroupBox(this);
    QHBoxLayout *color_layout = new QHBoxLayout();
    color_layout->setMargin(2);
    color_layout->setSpacing(2);
    Color_Group->setLayout(color_layout);
    Color_Control = new QGroupBox(this);
    QVBoxLayout *layout_control = new QVBoxLayout;
    Color_Control->setLayout(layout_control);
    color_FluorBox = new QComboBox(this);
    color_FluorBox->setItemDelegate(fluor_delegate);
    color_TypeBox = new QComboBox(this);
    QStringList list_type;
    list_type << tr("TPeaks") << tr("Amplitude");
    color_TypeBox->addItems(list_type);
    color_TypeBox->setCurrentIndex(0);
    color_PlateBox = new QComboBox(this);
    list_type.clear();
    list_type << tr("Plate") << tr("ThermoBloks");
    color_PlateBox->addItems(list_type);
    color_PlateBox->setCurrentIndex(0);
    color_ViewValue = new QCheckBox(tr("view digital values"), this);
    color_ViewValue->setCheckState(Qt::Unchecked);
    color_3D = new QPushButton("3D", this);
    //color_3D->setDisabled(true);

    color_min = new QLabel(tr("min = "), this);
    color_max = new QLabel(tr("max = "), this);
    color_mean = new QLabel(tr("mean = "), this);
    color_sigma = new QLabel(tr("sigma = "), this);
    color_peakTopeak = new QLabel(tr("P_to_P = "), this);

    layout_control->addWidget(color_FluorBox, 0, Qt::AlignTop | Qt::AlignLeft);
    layout_control->addWidget(color_TypeBox, 0, Qt::AlignTop | Qt::AlignLeft);
    layout_control->addWidget(color_PlateBox, 0, Qt::AlignTop | Qt::AlignLeft);
    layout_control->addWidget(color_ViewValue, 0, Qt::AlignTop | Qt::AlignLeft);
    layout_control->addWidget(color_3D, 1, Qt::AlignTop | Qt::AlignLeft);

    layout_control->addWidget(color_min, 1, Qt::AlignBottom | Qt::AlignLeft);
    layout_control->addWidget(color_max, 0, Qt::AlignBottom | Qt::AlignLeft);
    layout_control->addWidget(color_mean, 0, Qt::AlignBottom | Qt::AlignLeft);
    layout_control->addWidget(color_sigma, 0, Qt::AlignBottom | Qt::AlignLeft);
    layout_control->addWidget(color_peakTopeak, 0, Qt::AlignBottom | Qt::AlignLeft);



    color_layout->addWidget(Table_Color, 1);
    color_layout->addWidget(Color_Control);


    Result_Tab->addTab(Table_Result, QIcon(":/images/temperature.png"), "");
    Result_Tab->addTab(Table_ResultSpectrum, QIcon(":/images/spectrum.png"), "");
    Result_Tab->addTab(Color_Group, QIcon(":/images/wave.png"), "");

    control_layout->addWidget(Result_Group);
    control_layout->addWidget(Chart_Group);

    main_progress = new QProgressBar(this);
    main_progress->setTextVisible(false);
    main_progress->setRange(0,100);
    PrBar_status = new QLabel(this);
    PrBar_status->setAlignment(Qt::AlignCenter);
    QHBoxLayout *probar_layout = new QHBoxLayout();
    main_progress->setLayout(probar_layout);
    probar_layout->addWidget(PrBar_status);

    main_layout->addWidget(Control_Group, 1);
    main_layout->addWidget(main_progress);    

    connect(Fluor_Box, SIGNAL(currentIndexChanged(int)), this, SLOT(Change_Tube()));
    connect(row_Box, SIGNAL(currentIndexChanged(int)), this, SLOT(Change_Tube()));
    connect(col_Box, SIGNAL(currentIndexChanged(int)), this, SLOT(Change_Tube()));
    connect(type_Box, SIGNAL(currentIndexChanged(int)), this, SLOT(Change_Tube()));
    connect(Result_Tab, SIGNAL(currentChanged(int)), this, SLOT(Change_CurrentIndex(int)));
    connect(color_FluorBox, SIGNAL(currentIndexChanged(int)), this, SLOT(Fill_ResultsColor()));
    connect(color_TypeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(Fill_ResultsColor()));
    connect(color_PlateBox, SIGNAL(currentIndexChanged(int)), this, SLOT(Fill_ResultsColor()));
    connect(color_ViewValue, SIGNAL(clicked(bool)), this, SLOT(Fill_ResultsColor()));
    connect(color_3D, SIGNAL(clicked(bool)), this, SLOT(View_3D()));

    TemperatureValid_status = true;
    OpticValid_status = true;

    Overflow_A = false;
    Overflow_T = false;

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Melt_Curve::~Melt_Curve()
{
    Clear_AllMaps();
    Peaks_3D.clear();

    delete fluor_delegate;
    delete Fluor_Box;    
    delete delegate;
    delete type_Box;
    delete row_Box;
    delete col_Box;
    delete mc_chart;
    delete results_Delegate;
    delete Table_Result;
    delete Table_Color;
    delete color_Delegate;
    delete color_FluorBox;
    delete color_TypeBox;
    delete color_PlateBox;
    delete color_ViewValue;
    delete color_min;
    delete color_max;
    delete color_mean;
    delete color_sigma;
    delete color_peakTopeak;
    delete Color_Control;
    delete Color_Group;
    delete spectrum_Delegate;
    delete Table_ResultSpectrum;
    delete Result_Tab;


    delete Chart_Group;
    delete Result_Group;
    delete Control_Group;
    delete PrBar_status;
    delete main_progress;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Change_CurrentIndex(int id)
{
    bool state = true;
    if(id == 2) state = false;

    Chart_Group->setVisible(state);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::LoadFluor(rt_Protocol *p)
{
    int i;
    QIcon icon;
    QString fluor_name[count_CH] = fluor_NAME;

    Fluor_Box->blockSignals(true);
    color_FluorBox->blockSignals(true);
    for(i=0; i<count_CH; i++)
    {
        Fluor_Box->addItem(fluor_name[i]);
        color_FluorBox->addItem(fluor_name[i]);
        switch(i)
        {
        case 0:  icon.addFile(":/images/fam_flat.png");   break;
        case 1:  icon.addFile(":/images/hex_flat.png");   break;
        case 2:  icon.addFile(":/images/rox_flat.png");   break;
        case 3:  icon.addFile(":/images/cy5_flat.png");   break;
        case 4:  icon.addFile(":/images/cy55_flat.png");   break;
        default: icon.addFile(NULL);   break;
        }
        Fluor_Box->setItemIcon(i,icon);
        color_FluorBox->setItemIcon(i,icon);
        if(!(p->active_Channels & (0x0f<<i*4)))
        {
           Fluor_Box->setItemData(i,0,Qt::UserRole - 1);
           color_FluorBox->setItemData(i,0,Qt::UserRole - 1);
        }
    }
    Fluor_Box->blockSignals(false);
    color_FluorBox->blockSignals(false);

    type_Box->blockSignals(true);
    type_Box->addItem("F(t) = ");
    type_Box->addItem("dF/dt = ");
    type_Box->setCurrentIndex(1);
    type_Box->blockSignals(false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Load_RowCol(rt_Protocol *prot)
{
    int i;
    int row,col;

    row_Box->blockSignals(true);
    col_Box->blockSignals(true);

    row_Box->clear();
    col_Box->clear();

    prot->Plate.PlateSize(prot->count_Tubes, row, col);

    for(i=0; i<row; i++) row_Box->addItem(QChar(0x41 + i));
    for(i=0; i<col; i++) col_Box->addItem(QString::number(i+1));

    row_Box->blockSignals(false);
    col_Box->blockSignals(false);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Melt_Curve::Analyser(rt_Protocol *prot)
{
    int i,j,k;
    int num = 0;
    int res = 0;
    int y,x;
    double yd,xd;
    QString text;
    QVector<QVector2D> Val, ValDerivate;
    QVector<QVector2D> Der;
    int count = prot->count_MC;
    double T0 = prot->T_initial;
    double dT = prot->dT_mc;
    int count_tube = prot->count_Tubes;

    //...
    QVector<double> coeff_optics, coeff;
    QStringList list;
    double dval;
    bool ok;
    bool sts_optic = true;
    rt_Preference   *preference;

    /*for(i=0; i<prot->preference_Pro.size(); i++)
    {
        preference = prot->preference_Pro.at(i);

        //if(preference->name == USE_CROSSTALK) {if(preference->value != "yes") sts_spectr = false; continue;}
        if(preference->name == USE_OPTICALCORRECTION) {if(preference->value != "yes") sts_optic = false; break;}
    }

    coeff_optics.reserve(prot->count_Tubes*count_CH);
    for(i=0; i<prot->COEFF_Optic.size(); i++)
    {
        text = QString::fromStdString(prot->COEFF_Optic.at(i));
        list = text.trimmed().split(QRegExp("\\s+"));
        for(j=0; j<list.size(); j++)
        {
            text = list.at(j);
            dval = text.toDouble(&ok);
            if(!ok) dval = 0;
            if(!sts_optic) dval = 1.;
            coeff_optics.append(dval);
        }
    }*/
    //qDebug() << "coeff_optics: " << coeff_optics.size() << coeff_optics;

    //...

    //qDebug() << "mc_Info: " << count << T0 << dT;
    Prot = prot;
    LoadFluor(prot);

    int count_ActiveCh = 0;
    for(j=0; j<count_CH; j++)
    {
        if(prot->active_Channels & (0x0f<<4*j)) count_ActiveCh++;
    }
    results_Delegate->count_ActiveCh = count_ActiveCh;
    spectrum_Delegate->count_ActiveCh = count_ActiveCh;

    Clear_AllMaps();
    Load_RowCol(prot);
    Fill_ResultsTable();
    Fill_ResultsTableSpectrum();    

    list_FitdFdT.reserve(count_tube*count_ActiveCh);
    list_FitMC.reserve(count_tube*count_ActiveCh);

    double dT_peaks = 15.;

    for(i=0; i<prot->count_MC; i++) X_MeltCurve.append(prot->X_MC.at(i));

    main_progress->setRange(0, count_ActiveCh*count_tube);
    for(i=0; i<count_CH; i++)
    {
        if(!(prot->active_Channels & (0x0f<<4*i))) continue;

        QVector<double> *Vec_RawMC = new QVector<double>;
        Vec_RawMC->reserve(count*count_tube);
        map_RawMC.insert(i, Vec_RawMC);
        QVector<double> *Vec_RawdFdT = new QVector<double>;
        Vec_RawdFdT->reserve(count*count_tube);
        map_RawdFdT.insert(i, Vec_RawdFdT);
        QVector<double> *Vec_Param = new QVector<double>;
        Vec_Param->reserve(10*count_tube);
        map_Param.insert(i, Vec_Param);

        //coeff.clear();
        //coeff = coeff_optics.mid(count_tube*i, count_tube);

        for(j=0; j<count_tube; j++)
        {
            Val.clear();                // clear ...
            ValDerivate.clear();        //
            Der.clear();                //

            //dval = coeff.at(j);

            // 1. Raw data
            for(k=0; k<count; k++)
            {
                x = X_MeltCurve.at(k);
                y = map_RawData->value(i)->at(j*count+k);
                //y = y/dval;
                Vec_RawMC->append(y);
                Val.append(QVector2D(x,y));
            }

            // 2. Darivate
            ValDerivate = calcDerivate(Val);
            for(k=0; k<count; k++)
            {
                xd = ValDerivate.at(k).x();
                yd = ValDerivate.at(k).y();
                Vec_RawdFdT->append(yd);

                if(xd < (TPeaks.at(i) - dT_peaks)) continue;
                if(xd > (TPeaks.at(i) + dT_peaks)) continue;

                //if(yd < 0.) continue;

                Der.append(ValDerivate.at(k));
            }

            // 3. Fitting
            Par[0] = Amplitude.at(i);   // A amplituda
            Par[1] = 20.;               // l widht peak
            Par[2] = TPeaks.at(i);      // temp peak
            Par[3] = 12.;               // form coeff

            Meas = ValDerivate;
            calcFitting(Par, Der, Vec_Param);       // Fitting Weibull function
            Fit = Calculate_Weibull(Par, &Der);     // Calculate Weibull fitting
            //Draw_Curves();

            QVector<QVector2D> *FitdFdT = new QVector<QVector2D>;
            list_FitdFdT.append(FitdFdT);
            *FitdFdT = Fit;

            QVector<QVector2D> *FitMC = new QVector<QVector2D>;
            list_FitMC.append(FitMC);
            Fit = Calculate_WeibullC(Par, &Der);     // Calculate WeibullC fitting
            *FitMC = Fit;

            //... AFF ...
            //if(res == 0) res = Validate_AFF(i,&Fit);          // 10.02.2021
            //...

            num++;
            text = QString("%1%").arg((int)(num*100./(count_ActiveCh*count_tube)));
            Display_ProgressBar(num, text);            
        }
    }
    //qDebug() << "melt: all channels";

    Val.clear();    
    ValDerivate.clear();

    //... Additional analysis: validate data ...
    if(res == 0)
    {
        res = Validate_Data();
    }
    //...

    Change_Tube();
    //qDebug() << "melt: Change_Tube";

    //if(res == 0)
    {
        Calculate_Results(prot);    // Calculate and fill results
        //qDebug() << "melt: Calculate_Results";
    }

    // Pick_2_Pick_Block
    int index = -1;
    QVector<double> Vec_Temp;
    int row,col;
    double min_val, max_val;
    prot->Plate.PlateSize(prot->count_Tubes, row, col);

    for(i=0; i<count_CH; i++)
    {
        if(!(prot->active_Channels & (0x0f<<4*i))) continue;
        index++;
        Vec_Temp.clear();

        for(j=0; j<prot->count_Tubes; j++)
        {
            Vec_Temp.append(map_Param.value(index)->at(j*COUNT_RESULT_PARAM + 2));  // TPeaks
        }
        Convert_To_ThermoBloks(&Vec_Temp, prot->count_Tubes, &col, &row);

        min_val = *std::min_element(Vec_Temp.begin(), Vec_Temp.end());
        max_val = *std::max_element(Vec_Temp.begin(), Vec_Temp.end());

        Pick_2_Pick_Block.append(max_val - min_val);
    }
    //...


    Fill_ResultsColor();        // Color Analysis
    //qDebug() << "melt: Fill_ResultsColor";

    Sleep(1000);
    Result_Tab->setCurrentIndex(2);
    Sleep(1000);
    Result_Tab->setCurrentIndex(0);

    Display_ProgressBar(0, "");
    main_progress->setVisible(false);

    coeff_optics.clear();
    coeff.clear();

    return(res);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Melt_Curve::Validate_AFF(int ch, QVector<QVector2D> *fit)
{
    int res = 0;
    QVector2D P;
    QVector<double> vec;
    double min_value, max_value, value;

    double border = Amplitude.at(ch);

    foreach(P, *fit)
    {
        vec.append(P.y());
    }
    min_value = *std::min_element(vec.begin(), vec.end());
    max_value = *std::max_element(vec.begin(), vec.end());
    value = fabs(max_value - min_value);
    if(value < border*0.30)                 // 30%
    {
        res = -3;
    }

    return(res);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Melt_Curve::Validate_Data()
{
    int i,j,k;
    int num_ch;
    int res = 0;
    QVector<double> *vec;
    int count = Prot->count_Tubes;
    QVector<double> vec_param, *vec_original;
    QVector<QVector<double>*> Vec;
    double value, mean_value;
    int offset = 0;
    double border = border_IrregularityPlateOptic;
    int num_mistakes = 0;
    int pos, num;
    QVector<int> *vec_mistakes;
    QVector<QVector<int>*> Pos_Mistakes;
    QVector<double> vec_avrg;

    int curr_row, curr_col;
    int avrg_row, avrg_col;
    int pos_avrg;
    int col,row;
    Prot->Plate.PlateSize(count, row, col);

    int percent_3 = roundTo((double)count/100.*4., 0);
    int percent_5 = roundTo((double)count/100.*5., 0);
    //qDebug() << "percent_5: " << percent_5 << percent_3;


    for(i=0; i<2; i++)      //cycle for Ampl and Peak
    {
        num_ch = 0;
        if(i) {offset = 2; border = border_IrregularityPlate;}

        //qDebug() << "border: " << border;
        for(j=0; j<count_CH; j++)
        {
            if(!(Prot->active_Channels & (0x0f<<4*j))) continue;
            num_ch++;

            vec = map_Param.value(j);
            vec_param.clear();
            vec_original = new QVector<double>();
            Vec.append(vec_original);
            vec_mistakes = new QVector<int>();
            Pos_Mistakes.append(vec_mistakes);
            num_mistakes = 0;
            for(k=0; k<count; k++)
            {
                value = vec->at(k*COUNT_RESULT_PARAM + offset);
                vec_param.append(value);
                vec_original->append(value);
            }
            // 1. Sorting
            qSort(vec_param.begin(), vec_param.end());

            // 2. Removing percent_5 Min&Max
            vec_param.remove(count - percent_5, percent_5);
            vec_param.remove(0, percent_5);

            // 3. Mean value
            mean_value = Find_MeanValue(vec_param);
            //if(!i && !j) qDebug() << "vec_param: " << vec_param.size() << mean_value << vec_param;

            // 4. Mistakes
            pos = 0;
            foreach(value, *vec_original)
            {
                if(fabs(value - mean_value) > border)
                {
                    num_mistakes++;
                    vec_mistakes->append(pos);
                    //qDebug() << "mistakes: " << num_mistakes << value << mean_value << border;
                }

                if(num_mistakes > percent_3)
                {
                    //qDebug() << "num_mistakes: " << num_mistakes << mean_value << *vec_original;
                    res = -4;
                    if(i) Overflow_T = true;
                    else  Overflow_A = true;
                    break;
                }
                pos++;
            }
            Num_Mistakes.append(num_mistakes);
            if(res < 0) break;
        }
        //if(res < 0) break;
    }

    // Num mistakes < border  ...=>... Averaging mistakes!!!
    if(res == 0)
    {
        num = 0;
        foreach(vec_mistakes, Pos_Mistakes)
        {
            vec = map_Param.value(div(num,num_ch).rem);
            offset = 0;
            if(div(num,num_ch).quot) offset = 2;

            foreach(pos, *vec_mistakes)
            {
                curr_row = div(pos,col).quot;
                curr_col = pos - curr_row*col;
                vec_avrg.clear();

                //... +/- row/col
                for(i=0; i<4; i++)
                {
                    avrg_row = curr_row;
                    avrg_col = curr_col;

                    if(div(i,2).rem) k = -1;
                    else k = 1;
                    if(div(i,2).quot) avrg_row = curr_row + k;
                    else  avrg_col = curr_col + k;

                    if(avrg_row < 0 || avrg_row >= row) continue;
                    if(avrg_col < 0 || avrg_col >= col) continue;
                    pos_avrg = avrg_row*col + avrg_col;

                    if(pos_avrg < 0 || pos_avrg >= count) continue;
                    if(vec_mistakes->contains(pos_avrg)) continue;

                    value = vec->at(pos_avrg*COUNT_RESULT_PARAM + offset);
                    vec_avrg.append(value);
                }

                if(vec_avrg.size() > 1) // Averaging...
                {
                    mean_value = Find_MeanValue(vec_avrg);
                    vec->replace(pos*COUNT_RESULT_PARAM + offset, mean_value);
                }

                //qDebug() << "vec_avrg: " << vec_avrg;
            }

            //qDebug() << "Averaging: " << num << *vec_mistakes;
            num++;
        }
    }

    qDeleteAll(Vec);
    Vec.clear();
    qDeleteAll(Pos_Mistakes);
    Pos_Mistakes.clear();

    //qDebug() << "Overflow: " << Overflow_T << Overflow_A;

    return(res);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Calculate_Results(rt_Protocol *prot)
{
    int i,j,k;
    int id;
    int count = prot->count_Tubes;
    int count_block;
    QVector<double> vec_peak, vec_ampl, vec_temp;
    QVector<double> *param;
    QVector<double> Ampl;
    QString str_0,str_1;
    bool sts;
    QMap<int, QVector<double>*> map_plate;
    QVector<double> *vec;
    int id_attention = 0;
    int id_attention_optic = 0;
    QString key, text;
    QString fluor_name[count_CH] = fluor_NAME;

    double mean, std_dev, max_val, min_val, delta;    

    //qDebug() << "temp_borders: " << border_AbsDeviation << border_IrregularityPlate << border_PlateDeviation;

    int count_ch = results_Delegate->count_ActiveCh;

    // 1. Temperature validity
    Result_Tab->setCurrentIndex(0);
    for(i=0; i<count_CH; i++)
    {
        if(!(prot->active_Channels & (0x0f<<4*i))) continue;

        vec_peak.clear();
        vec_ampl.clear();

        param = map_Param.value(i);
        for(j=0; j<count; j++)
        {
            vec_peak.append(param->at(j*COUNT_RESULT_PARAM + 2));
            vec_ampl.append(param->at(j*COUNT_RESULT_PARAM + 0));
        }

        // 1. Mean
        mean = gsl_stats_mean(&vec_peak.at(0), 1, vec_peak.size());
        if(fabs(mean-TPeaks.at(i)) > border_AbsDeviation) sts = false;
        else sts = true;
        if(TemperatureValid_status) TemperatureValid_status = sts;
        str_0 = QString("%1 (%2)").arg(mean,0,'f',1).arg(TPeaks.at(i),0,'f',1);
        str_1 = QString("%1").arg(sts);
        Table_Result->item(i,1)->setText(str_0);
        Table_Result->item(i,2)->setText(str_1);
        if(!sts)
        {
            key = QString(QChar(0x41 + id_attention)) + ".";
            text = tr("Absolute deviation in channel: ") + fluor_name[i];
            map_ATTENTION_Temp.insert(key, text);
            id_attention++;
        }
        Mean_temperature.append(qAbs(mean-TPeaks.at(i)));

        // 2. Std Deviation + max
        std_dev = gsl_stats_sd_m(&vec_peak.at(0), 1, vec_peak.size(), mean);
        max_val = gsl_stats_max(&vec_peak.at(0), 1, vec_peak.size());
        min_val = gsl_stats_min(&vec_peak.at(0), 1, vec_peak.size());
        delta = max_val - min_val;
        if(std_dev > border_IrregularityPlate/2. || delta > border_IrregularityPlate/2.*6) sts = false;
        else sts = true;
        if(TemperatureValid_status) TemperatureValid_status = sts;
        str_0 = QString("%1 (%2)").arg(std_dev,0,'f',2).arg(delta,0,'f',2);
        str_1 = QString("%1").arg(sts);
        Table_Result->item(count_ch+1+i,1)->setText(str_0);
        Table_Result->item(count_ch+1+i,2)->setText(str_1);
        if(!sts)
        {
            key = QString(QChar(0x41 + id_attention)) + ".";
            text = tr("Temperature irregularity in channel: ") + fluor_name[i];
            map_ATTENTION_Temp.insert(key, text);
            id_attention++;
        }
        Std_temperature.append(std_dev);

        // 3.
        if(count == 384)        // only for 384 device
        {
            for(j=0; j<6; j++)
            {
                vec = new QVector<double>;
                map_plate.insert(j,vec);
            }
            for(j=0; j<16; j++)
            {
                for(k=0; k<24; k++)
                {
                    id = div(k,8).quot + div(j,8).quot*3;
                    vec = map_plate.value(id);
                    vec->append(vec_peak.at(k+j*24));
                }
            }
        }
        if(count == 96)        // only for 384 device
        {
            for(j=0; j<6; j++)
            {
                vec = new QVector<double>;
                map_plate.insert(j,vec);
            }
            for(j=0; j<8; j++)
            {
                for(k=0; k<12; k++)
                {
                    id = div(k,4).quot + div(j,4).quot*3;
                    vec = map_plate.value(id);
                    vec->append(vec_peak.at(k+j*12));
                }
            }
        }
        if(count == 48)      // only for 48 device
        {
            for(j=0; j<2; j++)
            {
                vec = new QVector<double>;
                map_plate.insert(j,vec);
            }
            for(j=0; j<6; j++)
            {
                for(k=0; k<8; k++)
                {
                    id = div(k,4).quot;
                    vec = map_plate.value(id);
                    vec->append(vec_peak.at(k+j*8));
                }
            }
        }
        if(count == 192)     // only for 192 device
        {
            for(j=0; j<2; j++)
            {
                vec = new QVector<double>;
                map_plate.insert(j,vec);
            }
            for(j=0; j<12; j++)
            {
                for(k=0; k<16; k++)
                {
                    id = div(k,8).quot;
                    vec = map_plate.value(id);
                    vec->append(vec_peak.at(k+j*16));
                }
            }
        }

        count_block = 6;
        if(count == 48 || count == 192) count_block = 2;

        vec_temp.clear();
        for(j=0; j<count_block; j++)
        {
            vec = map_plate.value(j);
            mean = gsl_stats_mean(&vec->at(0), 1, vec->size());
            vec_temp.append(mean);
        }
        max_val = gsl_stats_max(&vec_temp.at(0), 1, vec_temp.size());
        min_val = gsl_stats_min(&vec_temp.at(0), 1, vec_temp.size());
        delta = max_val - min_val;
        if(delta > border_PlateDeviation) sts = false;
        else sts = true;
        if(TemperatureValid_status) TemperatureValid_status = sts;
        str_0 = QString("%1").arg(delta,0,'f',2);
        str_1 = QString("%1").arg(sts);
        Table_Result->item(count_ch*2+2+i,1)->setText(str_0);
        Table_Result->item(count_ch*2+2+i,2)->setText(str_1);
        if(!sts)
        {
            key = QString(QChar(0x41 + id_attention)) + ".";
            text = tr("Plate deviation in channel: ") + fluor_name[i];
            map_ATTENTION_Temp.insert(key, text);
            id_attention++;
        }


        // 4. Spectrum mean value
        mean = gsl_stats_mean(&vec_ampl.at(0), 1, vec_ampl.size());
        if(fabs(mean-Amplitude.at(i))/Amplitude.at(i) > border_AbsDeviationOptic/100.) sts = false;      // 30%
        else sts = true;
        if(OpticValid_status) OpticValid_status = sts;
        str_0 = QString("%1 (%2)").arg(mean,0,'f',0).arg(Amplitude.at(i),0,'f',0);
        str_1 = QString("%1").arg(sts);
        Table_ResultSpectrum->item(i,1)->setText(str_0);
        Table_ResultSpectrum->item(i,2)->setText(str_1);
        if(!sts)
        {
            key = QString(QChar(0x41 + id_attention_optic)) + ".";
            text = tr("Absolute deviation in channel: ") + fluor_name[i];
            map_ATTENTION_Optic.insert(key, text);
            id_attention_optic++;
        }
        Deviation_amplitude.append(qAbs(mean-Amplitude.at(i))/Amplitude.at(i)*100.);


        // 5. Maximum deviation
        //max_val = gsl_stats_max(&vec_ampl.at(0), 1, vec_ampl.size());
        //min_val = gsl_stats_min(&vec_ampl.at(0), 1, vec_ampl.size());
        //delta = (max_val - min_val)/mean*100;

        std_dev = gsl_stats_sd_m(&vec_ampl.at(0), 1, vec_ampl.size(), mean);
        delta = fabs(std_dev/mean)*100.;


        if(std_dev > border_IrregularityPlateOptic/2) sts = false;
        else sts = true;
        if(OpticValid_status) OpticValid_status = sts;
        str_0 = QString("%1%").arg(delta,0,'f',0);
        str_1 = QString("%1").arg(sts);
        Table_ResultSpectrum->item(count_ch+1+i,1)->setText(str_0);
        Table_ResultSpectrum->item(count_ch+1+i,2)->setText(str_1);
        if(!sts)
        {
            key = QString(QChar(0x41 + id_attention_optic)) + ".";
            text = tr("Maximum deviation in channel: ") + fluor_name[i];
            map_ATTENTION_Optic.insert(key, text);
            id_attention_optic++;
        }
        Std_amplitude.append(delta);

        // 6. Maximum deviation between channels
        //qDebug() << "i, mean, FluorCorrection" << i << mean << FluorCorrection.at(i);
        Ampl.append(mean*FluorCorrection.at(i));

    }

    vec_peak.clear();
    vec_ampl.clear();
    vec_temp.clear();
    Sleep(1000);

    // 2. Spectrum (amplitude) validity
    /*                                                              // временно не выводим последнюю строку
    Result_Tab->setCurrentIndex(1);
    max_val = gsl_stats_max(&Ampl.at(0), 1, Ampl.size());
    min_val = gsl_stats_min(&Ampl.at(0), 1, Ampl.size());
    mean = gsl_stats_mean(&Ampl.at(0), 1, Ampl.size());
    delta = (max_val - min_val)/mean*100;
    if(delta > border_PlateDeviationOptic) sts = false;
    else sts = true;
    if(OpticValid_status) OpticValid_status = sts;
    str_0 = QString("%1%").arg(delta,0,'f',0);
    str_1 = QString("%1").arg(sts);

    Table_ResultSpectrum->item(count_ch*2+2,1)->setText(str_0);
    Table_ResultSpectrum->item(count_ch*2+2,2)->setText(str_1);
    if(!sts)
    {
        key = QString(QChar(0x41 + id_attention_optic)) + ".";
        text = tr("Maximum deviation (between channels)");
        map_ATTENTION_Optic.insert(key, text);
        id_attention_optic++;
    }
    */

    //qDebug() << "Ampl: " << Ampl.size() << Ampl;
    Ampl.clear();
    //Sleep(1000);
    //Result_Tab->setCurrentIndex(0);


    //...
    foreach(vec, map_plate.values())
    {
        vec->clear();
        delete vec;
    }
    map_plate.clear();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Fill_ResultsTable()
{
    int i,j;
    QStringList header;
    int row_current = 0;
    QTableWidgetItem *item;

    header << tr("Name") << tr("Value") << tr("Result");

    Table_Result->clear();
    Table_Result->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Table_Result->setSelectionMode(QAbstractItemView::NoSelection);
    Table_Result->setColumnCount(3);
    Table_Result->setHorizontalHeaderLabels(header);
    Table_Result->verticalHeader()->hide();
    Table_Result->verticalHeader()->setMinimumSectionSize(1);
    Table_Result->setColumnWidth(0, 100);
    Table_Result->setColumnWidth(1, 110);
    Table_Result->setColumnWidth(2, 70);
    for(i=0; i<Table_Result->columnCount(); i++) Table_Result->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    Table_Result->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    int count_ch = 0;
    for(i=0; i<count_CH; i++)
    {
        if(Prot->active_Channels & (0x0f<<4*i)) count_ch++;
    }
    Table_Result->setRowCount(count_ch*3 + 2);   // Mean value(4+1) + (1) + Sigma(4+1) + (1) + Plate(4+1)

    // 1. Mean value (4+1)
    for(i=0; i<count_ch; i++)
    {
        for(j=0; j<Table_Result->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            switch(j)
            {
            case 0:     item->setText(tr("Melting Temperature Peak"));    break;
            default:
            case 1:
            case 2:     item->setText("");  break;
            }
            Table_Result->setItem(row_current, j, item);
        }
        Table_Result->setRowHeight(row_current, 20);
        row_current++;
    }
    Table_Result->setSpan(0,0,count_ch,1);

    // 2. separator
    for(j=0; j<Table_Result->columnCount(); j++)
    {
        item = new QTableWidgetItem();
        item->setText("");
        Table_Result->setItem(row_current, j, item);
    }
    Table_Result->setRowHeight(row_current, 2);
    row_current++;

    // 3. Standart deviation (4+1)
    for(i=0; i<count_ch; i++)
    {
        for(j=0; j<Table_Result->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            switch(j)
            {
            case 0:     item->setText(tr("Standart deviation,(peak-to-peak)"));    break;
            default:
            case 1:
            case 2:     item->setText("");  break;
            }
            Table_Result->setItem(row_current, j, item);
        }
        Table_Result->setRowHeight(row_current, 20);
        row_current++;
    }
    Table_Result->setSpan(count_ch+1,0,count_ch,1);

    // 4. separator
    for(j=0; j<Table_Result->columnCount(); j++)
    {
        item = new QTableWidgetItem();
        item->setText("");
        Table_Result->setItem(row_current, j, item);
    }
    Table_Result->setRowHeight(row_current, 2);
    row_current++;

    // 5. Plate (4+1)
    for(i=0; i<count_ch; i++)
    {
        for(j=0; j<Table_Result->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            switch(j)
            {
            case 0:     item->setText(tr("Temperature irregularity on Plate"));    break;
            default:
            case 1:
            case 2:     item->setText("");  break;
            }
            Table_Result->setItem(row_current, j, item);
        }
        Table_Result->setRowHeight(row_current, 20);
        row_current++;
    }
    Table_Result->setSpan(count_ch*2 + 2,0,count_ch,1);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Fill_ResultsTableSpectrum()
{
    int i,j;
    QStringList header;
    int row_current = 0;
    QTableWidgetItem *item;

    header << tr("Name") << tr("Value") << tr("Result");

    Table_ResultSpectrum->clear();
    Table_ResultSpectrum->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Table_ResultSpectrum->setSelectionMode(QAbstractItemView::NoSelection);
    Table_ResultSpectrum->setColumnCount(3);
    Table_ResultSpectrum->setHorizontalHeaderLabels(header);
    Table_ResultSpectrum->verticalHeader()->hide();
    Table_ResultSpectrum->verticalHeader()->setMinimumSectionSize(1);
    Table_ResultSpectrum->setColumnWidth(0, 100);
    Table_ResultSpectrum->setColumnWidth(1, 110);
    Table_ResultSpectrum->setColumnWidth(2, 70);
    for(i=0; i<Table_ResultSpectrum->columnCount(); i++) Table_ResultSpectrum->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    Table_ResultSpectrum->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    int count_ch = 0;
    for(i=0; i<count_CH; i++)
    {
        if(Prot->active_Channels & (0x0f<<4*i)) count_ch++;
    }
    Table_ResultSpectrum->setRowCount(count_ch*2 + 3);   // Mean value(4+1) + (1) + Sigma(4+1) + (1) + 1

    // 1. Mean value (4+1)
    for(i=0; i<count_ch; i++)
    {
        for(j=0; j<Table_ResultSpectrum->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            switch(j)
            {
            case 0:     item->setText(tr("Signal amplitude"));    break;
            default:
            case 1:
            case 2:     item->setText("");  break;
            }
            Table_ResultSpectrum->setItem(row_current, j, item);
        }
        Table_ResultSpectrum->setRowHeight(row_current, 20);
        row_current++;
    }
    Table_ResultSpectrum->setSpan(0,0,count_ch,1);

    // 2. separator
    for(j=0; j<Table_ResultSpectrum->columnCount(); j++)
    {
        item = new QTableWidgetItem();
        item->setText("");
        Table_ResultSpectrum->setItem(row_current, j, item);
    }
    Table_ResultSpectrum->setRowHeight(row_current, 2);
    row_current++;

    // 3. Standart deviation (4)
    for(i=0; i<count_ch; i++)
    {
        for(j=0; j<Table_ResultSpectrum->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            switch(j)
            {
            case 0:     item->setText(tr("Maximum deviation (%)"));    break;
            default:
            case 1:
            case 2:     item->setText("");  break;
            }
            Table_ResultSpectrum->setItem(row_current, j, item);
        }
        Table_ResultSpectrum->setRowHeight(row_current, 20);
        row_current++;
    }
    Table_ResultSpectrum->setSpan(count_ch+1,0,count_ch,1);



    // 4. separator
    for(j=0; j<Table_ResultSpectrum->columnCount(); j++)
    {
        item = new QTableWidgetItem();
        item->setText("");
        Table_ResultSpectrum->setItem(row_current, j, item);
    }
    //Table_ResultSpectrum->setRowHeight(row_current, 2);
    Table_ResultSpectrum->setRowHeight(row_current, 0);
    row_current++;

    // 4.
    for(j=0; j<Table_ResultSpectrum->columnCount(); j++)
    {
        item = new QTableWidgetItem();
        switch(j)
        {
        case 0:     item->setText(tr("Maximum deviation between channels"));    break;
        default:
        case 1:
        case 2:     item->setText("");  break;
        }
        Table_ResultSpectrum->setItem(row_current, j, item);
    }
    //Table_ResultSpectrum->setRowHeight(row_current, 50);
    Table_ResultSpectrum->setRowHeight(row_current, 0);
    row_current++;

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Fill_ResultsColor()
{
    int i,j;
    int pos;
    QStringList header;
    QTableWidgetItem* item;
    int col, row;
    int index = 0;  // fam
    int pos_min, pos_max;
    double value, val;
    QVector<double> peaks;
    QVector<double> sum;

    Prot->Plate.PlateSize(Prot->count_Tubes, row, col);

    // index fluor
    int num = color_FluorBox->currentIndex();
    for(i=0; i<num; i++)
    {
        if(Prot->active_Channels & (0x0f<<4*i)) index++;
    }

    // type: TPeaks or Amplitude
    int K = 2;
    color_Delegate->type_view = 0;
    if(color_TypeBox->currentIndex() == 1)
    {
        K = 0;
        color_Delegate->type_view = 1;
    }

    // view value
    bool state = false;
    if(color_ViewValue->checkState() == Qt::Checked) state = true;
    color_Delegate->view_value = state;

    // min,max
    for(i=0; i<Prot->count_Tubes; i++)
    {
        peaks.append(map_Param.value(index)->at(i*COUNT_RESULT_PARAM + K));
    }
    Peaks_3D.clear();
    Peaks_3D = peaks;

    // mode: Plate or ThermoBloks
    color_Delegate->mode_view = color_PlateBox->currentIndex();
    if(color_PlateBox->currentIndex() == 1)
    {
        Convert_To_ThermoBloks(&peaks, Prot->count_Tubes, &col, &row);
        color_3D->setDisabled(true);
    }
    else color_3D->setDisabled(false);

    color_Delegate->min_Value = *std::min_element(peaks.constBegin(), peaks.constEnd());
    color_Delegate->max_Value = *std::max_element(peaks.constBegin(), peaks.constEnd());
    pos_min = std::min_element(peaks.constBegin(), peaks.constEnd()) - peaks.constBegin();
    pos_max = std::max_element(peaks.constBegin(), peaks.constEnd()) - peaks.constBegin();


    color_min->setText(QString("min = %1 (%2)").arg(color_Delegate->min_Value,0,'f',K).arg(Convert_IndexToName(pos_min,col)));
    color_max->setText(QString("max = %1 (%2)").arg(color_Delegate->max_Value,0,'f',K).arg(Convert_IndexToName(pos_max,col)));
    //mean
    value = std::accumulate(peaks.constBegin(), peaks.constEnd(), 0.)/peaks.size();
    color_mean->setText(QString("mean = %1").arg(value,0,'f',K));

    //sigma
    foreach(val, peaks)
    {
        sum.append(qPow(val-value, 2));
    }
    value = qSqrt(std::accumulate(sum.constBegin(), sum.constEnd(), 0.)/(sum.size()-1));
    color_sigma->setText(QString("sigma = %1").arg(value,0,'f',K));

    //p_To_p
    value = qAbs(color_Delegate->max_Value - color_Delegate->min_Value);
    color_peakTopeak->setText(QString("p_To_p = %1").arg(value,0,'f',K));

    //qDebug() << "max,min: " << color_Delegate->max_Value << color_Delegate->min_Value;

    //....

    Table_Color->clear();
    Table_Color->setRowCount(row);
    Table_Color->setColumnCount(col);

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            pos = i*col + j;
            item = new QTableWidgetItem();
            Table_Color->setItem(i,j,item);

            value = peaks.at(pos); // map_Param.value(index)->at(pos*COUNT_RESULT_PARAM + K);
            item->setText(QString::number(value,'f',3));
        }
    }

    header.clear();
    for(i=0; i<col; i++) header.append(QString::number(i+1));   // Column Header
    Table_Color->setHorizontalHeaderLabels(header);
    for(i=0; i<col; i++) Table_Color->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);

    header.clear();
    for(i=0; i<row; i++) header.append(QChar(0x41 + i));        // Row Header
    Table_Color->setVerticalHeaderLabels(header);
    for(i=0; i<row; i++) Table_Color->verticalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::View_3D()
{    
    QString text;
    double value;

    //QTemporaryFile file_color;
    if(file_color.exists()) file_color.remove();
    if(file_color.open())
    {
        //qDebug() << " file temp: " << file_color.fileName();
        qDebug() << "Peaks: " << Peaks_3D.size();
        foreach(value, Peaks_3D)
        {
            if(text.length()) text += "\r\n";
            text += QString::number(value, 'f', 2);
        }

        file_color.write(text.toLatin1());
        file_color.close();
    }

    QString lang = *Lang;


    QProcess *vec = new QProcess(this);
    text = QString("surface.exe \"%1\" %2").arg(file_color.fileName()).arg(lang);
    vec->startDetached(text);
    qDebug() << "cmd: " << text;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Convert_To_ThermoBloks(QVector<double> *peaks, int tubes_count, int *col_Table, int *row_Table)
{
    QVector<double> vec;
    QVector<double> mean;
    int i,j,k,m;
    int col,row;
    int pos;
    int N_col, N_row;
    double value;

    switch(tubes_count)
    {
    default:    N_col = 0;  N_row = 0;  break;
    case 96:
    case 384:   N_col = 3;  N_row = 2;  break;
    case 48:    N_col = 2;  N_row = 1;  break;
    case 192:   N_col = 2;  N_row = 1;  break;
    }
    if(!N_col || !N_row) return;

    Prot->Plate.PlateSize(Prot->count_Tubes, row, col);

    int count_i = row/N_row;
    int count_j = col/N_col;

    *col_Table = N_col;
    *row_Table = N_row;

    for(i=0; i<N_row; i++)
    {
        for(j=0; j<N_col; j++)
        {
            vec.clear();

            for(k=0; k<count_i; k++)
            {
                for(m=0; m<count_j; m++)
                {
                    pos = m + j*count_j + k*col + i*col*count_i;
                    vec.append(peaks->at(pos));
                }
            }
            value = std::accumulate(vec.constBegin(), vec.constEnd(), 0.)/vec.size();
            mean.append(value);
        }
    }

    peaks->clear();
    foreach(value, mean)
    {
        peaks->append(value);
    }

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QVector<QVector2D> Melt_Curve::calcDerivate(QVector<QVector2D> d)
{
    double dx,dy;
    int i,n,nc;
    nc = 4;

    QVector2D v0,v1,v,first;
    QVector<QVector2D> cl;
    n = d.size();
    cl.clear();

    for(i=1; i<n-1; i++)
    {
        v0 = d[i-1];
        v1 = d[i+1];
        dx = v1.x() - v0.x();
        dy = (v1.y() - v0.y()) / dx;
        v = QVector2D(d[i].x(),dy);
        cl.append(v);

        if(i==1) first = v;
    }
    //n = 0;

    cl.append(QVector2D(d.at(n-1).x(), v.y()));        // add last element
    cl.insert(0, QVector2D(d.at(0).x(), first.y()));   // insert first element

    return(cl);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Melt_Curve::calcFitting(double *par, QVector<QVector2D> d, QVector<double> *q)
{

    int i;
    size_t nd = d.size();      // count of fitting points
    double A,L,S;
    QVector2D P;
    QVector<double> X,Y;
    unsigned int iter;
    int status, info;
    double chi, chi0, dev;

    foreach(P, d)
    {
        X.append(P.x());
        Y.append(P.y());
    }

    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver *s;
    gsl_matrix *J = gsl_matrix_alloc(nd, ORDER);
    gsl_matrix *covar = gsl_matrix_alloc(ORDER, ORDER);    // allocation memory
    gsl_multifit_function_fdf f;
    gsl_vector *res_f;
    double x_init[3];
    Data source = {nd, Y.data(), X.data(), par[3]};


    const double xtol = 1e-8;
    const double gtol = 1e-8;
    const double ftol = 0.0;

    A = par[0];     // a - amplituda
    L = par[1];     // l - width peak
    S = par[2];     // s - peak (temperatura)

    //qDebug() << X.size() << X;
    //qDebug() << Y.size() << Y;
    //qDebug() << "start: " << A << L << S;

    while(1)
    {
        //... init ...
        x_init[0] = A;
        x_init[1] = L;
        x_init[2] = S;

        gsl_vector_view x = gsl_vector_view_array(x_init, ORDER);

        f.f = &expb_f;
        f.df = NULL;
        f.n = nd;
        f.p = ORDER;
        f.params = &source;

        iter = 0;

        s = gsl_multifit_fdfsolver_alloc(T, nd, ORDER);

        // initialize solver with starting point
        gsl_multifit_fdfsolver_set(s, &f, &x.vector);

        /* compute initial residual norm */
        res_f = gsl_multifit_fdfsolver_residual(s);
        chi0 = gsl_blas_dnrm2(res_f);

        /* solve the system with a maximum of 100 iterations */
        status = gsl_multifit_fdfsolver_driver(s, 100, xtol, gtol, ftol, &info);

        gsl_multifit_fdfsolver_jac(s, J);
        gsl_multifit_covar (J, 0.0, covar);

        /* compute final residual norm */
        chi = gsl_blas_dnrm2(res_f);

        // PRINT:
        dev = pow(chi, 2.0)/(nd - ORDER);
        A = gsl_vector_get(s->x, 0);
        L = gsl_vector_get(s->x, 1);
        S = gsl_vector_get(s->x, 2);
        /*
        qDebug() << "number of iterations: " << gsl_multifit_fdfsolver_niter(s);
        qDebug() << "chisq/dof: " << dev;
        qDebug() << "A: " << A << " +/- " << sqrt(gsl_matrix_get(covar,0,0));
        qDebug() << "L: " << L << " +/- " << sqrt(gsl_matrix_get(covar,1,1));
        qDebug() << "S: " << S << " +/- " << sqrt(gsl_matrix_get(covar,2,2));

        qDebug() << "status: " << gsl_strerror(status);
        */

        break;
    }    

    q->append(A);
    q->append(L);
    q->append(S);

    par[0] = A;
    par[1] = L;
    par[2] = S;

    //-----------------------------
    gsl_multifit_fdfsolver_free(s);
    gsl_matrix_free(covar);
    gsl_matrix_free(J);
    //-----------------------------

    return 0;

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QVector<QVector2D> Melt_Curve::Calculate_Weibull(double *par, QVector<QVector2D> *source)
{    
    double x,y;
    QVector2D P;
    QVector<QVector2D> res;

    foreach(P, *source)
    {
        x = P.x();
        y = weib(par[0],par[1],par[2],par[3],x);

        P.setY(y);
        res.append(P);
    }

    return(res);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QVector<QVector2D> Melt_Curve::Calculate_WeibullC(double *par, QVector<QVector2D> *source)
{
    double x,y;
    QVector2D P;
    QVector<QVector2D> res;

    foreach(P, *source)
    {
        x = P.x();
        y = weibc(par[0],par[1],par[2],par[3],x);

        P.setY(y);
        res.append(P);
    }

    return(res);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double weib(double a, double l, double s, double k, double x)
{
    double res,xm;

    xm = x/l - s/l + 1;
    if(xm < 0.) return(0.);
    res = a*((k/l) * pow((xm),(k-1)) * exp(-pow((xm),k)));

    return res;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double weibc(double a, double l, double s, double k, double x)
{
    double res,xm;

    xm = x/l - s/l + 1;
    if(xm < 0.) return(0.);
    res = a*(1. - exp(-pow((xm),k)));

    return res;
}
//-----------------------------------------------------------------------------
//---
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
        gsl_vector_set(f, i, Yi - y[i]);

        //qDebug() << "expb_f: " << i << Yi;
    }

    return GSL_SUCCESS;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Change_Tube()
{
    int i;
    int num = 0;
    QVector<double> *vec;
    QVector<QVector2D> *vec2D;
    QVector2D P;
    int count_row = row_Box->count();
    int count_col = col_Box->count();
    int count_meas = X_MeltCurve.size();

    int index = Fluor_Box->currentIndex();
    int row = row_Box->currentIndex();
    int col = col_Box->currentIndex();

    for(i=0; i<index; i++)
    {
        if(Prot->active_Channels & (0x0f<<4*i)) num++;
    }

    QVector<QVector<QVector2D>*> *list_Fit;
    QMap<int, QVector<double>*>  *map_Raw;
    int type = type_Box->currentIndex();
    switch(type)
    {
    case 0:     list_Fit = &list_FitMC;
                map_Raw = &map_RawMC;
                break;

    case 1:
    default:    list_Fit = &list_FitdFdT;
                map_Raw = &map_RawdFdT;
                break;
    }

    Fit.clear();
    Meas.clear();

    int id = num*count_row*count_col + row*count_col + col;
    vec2D = list_Fit->at(id);
    Fit = *vec2D;

    vec = map_Raw->value(index);
    id = row*count_col + col;
    for(i=0; i<count_meas; i++)
    {
        P.setX(X_MeltCurve.at(i));
        P.setY(vec->at(i+id*count_meas));
        Meas.append(P);
    }

    // Labels
    double peak = map_Param.value(index)->at((row*count_col + col)*COUNT_RESULT_PARAM + 2);
    double ampl = map_Param.value(index)->at((row*count_col + col)*COUNT_RESULT_PARAM + 0);
    mc_chart->peak_Label->setText(tr("peak") +  QString(" = %1").arg(peak,0,'f',1));
    mc_chart->ampl_Label->setText(tr("amplitude") + QString(" = %1").arg(ampl,0,'f',0));


    Draw_Curves();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Draw_Curves()
{
    int i;
    QVector2D P;
    QVector<double> x,y;

    // Clear curves
    mc_chart->Measure_curve->setData(NULL);
    mc_chart->Fit_curve->setData(NULL);

    // Load Data
    foreach(P, Meas)
    {
        x.append(P.x());
        y.append(P.y());
    }
    mc_chart->Measure_curve->setSamples(x,y);

    x.clear();
    y.clear();
    foreach(P, Fit)
    {
        x.append(P.x());
        y.append(P.y());
    }
    mc_chart->Fit_curve->setSamples(x,y);


    mc_chart->updateAxes();
    mc_chart->show();
    mc_chart->replot();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Display_ProgressBar(int percent, QString text)
{
   main_progress->setValue(percent);
   PrBar_status->setText(text);
   PrBar_status->setMinimumSize(PrBar_status->sizeHint());
   qApp->processEvents();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Melt_Curve::Clear_AllMaps()
{
    int i,j;
    QVector<double> *vec;
    QVector<QVector2D> *vec2D;


    X_MeltCurve.clear();

    for(i=0; i<map_RawMC.size(); i++)
    {
        vec = map_RawMC.values().at(i);
        vec->clear();
        delete vec;
    }
    map_RawMC.clear();

    for(i=0; i<map_RawdFdT.size(); i++)
    {
        vec = map_RawdFdT.values().at(i);
        vec->clear();
        delete vec;
    }
    map_RawdFdT.clear();

    for(i=0; i<list_FitMC.size(); i++)
    {
        vec2D = list_FitMC.at(i);
        vec2D->clear();
        delete vec2D;
    }
    list_FitMC.clear();

    for(i=0; i<list_FitdFdT.size(); i++)
    {
        vec2D = list_FitdFdT.at(i);
        vec2D->clear();
        delete vec2D;
    }
    list_FitdFdT.clear();

    for(i=0; i<map_Param.size(); i++)
    {
        vec = map_Param.values().at(i);
        vec->clear();
        delete vec;
    }
    map_Param.clear();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
MeltCurvePlot::MeltCurvePlot(QWidget *parent):
    QwtPlot(parent)
{
    QString text;
    QwtText text_x, text_y;
    QwtText title;

    QFont font_title = qApp->font(); //font_plot;
    font_title.setBold(false);
    font_title.setPointSize(font_title.pointSize()+1);
    title.setFont(font_title);

    canvas()->setCursor(Qt::ArrowCursor);

    // Measure Curve
    Measure_curve = new QwtPlotCurve(tr("Raw data"));
    Measure_curve->setStyle(QwtPlotCurve::Dots);
    Measure_curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse,
                                   QBrush(Qt::green),
                                   QPen(Qt::black, 1),
                                   QSize(3,3)));
    Measure_curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
    Measure_curve->attach(this);

    // Fit_curve
    Fit_curve = new QwtPlotCurve(tr("Fit_curve"));
    Fit_curve->setStyle(QwtPlotCurve::Lines);
    Fit_curve->setPen(Qt::blue,2);
    Fit_curve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    Fit_curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
    Fit_curve->attach(this);    

    // Axis
    QFont f = qApp->font();
    f.setBold(false);
    f.setPointSize(f.pointSize()-1);
    setAxisFont(QwtPlot::yLeft, f);
    setAxisFont(QwtPlot::xBottom, f);

    setAxisAutoScale(QwtPlot::yLeft, true);
    setAxisAutoScale(QwtPlot::xBottom, true);

    // Grid
    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    // Title
    //title.setText(tr("Melting Curve"));
    //setTitle(title);
    text_x.setText(tr("Temperature, C"));
    text_x.setFont(font_title);
    setAxisTitle(QwtPlot::xBottom, text_x);
    text_y.setText(tr("Fluor..."));
    text_y.setFont(font_title);
    //setAxisTitle(QwtPlot::yLeft, text_y);

    //... Legend ...
    legend = new QwtLegend();
    insertLegend(legend, QwtPlot::BottomLegend);

    //... Labels
    peak_Label = new QLabel("peak = ", this);
    ampl_Label = new QLabel("amplitude = ", this);
    QHBoxLayout *graph_H_layout_0 = new QHBoxLayout();
    QHBoxLayout *graph_H_layout_1 = new QHBoxLayout();
    QVBoxLayout *graph_V_layout = new QVBoxLayout();
    graph_H_layout_0->setMargin(0);
    graph_H_layout_1->setMargin(0);
    graph_H_layout_0->addWidget(peak_Label,0,Qt::AlignLeft);
    graph_H_layout_1->addWidget(ampl_Label,0,Qt::AlignLeft);
    canvas()->setLayout(graph_V_layout);
    graph_V_layout->addLayout(graph_H_layout_0,0);
    graph_V_layout->addLayout(graph_H_layout_1,0);
    graph_V_layout->addStretch();


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
MeltCurvePlot::~MeltCurvePlot()
{
    delete grid;
    delete legend;
    delete peak_Label;
    delete ampl_Label;

    delete Measure_curve;
    delete Fit_curve;

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ResultsMCItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text,str;
    int flag;
    bool ok;
    QStringList list;
    QFont f = qApp->font();
    painter->setFont(f);

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;
    text = index.data().toString();
    int row = index.row();
    int col = index.column();

    QPixmap pixmap(":/images/error_16.png");
    QPixmap pixmap_ch(":/images/fam_flat.png");
    QColor bg_color = Qt::white;

    // Background
    painter->fillRect(rect, bg_color);

    // 1. Mean value
    //if(row < 4 || (row >= 5 && row <= 8) || (row >= 10 && row <= 13))
    if(row < count_ActiveCh || (row >= count_ActiveCh+1 && row <= count_ActiveCh*2) || (row >= count_ActiveCh*2+2 && row <= count_ActiveCh*3+1))
    {
        switch(col)
        {
        case 0: painter->drawText(rect, Qt::AlignCenter, text); break;

        case 1:
                /*switch(row)
                {
                default:
                case 10:
                case 5:
                case 0: pixmap_ch.load(":/images/fam_flat.png");    break;

                case 11:
                case 6:
                case 1: pixmap_ch.load(":/images/hex_flat.png");    break;

                case 12:
                case 7:
                case 2: pixmap_ch.load(":/images/rox_flat.png");    break;

                case 13:
                case 8:
                case 3: pixmap_ch.load(":/images/cy5_flat.png");    break;
                }*/
                if(row == 0 || row == count_ActiveCh+1 || row == count_ActiveCh*2+2) pixmap_ch.load(":/images/fam_flat.png");
                if(row == 1 || row == count_ActiveCh+2 || row == count_ActiveCh*2+3) pixmap_ch.load(":/images/hex_flat.png");
                if(row == 2 || row == count_ActiveCh+3 || row == count_ActiveCh*2+4) pixmap_ch.load(":/images/rox_flat.png");
                if(row == 3 || row == count_ActiveCh+4 || row == count_ActiveCh*2+5) pixmap_ch.load(":/images/cy5_flat.png");
                if(row == 4 || row == count_ActiveCh+5 || row == count_ActiveCh*2+6) pixmap_ch.load(":/images/cy55_flat.png");

                painter->drawPixmap(rect.left(), rect.top() + (rect.height()-8)/2 - 2, pixmap_ch);
                rect.setLeft(rect.left() + 10);
                painter->drawText(rect, Qt::AlignCenter, text); break;

        case 2:
                if(text.isEmpty()) break;
                id = text.toInt(&ok);
                if(ok && id > 0) pixmap.load(":/images/check_16.png");
                else  pixmap.load(":/images/error_16.png");
                painter->drawPixmap(option.rect.x() + (option.rect.width()-16)/2,
                                    option.rect.y() + (option.rect.height()-16)/2,
                                    pixmap);
                    break;
        default:    break;
        }
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ResultsSpectrumItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text,str;
    int flag;
    bool ok;
    QStringList list;
    QFont f = qApp->font();
    painter->setFont(f);

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;
    text = index.data().toString();
    int row = index.row();
    int col = index.column();

    QPixmap pixmap(":/images/error_16.png");
    QPixmap pixmap_ch(":/images/fam_flat.png");
    QColor bg_color = Qt::white;

    // Background
    painter->fillRect(rect, bg_color);

    // 1. Mean value
    if(row < count_ActiveCh || (row >= count_ActiveCh+1 && row <= count_ActiveCh*2))
    {
        switch(col)
        {
        case 0: painter->drawText(rect, Qt::AlignCenter, text); break;

        case 1:
                /*switch(row)
                {
                default:
                //case 10:
                case 5:
                case 0: pixmap_ch.load(":/images/fam_flat.png");    break;

                //case 11:
                case 6:
                case 1: pixmap_ch.load(":/images/hex_flat.png");    break;

                //case 12:
                case 7:
                case 2: pixmap_ch.load(":/images/rox_flat.png");    break;

                //case 13:
                case 8:
                case 3: pixmap_ch.load(":/images/cy5_flat.png");    break;
                }*/
                if(row == 0 || row == count_ActiveCh+1) pixmap_ch.load(":/images/fam_flat.png");
                if(row == 1 || row == count_ActiveCh+2) pixmap_ch.load(":/images/hex_flat.png");
                if(row == 2 || row == count_ActiveCh+3) pixmap_ch.load(":/images/rox_flat.png");
                if(row == 3 || row == count_ActiveCh+4) pixmap_ch.load(":/images/cy5_flat.png");
                if(row == 4 || row == count_ActiveCh+5) pixmap_ch.load(":/images/cy55_flat.png");

                painter->drawPixmap(rect.left(), rect.top() + (rect.height()-8)/2 - 2, pixmap_ch);
                rect.setLeft(rect.left() + 10);
                painter->drawText(rect, Qt::AlignCenter, text); break;

        case 2:
                if(text.isEmpty()) break;
                id = text.toInt(&ok);
                if(ok && id > 0) pixmap.load(":/images/check_16.png");
                else  pixmap.load(":/images/error_16.png");
                painter->drawPixmap(option.rect.x() + (option.rect.width()-16)/2,
                                    option.rect.y() + (option.rect.height()-16)/2,
                                    pixmap);
                    break;
        default:    break;
        }
    }

    if(row == count_ActiveCh*2+2)
    {
        switch(col)
        {
        case 0: painter->drawText(rect, Qt::AlignCenter, text); break;

        case 1: painter->drawText(rect, Qt::AlignCenter, text); break;

        case 2: if(text.isEmpty()) break;
                id = text.toInt(&ok);
                if(ok && id > 0) pixmap.load(":/images/check_16.png");
                else  pixmap.load(":/images/error_16.png");
                painter->drawPixmap(option.rect.x() + (option.rect.width()-16)/2,
                                    option.rect.y() + (option.rect.height()-16)/2,
                                    pixmap);
                    break;
        default:    break;
        }
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void FluorMC_ItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QRect rect = option.rect;
    QString text = index.data().toString();
    QFont font;
    QPixmap pixmap(":/images/fam_flat.png");
    int row = index.row();

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background

    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(255,255,255)); //QColor(255,255,225)
    }

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    //font = GraphPlot::font_plot;
    //font = painter->font();
    //font.setBold(true);
    //painter->setFont(QFont("Comic Sans MS", 10, QFont::Normal));

    if(option.state & QStyle::State_Enabled)
    {
        switch(row)
        {
        case 0:  pixmap.load(":/images/fam_flat.png");    break;
        case 1:  pixmap.load(":/images/hex_flat.png");    break;
        case 2:  pixmap.load(":/images/rox_flat.png");    break;
        case 3:  pixmap.load(":/images/cy5_flat.png");    break;
        case 4:  pixmap.load(":/images/cy55_flat.png");   break;
        }
    }
    else
    {
        painter->setPen(QPen(QColor(192,192,192) ,1,Qt::SolidLine));
        pixmap.load(":/images/disable_flat.png");
    }

    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter->drawPixmap(0, rect.y()+2, pixmap);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void RowColMC_ItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QRect rect = option.rect;
    QString text = index.data().toString();

    int row = index.row();

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background

    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(255,255,255)); //QColor(255,255,225)
    }

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));

    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TableColorItemDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    int i,id;
    int delta;
    int r,g,b;
    QRect rect;
    QString text;
    double value;
    bool ok;
    QColor color;
    QFont f = qApp->font();
    painter->setFont(f);

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;
    text = index.data().toString();
    value = text.toDouble(&ok);
    if(!ok) value = 0.;
    int row = index.row();
    int col = index.column();

    QColor bg_color = Qt::white;

    // Background
    painter->fillRect(rect, bg_color);

    // 1.
    if(value > 0)
    {        
        delta = (value - min_Value)*510./(max_Value - min_Value);
        if(delta <= 0xff)
        {
            r = 0;
            g = delta;
            b = 0xff - delta;
        }
        else
        {
            delta = (delta - 0xff) & 0xff;
            b = 0;
            r = delta;
            g = 0xff - delta;
        }
        //qDebug() << "rgb: " << r << g << b << text << row << col << delta << value;
        painter->fillRect(rect, QColor(r,g,b));

        if(view_value)
        {
            id = 2;
            if(type_view == 1) id = 0;
            text = QString::number(value,'f',id);
            if(mode_view == 1)
            {
                f.setBold(true);
                f.setPointSize(f.pointSize()+5);
                painter->setFont(f);
            }

            painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
        }
    }

}
