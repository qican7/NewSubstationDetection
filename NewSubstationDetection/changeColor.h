#ifndef CHANGECOLOR_H
#define CHANGECOLOR_H

#include "stdafx.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;


template<class T> class Image
{
private:
	IplImage* imgp;
public:
	Image(IplImage* img=0) {imgp=img;}
	~Image(){imgp=0;}
	void operator=(IplImage* img) {imgp=img;}
	inline T* operator[](const int rowIndx) {
		return ((T *)(imgp->imageData + rowIndx*imgp->widthStep));}
}; 

typedef struct{
	unsigned char b,g,r;
} RgbPixel; 

typedef struct{
	float b,g,r;
} RgbPixelFloat; 

typedef Image<RgbPixel>       RgbImage;
typedef Image<RgbPixelFloat>  RgbImageFloat;
typedef Image<unsigned char>  BwImage;
typedef Image<float>          BwImageFloat;

extern vector<int> getRGB(IplImage* image, int x, int y);  //获取RGB值
extern void changeColor(IplImage* img, int x, int y, int R, int G, int B);//改变RGB值

extern int getGray(IplImage* image, int x, int y);

extern void changeGray(IplImage* image, int x, int y,int value);

#endif