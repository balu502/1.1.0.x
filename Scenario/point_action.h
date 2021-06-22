#ifndef POINT_ACTION_H
#define POINT_ACTION_H

#include <QtWidgets>
#include <QWidget>
#include <QMap>
#include <QEvent>

#define OPEN_PROTOCOL           "OpenProtocol"
#define OPEN_DEVICE             "OpenDevice"
#define RUN_PROTOCOL            "RunProtocol"
#define RUN_STOP                "Stop"
#define GET_BARCODE             "GetBarcode"
#define OPEN_PROTOCOL_BARCODE   "OpenProtocolBarcode"
#define SCANCOEFFICIENTS        "ScanCoefficients"
#define SET_PLATE               "SetPlate"
#define OPEN_PROTOCOL_ID        "OpenProtocolID"
#define CHANGE_EXPOSURE         "ChangeExposure"
#define CHANGE_HEIGHT_TUBES     "ChangeHeightTubes"
#define BACKUP_EXPOSURE         "BackupExposure"
#define BACKUP_HEIGHT_TUBES     "BackupHeightTubes"
#define OPENBLOCK               "OpenBlock"
#define CLOSEBLOCK              "CloseBlock"

#define ANALYSIS_PROTOCOL       "AnalysisProtocol"

#define LIST_ACTIONPOINTS_RUN       OPEN_PROTOCOL << OPEN_DEVICE << RUN_PROTOCOL << RUN_STOP << GET_BARCODE << OPEN_PROTOCOL_BARCODE << SCANCOEFFICIENTS << SET_PLATE << OPEN_PROTOCOL_ID << CHANGE_EXPOSURE << CHANGE_HEIGHT_TUBES << BACKUP_EXPOSURE << BACKUP_HEIGHT_TUBES << OPENBLOCK << CLOSEBLOCK
#define LIST_ACTIONPOINTS_ANALYSIS  ANALYSIS_PROTOCOL




//-----------------------------------------------------------------------------
class Point_Action
{
    public:
    Point_Action()
    {
        receiver = NULL;
        sender = NULL;
        Unique_Name = "";
    }

    QWidget                     *receiver;
    QWidget                     *sender;
    QString                     Unique_Name;
    QHash<QString,QString>      Param;


    ~Point_Action()
    {
        Param.clear();
    }
};
//-----------------------------------------------------------------------------
class Action_Event: public QEvent
{
public:
    Action_Event(): QEvent((Type)2002)
    {
    }
    Point_Action    *point_action;
};

#endif // POINT_ACTION_H
