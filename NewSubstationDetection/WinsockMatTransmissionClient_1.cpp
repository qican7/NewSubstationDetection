    /*M/////////////////////////////////////////////////////////////////////////////////////// 
    // 
    //  基于OpenCV和Winsock的图像传输（发送） 
    //   
    //  By 彭曾 , at CUST, 2016.08.06  
    // 
    //  website: www.pengz0807.com  email: pengz0807@163.com  
    //   
    //M*/  
      
     #include "StdAfx.h" 
    #include "WinsockMatTransmissionClient_1.h"  
      
      
    WinsockMatTransmissionClient_1::WinsockMatTransmissionClient_1(void)  
    {  
    }  
      
      
    WinsockMatTransmissionClient_1::~WinsockMatTransmissionClient_1(void)  
    {  
    }  
      
      
    int WinsockMatTransmissionClient_1::socketConnect(const char* IP, int PORT)  
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
      
      
    void WinsockMatTransmissionClient_1::socketDisconnect(void)  
    {  
        closesocket(sockClient);  
        WSACleanup();  
    }  
      
    int WinsockMatTransmissionClient_1::transmit(cv::Mat image_infrare,cv::Mat image_light)
    {  

        if (image_infrare.empty() || image_light.empty())  
        {  
            printf("empty image\n\n");  
            return -1;  
        }  
      
        if(image_infrare.cols != IMG_WIDTH || image_infrare.rows != IMG_HEIGHT || image_infrare.type() != CV_8UC3)  
        {  
            printf("the image_infrare must satisfy : cols == IMG_WIDTH（%d）  rows == IMG_HEIGHT（%d） type == CV_8UC3\n\n", IMG_WIDTH, IMG_HEIGHT);  
            return -1;  
        }  

		if(image_light.cols != IMG_WIDTH || image_light.rows != IMG_HEIGHT || image_light.type() != CV_8UC3)  
		{  
			printf("the image_light must satisfy : cols == IMG_WIDTH（%d）  rows == IMG_HEIGHT（%d） type == CV_8UC3\n\n", IMG_WIDTH, IMG_HEIGHT);  
			return -1;  
		} 
      
        for(int k = 0; k < 40; k++)   
        {  

			//分割红外图像
            int num1 = IMG_HEIGHT / 40 * k;  
            for (int i = 0; i < IMG_HEIGHT / 40; i++)  
            {  
                int num2 = i * IMG_WIDTH * 3;  
                uchar* ucdata = image_infrare.ptr<uchar>(i + num1);  
                for (int j = 0; j < IMG_WIDTH * 3; j++)  
                {  
                    data.buf[num2 + j] = ucdata[j];  
                }  
            }  

			//分割可见光图像
			int num1_1 = IMG_HEIGHT / 40 * k;  
			for (int i = 0; i < IMG_HEIGHT / 40; i++)  
			{  
				int num2_1 = i * IMG_WIDTH * 3;  
				uchar* ucdata = image_light.ptr<uchar>(i + num1_1);  
				for (int j = 0; j < IMG_WIDTH * 3; j++)  
				{  
					data.buf[num2_1 + j] = ucdata[j];  
				}  
			}  

			
      
            if(k == 39)
			{

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