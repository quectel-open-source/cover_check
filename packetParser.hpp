
#ifndef _PACKETPARSER_H__
#define _PACKETPARSER_H__

int resolv_pack(char *str, void *buffer);
int get_head(void *para, int type, void *buffer);
int get_pic_mem(void);
void free_pic_mem(void);
void rst_pic_mem(void);
unsigned save_file(char *path);

char *messageLoginJsonBodyCreate(char *deviceId, char *clientIp);

char *messageHeatBeatJsonBodyCreate(char *deviceId, char *clientIp);

char *messageConfigJsonBodyCreate(char *configJsonName);

int messageResponseJsonBodyParse(char *jsonStr);

char *messageResponseJsonBodyCreate(int result);

char *messageDetectResultJsonBodyCreate(char *channelNo, char *rawImgPath, char* markImgPath, 
                                    char* defectTypes, char*   defectDesc, char *snapTimeMillis,
                                    int     algorithmTimeConsuming, int     checkTotalNumber,
                                    int     checkNgNumber, double  yieldRate);
#endif

