#ifndef UTILITY
#define UTILITY

#if defined(PROTOCOL_LIBRARY)
#  define PROTOCOLSHARED_EXPORT __declspec(dllexport)
#else
#  define PROTOCOLSHARED_EXPORT __declspec(dllimport)
#endif

#include <QObject>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QColor>
#include <QDebug>
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <QChar>
#include <QTime>
#include <QCryptographicHash>


#include <QDomDocument>


typedef void (callback_process)(void*, int, QString);


PROTOCOLSHARED_EXPORT  void Parsing_ProgramAmplification(rt_Protocol*);                    // Parsing Programm
PROTOCOLSHARED_EXPORT  void Check(void *, callback_process* Callback);

PROTOCOLSHARED_EXPORT  bool SaveAsXML(void *, callback_process*, rt_Protocol *p, QString fn, QString, bool only_config = false); // Save as XML
PROTOCOLSHARED_EXPORT  bool SaveResultsXML(void *, callback_process*, rt_Protocol *p, QString fn_in, QString, QString fn_out="");  // save resutls

PROTOCOLSHARED_EXPORT   int Read_XML(void *, callback_process*, rt_Protocol *p, QString fn, QString, bool only_config = false);// Read XML protocol
PROTOCOLSHARED_EXPORT   int Read_r96(void *, callback_process*, rt_Protocol *p, QString fn, QString);// Read OLD(*.r96) protocol
PROTOCOLSHARED_EXPORT   int Transpose_RawData(void *, callback_process*, rt_Protocol*, QString);     // Transpose RawData
PROTOCOLSHARED_EXPORT   int Filtered_RawData(void *, callback_process*, rt_Protocol*, QString);      // Filtered RawData
PROTOCOLSHARED_EXPORT   int Bace_Data(void *, callback_process*, rt_Protocol*, QString);             // Bace and Fitting PCR Data
PROTOCOLSHARED_EXPORT   int dFdT_MCData(void *, callback_process*, rt_Protocol*, QString, bool negative = false);           // dF/dT Melting Curve Data

PROTOCOLSHARED_EXPORT   void Calculate_Ct(rt_Protocol*, int ch);                            // Calculate Ct

PROTOCOLSHARED_EXPORT   int Convert_NameToIndex(QString name, int column_count);            // Name(A1..) to Index(0-95(383))
PROTOCOLSHARED_EXPORT   QString Convert_IndexToName(int pos, int column_count);             // Index(0-95(383)) to Name(A1..)
PROTOCOLSHARED_EXPORT   QString FindNameByTest(rt_Test *ptest, int id_tube, int id_channel);//

PROTOCOLSHARED_EXPORT   int CrossTalk_Correction(rt_Protocol*);                             // CrossTalk correction

PROTOCOLSHARED_EXPORT   void Criterion_Structure(rt_Protocol *p);                           // create Criterion

//... xml utility ...

PROTOCOLSHARED_EXPORT   QDomElement MakeElement(QDomDocument &doc, QString name, QString text);
PROTOCOLSHARED_EXPORT   QDomElement MakeElementResults(QDomDocument &doc, QString name, QVector<string> *v);
PROTOCOLSHARED_EXPORT   QDomElement SaveXML_Test(QDomDocument &doc, rt_Test *ptest);
PROTOCOLSHARED_EXPORT   QDomElement SaveXML_SourceSet(QDomDocument &doc, rt_GroupSamples *pgroup, rt_Protocol *p);
PROTOCOLSHARED_EXPORT   QDomElement SaveXML_Sample(QDomDocument &doc, rt_Sample *psample, rt_Protocol *p);

PROTOCOLSHARED_EXPORT   QDomElement SaveXML_Device(QDomDocument &doc, rt_Protocol *p);
PROTOCOLSHARED_EXPORT   QDomElement SaveXML_Measurements(QDomDocument &doc, rt_Protocol *p);

PROTOCOLSHARED_EXPORT   QDomElement SaveXML_AnalysisCurves(QDomDocument &doc, rt_Protocol *p);
PROTOCOLSHARED_EXPORT   QDomElement SaveXML_AnalysisVersion(QDomDocument &doc, rt_Protocol *p);
PROTOCOLSHARED_EXPORT          void LoadXML_Test(QDomNode &node, rt_Test *ptest);
PROTOCOLSHARED_EXPORT          void LoadXML_Group(QDomNode &node, rt_GroupSamples *group, rt_Protocol *p);
PROTOCOLSHARED_EXPORT          void LoadXML_Sample(QDomNode &node, rt_Sample *sample, rt_Protocol *p);
PROTOCOLSHARED_EXPORT          void LoadXML_Tube(QDomNode &node, rt_Tube *tube, rt_Protocol *p);
PROTOCOLSHARED_EXPORT          void LoadXML_Channel(QDomNode &node, rt_Channel *channel, rt_Protocol *p);
PROTOCOLSHARED_EXPORT          void LoadXML_Measure(QDomNode &node, rt_Measurement *measure);
PROTOCOLSHARED_EXPORT          void LoadXML_Preference(QDomNode &node, rt_Preference *preference);

                        void Adding_DropInformation(QDomDocument &doc, QDomElement &element, rt_Protocol *p);

//... r96 format test ...

                        void Load_BiocenoseChannel(rt_Test*, QString);          // Info about names channel (Biocenose)
                        void Load_HLAChannel(rt_Test*, QString);                // Info about names channel (HLA_DQ)

//.......................

PROTOCOLSHARED_EXPORT void Sleep(int ms);
PROTOCOLSHARED_EXPORT QString GetRandomString(int count = 12);


#endif // UTILITY

