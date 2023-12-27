//==============================================================================
//
//  Copyright (c) 2017-2021 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <sys/stat.h>
#include <cerrno>
#include <limits>

#include "Util.hpp"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
//#include "DlSystem/ITensorFactory.hpp"
//#include "DlSystem/TensorShape.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include "jpeglib.h"

#include <sys/stat.h>
#include <dirent.h>	


#define JPEG_QUALITY 100
using namespace std;


//using namespace cv;
size_t resizable_dim;
//struct output_result output_result;
//cv::Mat resImage;
char	defectTy;
float	areas;
float   lengths;

int Min = 10;

long calculate_interval(const struct timeval *start, const struct timeval *end);

#if 0
void getOutputResult(output_result* output) {
	
	std::cout <<"getOutputResult started"<<"\n";
	
    output-> defectType = defectTy;
	//strcpy(output-> defectType, defectTy);
	output-> length = lengths;
	output-> area = areas;
	
	//output->ResImg = (cv::Mat)cv::imread("/sdcard/DCIM/mbgrImg.jpg");

    output->resImg =cv::Mat::zeros(cv::Size(resImage.rows,resImage.cols),CV_8UC3);
    output->resImg = resImage;

	std::cout <<"getOutputResult done"<<"\n";
}

size_t calcSizeFromDims(const zdl::DlSystem::Dimension *dims, size_t rank, size_t elementSize )
{
   if (rank == 0) return 0;
   size_t size = elementSize;
   while (rank--) {
      (*dims == 0) ? size *= resizable_dim : size *= *dims;
      dims++;
   }
   return size;
}

