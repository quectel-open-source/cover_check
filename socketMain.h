#ifndef _SOCKETMAIN_H__
#define _SOCKETMAIN_H__

#define DETECTNO_LEN 32
#define CMD_LEN_MAX 16


enum {
    AINONE,
    AIDETECT,
    AIRESULT,
    HEARTBEAT,
    AIRAW,
    AICNTMAX,
};

struct heartBeat {
    char               cmd[CMD_LEN_MAX];
    char               deviceId;
};

struct serverReqPamara {
    char               cmd[CMD_LEN_MAX];
    char               detectNo[DETECTNO_LEN];
	unsigned int       areaThreshold;
	unsigned int       lengthThreshold;
};

struct clientResPamara {
    char               cmd[CMD_LEN_MAX];
    char               deviceId;
    char               detectNo[DETECTNO_LEN];
    char               deviceNo;
    char               defectType;
    char               defectDes[64];
	unsigned int       innerThickness;
	unsigned int       outerThickness; 
    unsigned int       picLen;
    unsigned int       dataLen;
    char              *picData;
};

struct clientRawPamara {
    char               cmd[CMD_LEN_MAX];
    char               deviceId;
    char               detectNo[DETECTNO_LEN];
    unsigned int       picLen;
    unsigned int       dataLen;
    char              *picData;
};


#define PORT 60000

#define IMAGE_BUFF_LEN (10*1024*1024)
#define TEMP_BUFF_LEN  (1024 + 128)

#endif
