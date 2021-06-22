#ifndef DEVICE_PARAM_H
#define DEVICE_PARAM_H

#define MAX_OPTCHAN 8
#define COEF_EXPO 0.308
#define COEF_EXPO_1 5

typedef struct
{
    char filter;					// номер светофильтра
    char nexp;       				// количество экспозиций(0-2)
    unsigned char light;			// яркость прожектора
    char led;						// номер прожектора
    short video_gain;				// усиление
    short blk_level;				// уровень чёрного (нач. смещение)
    unsigned short exp[2];			// 2 экспозиции
}Optics_Par;

typedef struct
{
    int sec_type;
    int sec_sequence_number;
    Optics_Par  optics_ch[MAX_OPTCHAN];
    short Rx;
    short Ry;
    char d_xy[16];                  // смещения для 8-ми каналов по "X" и "Y"
    char reserve[4];
    short X_Y[96*2];
}Device_Par;

typedef struct
{
    char id_median[8];              // "median"
    short median_xy[8];             //
    char reserve[488];
}Device_ParMedian;

typedef struct
{
    char id_default[8];             // "default"
    short Expo_def[16];             //
    char reserve[472];
}Device_ParDefault;

#endif // DEVICE_PARAM_H