#endif  //todo
bool EnsureDirectory(const std::string& dir)
{
   auto i = dir.find_last_of('/');
   std::string prefix = dir.substr(0, i);

   if (dir.empty() || dir == "." || dir == "..")
   {
      return true;
   }

   if (i != std::string::npos && !EnsureDirectory(prefix))
   {
      return false;
   }

   int rc = mkdir(dir.c_str(),  S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
   if (rc == -1 && errno != EEXIST)
   {
      return false;
   }
   else
   {
      struct stat st;
      if (stat(dir.c_str(), &st) == -1)
      {
         return false;
      }

      return S_ISDIR(st.st_mode);
   }
}

std::vector<float> loadFloatDataFile(const std::string& inputFile)
{
    std::vector<float> vec;
    loadByteDataFile(inputFile, vec);
    return vec;
}

std::vector<unsigned char> loadByteDataFile(const std::string& inputFile)
{
   std::vector<unsigned char> vec;
   loadByteDataFile(inputFile, vec);
   return vec;
}

template<typename T>
bool loadByteDataFile(const std::string& inputFile, std::vector<T>& loadVector)
{
   std::ifstream in(inputFile, std::ifstream::binary);
   if (!in.is_open() || !in.good())
   {
      std::cerr << "Failed to open input file: " << inputFile << "\n";
   }

   in.seekg(0, in.end);
   size_t length = in.tellg();
   in.seekg(0, in.beg);

   if (length % sizeof(T) != 0) {
      std::cerr << "Size of input file should be divisible by sizeof(dtype).\n";
      return false;
   }

   if (loadVector.size() == 0) {
      loadVector.resize(length / sizeof(T));
   } else if (loadVector.size() < length / sizeof(T)) {
      std::cerr << "Vector is not large enough to hold data of input file: " << inputFile << "\n";
      loadVector.resize(length / sizeof(T));
   }

   if (!in.read(reinterpret_cast<char*>(&loadVector[0]), length))
   {
      std::cerr << "Failed to read the contents of: " << inputFile << "\n";
   }
   return true;
}

std::vector<unsigned char> loadByteDataFileBatched(const std::string& inputFile)
{
   std::vector<unsigned char> vec;
   size_t offset=0;
   loadByteDataFileBatched(inputFile, vec, offset);
   return vec;
}

template<typename T>
bool loadByteDataFileBatched(const std::string& inputFile, std::vector<T>& loadVector, size_t offset)
{
    std::ifstream in(inputFile, std::ifstream::binary);
    if (!in.is_open() || !in.good())
    {
        std::cerr << "Failed to open input file: " << inputFile << "\n";
    }

    in.seekg(0, in.end);
    size_t length = in.tellg();
    in.seekg(0, in.beg);

    if (length % sizeof(T) != 0) {
        std::cerr << "Size of input file should be divisible by sizeof(dtype).\n";
        return false;
    }

    if (loadVector.size() == 0) {
        loadVector.resize(length / sizeof(T));
    } else if (loadVector.size() < length / sizeof(T)) {
        std::cerr << "Vector is not large enough to hold data of input file: " << inputFile << "\n";
    }

    loadVector.resize( (offset+1) * length / sizeof(T) );

    if (!in.read( reinterpret_cast<char*> (&loadVector[offset * length/ sizeof(T) ]), length) )
    {
        std::cerr << "Failed to read the contents of: " << inputFile << "\n";
    }
    return true;
}

#if 0 //todo
void getRegionalStatistics(cv::Mat image,std::vector<double>&area, std::vector<double>& length) {
 //连通域计算
 cv::Mat labels,  stats, centroids;
 int retval = cv::connectedComponentsWithStats(image, labels, stats, centroids,8);
 for (int i = 1; i < retval; i++)
 {
  double sum = 0.0;
  //创建一个和labels相同大小的图像
  cv::Mat mask = cv::Mat::zeros(cv::Size(labels.cols, labels.rows), CV_8UC1);
  uchar* pLabel = labels.data;
  uchar* pMask = mask.data;
  int size = labels.cols * labels.rows;
  //统计缺陷面积大小
  for (int j = 0; j < size; j++)
  {
   if (pLabel[j*4]== uchar(i))
   {
    pMask[j] = 255;
    sum += 1.0;
   }
  }
  std::vector< std::vector< cv::Point> > contours;
  cv::findContours(mask, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  if (contours.size() == 0)
  {
   continue;
  }
  //计算缺陷长度
  double arcLength = cv::arcLength(contours[0], true)/2;
  if (sum>0.0)
  {
   area.push_back(sum);
   length.push_back(arcLength);
  }
 }
}

void TfNToFloat(float *out,
                uint8_t *in,
                const unsigned char stepEquivalentTo0,
                const float quantizedStepSize,
                size_t numElement,
                int bitWidth)
{
//    std::vector<float> outputTemp;
    for (size_t i = 0; i < numElement; ++i) {
       if (8 == bitWidth) {
           double quantizedValue = static_cast <double> (in[i]);
           double stepEqTo0 = static_cast <double> (stepEquivalentTo0);
           out[i] = static_cast <double> ((quantizedValue - stepEqTo0) * quantizedStepSize);
//           outputTemp.emplace_back(out[i]);
       }
       else if (16 == bitWidth) {
           uint16_t *temp = (uint16_t *)in;
           double quantizedValue = static_cast <double> (temp[i]);
           double stepEqTo0 = static_cast <double> (stepEquivalentTo0);
           out[i] = static_cast <double> ((quantizedValue - stepEqTo0) * quantizedStepSize);
       }
   }
}

#endif //todo
double sigmoid(double input) {
        return (double) (1 / (1 + exp(-input)));
    }

bool FloatToTfN(uint8_t* out,unsigned char& stepEquivalentTo0,float& quantizedStepSize, bool staticQuantization,
                float* in,size_t numElement,int bitWidth)
{

   double encodingMin;
   double encodingMax;
   double encodingRange;
   double trueBitWidthMax = pow(2, bitWidth) -1;

   if (!staticQuantization) {
      float trueMin = std::numeric_limits <float>::max();
      float trueMax = std::numeric_limits <float>::min();

      for (size_t i = 0; i < numElement; ++i) {
         trueMin = fmin(trueMin, in[i]);
         trueMax = fmax(trueMax, in[i]);
      }

      double stepCloseTo0;

      if (trueMin > 0.0f) {
         stepCloseTo0 = 0.0;
         encodingMin = 0.0;
         encodingMax = trueMax;
      } else if (trueMax < 0.0f) {
         stepCloseTo0 = trueBitWidthMax;
         encodingMin = trueMin;
         encodingMax = 0.0;
      } else {
         double trueStepSize = static_cast <double>(trueMax - trueMin) / trueBitWidthMax;
         stepCloseTo0 = -trueMin / trueStepSize;
         if (stepCloseTo0 == round(stepCloseTo0)) {
            // 0.0 is exactly representable
            encodingMin = trueMin;
            encodingMax = trueMax;
         } else {
            stepCloseTo0 = round(stepCloseTo0);
            encodingMin = (0.0 - stepCloseTo0) * trueStepSize;
            encodingMax = (trueBitWidthMax - stepCloseTo0) * trueStepSize;
         }
      }

      const double minEncodingRange = 0.01;
      double encodingRange = encodingMax - encodingMin;
      quantizedStepSize = encodingRange / trueBitWidthMax;
      stepEquivalentTo0 = static_cast <unsigned char> (round(stepCloseTo0));

      if (encodingRange < minEncodingRange) {
         std::cerr << "Expect the encoding range to be larger than " << minEncodingRange << "\n"
                   << "Got: " << encodingRange << "\n";
         return false;
      }
   }
   else
   {
      if (bitWidth == 8) {
         encodingMin = (0 - static_cast <uint8_t> (stepEquivalentTo0)) * quantizedStepSize;
      } else if (bitWidth == 16) {
         encodingMin = (0 - static_cast <uint16_t> (stepEquivalentTo0)) * quantizedStepSize;
      } else {
         std::cerr << "Quantization bitWidth is invalid " << std::endl;
         return false;
      }
      encodingMax = (trueBitWidthMax - stepEquivalentTo0) * quantizedStepSize;
      encodingRange = encodingMax - encodingMin;
   }

   for (size_t i = 0; i < numElement; ++i) {
      int quantizedValue = round(trueBitWidthMax * (in[i] - encodingMin) / encodingRange);  
      if (quantizedValue < 0)
         quantizedValue = 0;
      else if (quantizedValue > (int)trueBitWidthMax)
         quantizedValue = (int)trueBitWidthMax;

      if(bitWidth == 8){
         //std::cout << bitWidth<<"uint8_t"<< std::endl;
         out[i] = static_cast <uint8_t> (quantizedValue);
      }
      else if(bitWidth == 16){
         uint16_t *temp = (uint16_t *)out;
         //std::cout << bitWidth<< "uint16_t" <<std::endl;
         temp[i] = static_cast <uint16_t> (quantizedValue);
      }
      
   }

   return true;
}
// loadVector(uint8_t type, 是量化后的结果) == applicationBuffers.at(name), 已经绑定到inputMap.
bool loadByteDataFileBatchedTfN(const std::string& inputFile, std::vector<uint8_t>& loadVector, size_t offset,
                                unsigned char& stepEquivalentTo0, float& quantizedStepSize, bool staticQuantization, int bitWidth)
{
   std::ifstream in(inputFile, std::ifstream::binary);
   std::vector<float> inVector; // FIXME: load input from raw file to inVertor
   if (!in.is_open() || !in.good())
   {
      std::cerr << "Failed to open input file: " << inputFile << "\n";
   }

   in.seekg(0, in.end);
   size_t length = in.tellg(); // uint8_t的长度，是实际float长度的4倍.
   in.seekg(0, in.beg);

   // sizeof(uint8_t) == 1 && sizeof(float) == 4
   if (loadVector.size() == 0) {
      loadVector.resize(length / sizeof(uint8_t));
   } else if (loadVector.size() < length/sizeof(float)) {
      std::cerr << "Vector is not large enough to hold data of input file: " << inputFile << "\n";
   }

   inVector.resize(length / sizeof(float));
   if (!in.read( reinterpret_cast<char*> (&inVector[0]), length) )
   {
      std::cerr << "Failed to read the contents of: " << inputFile << "\n";
   }
   int elementSize = bitWidth / 8;


   // FIXME: for debug input inVector.
//    double minVal;
//    double maxVal;
//    cv::minMaxLoc(inVector, &minVal, &maxVal);
//    std::cout << "inVector in RAW (range) is: " << " (" << minVal << " / " << maxVal << ")" << std::endl;
//    cv::Mat mean, std;
//    cv::meanStdDev(inVector, mean, std);
//    std::cout << "inVector in RAW (mean / std): " << " (" << mean.at<double >(0, 0) << " / " << std.at<double >(0, 0) << ")" << std::endl;
//
//    for (int i=0; i<=100; i+=20)
//    {
//        std::cout << "inVector ids-value: " << "(" << i << " - " << inVector[i] << std::endl;
//    }



    // offset == batchSize, 当batchSize为1时候，offset==0;  elementSize == 1, length 为raw uint8_t的长度
   size_t dataStartPos = (offset * length * elementSize) / sizeof(float);
   std::cout << "dataStartPos is: " << dataStartPos << std::endl;

   if(!FloatToTfN(&loadVector[dataStartPos], stepEquivalentTo0, quantizedStepSize, staticQuantization, inVector.data(), inVector.size(), bitWidth))
   {
     return false;
   }
   return true;
}

/*uchar* Matimage loadByteDataFileBatchedTfN(uchar* Matimage,std::vector<uint8_t>& loadVector,unsigned char& stepEquivalentTo0, 
                                float& quantizedStepSize, bool staticQuantization, int bitWidth,int classes)
{
   std::ofstream outFile_int8("/sdcard/DCIM/int8_out", std::ofstream::out);

   std::cout<<"lowell loadByteDataFileBatchedTfN"<<std::endl;
   uchar* Matimage dstMat = NULL;
   
   int elementSize = bitWidth / 8;
   //size_t dataStartPos = (length * elementSize) / sizeof(uint8_t);
   for(int i=0;i<480*480*classes;i++)
   {

    loadVector[i] = round((Matimage[i]+255)/2);
	outFile_int8 << loadVector[i]; 
    //loadVector[i]=uint8_t(inVector.data()[i]);

   }
   outFile_int8.close();
   std::cout<<"loadVector"<<std::endl;
   if (loadVector.empty) {
      return dstMat;
   }
   dstMat.data = loadVector.data();
   //if(!FloatToTfN(&loadVector[dataStartPos], stepEquivalentTo0, quantizedStepSize, staticQuantization, inVector.data(), inVector.size(), bitWidth))
   //{
     //return false;
   //}
    std::cout<<"loadVector done"<<std::endl;
   return dstMat;
}*/


void setResizableDim(size_t resizableDim)
{
    resizable_dim = resizableDim;
}

size_t getResizableDim()
{
    return resizable_dim;
}

 long calculate_interval(const struct timeval *start, const struct timeval *end)
{
    long d;
    time_t s;
	suseconds_t u;
	s = end->tv_sec - start->tv_sec;
	u = end->tv_usec - start->tv_usec;
	d = s;
	d *= 1000000;
    d += u;
    return d;
}

float u8Arry2float(const uint8_t *data, bool key)
{
    float fa = 0;
    uint8_t uc[4];
    if (key)
    {
        uc[3] = data[0];
        uc[2] = data[1];
        uc[1] = data[2];
        uc[0] = data[3];
    }
    else
    {
        uc[0] = data[0];
        uc[1] = data[1];
        uc[2] = data[2];
        uc[3] = data[3];
    }

    memcpy(&fa, uc, 4);
    return fa;
}

void float2u8Arry(uint8_t *u8Arry, float *floatdata, bool key)
{
    uint8_t farray[4];
    *(float *)farray = *floatdata;
    if (key)
    {
        u8Arry[3] = farray[0];
        u8Arry[2] = farray[1];
        u8Arry[1] = farray[2];
        u8Arry[0] = farray[3];
    }
    else
    {
        u8Arry[0] = farray[0];
        u8Arry[1] = farray[1];
        u8Arry[2] = farray[2];
        u8Arry[3] = farray[3];
    }
}
extern "C" int rgb2jpgAction(struct jpeg_compress_struct* pCinfo, const char *pRgbData, const int width, const int height)
{
    int depth = 3;
    JSAMPROW row_pointer[1];
 
    pCinfo->image_width      = width;
    pCinfo->image_height     = height;
    pCinfo->input_components = depth;
    pCinfo->in_color_space   = JCS_RGB;

  printf("%s called @ line %d\n",__func__,__LINE__);
    jpeg_set_defaults(pCinfo); 
    jpeg_set_quality(pCinfo, JPEG_QUALITY, TRUE);
    jpeg_start_compress(pCinfo, TRUE);
  printf("%s called @ line %d\n",__func__,__LINE__);
    int row_stride = width * depth;

    //

    while (pCinfo->next_scanline < pCinfo->image_height)
    {
        row_pointer[0] = (JSAMPROW)(pRgbData + pCinfo->next_scanline * row_stride);
        jpeg_write_scanlines(pCinfo, row_pointer, 1);
    }
  printf("%s called @ line %d\n",__func__,__LINE__);
    jpeg_finish_compress(pCinfo);
     printf("%s called @ line %d\n",__func__,__LINE__);
    jpeg_destroy_compress(pCinfo);
 
    return 0;
}

extern "C" int gray2jpgAction(struct jpeg_compress_struct* pCinfo, const char *pRgbData, const int width, const int height)
{
    JSAMPROW row_pointer[1];

    pCinfo->image_width      = width;
    pCinfo->image_height     = height;
    pCinfo->input_components = 1;
    pCinfo->in_color_space   = JCS_GRAYSCALE;

  //printf("%s called @ line %d\n",__func__,__LINE__);
    jpeg_set_defaults(pCinfo);
    jpeg_set_quality(pCinfo, JPEG_QUALITY, TRUE);
    jpeg_start_compress(pCinfo, TRUE);
  //printf("%s called @ line %d\n",__func__,__LINE__);
    int row_stride = width * 1;

    //

    while (pCinfo->next_scanline < pCinfo->image_height)
    {
        row_pointer[0] = (JSAMPROW)(pRgbData + pCinfo->next_scanline * row_stride);
        jpeg_write_scanlines(pCinfo, row_pointer, 1);
    }
  //printf("%s called @ line %d\n",__func__,__LINE__);
    jpeg_finish_compress(pCinfo);
     //printf("%s called @ line %d\n",__func__,__LINE__);
    jpeg_destroy_compress(pCinfo);

    return 0;
}


/**
*gray to jpeg
*/
extern "C" unsigned char* gray2jpg(const char *pRgbData, const int width, const int height, int type, long* pSize)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE* pOutFile = NULL;
    unsigned char *destBuffer = NULL;
    //printf("%s called @ line %d\n",__func__,__LINE__);
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    //printf("%s called @ line %d\n",__func__,__LINE__);
    jpeg_mem_dest(&cinfo, &destBuffer,(size_t *)pSize);

    //printf("%s called @ line %d\n",__func__,__LINE__);
    gray2jpgAction(&cinfo, pRgbData, width, height);

     //printf("%s called @ line %d\n",__func__,__LINE__);
    return destBuffer;
}
 
/**
*rgb to jpeg
*/
extern "C" unsigned char* rgb2jpg(const char *pRgbData, const int width, const int height, int type, long* pSize)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
 
    FILE* pOutFile = NULL;
    unsigned char *destBuffer = NULL;
    printf("%s called @ line %d\n",__func__,__LINE__);
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    printf("%s called @ line %d\n",__func__,__LINE__);

    /*if (type)
    {
        if ((pOutFile = fopen((char *)pDest, "wb")) == NULL)
        {
            return -1;
        }
        jpeg_stdio_dest(&cinfo, pOutFile);
    }
    else*/
    {
        printf("$$$$$$$$$$$l=%d s=%d i=%d\n",sizeof(long unsigned int),sizeof(size_t),sizeof(int));
        jpeg_mem_dest(&cinfo, &destBuffer,(size_t *)pSize);
    }
  printf("%s called @ line %d\n",__func__,__LINE__);
    rgb2jpgAction(&cinfo, pRgbData, width, height);
    /*if (type)
    {
        fclose(pOutFile);
    }*/

    /*FILE *file = fopen("/data/1.jpg", "w+");
    if (file != NULL)
    {
        fwrite(pTest, 1, *pSize, file);
        fclose(file);
    }*/
     printf("%s called @ line %d\n",__func__,__LINE__);
    return destBuffer;
}

