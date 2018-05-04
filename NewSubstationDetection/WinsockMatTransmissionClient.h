    /*M/////////////////////////////////////////////////////////////////////////////////////// 
    // 
    //  基于OpenCV和Winsock的图像传输（发送） 
    //   
    //  By 彭曾 , at CUST, 2016.08.06  
    // 
    //  website: www.pengz0807.com  email: pengz0807@163.com  
    //   
    //M*/  
      
#ifndef __WINSOCKMATTRANSMISSIONCLIENT_H__  
#define __WINSOCKMATTRANSMISSIONCLIENT_H__  
    #include "opencv2/opencv.hpp"  
    #include "opencv2/highgui/highgui.hpp"  
    #include "opencv2/imgproc/imgproc.hpp"  
    #include "opencv2/core/core.hpp"  
    #include <stdio.h>  
    #include <Winsock2.h>

    #include <string>

    using namespace std;
      
    #pragma comment(lib,"WS2_32.lib")  
      
      
    //待传输图像默认大小为 640*480，可修改  
    //#define IMG_WIDTH 640   // 需传输图像的宽  
    //#define IMG_HEIGHT 480  // 需传输图像的高  
    #define IMG_WIDTH 320   // 需传输图像的宽  
    #define IMG_HEIGHT 240  // 需传输图像的高 
    //默认格式为CV_8UC3  
    #define BUFFER_SIZE IMG_WIDTH*IMG_HEIGHT*3/40  
      
    struct sentbuf  
    {  
        char buf[BUFFER_SIZE];  
		char buf_1[BUFFER_SIZE];
		char buf_2[BUFFER_SIZE];
        int flag;  
		int position;
		string deviceType;
		int x;
		int y;
		int abnormalDimension;
		float areaMaxTemp;
		float areaAvgTemp;
		float referTemp;
		float enviTemp;
		float diffTemp;
		float speed;
		int level ;
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		
    };  
      
    class WinsockMatTransmissionClient  
    {  
    public:  
        WinsockMatTransmissionClient(void);  
        ~WinsockMatTransmissionClient(void);  
      
    private:  
         
      
    public:  
      
		SOCKET sockClient;  
		struct sentbuf data; 
        // 打开socket连接  
        // params : IP      服务器的ip地址  
        //          PORT    传输端口  
        // return : -1      连接失败  
        //          1       连接成功  
        int socketConnect(const char* IP, int PORT);  
      
      
        // 传输图像  
        // params : image 待传输图像  
        // return : -1      传输失败  
        //          1       传输成功  
        //int transmit(cv::Mat image,cv::Mat image_1,cv::Mat image_2,char* name,float temp,char* positionTemp);  
		int transmit(cv::Mat image,cv::Mat image_1,cv::Mat image_2,int position,string deviceType, int x,int y,int abnormalDimension,float areaMaxTemp,float areaAvgTemp,float referTemp,float enviTemp, float diffTemp,float speed,int level, int year,int month,int day,int hour,int minute,int second);
      
        // 断开socket连接  
        void socketDisconnect(void);  
		int picCount ;
    };  
      
    #endif  