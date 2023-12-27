//
// Created by quectel on 29/7/2022.
//

#ifndef SAMPLECODE_QUECTELUNET_H
#define SAMPLECODE_QUECTELUNET_H

#include <cstring>
#include <iostream>
#include <getopt.h>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <iterator>
#include <unordered_map>
#include <algorithm>
#include <chrono>


class QuectelUnetBuilderInfer;

using namespace std;

class Quectel_Infer {
public:
    Quectel_Infer();
    ~Quectel_Infer();

    int initial_inference(string infer_config); // initial model and create input && output data buffer.
    int inference(unsigned char *srcData, unsigned char *dstData, string &predLabel,int srcWidth, int srcHeight,int dstWidth, int dstHeight,  int nchannel,
                      int flags, bool show= false, const string& pathOut="./output");
    int unInit_inference();

    // 下面需要传入vector长度为 4的 int ，设置四个油污面积过滤参数
    void setOilAreafilter(vector<int> _oilAreaFilterThresh);
    // 下面需要传入vector长度为 3的 int 设置四个油污灰度过滤参数
    void setOilGrayfilter(vector<int> _oilGrayFilterThresh);

    vector<int> getOilAreafilter() const;
    vector<int> getOilGrayfilter() const;

	// 设置划痕长度阈值
    void setLengthFilter(float _lengthFilterThresh);
	// 设置亮点面积阈值（小压伤）
    void setHighLightFilter(float _highLightFilterThresh);
	// 设置压伤面积阈值
    void setCrushFilter(float _crushFilterThresh);


    float getLengthFilter() const;
    float getHighLightFilter() const;
    float getCrushFilter() const;



    // 设置四个面的边缘裁剪像素（除非出现屏蔽罩外误检，不建议改动
    void setPaddingA(vector<int> _paddingA);
    void setPaddingB(vector<int> _paddingB);
    void setPaddingC(vector<int> _paddingC);
    void setPaddingD(vector<int> _paddingD);


    vector<int> getPaddingA() const;
    vector<int> getPaddingB() const;
    vector<int> getPaddingC() const;
    vector<int> getPaddingD() const;





    void setLabels(std::vector<std::string> _labels);
private:
    QuectelUnetBuilderInfer *p_BuilderInfer;

};


#endif