int yuv420sp2Jpeg(const char *scr_frame, int iWidth, int iHeight, int iQuality, int iColorSpace, char *coded_frame, int& code_frame_size)
{
	//struct jpeg_decompress_struct cinfo;/* 用于解压缩 */
	struct jpeg_compress_struct jcs;      /* 用于压缩   */
	struct jpeg_error_mgr jerr;             /* 错误处理域 */
	int i = 0, j = 0;
	unsigned char *pY, *pU, *pV;
	unsigned char *yuvbuf = new unsigned char[iWidth * iHeight];
	jcs.err = jpeg_std_error(&jerr); /* 将申明的错误处理器赋值给jcs.err域 */
    /*jerr.pub.error_exit = my_compress_error_exit;
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_compress(&jcs);
        //fclose(infile);
        return -1;
    }
    */
	/* 以下为编码 */
	jpeg_create_compress(&jcs);
    jpeg_mem_dest(&jcs,(unsigned char **)&coded_frame,(long unsigned int *)&code_frame_size); /* 设置JPEG压缩编码的指针 */
	/*set parameters for compression */
	jcs.image_width = iWidth; 
	jcs.image_height = iHeight; 
    if(iColorSpace == CODE_COLOR_GRAY)
    {
        jcs.input_components = 1;          /* = 1表示灰度图， = 3 表示彩色图   */
        jcs.in_color_space = JCS_GRAYSCALE;              /* JCS_RGB/JCS_YCbCr/JCS_GRAYSCALE */
    }
    else if(iColorSpace == CODE_COLOR_YCbCr)
    {
        jcs.input_components = 3;
        jcs.in_color_space = JCS_YCbCr;
    }
    else if(iColorSpace == CODE_COLOR_RGB)
    {
        jcs.input_components = 3;
        jcs.in_color_space = JCS_RGB;
    }
	jcs.dct_method = JDCT_FLOAT;
	jpeg_set_defaults(&jcs);                   /* 采用默认参数进行压缩             */
    jpeg_set_quality(&jcs,iQuality,TRUE);
	(void) jpeg_start_compress(&jcs, TRUE);

	pY = (unsigned char *)scr_frame ;
	pU = (unsigned char *)scr_frame + iWidth* iHeight;
	pV = (unsigned char *)scr_frame + iWidth* iHeight*5/4;
	j = 1;
	int row_stride;                            /* physical row width in output buffer */
	row_stride = jcs.image_width*jcs.input_components; 
	JSAMPROW row_pointer[1]; 
	/* 设置数据源 */
	while(jcs.next_scanline < jcs.image_height)
	{
		int index = 0;
		for (i = 0; i < iWidth; i += 4){//输入的YUV图片格式为标准的YUV444格式，所以需要把YUV420转化成YUV444.
			yuvbuf[index++] = *pY;
			yuvbuf[index++] = *pU;
			yuvbuf[index++] = *pV;
			pY ++;
			yuvbuf[index++] = *pY;
			yuvbuf[index++] = *pU;
			yuvbuf[index++] = *pV;
			pY ++;
			yuvbuf[index++] = *pY;
			yuvbuf[index++] = *pU;
			yuvbuf[index++] = *pV;
			pY ++;
			yuvbuf[index++] = *pY;
			yuvbuf[index++] = *pU;
			yuvbuf[index++] = *pV;
			pY ++;
			pU ++;
			pV ++;
		}
		row_pointer[0] = yuvbuf;
		(void)jpeg_write_scanlines(&jcs, row_pointer, 1);//单行图片转换压缩
		j++;
	}
	jpeg_finish_compress(&jcs); 
	jpeg_destroy_compress(&jcs); 
	delete []yuvbuf;
	return 0;
}

