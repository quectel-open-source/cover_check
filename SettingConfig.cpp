#ifndef __SETTING_CONFIG_CPP__
#include <fstream>
#include <iostream>
#include <json/config.h>
#include <json/json.h>
#include <sys/time.h>
#include <unistd.h>
#include "ifaddrs.h"

#include<cstring>

#include "SettingConfig.h"
#include "ServerConnect.h"

using namespace std;

#include "ifaddrs.h"

int get_localip(const char * eth_name, char *local_ip_addr)
{
        int ret = -1;
   // register
        int fd;
    struct ifreq ifr;

        if (local_ip_addr == NULL || eth_name == NULL)
        {
        printf("[%s]line:%d\n", __func__, __LINE__);
                return ret;
        }
        if ((fd=socket(AF_INET, SOCK_DGRAM, 0)) > 0)
        {
                strcpy(ifr.ifr_name, eth_name);
                if (!(ioctl(fd, SIOCGIFADDR, &ifr)))
                {
                        ret = 0;
                        strcpy(local_ip_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
                }
        }
        if (fd > 0)
        {
                close(fd);
        }
    return ret;
}


static int difftimeval(const struct timeval *start, const struct timeval *end)
{
        double d;
        time_t s;
        suseconds_t u;

        s = end->tv_sec - start->tv_sec;
        u = end->tv_usec - start->tv_usec;

        d = s;
        d *= 1000000;
        d += u;

        return d;
}

SettingConfig::SettingConfig()
{
    deviceStr = (char *) malloc(64);
    ifstream eFile;
    eFile.open("/sys/class/net/eth0/address");
    eFile >> deviceStr; 
    cout << " eth0 data :" << deviceStr << endl;
    get_localip(CURRENT_ETH_NAME, localIpaddr);
    memset(&serverConfig, 0, sizeof(serverConfig));
    cout << " localIpaddr :" << localIpaddr << endl;
    Json::Reader reader;
    Json::Value root;
    ifstream in(CONFIG_JSON, ios::binary);
    //char buffer[256];
    //cout << buffer << endl;
    //const char* str = "{\"uploadid\": \"UP000000\",\"code\": 100,\"msg\": \"\",\"files\": \"\"}";  
    if (reader.parse(in, root))
    {
        parserLocalConfigValue(root);
        cout << "parserLocalConfigValue success" << endl;
        dumpConfig();
        cout << "dumpConfig success" << endl;
    }

#if 0
    FILE *file = fopen("/data/1.yuv", "r+");
    struct timeval lastTime, currentTime;
    char buffer[400*1024];
    fread(buffer, 1, 400*1024, file);

    
    gettimeofday(&lastTime, NULL);
    //FILE *data_out = fopen("/data/out.yuv", "w+");
    //fwrite(buffer, 1, 400*1024, data_out); 
    remove("/data/out.yuv");
    usleep(1000);
    gettimeofday(&currentTime, NULL);
    cout << " time cost: " << difftimeval(&lastTime, &currentTime) << endl;
#endif

    in.close();
}

void SettingConfig::setModelSetting(ModelSetting modelSetting)
{
    this->modelSetting = modelSetting;
}

void SettingConfig::setLocalStorage(LocalStorage localStorage)
{
    this->localStorage = localStorage;
}

void SettingConfig::setFtpStorage(FtpStorage ftpStorage)
{
    this->ftpStorage = ftpStorage;
}

void SettingConfig::setDataOutput(DataOutput dataOutput)
{
    this->dataOutput = dataOutput;
}

void SettingConfig::setUartConfig(UartConfig uartConfig)
{
    this->uartConfig = uartConfig;
}

bool SettingConfig::parserLocalConfigValue(Json::Value value)
{
    if (!value["webServer"].empty() && !value["webServer"]["serverIp"].empty() && !value["webServer"]["serverPort"].empty())
    {
        memcpy(serverConfig.server_ip, const_cast<char *>(value["webServer"]["serverIp"].asString().c_str()), sizeof(serverConfig.server_ip));
        serverConfig.port = stoi(value["webServer"]["serverPort"].asString().c_str());
        cout << "serverConfig server_ip" << serverConfig.server_ip <<endl;
        cout << "serverConfig port" << serverConfig.port <<endl;
    }

    if (!value["outputConf"].empty() && !value["outputConf"]["uartName"].empty() && !value["outputConf"]["speed"].empty()
            && !value["outputConf"]["bits"].empty() && !value["outputConf"]["parity"].empty() && !value["outputConf"]["stop"].empty())
    {
        memcpy(uartConfig.uart_name, const_cast<char *>(value["outputConf"]["uartName"].asString().c_str()), sizeof(uartConfig.uart_name));
        cout << "speed: " << value["outputConf"]["speed"] << endl;
        uartConfig.speed = stoi(value["outputConf"]["speed"].asString().c_str());
        cout << "bits: " << value["outputConf"]["bits"] << endl;
        uartConfig.bits = value["outputConf"]["bits"].asInt();
        cout << "parity: " << value["outputConf"]["parity"] << endl;
        uartConfig.parity = value["outputConf"]["parity"].asString().at(0);
        cout << "stop: " << value["outputConf"]["stop"] << endl;
        uartConfig.stop = value["outputConf"]["stop"].asInt();
    }

    cout << "localStorageConf: " << value["localStorageConf"] << endl;
    if (!value["localStorageConf"].empty())
    {
        localStorage.storage_days = value["localStorageConf"]["storageDay"].asInt();
        localStorage.storage_raw = value["localStorageConf"]["saveRawImg"].asInt();

        cout << "localStorage storage_days" << localStorage.storage_days <<endl;
        cout << "localStorage storage_raw" << localStorage.storage_raw <<endl;
    }

    cout << "ftpConf: " << value["ftpConf"] << endl;
    if (!value["ftpConf"].empty() && !value["ftpConf"]["destIp"].empty() && !value["ftpConf"]["destPort"].empty()
            && !value["ftpConf"]["name"].empty() && !value["ftpConf"]["password"].empty() && !value["ftpConf"]["strategy"].empty()
                && !value["ftpConf"]["enable"].empty())
    {
        cout << "set ftp storage" << endl;
        memcpy(ftpStorage.dest_ip, const_cast<char *>(value["ftpConf"]["destIp"].asString().c_str()), sizeof(ftpStorage.dest_ip));
        ftpStorage.dest_port = value["ftpConf"]["destPort"].asInt();
        memcpy(ftpStorage.name, const_cast<char *>(value["ftpConf"]["name"].asString().c_str()), sizeof(ftpStorage.name));
        cout << "name = " << value["ftpConf"]["name"] << endl;
        cout << "name = " << ftpStorage.name  << "size: " << sizeof(ftpStorage.name) << endl;
        memcpy(ftpStorage.password, const_cast<char *>(value["ftpConf"]["password"].asString().c_str()), sizeof(ftpStorage.password));
        ftpStorage.strategy = value["ftpConf"]["strategy"].asInt();
        ftpStorage.enable = value["ftpConf"]["enable"].asInt();
    }

    cout << "strategy: " << value["ftpConf"]["strategy"] << endl;

    if (!value["serverConf"].empty() && !value["serverConf"]["destIp"].empty() && !value["serverConf"]["destPort"].empty()
            && !value["serverConf"]["tryTimes"].empty() && !value["serverConf"]["enable"].empty())
    {
        memcpy(dataOutput.dest_ip, const_cast<char *>(value["serverConf"]["destIp"].asString().c_str()), sizeof(dataOutput.dest_ip));
        dataOutput.dest_port = stoi(value["serverConf"]["destPort"].asString().c_str());
        dataOutput.try_times = value["serverConf"]["tryTimes"].asInt();
        dataOutput.enable = value["serverConf"]["enable"].asInt();
    }

    cout << "try_times: " << value["serverConf"]["tryTimes"] << endl;

    if (!value["modelUpgradeConf"].empty() && !value["modelUpgradeConf"]["modelResourceUrl"].empty() && !value["modelUpgradeConf"]["modelVersion"].empty()
            && !value["modelUpgradeConf"]["modelName"].empty())
    {
        memcpy(modelSetting.model_url, const_cast<char *>(value["modelUpgradeConf"]["model_url"].asString().c_str()), sizeof(modelSetting.model_url));
        memcpy(modelSetting.model_name, const_cast<char *>(value["modelUpgradeConf"]["model_name"].asString().c_str()), sizeof(modelSetting.model_name));
        memcpy(modelSetting.model_version, const_cast<char *>(value["modelUpgradeConf"]["model_version"].asString().c_str()), sizeof(modelSetting.model_version));
    }
    cout << "model_version: " << value["modelUpgradeConf"]["model_version"] << endl;

    if (!value["programUpgradeConf"].empty() && !value["programUpgradeConf"]["programResourceUrl"].empty() && !value["programUpgradeConf"]["programName"].empty()
            && !value["programUpgradeConf"]["programVersion"].empty())
    {
        memcpy(modelSetting.bin_url, const_cast<char *>(value["programUpgradeConf"]["programResourceUrl"].asString().c_str()), sizeof(modelSetting.bin_url));
        memcpy(modelSetting.bin_name, const_cast<char *>(value["programUpgradeConf"]["programName"].asString().c_str()), sizeof(modelSetting.bin_name));
        memcpy(modelSetting.bin_version, const_cast<char *>(value["programUpgradeConf"]["programVersion"].asString().c_str()), sizeof(modelSetting.bin_version));
    }
    cout << "bin_version: " << value["programUpgradeConf"]["programVersion"] << endl;
    
    return true;
}

bool SettingConfig::updateConfigJson(Json::Value value)
{
    Json::Reader reader;
    Json::Value config_root;
    ifstream config_json(CONFIG_JSON, ios::binary);
    if (reader.parse(config_json, config_root))
    {
        config_root["webServer"]["serverIp"] = value["serverIp"];
        config_root["webServer"]["serverPort"] = value["serverPort"];
    } 

    cout << "serverIp: " << config_root["webServer"]["serverIp"] << endl;
    cout << "serverPort: " << config_root["webServer"]["serverPort"] << endl;
    config_json.close();
    ofstream out_json(CONFIG_JSON, ios::binary);

    Json::StyledWriter sw;
    out_json << sw.write(config_root);
    out_json.close();
    return true;
}

extern void stop_detect(int channel);
extern void resume_detect(int channel);

bool SettingConfig::updateDetectCtrl(struct MessageBox box)
{
    Json::Reader reader;
    Json::Value value;
    if (reader.parse(box.body, value))
    {
        std::string target = value["target"].asString();
        int RunFlag = target.compare("start");
        RunFlag = RunFlag == 0 ? 1:0;
        Json::Value arrayNum = value["channelNos"];
        for (unsigned int i = 0; i < arrayNum.size(); i++)
        {
            int no = stoi(arrayNum[i].asString().c_str());
            if(no > CHANNEL_NUM-1)
            {
                continue;
            }
            detectctrl.run_state[no] = RunFlag;
            if(!RunFlag)
            {
                stop_detect(no);
            }
            else
            {
                resume_detect(no);
            }
            printf("[%s]channelNos:%d flag:%d\n", __func__, i, RunFlag);
        }
        return true;
    }
    else
    {
        return false;
    }
    return true;
}

bool SettingConfig::updateDetectCls(struct MessageBox box)
{
    Json::Reader reader;
    Json::Value value;
    if (reader.parse(box.body, value))
    {
        Json::Value arrayNum = value["channelNos"];
        for (unsigned int i = 0; i < arrayNum.size(); i++)
        {
            int no = stoi(arrayNum[i].asString().c_str());
            detectctrl.run_state[no] = 1;//��������ͨ��
            printf("[%s]channelNos:%d cls\n", __func__, i);
        }
        return true;
    }
    else
    {
        return false;
    }
    return true;
}

bool SettingConfig::updateConfigJson(struct MessageBox box)
{
    Json::Reader reader;
    Json::Value config_root, value;
    ifstream config_json(CONFIG_JSON, ios::binary);
    if (!reader.parse(config_json, config_root))
    {
        cerr << "read default config failed" << endl;
        return false;
    } 

    if (!reader.parse(box.body, value))
    {
        cerr << "read box body failed" << endl;
        return false;
    } 

    cout << "error msgId: " << box.msgId << endl;
    switch (box.msgId)
    {
        case MSG_ID_UDP_SERVER:
            config_root["webServer"]["serverIp"] = value["serverIp"];
            config_root["webServer"]["serverPort"] = value["serverPort"];
            break;
        case MSG_ID_OUTPUT_CONFIG:
            config_root["outputConf"]["uartName"] = value["uartName"];
            config_root["outputConf"]["speed"] = value["speed"];
            config_root["outputConf"]["bits"] = value["bits"];
            config_root["outputConf"]["parity"] = value["parity"];
            config_root["outputConf"]["stop"] = value["stop"];
            break;
        case MSG_ID_STORAGE_CONFIG:
            config_root["localStorageConf"]["storageDay"] = value["storageDay"];
            config_root["localStorageConf"]["saveRawImg"] = value["saveRawImg"];
            break;
        case MSG_ID_FTP_CONFIG:
            config_root["ftpConf"]["destIp"] = value["destIp"];
            config_root["ftpConf"]["destPort"] = value["destPort"];
            config_root["ftpConf"]["name"] = value["name"];
            config_root["ftpConf"]["password"] = value["password"];
            config_root["ftpConf"]["strategy"] = value["strategy"];
            config_root["ftpConf"]["enable"] = value["enable"];
            break;
        case MSG_ID_DATA_REPORT:
            config_root["serverConf"]["destIp"] = value["destIp"];
            config_root["serverConf"]["destPort"] = value["destPort"];
            config_root["serverConf"]["tryTimes"] = value["tryTimes"];
            config_root["serverConf"]["enable"] = value["enable"];
            break;
        case MSG_ID_MODLE_UPGRADE:
            config_root["modelUpgradeConf"]["modelResourceUrl"] = value["modelResourceUrl"];
            config_root["modelUpgradeConf"]["modelMd5"] = value["modelMd5"];
            config_root["modelUpgradeConf"]["modelVersion"] = value["modelVersion"];
            config_root["modelUpgradeConf"]["modelName"] = value["modelName"];
            break;
        case MSG_ID_BIN_UPGRADE:
            config_root["programUpgradeConf"]["programResourceUrl"] = value["programResourceUrl"];
            config_root["programUpgradeConf"]["programMd5"] = value["programMd5"];
            config_root["programUpgradeConf"]["programVersion"] = value["programVersion"];
            config_root["programUpgradeConf"]["programName"] = value["programName"];
            break;
        
        default:
            cerr << "error msgId: " << box.msgId << endl;
            break;
    }
    parserLocalConfigValue(config_root);
    cout << "serverIp: " << config_root["webServer"]["serverIp"] << endl;
    cout << "serverPort: " << config_root["webServer"]["serverPort"] << endl;
    config_json.close();
    ofstream out_json(CONFIG_JSON, ios::binary);

    Json::StyledWriter sw;
    out_json << sw.write(config_root);
    out_json.close();
    return true;
}


void SettingConfig::dumpConfig()
{
    cout << "serverIp" << serverConfig.server_ip << endl;
    cout << "serverPort" << serverConfig.port << endl;
    cout << "ftpStorage dest_ip" << ftpStorage.dest_ip << endl;
    cout << "ftpStorage dest_port" << ftpStorage.dest_port << endl;
    cout << "uartname" << uartConfig.uart_name << endl;
    cout << "speed" << uartConfig.speed << endl;
    cout << "bits" << uartConfig.bits << endl;
    cout << "parity" << uartConfig.parity << endl;
    cout << "stop" << uartConfig.stop << endl;
}




#endif
