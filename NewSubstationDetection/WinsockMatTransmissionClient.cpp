﻿    /*M/////////////////////////////////////////////////////////////////////////////////////// 
    // 
    //  基于OpenCV和Winsock的图像传输（发送） 
    //   
    //  By 漆灿 , at CUST, 2017.08.06  
    // 
    //  website: www.pengz0807.com  email: pengz0807@163.com  
    //   
    //M*/  
      
     #include "StdAfx.h" 
    #include "WinsockMatTransmissionClient.h"  
      
      
    WinsockMatTransmissionClient::WinsockMatTransmissionClient(void)  
    {  
    }  
      
      
    WinsockMatTransmissionClient::~WinsockMatTransmissionClient(void)  
    {  
    }  
      
      
    int WinsockMatTransmissionClient::socketConnect(const char* IP, int PORT)  
    {  
        WORD wVersionRequested;  
        WSADATA wsaData;  
        int err;  
        picCount = 0;
        wVersionRequested = MAKEWORD( 1, 1 );  
      
        err = WSAStartup( wVersionRequested, &wsaData );  
        if ( err != 0 ) {  
            return -1;  
        }  
      
        if ( LOBYTE( wsaData.wVersion ) != 1 ||  
            HIBYTE( wsaData.wVersion ) != 1 ) {  
                WSACleanup( );  
                return -1;  
        }  
      
        err = (sockClient = socket(AF_INET,SOCK_STREAM,0));  
        if (err < 0) {  
            printf("create socket error: %s(errno: %d)\n\n", strerror(errno), errno);  
            return -1;  
        }  
        else  
        {  
            printf("create socket successful!\nnow connect ...\n\n");  
        }  
      
        SOCKADDR_IN addrSrv;  
        addrSrv.sin_addr.S_un.S_addr=inet_addr(IP);  
        addrSrv.sin_family=AF_INET;  
        addrSrv.sin_port=htons(PORT);  
      
        err = connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));  
        if (err < 0)   
        {  
            printf("connect error: %s(errno: %d)\n\n", strerror(errno), errno);  
            return -1;  
        }  
        else   
        {  
            printf("connect successful!\n\n");  
            return 1;  
        }  
    }  
      
      
    void WinsockMatTransmissionClient::socketDisconnect(void)  
    {  
        closesocket(sockClient);  
        WSACleanup();  
    }  
      
    int WinsockMatTransmissionClient::transmit(cv::Mat image,cv::Mat image_1,cv::Mat image_2,string deviceType, int x,int y,int abnormalDimension,float areaMaxTemp,float areaAvgTemp,float referTemp,float enviTemp, float diffTemp,float speed,int level, string detectDate, string detectTime)
    {  
		//data.name = new char[10];
		//data.name = nameString;
		/*memcpy(data.name,"qican",sizeof("qican"));
		std::cout<<data.name<<std::endl;*/
        if (image.empty())  
        {  
            printf("empty image\n\n");  
            return -1;  
        }  
      
        if(image.cols != IMG_WIDTH || image.rows != IMG_HEIGHT || image.type() != CV_8UC3)  
        {  
            printf("the image must satisfy : cols == IMG_WIDTH（%d）  rows == IMG_HEIGHT（%d） type == CV_8UC3\n\n", IMG_WIDTH, IMG_HEIGHT);  
            return -1;  
        }  
      
        for(int k = 0; k < 40; k++)   
        {  
            int num1 = IMG_HEIGHT / 40 * k;  
            for (int i = 0; i < IMG_HEIGHT / 40; i++)  
            {  
                int num2 = i * IMG_WIDTH * 3;  
                uchar* ucdata = image.ptr<uchar>(i + num1);  
                for (int j = 0; j < IMG_WIDTH * 3; j++)  
                {  
                    data.buf[num2 + j] = ucdata[j];  
                }  
            }  

			int num1_1 = IMG_HEIGHT / 40 * k;  
			for (int i = 0; i < IMG_HEIGHT / 40; i++)  
			{  
				int num2_1 = i * IMG_WIDTH * 3;  
				uchar* ucdata_1 = image_1.ptr<uchar>(i + num1_1);  
				for (int j = 0; j < IMG_WIDTH * 3; j++)  
				{  
					data.buf_1[num2_1 + j] = ucdata_1[j];  
				}  
			}  

			int num1_2 = IMG_HEIGHT / 40 * k;  
			for (int i = 0; i < IMG_HEIGHT / 40; i++)  
			{  
				int num2_2 = i * IMG_WIDTH * 3;  
				uchar* ucdata_2 = image_2.ptr<uchar>(i + num1_2);  
				for (int j = 0; j < IMG_WIDTH * 3; j++)  
				{  
					data.buf_2[num2_2 + j] = ucdata_2[j];  
				}  
			}  
      
            if(k == 39)
			{
				/*memcpy(data.name,nameString,strlen(nameString)+1);
				data.maxTemp = temp;
				memcpy(data.position,positionTemp,strlen(positionTemp)+1);*/
				//std::cout<<positionTemp<<" "<<strlen(positionTemp)<<std::endl;
				/*picCount ++;
				printf("%d\n",picCount); */
				data.deviceType = deviceType;
                data.x = x;
				data.y = y;
				data.abnormalDimension = abnormalDimension;
				data.areaMaxTemp = areaMaxTemp;
				data.areaAvgTemp = areaAvgTemp;
				data.referTemp = referTemp;
				data.enviTemp = enviTemp;
				data.diffTemp = diffTemp;
				data.speed = speed;
				data.level = level;
				data.detectDate = detectDate;
			    data.detectTime = detectTime;
				data.flag = 2;  //结束的标志，图像分为40次进行传输
				
			} 
            else  
                data.flag = 1;  

			//memcpy(data.name,"qican",sizeof("qican"));
      
            if (send(sockClient, (char *)(&data), sizeof(data), 0) < 0)  
            {  
                printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);  
                return -1;  
            }  
        }  
    }  