void savePicture(unsigned char* buffer,uint32_t ui32Width, uint32_t ui32Height,uint32_t size,const char *format)
{
    char szName[128] = {0};

    static int nRawFileIndex = 0;
    FILE* phImageFile = NULL;
    sprintf(szName, "/sdcard/DCIM/dump_%dx%d_%d.%s",ui32Width,ui32Height,nRawFileIndex++,format);
    phImageFile = fopen(szName,"w+");
    if (phImageFile == NULL)
    {
        printf("Save %s failed!\n", szName);
        return;
    }

    if(buffer != NULL)
    {
        fwrite(buffer, size, 1, phImageFile);
        fclose(phImageFile);
        printf("Save %s successed\n", szName);
		
		printf("Start openCV\n");
//	cv::Mat source_image = imread(szName);
//	    if (source_image.empty()) {
//			printf("empty source\n");
//			return;
//		}
//		printf("%s width:%d,height:%d\n",szName,source_image.cols,source_image.rows);
		return;
	}
    else
    {
        printf("Save %s failed!\n", szName);
    }
    return;
}


void ConvertRGBToBMP(unsigned char *RGBBuffer, unsigned char *BMPBuffer, unsigned int ImageWidth, unsigned int ImageHeight)
{
  BitMapFileHeader* BmpFileHeader = (BitMapFileHeader*)BMPBuffer;//填充BMP文件头信息
  BmpFileHeader->bfType = 0x4D42;//'BM‘
  BmpFileHeader->bfSize = sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) + ImageWidth*ImageHeight * 3;
  BmpFileHeader->bfOffBits = sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader);
  BmpFileHeader->bfReserved1 = 0;
  BmpFileHeader->bfReserved2 = 0;
 
  BitMapInfoHeader* BmpInfoHeader = (BitMapInfoHeader*)((unsigned char*)BMPBuffer + sizeof(BitMapFileHeader));
  
 
  BmpInfoHeader->biSize = 40;
  BmpInfoHeader->biWidth = ImageWidth;
  BmpInfoHeader->biHeight = -ImageHeight;
  BmpInfoHeader->biPlanes = 1;
  BmpInfoHeader->biBitCount = 24;//RGB图像
  BmpInfoHeader->biCompression = 0;
  BmpInfoHeader->biSizeImage = ImageWidth*ImageHeight * 3;
  BmpInfoHeader->biXPelsPerMeter = 0;
  BmpInfoHeader->biYPelsPerMeter = 0;
  BmpInfoHeader->biClrUsed = 0;
  BmpInfoHeader->biClrImportant = 0;
  
  
  unsigned char* ImageBufferHeader = (unsigned char*)((unsigned char*)BmpInfoHeader + sizeof(BitMapInfoHeader));
  
  memcpy(ImageBufferHeader, RGBBuffer, ImageWidth*ImageHeight * 3);
  printf("ConvertRGBToBMP succeed\n");
}

