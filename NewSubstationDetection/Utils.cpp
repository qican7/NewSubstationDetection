#include "stdafx.h"
#include "Utils.h"

CString intToCString(int data){

	CString t;
	t.Format(_T("%d"), data);
	return t;
}

CString floatToCString(float data){

	CString t;
	t.Format(_T("%f"), data);
	return t;

}

char* CStringToChar(CString data){

	char *data1 = new char[100];
	WideCharToMultiByte(CP_ACP,0,data.GetBuffer(),-1,data1,100,NULL,NULL);//Unicode转换成多字节字符串

	return data1;
}

char* CStringToChar_1(CString data){

	char *data1 = new char[20];
	WideCharToMultiByte(CP_ACP,0,data.GetBuffer(),-1,data1,20,NULL,NULL);//Unicode转换成多字节字符串

	return data1;
}

int CStringToInt(CString data){

	int number = _ttoi(data);
	return number;
}

float CStringToFloat(CString data){

	float number = _ttof(data);
	return number;
}

//float数据保留两位小数
extern CString Save2Float(float data){

	char data_1[10] = {0};
	sprintf(data_1,"%0.2f",data);
	return (CString)data_1;
}



//彩色转灰度
IplImage*  IplImage_colorToGray(IplImage* pic){

	IplImage* picGray = cvCreateImage(cvGetSize(pic), IPL_DEPTH_8U, 1);//创建目标图像  
	cvCvtColor(pic,picGray,CV_BGR2GRAY);//cvCvtColor(src,des,CV_BGR2GRAY) 

	return picGray;
}

//彩色转灰度
Mat Mat_colorToGray(Mat image){

	Mat destImg;
	cvtColor(image, destImg, CV_BGR2GRAY); // 转为灰度图像
	return destImg;
}

//Mat转IplImage
IplImage* MatToIplImage(Mat image){

	IplImage pImg = IplImage(image);

	return &pImg;
}

//IplImage转Mat
Mat IplImageToMat(IplImage* image){

	Mat img(image,0);

	return img;
}

