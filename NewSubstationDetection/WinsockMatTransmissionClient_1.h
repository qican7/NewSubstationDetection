    /*M/////////////////////////////////////////////////////////////////////////////////////// 
    // 
    //  ����OpenCV��Winsock��ͼ���䣨���ͣ� 
    //   
    //  By ���� , at CUST, 2016.08.06  
    // 
    //  website: www.pengz0807.com  email: pengz0807@163.com  
    //   
    //M*/  
      
#ifndef __WinsockMatTransmissionClient_1_H__  
#define __WinsockMatTransmissionClient_1_H__  
    #include "opencv2/opencv.hpp"  
    #include "opencv2/highgui/highgui.hpp"  
    #include "opencv2/imgproc/imgproc.hpp"  
    #include "opencv2/core/core.hpp"  
    #include <stdio.h>  
    #include <Winsock2.h>  
      
    #pragma comment(lib,"WS2_32.lib")  
      
      
    //������ͼ��Ĭ�ϴ�СΪ 640*480�����޸�  
    //#define IMG_WIDTH 640   // �贫��ͼ��Ŀ�  
    //#define IMG_HEIGHT 480  // �贫��ͼ��ĸ�  
    #define IMG_WIDTH 320   // �贫��ͼ��Ŀ�  
    #define IMG_HEIGHT 240  // �贫��ͼ��ĸ� 
    //Ĭ�ϸ�ʽΪCV_8UC3  
    #define BUFFER_SIZE IMG_WIDTH*IMG_HEIGHT*3/40  
      
    struct sentbuf_1  
    {  
        char buf[BUFFER_SIZE];  
        int flag;  
    };  
      
    class WinsockMatTransmissionClient_1  
    {  
    public:  
        WinsockMatTransmissionClient_1(void);  
        ~WinsockMatTransmissionClient_1(void);  
      
    private:  
         
      
    public:  
      
		SOCKET sockClient;  
		struct sentbuf_1 data; 
        // ��socket����  
        // params : IP      ��������ip��ַ  
        //          PORT    ����˿�  
        // return : -1      ����ʧ��  
        //          1       ���ӳɹ�  
        int socketConnect(const char* IP, int PORT);  
      
      
        // ����ͼ��  
        // params : image ������ͼ��  
        // return : -1      ����ʧ��  
        //          1       ����ɹ�  
        //int transmit(cv::Mat image,cv::Mat image_1,cv::Mat image_2,char* name,float temp,char* positionTemp);  
		int transmit(cv::Mat image_infrare,cv::Mat image_light_light);
      
        // �Ͽ�socket����  
        void socketDisconnect(void);  
		int picCount ;
    };  
      
    #endif  