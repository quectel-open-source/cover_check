#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include "socketMain.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <json/config.h>
#include <json/json.h>
#include<sys/time.h>
#include "SettingConfig.h"

static unsigned  buffUsed;
static char *picBuf;

char * get_pic_mem(void)
{

  if(picBuf == NULL){
      picBuf = (char *)malloc(IMAGE_BUFF_LEN);
    }
  return picBuf;
}
    
void free_pic_mem(void)
{
  if(picBuf){
       free(picBuf);
	picBuf = NULL;
	buffUsed = 0;
    }
}

void rst_pic_mem(void)
{
    if(picBuf){
    memset(picBuf, 0, IMAGE_BUFF_LEN);
	}
    buffUsed = 0;
}

unsigned save_file(char *path)
{
    int ret = 0;
    int fd;
    unsigned size;

    fd = open(path, O_RDWR | O_CREAT,0640);
    if (fd < 0) {
        printf("open faild\n");
        return ret;
    }

    size = write(fd, (void *)picBuf, buffUsed);
    sync();
    close(fd);
    return size;
}

static void image_data_process(char *data, unsigned size)
{
    if (data) {
        buffUsed += size;
        memcpy(picBuf + buffUsed, data, size);
    }
}

static int resol_head(char *str, int type, void *h)
{
    int ret = 0;
    char *s, *pstr = NULL;
    int id;
    struct serverReqPamara *para1;
    struct clientResPamara *para2;
    struct heartBeat *para3;
    int deviceId;
    char detectNo[DETECTNO_LEN] = {0};
	unsigned int       areaThreshold;
	unsigned int       lengthThreshold;

    if ((str == NULL) || (h == NULL)) {
        printf("para err\n");
        return -1;
    }

    if (pstr = strstr(str, "DeviceID")) {
        pstr += 9;
        deviceId = atoi(pstr);
    } else if (pstr = strstr(str, "DetectNo")) {
        pstr += 9;
        strncpy(detectNo, pstr, strlen(pstr));
    }
	
	else if (pstr = strstr(str, "AreaThreshold")) {
        pstr += 14;
		areaThreshold = atoi(pstr);
    }else if (pstr = strstr(str, "LengthThreshold")) {
        pstr += 16;
        lengthThreshold = atoi(pstr);
    }
	else if (pstr = strstr(str, "DeviceNo")) {
        pstr += 9;
        id = atoi(pstr);
        para2->deviceNo = id;
    } else if (pstr = strstr(str, "DefectType")) {
        pstr += 11;
        para2->defectType = id;
    } else if (pstr = strstr(str, "DefectDes")) {
        pstr += 10;
        strncpy(para2->defectDes, pstr, 64);
    } else if (pstr = strstr(str, "PicLen")) {
        pstr += 7;
        para2->picLen = id;
    } else if (pstr = strstr(str, "DataLen")) {
        pstr += 8;
        para2->dataLen = id;
    } else if (pstr = strstr(str, "PicData")) {
        pstr += 8;
        if(para2->picData){
          memcpy(para2->picData, pstr, para2->dataLen); 
	      image_data_process(para2->picData, para2->dataLen);
	    }
    }

    switch (type) {
        case AIDETECT:
            para1 = (struct serverReqPamara *)h;
            strncpy(para1->detectNo, detectNo, strlen(detectNo));
			para1->areaThreshold = areaThreshold;
			para1->lengthThreshold = lengthThreshold;
            break;
        case AIRESULT:
            para2 = (struct clientResPamara *)h;
            para2->deviceId = deviceId;
            strncpy(para2->detectNo, detectNo, strlen(detectNo));
            break;
        case HEARTBEAT:
            para3 = (struct heartBeat *)h;
            para3->deviceId = deviceId;
            break;
        default:
            printf("unkown head type:%d\n", type);
            ret = -1;
    }

    return ret;

}

