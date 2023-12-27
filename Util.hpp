//==============================================================================
//
//  Copyright (c) 2017-2021 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef UTIL_H
#define UTIL_H

#define OUTPUT_WIDTH  800
#define OUTPUT_HEIGHT 800

typedef enum {
	CODE_COLOR_GRAY,
	CODE_COLOR_YCbCr,
	CODE_COLOR_RGB
} CODE_COLOR;


#include <vector>
#include <string>
#include <stdio.h>
#include <sstream>
#include <cmath>
//#include "DlSystem/ITensorFactory.hpp"
//#include "DlSystem/TensorShape.hpp"
//#include<opencv2/opencv.hpp>
//using namespace cv;
template <typename Container> Container& split(Container& result, const typename Container::value_type & s, typename Container::value_type::value_type delimiter )
{
  result.clear();
  std::istringstream ss( s );
  while (!ss.eof())
  {
    typename Container::value_type field;
    getline( ss, field, delimiter );
    if (field.empty()) continue;
    result.push_back( field );
  }
  return result;
}

struct output_result { 
    char            defectType;
	float           area;
	float           length;
//	cv::Mat         resImg;
};
typedef struct
{
  unsigned int     biSize;
  int              biWidth;
  int              biHeight;
  unsigned short   biPlanes;
  unsigned short   biBitCount;
  unsigned int     biCompression;
  unsigned int     biSizeImage;
  int              biXPelsPerMeter;
  int              biYPelsPerMeter;
  unsigned int     biClrUsed;
  unsigned int     biClrImportant;
} BitMapInfoHeader;

typedef struct
{
  unsigned short   bfType;
  unsigned int     bfSize;
  unsigned short   bfReserved1;
  unsigned short   bfReserved2;
  unsigned int     bfOffBits;
}BitMapFileHeader;


void getOutputResult(output_result* output);

//size_t calcSizeFromDims(const zdl::DlSystem::Dimension *dims, size_t rank, size_t elementSize);

std::vector<float> loadFloatDataFile(const std::string& inputFile);
std::vector<unsigned char> loadByteDataFile(const std::string& inputFile);
template<typename T> bool loadByteDataFile(const std::string& inputFile, std::vector<T>& loadVector);

std::vector<unsigned char> loadByteDataFileBatched(const std::string& inputFile);
template<typename T> bool loadByteDataFileBatched(const std::string& inputFile, std::vector<T>& loadVector, size_t offset);
bool loadByteDataFileBatchedTf8(const std::string& inputFile, std::vector<uint8_t>& loadVector, size_t offset);
bool loadByteDataFileBatchedTfN(const std::string& inputFile, std::vector<uint8_t>& loadVector, size_t offset,
                                unsigned char& stepEquivalentTo0, float& quantizedStepSize, bool staticQuantization, int bitWidth);

//bool SaveITensorBatched(const std::string& path, const zdl::DlSystem::ITensor* tensor, size_t batchIndex=0, size_t batchChunk=0);
bool SaveUserBufferBatched(const std::string& path, const std::vector<uint8_t>& buffer, size_t batchIndex=0, size_t batchChunk=0);
bool SaveUserBufferBatchedYOLOV5(const std::string& path, const std::vector<uint8_t>& buffer, size_t batchIndex=0,
                                 size_t batchChunk=0, int classes=2, float confThres=0.3);

bool EnsureDirectory(const std::string& dir);
//void getRegionalStatistics(cv::Mat image,std::vector<double>&area, std::vector<double>& length);

void TfNToFloat(float *out, uint8_t *in, const unsigned char stepEquivalentTo0, const float quantizedStepSize, size_t numElement, int bitWidth);
bool FloatToTfN(uint8_t* out, unsigned char& stepEquivalentTo0, float& quantizedStepSize, bool staticQuantization, float* in, size_t numElement, int bitWidth);

void setResizableDim(size_t resizableDim);
size_t getResizableDim();
float u8Arry2float(const uint8_t *data, bool key = false);
void float2u8Arry(uint8_t *u8Arry, float *floatdata, bool key= false);
double sigmoid(double input);
extern "C" unsigned char * rgb2jpg(const char *pRgbData, const int width, const int height, int type, long* pSize);

extern "C" int rgb2jpgAction(struct jpeg_compress_struct* pCinfo, const char *pRgbData, const int width, const int height);

extern "C" unsigned char * gray2jpg(const char *pRgbData, const int width, const int height, int type, long* pSize);

extern "C" int gray2jpgAction(struct jpeg_compress_struct* pCinfo, const char *pRgbData, const int width, const int height);

int yuv420sp2Jpeg(const char *scr_frame, int iWidth, int iHeight, int iQuality, int iColorSpace, char *coded_frame, int& code_frame_size);
void savePicture(unsigned char* buffer,uint32_t ui32Width, uint32_t ui32Height,uint32_t size,const char *format);
void ConvertRGBToBMP(unsigned char *RGBBuffer, unsigned char *BMPBuffer, unsigned int ImageWidth, unsigned int ImageHeight);
void getCurrentTimeDay(int channelId, char *str);
void getCurrentTimeSec(int channelId, bool is_raw,char *str);
void remove_pics(std::string dirName ,int days);

bool removeDir(const std::string & path);
std::vector<std::string> getFiles(std::string cate_dir);
void remove_old_pics(std::string dirName ,char * myPic,int days);
void removeFirstFiles(std::string dirName,int reserveNum);
//*detecResult getResult();

#endif