void SavegetCurrentTimeSec(int channelId,int num, bool is_raw,char * str)
{
	struct timeval tv;
	struct timezone tz;
	struct tm *t;
	char picPath[128] = {0};

	printf("getCurrentTimeSec started\n");

	gettimeofday(&tv, &tz);
	t = localtime(&tv.tv_sec);

	printf("ready to sprintf\n");

    sprintf(str,"/home/xian/zhangleo/photo/cpp2/result/org__ch_%d_%d.jpg", channelId, num);
//    sprintf(str,"/home/xian/zhangleo/photo/cpp2/result_img/org_%d%02d%02d%02d%02d%02d_ch_%d_%d.jpg",1900+t->tm_year, 1+t->tm_mon, t->tm_mday,
//            t->tm_hour, t->tm_min, t->tm_sec, channelId, num);
//    sprintf(str,"/home/xian/zhangleo/photo/cpp2/result/org__ch_%d_%d.jpg",1900+t->tm_year, 1+t->tm_mon, t->tm_mday,
//            t->tm_hour, t->tm_min, t->tm_sec, channelId, num);
}

void getCurrentTimeSec(int channelId, bool is_raw,char * str)
{
    struct timeval tv;
    struct timezone tz;   
    struct tm *t;
	char picPath[128] = {0};

	printf("getCurrentTimeSec started\n");
   
    gettimeofday(&tv, &tz);
    t = localtime(&tv.tv_sec);

	printf("ready to sprintf\n");
	if (is_raw) {
        sprintf(str,"/home/xian/zhangleo/photo/cpp2/result/%d%02d%02d%02d%02d%02d_ch_%d.jpg",1900+t->tm_year, 1+t->tm_mon, t->tm_mday,
	        t->tm_hour, t->tm_min, t->tm_sec,channelId);
	} else {
        sprintf(str,"/home/xian/zhangleo/photo/cpp2/result/%d%02d%02d%02d%02d%02d_ch_%d.jpg",1900+t->tm_year, 1+t->tm_mon, t->tm_mday,
	        t->tm_hour, t->tm_min, t->tm_sec,channelId);
    }
	printf("str :%s\n",str);
}
void getCurrentTimeDay(char *str)
 {
    struct timeval tv;
    struct timezone tz;
    struct tm *t;
    gettimeofday(&tv, &tz);
    t = localtime(&tv.tv_sec);
    sprintf(str,"%d%02d%02d",1900+t->tm_year, 1+t->tm_mon, t->tm_mday);
 	printf("str :%s\n",str);
 }