//str:: cmd=AiResult; DeviceID= 00;DetectNo=20220621162345;DeviceNo=����; DefectType= 00;DevectDes=����;PicLen=10240;DataLen=1024;PicData=xxx&&
int resolv_pack(char *str, void *buffer)
{
    unsigned len;
    int head_t;
    char *pstr, *pstr0;
    struct serverReqPamara *para1 = (struct serverReqPamara *)buffer;
    struct clientResPamara *para2 = (struct clientResPamara *)buffer;
    struct heartBeat *para3 = (struct heartBeat *)buffer;

    if ((str == NULL) || (buffer == NULL)) {
        printf("para err\n");
        return -1;
    }

    pstr = strtok(str, ";");
    printf("++++pstr:%s\n", pstr);
    if (pstr0 = strstr(pstr, "cmd")) {
        pstr0 += 4;
        if (!strcmp(pstr0, "Heartbeat")) {
			printf("resolv_pack Heartbeat\n");
            head_t = HEARTBEAT;
            snprintf(para3->cmd, CMD_LEN_MAX, "%s", pstr0);
        } else if (!strcmp(pstr0, "AiDetect")) {
			printf("resolv_pack AiDetect\n");
            head_t = AIDETECT;
            snprintf(para1->cmd, CMD_LEN_MAX, "%s", pstr0);
        } else if (!strcmp(pstr, "AiResult")) {
			printf("resolv_pack AIRESULT\n");
            head_t = AIRESULT;
            snprintf(para2->cmd, CMD_LEN_MAX, "%s", pstr0);
        } else {
            head_t = -1;
        }
    }
    printf("---pstr:%s\n", pstr);

    while (pstr) {
        pstr = strtok(NULL, ";");
        printf("pstr:%s\n", pstr);
        if (pstr != NULL) {
            resol_head(pstr, head_t, buffer);
        }
    }
    return head_t;
}

int get_head(void *para, int type, void *buffer)
{
    char *str = NULL;
    int n;
    struct serverReqPamara *para1;
    struct clientResPamara *para2;
	struct clientRawPamara *rawImg;
    struct heartBeat *para3;

    str = (char *)buffer;
    switch (type) {
        case AIDETECT:
            para1 = (struct serverReqPamara *)para;
            n = snprintf(str, TEMP_BUFF_LEN, "cmd=%s;", para1->cmd);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DetectNo=%s", para1->detectNo);
	        str += n;
			n = snprintf(str, TEMP_BUFF_LEN, "AreaThreshold=%d", para1->areaThreshold);
	        str += n;
			n = snprintf(str, TEMP_BUFF_LEN, "LengthThreshold=%d", para1->lengthThreshold);
	        str += n;
            break;
        case AIRESULT:
            para2 = (struct clientResPamara *)para;
            n = snprintf(str, TEMP_BUFF_LEN, "cmd=%s;", para2->cmd);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DeviceID=%02d;", para2->deviceId);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DetectNo=%s;", para2->detectNo);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DeviceNo=%d;", para2->deviceNo);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DefectType=%d;", para2->defectType);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DefectDes=%s;", para2->defectDes);
            str += n;
			n = snprintf(str, TEMP_BUFF_LEN, "InnerThickness=%d;", para2->innerThickness);
            str += n;
			n = snprintf(str, TEMP_BUFF_LEN, "OuterThickness =%d;", para2->outerThickness);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "PicLen=%d;", para2->picLen);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DataLen=%d;", para2->dataLen);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "PicData=");
            str += n;
			//printf("AIresult buffer:%s \n", str);
            break;
        case HEARTBEAT:
            para3 = (struct heartBeat *)para;
            n = snprintf(str, TEMP_BUFF_LEN, "cmd=%s;", para3->cmd);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DeviceID=%02d&&", para3->deviceId);
            str += n;
            break;
		case AIRAW:
            rawImg = (struct clientRawPamara *)para;
            n = snprintf(str, TEMP_BUFF_LEN, "cmd=%s;", rawImg->cmd);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DeviceID=%02d;", rawImg->deviceId);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DetectNo=%s;", rawImg->detectNo);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "PicLen=%d;", rawImg->picLen);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "DataLen=%d;", rawImg->dataLen);
            str += n;
            n = snprintf(str, TEMP_BUFF_LEN, "PicData=");
            str += n;
			//printf("airaw buffer:%s \n", str);
            break;	
        default:
            printf("unkown head type:%d\n", type);
            return -1;
    }
    n = (str - (char *)buffer);
    //printf("head:%s\n", (char *)buffer);
    return n;
}


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
//Add by kelvin
#include <fstream>
#include <iostream>
#include <json/config.h>
#include <json/json.h>
#include <string>

using namespace std;

