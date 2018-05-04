#include "stdafx.h"
#include "changeColor.h"

//获取RGB值
vector<int> getRGB(IplImage* image, int x, int y){
	vector<int> rgb;
	RgbImage  imgA(image);

	int b = imgA[x][y].b ;
	int g = imgA[x][y].g ;
	int r = imgA[x][y].r ;

	rgb.push_back(b);
	rgb.push_back(g);
	rgb.push_back(r);

	return rgb;
}

//改变RGB值
void changeColor(IplImage* img, int x, int y, int R, int G, int B)
{
	RgbImage  imgA(img);

	imgA[x][y].b = R;
	imgA[x][y].g = G;
	imgA[x][y].r = B;

}

//获取灰度图像的灰度值
int getGray(IplImage* image, int x, int y){

	int value = 0;
	BwImage imgA(image);
	value = imgA[x][y] ;

	return value;

}

void changeGray(IplImage* image, int x, int y,int value){

	BwImage imgA(image);
	imgA[x][y] = value;

}