void deletePics(char *picPath){
	printf("%s called @ line %d\n",__func__,__LINE__);
    if (FILE *file = fopen(picPath, "r")) {
		printf("file exist\n");
		remove(picPath);
	}
}

bool removeDir(const std::string & path) {
	std::string strPath = path;

if (strPath.at(strPath.length() - 1) != '\\' || strPath.at(strPath.length() - 1) != '/')
		strPath.append("/");
	DIR *d = opendir(strPath.c_str());
	if (d != NULL)
	{ 
		struct dirent *dt = NULL;
		while (dt = readdir(d))
		{
			////判断是否为系统隐藏文件,系统有个系统文件，名为“..”和“.”,对它不做处理
			if (strcmp(dt->d_name, "..")!=0 && strcmp(dt->d_name, ".")!=0)
			{
				struct stat st;//文件的信息
				std::string fileName;//文件夹中的文件名
				fileName = strPath + std::string(dt->d_name);
				stat(fileName.c_str(), &st);
				if (S_ISDIR(st.st_mode))
				{
					removeDir(fileName);
				}
				else
				{
					remove(fileName.c_str());
				}
			}
		}
		closedir(d);
	}
	return rmdir(strPath.c_str())==0?true:false;
}


vector<std::string> getFiles(string cate_dir)  
{  
    vector<string> files;//存放文件名  
    
    DIR *dir;  
    struct dirent *ptr;  
    char base[1000];  
   
    if ((dir=opendir(cate_dir.c_str())) == NULL)  
        {  
           printf("Open dir error...");  
           return files;   
        }  
   
    while ((ptr=readdir(dir)) != NULL)  
    {  
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir  
            {continue;}  
        else if(ptr->d_type == 8) {    ///file  
            //printf("d_name:%s\n",ptr->d_name);  
            files.push_back(ptr->d_name); 
        } else if(ptr->d_type == 4)    ///dir  
        {  
            /*files.push_back(ptr->d_name);  
              memset(base,'\0',sizeof(base)); 
              strcpy(base,basePath); 
              strcat(base,"/"); 
              strcat(base,ptr->d_nSame); 
              readFileList(base); 
            */
            continue;
        }  
    }  
    closedir(dir);  
  
    //排序，按从小到大排序  
 //   sort(files.begin(), files.end());
    return files;  
} 