//注意json的释放问题，
//string类资源的释放问题处理
/*
 * 创建通用回复json数据body
 */
char *messageResponseJsonBodyCreate(int result)
{
    Json::Value root;
    root["result"] = result;
    string strJson = root.toStyledString();
    char *message = (char *)malloc(strlen((char *)strJson.c_str()) + 1);
    strcpy(message, (char *)strJson.c_str());
    return message;
}

/*
 * 创建注册json数据body
 */
char *messageLoginJsonBodyCreate(char *deviceId, char *clientIp)
{
    Json::Value root;
    root["deviceId"] = deviceId;
    root["clientIp"] = clientIp;
    root["channelNos"].append("0");
    root["channelNos"].append("1");
    string strJson = root.toStyledString();
    char *message = (char *)malloc(strlen((char *)strJson.c_str()) + 1);
    strcpy(message, (char *)strJson.c_str());
    return message;
}

/*
 * 创建心跳包json数据body
 */
char *messageHeatBeatJsonBodyCreate(char *deviceId, char *clientIp)
{
    Json::Value root;
    root["deviceId"] = deviceId;
    root["clientIp"] = clientIp;
    root["channelNos"].append("0");
    root["channelNos"].append("1");
    string strJson = root.toStyledString();
    char *message = (char *)malloc(strlen((char *)strJson.c_str()) + 1);
    strcpy(message, (char *)strJson.c_str());
    return message;
}

/*
 * 创建检测结果信息上传
 */
char *messageDetectResultJsonBodyCreate(char *channelNo, char *rawImgPath, char* markImgPath, 
                                    char* defectTypes, char*   defectDesc, char *snapTimeMillis,
                                    int     algorithmTimeConsuming, int     checkTotalNumber,
                                    int checkNgNumber, double  yieldRate)
{
    Json::Value root;
    root["channelNo"]       = Json::Value("0");
    root["rawImgPath"]      = Json::Value(rawImgPath);
    root["markImgPath"]     = Json::Value(markImgPath);
    root["defectDesc"]      = Json::Value(defectDesc);

    root["snapTimeMillis"]          = Json::Value(snapTimeMillis);
    root["algorithmTimeConsuming"]  = Json::Value(algorithmTimeConsuming);
    root["checkTotalNumber"]        = Json::Value(checkTotalNumber);
    root["checkNgNumber"]           = Json::Value(checkNgNumber);
    root["yieldRate"]               = Json::Value(yieldRate);
    
    //暂定3个，后面做修改，
    root["defectTypes"].append(defectTypes[0]-'0');
    root["defectTypes"].append(defectTypes[1]-'0');
    string strJson = root.toStyledString();
    char *message = (char *)malloc(strlen((char *)strJson.c_str()) + 1);
    strcpy(message, (char *)strJson.c_str());   
    printf("message:%s\n", message);
    return message;
}

 unsigned char *file_to_buf(char *name, int *size)
{
	unsigned char *buf = NULL;
	FILE *fp = fopen(name, "r");
	if(fp == NULL)
	{
		printf("Error in file：%s\n", name);
	}
	fseek(fp,0L,SEEK_END);  //定位到文件末尾
	int flen = ftell(fp);
	buf = (unsigned char *)malloc(flen);
	if(buf == NULL)
	{
		fclose(fp);
		return 0;
	}
	fseek(fp, 0L, SEEK_SET); //定位到文件开头
	fread(buf, flen, 1, fp);  //一次性读取全部文件内容
	printf("file flen is %d\n\n",flen);
	fclose(fp);
	*size = flen;
	return buf;
}

/*
 * 创建终端配置参数json数据body
 */
char *messageConfigJsonBodyCreate(char *configJsonName)
{
    int size;
    char *message = (char *)file_to_buf(configJsonName, &size);
    printf("message:%s,size:%d\n", message, size);
    return message;
}

/*
 * 解析应答包result的值
 */
int messageResponseJsonBodyParse(char *jsonStr)
{
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(jsonStr, root))
    {
        int result = root["result"].asInt();  
        return result;
    }
    else
    {
        return -1;
    }
}

/*
 * 解析应答包result的值
 */
int messageControlJsonBodyParse(char *jsonStr)
{
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(jsonStr, root))
    {
        std::string target = root["target"].asString();
        cout << target;
        return 0;
    }
    else
    {
        return -1;
    }
}

