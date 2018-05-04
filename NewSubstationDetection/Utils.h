#ifndef UTILS_H
#define UTILS_H

#include<stdio.h>  
#include <iostream> 
#include "stdafx.h"

#include "cv.h"
#include "highgui.h"

using namespace std; 
using namespace cv; 

//数据类型转换
extern CString intToCString(int data);
extern CString floatToCString(float data);

extern char* CStringToChar(CString data);
extern char* CStringToChar_1(CString data);
extern int CStringToInt(CString data);
extern float CStringToFloat(CString data);
extern CString Save2Float(float data); //float数据保留两位小数




extern IplImage*  IplImage_colorToGray(IplImage* pic); //彩色转灰度
extern Mat Mat_colorToGray(Mat image); //彩色转灰度

extern IplImage* MatToIplImage(Mat image); //Mat转IplImage
extern Mat IplImageToMat(IplImage* image); //IplImage转Mat


#endif