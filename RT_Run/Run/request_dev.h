#ifndef REQUEST_DEV_H
#define REQUEST_DEV_H

//.............................................................................
#define INFO_DATA           "InfoData"

#define INFO_status         "status_dev"
#define INFO_rdev           "rdev"
#define INFO_Temperature    "Temperature_dev"
#define INFO_fn             "fn"
#define INFO_time           "time"
#define INFO_Levels         "Levels_dev"
#define INFO_isMeasuring    "isMeasuring"
#define INFO_pressRunButton "pressRunButton"
#define INFO_fmode          "fmode"
//.............................................................................
#define INFO_DEVICE         "InfoDevice"

#define INFODEV_version     "version"
#define INFODEV_serName     "serial_Name"
#define INFODEV_devMask     "fluor_Mask"
#define INFODEV_thermoBlock "type_ThermoBlock"
#define INFODEV_parameters  "dev_Parameters"

#define INFODEV_SpectralCoeff   "spectral_coeff"
#define INFODEV_OpticalCoeff    "optical_coeff"
#define INFODEV_UnequalCoeff    "unequal_coeff"

#define INFODEV_TempProgram "temperature_program"
#define INFODEV_devHW       "device_hw"

//.............................................................................
#define INFO_PROTOCOL       "InfoProtocol"
#define RUN_REQUEST         "RUN"

#define run_name            "run_name"
#define run_programm        "run_programm"
#define run_activechannel   "run_activechannel"
#define run_IDprotocol      "run_IDprotocol"
#define run_operator        "run_operator"
//.............................................................................
#define STOP_REQUEST        "STOP"
#define PAUSE_REQUEST       "PAUSE"
#define CONTINUE_REQUEST    "CONTINUE"

#define OPENBLOCK_REQUEST   "OPEN_BLOCK"
#define CLOSEBLOCK_REQUEST  "CLOSE_BLOCK"
#define barcode_name         "barcode_name"
//.............................................................................
#define MEASURE_REQUEST     "MEASURE"

#define MEASURE_Data        "MEASURE_Data"
//.............................................................................
#define EXECCMD_REQUEST     "EXECCMD"

#define EXECCMD_CMD         "cmd_string"
#define EXECCMD_UC          "uc_name"
#define EXECCMD_ANSWER      "answer"
#define EXECCMD_SENDER      "sender"
//.............................................................................
#define PRERUN_REQUEST     "PRERUN"
//.............................................................................
#define GETPIC_REQUEST      "GETPIC"

#define GETPIC_CHANNEL      "channel"
#define GETPIC_EXP          "exposition"
#define GETPIC_CTRL         "control"
#define GETPIC_DATA         "GETPIC_Data"
#define GETPIC_VIDEO        "GETPIC_Video"
//.............................................................................
#define SAVEPAR_REQUEST     "SAVEPAR"

#define SAVEPAR_CTRL        "control"
#define SAVEPAR_DATA        "SAVEPAR_Data"
//.............................................................................
#define CONNECT_REQUEST     "ISCONNECTED"
#define CONNECT_STATUS      "status"        // can be READY or BUSY

//.............................................................................
#define SECTORREAD_REQUEST  "sector_read"
#define SECTORWRITE_REQUEST "sector_write"

#define SECTOR_DATA         "sector_data"
#define SECTOR_CMD          "sector_cmd"

//.............................................................................
#define ID_PARAM            "id_param"

#define DEVICE_REQUEST      "DEV_STATUS"
#define PRESS_BTN_RUN       "press_btn_run"

//.............................................................................
#define HTMEAS_REQUEST       "HIGHTUBEMEAS_REQUEST"
#define HIGHTUBE_SAVE        "HIGHTUBE_SAVE"

//.............................................................................
#define DTMASTERLOG_REQUEST  "DTMASTERLOG_REQUEST"
#define DTMASTERLOG_DATA     "DTMASTERLOG_DATA"

//.............................................................................
#define GETALLVIDEO_REQUEST    "GETALLVIDEO_REQUEST"

#define OPTICMASK_DATA          "OPTICMASK_DATA"
#define OPTICFN_DATA            "OPTICFN_DATA"

//.............................................................................
#define CRYPTO_REQUEST       "CRYPTO_REQUEST"

#define CRYPTO_CTRL          "crypto_ctrl"
#define CRYPTO_DATA          "crypto_data"

//... Error ...................................................................
enum CodeErrors
{
    NONE = 0,              // 0
    INITIALISE_ERROR,      // 1  ошибка инициализации (возможно в командной строке неверно указан номер прибора)
    USB_ERROR,             // 2  ошибка USB
    CAN_ERROR,             // 3  ошибка на CAN канале
    USBBULK_ERROR,         // 4  ошибка записи чтения bulk USB
    NOTREADY,              // 5  прибор не готов к работе, требуется подождать завершнния инициализации
    DEVHW_ERROR,           // 6  ошибка работы с аппаратурой внутри прибора (необходимо выключение прибора)
    OPENINRUN,             // 7  попытка открытия термоблока в момент выполнения температурной программы
    MOTORALARM,            // 8  ошибка привода
    STARTONOPEN,           // 9  попытка выполнения температурной программы при открытом термоблоке
    PRERUNFAULT_ERROR,     // 10  ошибка при попытке выполнить подготовку к запуску программы
    PRERUNSEMIFAULT_ERROR, // 11 была ошибка при попытке выполнить подготовку к запуску, но она была устранена
    DATAFRAME_ERROR,       // 12 ошибка в принятии блока данных, число полученных данных не соответствует ожидаемому
    LEDSSETUP_ERROR,       // 13 ошибка позиционирования колеса с фильтрами, колесо не провернулось
    UNKNOWN_ERROR,         // 14
    EXTERNAL_TIMEOUT       // 15

};

#define CATALOGUE   INFO_DATA << INFO_DEVICE << RUN_REQUEST << STOP_REQUEST << PAUSE_REQUEST << MEASURE_REQUEST << OPENBLOCK_REQUEST\
                    << CLOSEBLOCK_REQUEST << EXECCMD_REQUEST << PRERUN_REQUEST << GETPIC_REQUEST << SAVEPAR_REQUEST\
                    << SECTORREAD_REQUEST << SECTORWRITE_REQUEST << HTMEAS_REQUEST << HIGHTUBE_SAVE << GETALLVIDEO_REQUEST << CRYPTO_REQUEST

#endif // REQUEST_DEV_H