void remove_pics(string dirName ,int days) {
    printf("%s called,dirName :%s,days :%d \n",__func__,dirName.c_str(),days);
    char curTime[24] = {0};
    getCurrentTimeDay(curTime);
    printf("curTime:%s\n",curTime);
	
    char dstCut[16] = {0};
    memcpy(dstCut, curTime,8);
    long dstLong = atol(dstCut);

	
    std::string myDir = dirName;
    vector<string> fileVector = getFiles(dirName);
    int fileSize = fileVector.size();
    if (fileSize == 0) {
	printf("empty directory\n");
	return;
    }
    printf("fileSize:%d \n",fileSize);
    for (int i=0; i < fileSize; i++)  
    {  
        char srcCut[16] = {0};
        memcpy(srcCut, fileVector[i].c_str(),8);
	long srcLong = atol(srcCut);
		
    	long cmp = dstLong - srcLong;
	if (cmp > days) {
	    myDir += fileVector[i].c_str();
	    printf("dirName :%s\n",myDir.c_str());
	    int result = remove(myDir.c_str());
	    printf("remove %s result :%d\n",fileVector[i].c_str(),result);
	    myDir = dirName;
	} 
    }
    printf("remove_pics finished!!!\n");
}


void remove_old_pics(string dirName ,char * myPic,int days) {
    printf("%s called,dirName :%s,days :%d \n",__func__,dirName.c_str(),days);
    char dstCut[16] = {0};
    memcpy(dstCut, myPic + 4,8);
    printf("dstCut:%s\n",dstCut);
    long dstLong = atol(dstCut);
    printf("dstLong:%ld\n",dstLong);
	
    std::string myDir = dirName;
    vector<string> fileVector = getFiles(dirName);
    for (int i=0; i<fileVector.size(); i++)  
    {  
        char srcCut[16] = {0};
        memcpy(srcCut, fileVector[i].c_str() + 4,8);
        printf("srcCut:%s\n",srcCut);
	long srcLong = atol(srcCut);
	printf("srcLong:%ld\n",srcLong);
		
	long cmp = dstLong - srcLong;
	printf("cmp = :%ld\n",cmp);
        if (cmp > days) {
	    myDir += fileVector[i].c_str();
	    printf("dirName :%s\n",myDir.c_str());
	    int result = remove(myDir.c_str());
	    printf("remove %s result :%d\n",fileVector[i].c_str(),result);
	    myDir = dirName;
	}  
    }
}

void removeFirstFiles(string dirName,int reserveNum) {
    printf("%s strated,dir:%s,reserveNum:%d\n",__func__,dirName.c_str(),reserveNum);
    std::string myDir = dirName;
    vector<string> fileVector = getFiles(dirName);
    int fileSize = fileVector.size();

    if (fileSize > reserveNum) {
    for (int i=0; i < (fileSize-reserveNum); i++)  
        {  	
         myDir += fileVector[i].c_str();
	 printf("dirName :%s\n",myDir.c_str());
	 int result = remove(myDir.c_str());
	 printf("remove %s result :%d\n",fileVector[i].c_str(),result);
	 myDir = dirName;
        }
   }
    printf("%s done\n",__func__);
}
