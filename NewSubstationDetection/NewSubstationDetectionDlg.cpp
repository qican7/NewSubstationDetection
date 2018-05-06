
// NewSubstationDetectionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "NewSubstationDetection.h"
#include "NewSubstationDetectionDlg.h"
#include "afxdialogex.h"
#include "Utils.h"
#include "changeColor.h"
#include "WinsockMatTransmissionClient.h" 
#include "WinsockMatTransmissionClient_1.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool checkPixelTemp(IplImage* &src, int x, int y, int value,int space);


//void insertIntoList(int x,int y,int AbnormalDimension,float areaMinTemp,float areaMaxTemp,float areaAvgTemp,CString m_abnormalDeviceTemp,float referDiffTemp,int abnormalLevel);
//void findArea(IplImage* colorImage,IplImage* grayImage,int index,int AbnormalCount,int x,int y);

UINT VideoProcess_frare(LPVOID lpParameter);//线程函数的声明
UINT PictureProcess_frare(LPVOID lpParameter);//线程函数的声明

UINT message_socket(LPVOID lpParameter);//线程函数的声明


CWinThread* FrareThread;		//读红外视频线程
CWinThread* socketThread;
CWinThread* FrareThread_deal;	//处理帧图片线程


IplImage *frame_frare; //视频帧
IplImage *frame_light; //视频帧
CvCapture *capture_frare;
CvCapture *capture_light;

//int detectStartRow = 120;  //设置检测区域，在实际应用中应设置为全图
//int detectEndRow = 220;
//int detectStatCol = 40;
//int detectEndCol = 80;
//int detectStatCol = 200;
//int detectEndCol = 300;

int detectStartRow = 60;  //设置检测区域，在实际应用中应设置为全图
int detectEndRow = 120;
int detectStatCol = 10;
int detectEndCol = 80;

int firstValue = 225;
int secondValue = 500;
int thirdValue = 500;

static int flag[240][320] = {1}; //存储像素点的标志，0：未访问 1：已访问
int AbnormalDimension[3] = {1}; //存储异常区域的面积

int maxGrayValue = 0; //异常区域的最大灰度值
int total_grayValue = 0;

int center_x = -1;
int center_y = -1;

//计算温升速度
float last_firstTemp  = -1;
float last_secondTemp = -1;
float last_thirdTemp  = -1;

//参数
string deviceType = "breaker";
float areaMaxTemp = 0;  //最高温度
float areaAvgTemp = 0;  //平均温度
float referDiffTemp = 0; //相对温差
float enviTemp = 25;   //环境温度
float referTemp = 26;  //参考温度
float tempUpSpeed = -1000;


//配准参数
int PeiZhun_flag = 1;
Mat H ;//变换矩阵

IplImage* transColorImage;

vector<Point2f> firstType,secondType,thirdType;

WinsockMatTransmissionClient socketMat;   //传输异常信息
WinsockMatTransmissionClient_1 socketMat_transVideo;  //传输视频


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNewSubstationDetectionDlg 对话框

CNewSubstationDetectionDlg::CNewSubstationDetectionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNewSubstationDetectionDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNewSubstationDetectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_frare, m_frare);
	DDX_Control(pDX, IDC_light, m_light);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_show, m_show);
}

BEGIN_MESSAGE_MAP(CNewSubstationDetectionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CNewSubstationDetectionDlg 消息处理程序

BOOL CNewSubstationDetectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//连接服务器
	int type =0;
	if ((type = socketMat.socketConnect("127.0.0.1", 6666) )< 0)  
	{  
		AfxMessageBox(intToCString(type));
		return FALSE;  
	}

	if ((type =socketMat_transVideo.socketConnect("127.0.0.1", 6667)) < 0)  
	{  
		AfxMessageBox(intToCString(type));
		return FALSE;  
	}

	


	//list control控件初始化
	CRect rect;   

	// 获取编程语言列表视图控件的位置和大小   
	m_list.GetClientRect(&rect);   

	// 为列表视图控件添加全行选中和栅格风格   
	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);   

	// 为列表视图控件添加三列   
	m_list.InsertColumn(0, _T("编号"), LVCFMT_CENTER, rect.Width()/24, 0);  
	m_list.InsertColumn(1, _T("设备类型"), LVCFMT_CENTER, rect.Width()/12, 1);  
	m_list.InsertColumn(2, _T("异常中心"), LVCFMT_CENTER, rect.Width()/12, 2);   
	m_list.InsertColumn(3, _T("异常面积"), LVCFMT_CENTER, rect.Width()/12, 3); 
	m_list.InsertColumn(4, _T("最高温度(°C)"), LVCFMT_CENTER, rect.Width()/12, 4);
	m_list.InsertColumn(5, _T("平均温度(°C)"), LVCFMT_CENTER, rect.Width()/12, 5);
	m_list.InsertColumn(6, _T("参考温度(°C)"), LVCFMT_CENTER, rect.Width()/12, 6);
	m_list.InsertColumn(7, _T("环境温度(°C)"), LVCFMT_CENTER, rect.Width()/12, 7);
	m_list.InsertColumn(8, _T("相对温差(%)"), LVCFMT_CENTER, rect.Width()/12, 8);
	m_list.InsertColumn(9, _T("温升速度(°C/s)"), LVCFMT_CENTER, rect.Width()/12, 9);
	m_list.InsertColumn(10, _T("异常类型"), LVCFMT_CENTER, rect.Width()/12, 10);
	m_list.InsertColumn(11, _T("监测时间"), LVCFMT_CENTER, rect.Width()*3/24, 11);

	////创建并启动线程 
	FrareThread = AfxBeginThread(VideoProcess_frare,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
	//socketThread = AfxBeginThread(message_socket,this,THREAD_PRIORITY_NORMAL,0,0,NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CNewSubstationDetectionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}


void CNewSubstationDetectionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);

		
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//线程函数，播放视频
UINT VideoProcess_frare(LPVOID lpParameter)//必须声明为UINT类型
{

	CNewSubstationDetectionDlg *Dlg = (CNewSubstationDetectionDlg*) lpParameter;

	capture_frare = cvCreateFileCapture ("video/FLIR0095.mp4");  //读取红外视频
	capture_light = cvCreateFileCapture ("video/FLIR0095.mp4");  //读取可见光视频
	
	if(capture_frare==NULL) {
		AfxMessageBox(L"NO frare capture");
		return 1;
	};  

	if(capture_light==NULL) {
		AfxMessageBox(L"NO light capture");
		return 1;
	}; 

	int picNumber = 0;
	MSG msg;   //增加一个MSG的变量msg来接收消息
	while(frame_frare = cvQueryFrame( capture_frare ))     
	{
		frame_light = cvQueryFrame( capture_light ); //获取可见光视频帧

		Dlg -> DrawPicToHDC(frame_frare, IDC_frare);
		Dlg -> DrawPicToHDC(frame_light, IDC_light); //显示红外视频帧和可见光视频帧
		
		//传输红外和可见光视频
		socketMat_transVideo.transmit(IplImageToMat(frame_frare),IplImageToMat(frame_light));

		picNumber ++;
		if (picNumber % 80 == 0) //每80帧图像处理1帧
		{
			FrareThread_deal = AfxBeginThread(PictureProcess_frare,Dlg,THREAD_PRIORITY_NORMAL,0,0,NULL);
		}else if (picNumber>=400)
		{
			picNumber = 0;
		}

		
		Sleep(35); 

		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){      //将消息队列里的消息逐个读入msg

			if(msg.message == WM_QUIT){     //如果收到终止消息则退出

				//TODO：放在堆里的变量要在这里手动清理
				return 0;      //线程正常返回，会释放局部变量等内存资源
			}
			else{
				DispatchMessage(&msg);//字面意思，不解释
			}
		}
	} 

	cvReleaseCapture(&capture_frare);
	//释放连接
	socketMat.socketDisconnect();
	socketMat_transVideo.socketDisconnect();
	return 0;
}



//线程函数，播放视频
UINT message_socket(LPVOID lpParameter)//必须声明为UINT类型
{

	CNewSubstationDetectionDlg *Dlg = (CNewSubstationDetectionDlg*) lpParameter;

	MSG msg;   //增加一个MSG的变量msg来接收消息
	while(1)     
	{
		char buff[256];
		char szText[256] ;
		int ad = 1;
		       
		//从服务器端接收数据
		cout << " 从服务器端接收数据: " ;
		int nRecv = ::recv(socketMat.sockClient, buff, 256, 0);
		if(nRecv > 0)
		{
		    buff[nRecv] = '\0';
		    printf("接收到数据：%s\n", buff);
			if (buff[0] == 'q')
			{
				//AfxMessageBox(L"用户名和密码不匹配");
				Dlg->DrawPicToHDC(frame_light,IDC_show);
				//socketMat_transVideo.transmit(frame_light,ad);
				memset(buff,0,256);
			}
		}

		//Dlg -> DrawPicToHDC(frame_light, IDC_light);
		Sleep(35);

		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){      //将消息队列里的消息逐个读入msg

			if(msg.message == WM_QUIT){     //如果收到终止消息则退出

				//TODO：放在堆里的变量要在这里手动清理
				return 0;      //线程正常返回，会释放局部变量等内存资源
			}
			else{
				DispatchMessage(&msg);//字面意思，不解释
			}
		}
	} 
	cvReleaseCapture(&capture_light);
	return 0;
}


//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CNewSubstationDetectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//图像处理
UINT PictureProcess_frare(LPVOID lpParameter)//必须声明为UINT类型
{
	CNewSubstationDetectionDlg *Dlg = (CNewSubstationDetectionDlg*) lpParameter;

	vector<float> tempture;
	Mat image = IplImageToMat(frame_frare);

	Mat destImg = Mat_colorToGray(image);  //彩色转灰度


	 IplImage* colorImage_detect = cvCloneImage(frame_frare);
	 IplImage* lightImage = cvCloneImage(frame_light);

	 IplImage* grayImage = IplImage_colorToGray(colorImage_detect);
	 IplImage* colorImage = cvCloneImage(colorImage_detect);


	 CTime tm = CTime::GetCurrentTime();
	 CString Time = tm.Format("%Y-%m-%d-%H-%M-%S");
	 //获取时间，存入数据库
	 //int m_nYear = tm.GetYear(); ///年
	 //int m_nMonth = tm.GetMonth(); ///月  
	 //int m_nDay = tm.GetDay(); ///日
	 //int m_nHour = tm.GetHour(); //时
	 //int m_nMin = tm.GetMinute(); //分
	 //int m_nSec = tm.GetSecond(); //秒
	 //CString Time = intToCString(m_nYear) + "-" +intToCString(m_nMonth)+ "-" +intToCString(m_nDay) 
		// + "-" +intToCString(m_nHour)+ "-" +intToCString(m_nMin)+ "-" +intToCString(m_nSec);

	 //cvShowImage("image",grayImage_1);

	 int src_rows = grayImage->height;
	 int src_cols = grayImage->width;
	 int space = 8;
	 int grayValue = 0;
	 firstType.clear();
	 secondType.clear();
	 thirdType.clear();
	
	 //用不同的颜色标识出异常区域的边缘
	 Point2f temp;
	 for (int i=detectStartRow;i<=detectEndRow;i++)
	 {
		 for (int j=detectStatCol;j<=detectEndCol;j++)
		 {

			 grayValue = getGray(grayImage,i,j);
			 if (abs(grayValue-firstValue) < space){


				 if (checkPixelTemp(grayImage,i,j,firstValue,5))
				 {
					 changeColor(colorImage_detect,i,j,0,255,0);
					 temp.x = j;
					 temp.y = i;
					 firstType.push_back(temp);
				 }

			 }else if (abs(grayValue-secondValue) < space){


				 if (checkPixelTemp(grayImage,i,j,secondValue,5))
				 {
					 changeColor(colorImage_detect,i,j,62,178,102);
					 temp.x = j;
					 temp.y = i;
					 secondType.push_back(temp);
				 }


			 }else if (abs(grayValue-thirdValue) < space){


				 if (checkPixelTemp(grayImage,i,j,thirdValue,5))
				 {
					 //changeColor(colorImage_2,i,j,0,97,56);
					 changeColor(colorImage_detect,i,j,0,255,0);
					 temp.x = j;
					 temp.y = i;
					 thirdType.push_back(temp);
				 }

			 }

		 }

	 }
	 //Dlg->DrawPicToHDC(colorImage_2,IDC_light);
	 cvSaveImage("colorImage_2.jpg",colorImage_detect);
	
	 
	 //检测异常区域
	 Dlg->detectAbnormalArea(grayImage,colorImage,colorImage_detect,lightImage);

	 //释放资源
	 image.release();
	 destImg.release();
	 cvReleaseImage(&colorImage_detect);
	 cvReleaseImage(&lightImage);
	 cvReleaseImage(&grayImage);
	 cvReleaseImage(&colorImage);
	 
	return 0 ;
}



//检查像素是否为边缘像素
bool checkPixelTemp(IplImage* &src, int x, int y, int value,int space)
{
	int big_count = 0;
	int small_count = 0;
	int x_range = src->width - 1;
	int y_range = src->height - 1;
	for (int i=x-1;i<x+2;i+=1)
	{
		for (int j=y-1;j<y+2;j+=1)
		{
			if (i>=0 && i<=x_range && j>=0 && j<=y_range) //半段下标是否越界
			{
				if ((getGray(src,i,j)-value) > space)
				{
					big_count ++;
				}else 
				{
					//small_count ++;
				}
			}
		}
	}

	if (big_count >= 2)
	{
		return true;
	}
	return false;
}

void CNewSubstationDetectionDlg::detectAbnormalArea(IplImage* grayImage,IplImage* colorImage,IplImage* colorImage_detect,IplImage* lightImage){

	//Mat colorImage = IplImageToMat(grayImage);

	/*cvShowImage("colorImage1",colorImage);
	AfxMessageBox(intToCString(firstValue));
	AfxMessageBox(intToCString(secondValue));
	AfxMessageBox(intToCString(thirdValue));*/
	//图像配准
	if (PeiZhun_flag == 1)
	{
		//获取红外和可见光的变换矩阵
		IplImage* infraredImage_H = cvLoadImage("infraredImage_H.jpg");
		IplImage* lightImage_H = cvLoadImage("lightImage_H.jpg");

		while(H.empty())
		{
			H = getH(infraredImage_H,lightImage_H);
			//AfxMessageBox(_T("H矩阵为空"));
		}

		PeiZhun_flag = 2;

		cvReleaseImage(&infraredImage_H);
		cvReleaseImage(&lightImage_H);

		//H = getH(colorImage,frame_light);
		//PeiZhun_flag = 2;
	}
	IplImage* lightImage_1 = PeiZhun(lightImage,firstType,secondType,thirdType);

	//cvSaveImage("lightImage.jpg",lightImage_1);

	//需要传送的红外原图
	transColorImage = cvCloneImage(colorImage);

	int grayValue = 0;
	//检测三级异常，每一种异常区域标识一种颜色，方便后面统计异常区域
	for (int i=detectStartRow;i<=detectEndRow;i++)
	{
		for (int j=detectStatCol;j<=detectEndCol;j++)
		{
			grayValue = getGray(grayImage,i,j);
			if (grayValue >= firstValue && grayValue <= secondValue)
			{
				changeColor(colorImage,i,j,255,0,0); //蓝色

			}else if (grayValue >= secondValue && grayValue <= thirdValue)
			{
				changeColor(colorImage,i,j,0,255,0); //绿色

			}else if (grayValue > thirdValue){

				changeColor(colorImage,i,j,0,0,255); //红色

			}else{
				changeColor(colorImage,i,j,0,0,0);
			}
		}
	}

	//cvSaveImage("colorImage_3.jpg",colorImage);

	
	//设置检测区域标记
	for (int i=detectStartRow;i<=detectEndRow;i++)
	{
		for (int j=detectStatCol;j<=detectEndCol;j++)
		{
			flag[i][j] = 0;
		}
	}

	//测试代码
	//for (int i=60;i<=120;i++)
	//{
	//	for (int j=40;j<=60;j++)
	//	{
	//		changeColor(colorImage,i,j,255,0,0);
	//	}
	//}

	//for (int i=121;i<=220;i++)
	//{
	//	for (int j=40;j<=60;j++)
	//	{
	//		changeColor(colorImage,i,j,0,255,0);
	//	}
	//}
	//for (int i=60;i<=220;i++)
	//{
	//	for (int j=60;j<=80;j++)
	//	{
	//		changeColor(colorImage,i,j,0,0,255);
	//	}
	//}

	
	vector<int> rgb;
	int firstAbnormalCount = 0;
	int secondAbnormalCount = 0;
	int thirdAbnormalCount = 0;

	//cvShowImage("colorImage",colorImage);



	CTime tm = CTime::GetCurrentTime();
	//CString detectTime = tm.Format("%Y-%m-%d-%H-%M-%S");

	CString detectDate = tm.Format("%Y-%m-%d");
	CString detectTime = tm.Format("%H-%M-%S");
	/*CString detectDate = tm.Format(_T("%x"));
	CString detectTime = tm.Format(_T("%X"));*/

	//获取时间，存入数据库
	//int year = tm.GetYear(); ///年
	//int month = tm.GetMonth(); ///月  
	//int day = tm.GetDay(); ///日
	//int hour = tm.GetHour(); //时
	//int minute = tm.GetMinute(); //分
	//int second = tm.GetSecond(); //秒

	//CString detectTime = intToCString(year) + "-" +intToCString(month)+ "-" +intToCString(day) 
	//	+ "-" +intToCString(hour)+ "-" +intToCString(minute)+ "-" +intToCString(second);


	
	//统计异常区域
	for (int i=detectStartRow;i<=detectEndRow;i++)
	{
		for (int j=detectStatCol;j<=detectEndCol;j++)
		{
			if (flag[i][j] == 0)
			{
				rgb = getRGB(colorImage,i,j);
				if (rgb[0] == 255)
				{
					maxGrayValue = getGray(grayImage,i,j); //存储该区域的最大灰度值
					total_grayValue = getGray(grayImage,i,j); //存储该区域的总灰度值
					rgb.clear();
					flag[i][j] = 1;
					findArea(colorImage,grayImage,0,firstAbnormalCount,i,j);
					//在右上角显示firstAbnormalCount
					if (AbnormalDimension[0]>20)
					{
						firstAbnormalCount ++;
						areaMaxTemp = maxGrayValue; //获取区域最高温度,以最大灰度值代替，下面类似
						areaAvgTemp = total_grayValue/AbnormalDimension[0]; //获取区域平均温度
						referTemp = 10;
						//referDiffTemp = (T1 - T2)/(T1 - T0) T1:发热 T2:正常 T0:环境
						referDiffTemp = (areaMaxTemp - referTemp)/(areaMaxTemp - enviTemp);
						if (tempUpSpeed >= -100)
						{
							tempUpSpeed = areaMaxTemp - last_firstTemp;
						}else{
							tempUpSpeed = 0;
						}
						
						//insertIntoList(i,j,AbnormalDimension[0],areaMinTemp,areaMaxTemp,areaAvgTemp,m_abnormalDeviceTemp,referDiffTemp,0);
						insertIntoList(stringToCString(deviceType), center_x,center_y,AbnormalDimension[0],areaMaxTemp,areaAvgTemp,Save2Float(referTemp),Save2Float(enviTemp), referDiffTemp,tempUpSpeed,0, detectDate + _T("-") + detectTime);
						//传输数据
						//参数分别为：红外原图 红外异常检测图 可见光配准图 监测位置 区域中心x 区域中心y 区域面积 最低温度 最高温度 平均温度 参考温度 相对温差 温升速度 缺陷等级
						socketMat.transmit(IplImageToMat(transColorImage),IplImageToMat(colorImage_detect),IplImageToMat(lightImage_1),deviceType,center_x,center_y,AbnormalDimension[0],areaMaxTemp,areaAvgTemp,referTemp,enviTemp,referDiffTemp,tempUpSpeed,0,CStringToString(detectDate), CStringToString(detectTime)); 
						//AfxMessageBox(intToCString(i)+_T(" ")+intToCString(j));
						//AfxMessageBox(intToCString(i)+_T(" ")+intToCString(j));
						//AfxMessageBox(_T("AbnormalDimension[0]")+intToCString(AbnormalDimension[0])); 
					}
					
					AbnormalDimension[0] = 0;
					total_grayValue = 0;
					
					
				}else if (rgb[1] == 255)
				{
					maxGrayValue = getGray(grayImage,i,j); //存储该区域的最大灰度值
					total_grayValue = getGray(grayImage,i,j); //存储该区域的总灰度值
					rgb.clear();
					flag[i][j] = 1;
					findArea(colorImage,grayImage,0,firstAbnormalCount,i,j);
					//在右上角显示firstAbnormalCount
					if (AbnormalDimension[0]>20)
					{
						firstAbnormalCount ++;
						areaMaxTemp = maxGrayValue; //获取区域最高温度,以最大灰度值代替，下面类似
						areaAvgTemp = total_grayValue/AbnormalDimension[0]; //获取区域平均温度
						referTemp = 10;
						//referDiffTemp = (T1 - T2)/(T1 - T0) T1:发热 T2:正常 T0:环境
						referDiffTemp = (areaMaxTemp - referTemp)/(areaMaxTemp - enviTemp);
						if (tempUpSpeed >= -100)
						{
							tempUpSpeed = areaMaxTemp - last_firstTemp;
						}else{
							tempUpSpeed = 0;
						}

						//insertIntoList(i,j,AbnormalDimension[0],areaMinTemp,areaMaxTemp,areaAvgTemp,m_abnormalDeviceTemp,referDiffTemp,0);
						insertIntoList(stringToCString(deviceType), center_x,center_y,AbnormalDimension[0],areaMaxTemp,areaAvgTemp,Save2Float(referTemp),Save2Float(enviTemp), referDiffTemp,tempUpSpeed,1,detectTime);
						//传输数据
						//参数分别为：红外原图 红外异常检测图 可见光配准图 监测位置 区域中心x 区域中心y 区域面积 最低温度 最高温度 平均温度 参考温度 相对温差 温升速度 缺陷等级
						socketMat.transmit(IplImageToMat(transColorImage),IplImageToMat(colorImage_detect),IplImageToMat(lightImage_1),deviceType,center_x,center_y,AbnormalDimension[1],areaMaxTemp,areaAvgTemp,referTemp,enviTemp,referDiffTemp,tempUpSpeed,1,CStringToString(detectDate), CStringToString(detectTime)); 
						//socketMat.transmit(IplImageToMat(transColorImage),IplImageToMat(colorImage_2),IplImageToMat(lightImage_1),1,"breaker",i,j,AbnormalDimension[0],areaMinTemp,areaMaxTemp,areaAvgTemp,25.0,referDiffTemp,0.2,1); 
						//AfxMessageBox(intToCString(i)+_T(" ")+intToCString(j));
						//AfxMessageBox(_T("AbnormalDimension[1]")+intToCString(AbnormalDimension[1]));
					}

					AbnormalDimension[1] = 0;
					total_grayValue = 0;
					
				}else if (rgb[2] == 255)
				{
					maxGrayValue = getGray(grayImage,i,j); //存储该区域的最大灰度值
					total_grayValue = getGray(grayImage,i,j); //存储该区域的总灰度值
					rgb.clear();
					flag[i][j] = 1;
					findArea(colorImage,grayImage,0,firstAbnormalCount,i,j);
					//在右上角显示firstAbnormalCount
					if (AbnormalDimension[0]>20)
					{
						firstAbnormalCount ++;
						areaMaxTemp = maxGrayValue; //获取区域最高温度,以最大灰度值代替，下面类似
						areaAvgTemp = total_grayValue/AbnormalDimension[0]; //获取区域平均温度
						referTemp = 10;
						//referDiffTemp = (T1 - T2)/(T1 - T0) T1:发热 T2:正常 T0:环境
						referDiffTemp = (areaMaxTemp - referTemp)/(areaMaxTemp - enviTemp);
						if (tempUpSpeed >= -100)
						{
							tempUpSpeed = areaMaxTemp - last_firstTemp;
						}else{
							tempUpSpeed = 0;
						}

						//insertIntoList(i,j,AbnormalDimension[0],areaMinTemp,areaMaxTemp,areaAvgTemp,m_abnormalDeviceTemp,referDiffTemp,0);
						insertIntoList(stringToCString(deviceType), center_x,center_y,AbnormalDimension[0],areaMaxTemp,areaAvgTemp,Save2Float(referTemp),Save2Float(enviTemp), referDiffTemp,tempUpSpeed,2,detectTime);
						//传输数据
						//参数分别为：红外原图 红外异常检测图 可见光配准图  区域中心x 区域中心y 异常面积 最高温度  平均温度 参考温度  环境温度 相对温差 温升速度 异常类型 检测时间
						socketMat.transmit(IplImageToMat(transColorImage),IplImageToMat(colorImage_detect),IplImageToMat(lightImage_1),deviceType,center_x,center_y,AbnormalDimension[2],areaMaxTemp,areaAvgTemp,referTemp,enviTemp,referDiffTemp,tempUpSpeed,2,CStringToString(detectDate), CStringToString(detectTime)); 
						//socketMat.transmit(IplImageToMat(transColorImage),IplImageToMat(colorImage_2),IplImageToMat(lightImage_1),1,"breaker",i,j,AbnormalDimension[0],areaMinTemp,areaMaxTemp,areaAvgTemp,25.0,referDiffTemp,0.2,2,year,month,day,hour,minute,second); 
						//socketMat.transmit(IplImageToMat(colorImage),IplImageToMat(lightImage_1),"qican",areaMaxTemp,CStringToChar(path)); 
						//AfxMessageBox(intToCString(i)+_T(" ")+intToCString(j));
						//AfxMessageBox(_T("AbnormalDimension[2]")+intToCString(AbnormalDimension[2]));
					}

					AbnormalDimension[2] = 0;
					total_grayValue = 0;
				
				}

				
			}
		}
	}
}

//将异常信息插入到列表中
void CNewSubstationDetectionDlg::insertIntoList(CString deviceType,int x,int y,int AbnormalDimension,float areaMaxTemp,float areaAvgTemp,CString referTemp,CString enviTemp, float referDiffTemp,float speed,int abnormalLevel,CString time){

	int nIndex = m_list.GetItemCount() + 1; //插入数据序号，从1开始
	CString t;
	t.Format(_T("%d"), nIndex);
	//LV_ITEM lvItem;
	//lvItem.mask = LVIF_TEXT;
	//lvItem.iItem = nIndex;     //行数
	//lvItem.iSubItem = 0;
	//lvItem.pszText = (LPWSTR)(LPCWSTR)t;
	//m_video_list.InsertItem(&lvItem);
	////插入其它列

	CString positon = _T("(") + intToCString(x) + _T(",") + intToCString(y) + _T(")");
	CString abnormalType;

	if (abnormalLevel == 0)
	{
		abnormalType = _T("一般缺陷");
	}else if (abnormalLevel == 1)
	{
		abnormalType = _T("严重缺陷");
	}else if (abnormalLevel == 2)
	{
		abnormalType = _T("视同紧急缺陷");
	}



	//插入列表数据
	m_list.InsertItem(0,(LPCTSTR)t);
	m_list.SetItemText(0, 1, deviceType);
	m_list.SetItemText(0, 2, positon);
	m_list.SetItemText(0, 3,intToCString(AbnormalDimension));
	m_list.SetItemText(0, 4, Save2Float(areaMaxTemp)); 
	m_list.SetItemText(0, 5, Save2Float(areaAvgTemp)); 
	m_list.SetItemText(0, 6, referTemp); 
	m_list.SetItemText(0, 7, enviTemp); 
	m_list.SetItemText(0, 8, Save2Float(referDiffTemp)); 
	m_list.SetItemText(0, 9, Save2Float(speed)); 
	m_list.SetItemText(0, 10, abnormalType); 
	m_list.SetItemText(0, 11, time); 


}

	//获取异常区域面积
	

void CNewSubstationDetectionDlg::findArea(IplImage* colorImage,IplImage* grayImage,int index,int AbnormalCount,int x,int y){

	vector<int> rgb; //暂时存储rgb值
	int temp_grayValue = 0;  //暂时存储灰度值

	for (int i=x-1;i<=x+1;i++)
	{
		for (int j=y-1;j<=y+1;j++)
		{
			if ((flag[i][j] == 0) && (i>=detectStartRow) && (i<=detectEndRow) && (j>=detectStatCol) && (j<=detectEndCol))
			{
				rgb = getRGB(colorImage,i,j);
				if (rgb[index] == 255)
				{
					temp_grayValue = getGray(grayImage,i,j);

					if (temp_grayValue > maxGrayValue) //获取区域最大温度值
					{
						maxGrayValue = temp_grayValue;
						center_x = i;
						center_y = j;
					}

					total_grayValue += temp_grayValue; //获取总灰度值

					rgb.clear();
					flag[i][j] = 1;
					AbnormalDimension[index] ++;
					findArea(colorImage,grayImage,index,AbnormalCount,i,j);
				}
			}	
		}
	}
}

	/*AfxMessageBox(_T("firstAbnormalCount")+intToCString(firstAbnormalCount)); 
	AfxMessageBox(_T("secondAbnormalCount")+intToCString(secondAbnormalCount));
	AfxMessageBox(_T("thirdAbnormalCount")+intToCString(thirdAbnormalCount));*/

//将视频帧显示到pictureControl
void CNewSubstationDetectionDlg::DrawPicToHDC(IplImage *img, UINT ID)
{
	CDC *pDC = GetDlgItem(ID)->GetDC(); 
	HDC hDC= pDC->GetSafeHdc(); 
	CRect rect; 
	GetDlgItem(ID)->GetClientRect(&rect); 
	CvvImage cimg; 

	cimg.CopyOf(img); 
	cimg.DrawToHDC(hDC,&rect); 

	ReleaseDC(pDC); 
}

//获取变换矩阵
Mat CNewSubstationDetectionDlg::getH(IplImage* &frame_infrare,IplImage* &frame_light){

	initModule_nonfree();//初始化模块，使用SIFT或SURF时用到      
	Ptr<FeatureDetector> detector = FeatureDetector::create( "SURF" );//创建SIFT特征检测器，可改成SURF/ORB     
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create( "SURF" );//创建特征向量生成器，可改成SURF/ORB     
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create( "BruteForce" );//创建特征匹配器       
	if( detector.empty() || descriptor_extractor.empty() )    
		AfxMessageBox(_T("配准过程中，创建 detector 失败!")); 
		//cout<<"fail to create detector!";      

	//读入图像     
	//DWORD start_time=GetTickCount();

	Mat img_infrare = IplImageToMat(frame_infrare);
	Mat img_light = IplImageToMat(frame_light);
 
	//灰度化
	cvtColor(img_infrare,img_infrare,CV_BGR2GRAY);  
	cvtColor(img_light,img_light,CV_BGR2GRAY); 

	//缩放

	if (img_infrare.rows != 240 || img_infrare.cols != 320)
	{
		Size imgSize_infrare(320,240);  
		resize(img_infrare, img_infrare, imgSize_infrare); 
	}

	if (img_light.rows != 240 || img_light.cols != 320)
	{
		Size imgSize_light(320,240);  
		resize(img_light, img_light, imgSize_light); 
	}
	 
	Mat img_infrare_edge,img_light_edge;
	//Sobel(img_1,img1,img_1.depth(),1,1);
	//Sobel(img_2,img2,img_2.depth(),1,1);
	Canny(img_infrare,img_infrare_edge,50,150,3);  
	Canny(img_light,img_light_edge,50,150,3);  

	//特征点检测       
	double t = getTickCount();//当前滴答数       
	vector<KeyPoint> m_LeftKey,m_RightKey;      
	detector->detect( img_infrare_edge, m_LeftKey );//检测img1中的SIFT特征点，存储到m_LeftKey中       
	detector->detect( img_light_edge, m_RightKey );      
/*	cout<<"图像1特征点个数:"<<m_LeftKey.size()<<endl;      
	cout<<"图像2特征点个数:"<<m_RightKey.size()<<endl;   */   

	//根据特征点计算特征描述子矩阵，即特征向量矩阵       
	Mat descriptors1,descriptors2;      
	descriptor_extractor->compute( img_infrare_edge, m_LeftKey, descriptors1 );      
	descriptor_extractor->compute( img_light_edge, m_RightKey, descriptors2 );      
	t = ((double)getTickCount() - t)/getTickFrequency();      
	/*	cout<<"SIFT算法用时："<<t<<"秒"<<endl;      

	std::cout<<"图像1特征描述矩阵大小："<< descriptors1.size      
	<<"，特征向量个数："<<descriptors1.rows<<"，维数："<<descriptors1.cols<<endl;      
	cout<<"图像2特征描述矩阵大小："<<descriptors2.size      
	<<"，特征向量个数："<<descriptors2.rows<<"，维数："<<descriptors2.cols<<endl;  */    

	//画出特征点       
	/*	Mat img_m_LeftKey,img_m_RightKey;      
	drawKeypoints(img1,m_LeftKey,img_m_LeftKey,Scalar::all(-1),0);      
	drawKeypoints(img2,m_RightKey,img_m_RightKey,Scalar::all(-1),0);      
	imshow("Src1",img_m_LeftKey);       
	imshow("Src2",img_m_RightKey);  */     

	//特征匹配       
	vector<DMatch> matches;//匹配结果       
	descriptor_matcher->match( descriptors1, descriptors2, matches );//匹配两个图像的特征矩阵       
	//cout<<"Match个数："<<matches.size()<<endl;      

	//计算匹配结果中距离的最大和最小值       
	//距离是指两个特征向量间的欧式距离，表明两个特征的差异，值越小表明两个特征点越接近       
	double max_dist = 0;      
	double min_dist = 100;      
	for(int i=0; i<matches.size(); i++)      
	{      
		double dist = matches[i].distance;      
		if(dist < min_dist) min_dist = dist;      
		if(dist > max_dist) max_dist = dist;      
	}      
	/*	cout<<"最大距离："<<max_dist<<endl;      
	cout<<"最小距离："<<min_dist<<endl; */     

	//筛选出较好的匹配点       
	vector<DMatch> goodMatches;      
	for(int i=0; i<matches.size(); i++)      
	{      
		if(matches[i].distance < 0.2 * max_dist)      
		{      
			goodMatches.push_back(matches[i]);      
		}      
	}      
	//cout<<"goodMatch个数："<<goodMatches.size()<<endl;      

	//画出匹配结果       
	//Mat img_matches;      
	////红色连接的是匹配的特征点对，绿色是未匹配的特征点       
	//drawMatches(img1,m_LeftKey,img2,m_RightKey,goodMatches,img_matches,      
	//	Scalar::all(-1)/*CV_RGB(255,0,0)*/,CV_RGB(0,255,0),Mat(),2);      

	//imshow("MatchSIFT",img_matches);      
	//IplImage result=img_matches;    

	//waitKey(10);    


	//RANSAC匹配过程     
	vector<DMatch> m_Matches=goodMatches;    
	// 分配空间     
	int ptCount = (int)m_Matches.size();    

	if ( ptCount<30)  
	{         
		cout<<"没有找到足够的匹配点"<<endl;  
		waitKey(0);  
		return H;  
	}  
	Mat p1(ptCount, 2, CV_32F);    
	Mat p2(ptCount, 2, CV_32F);    

	// 把Keypoint转换为Mat     
	Point2f pt;    
	for (int i=0; i<ptCount; i++)    
	{    
		pt = m_LeftKey[m_Matches[i].queryIdx].pt;    
		p1.at<float>(i, 0) = pt.x;    
		p1.at<float>(i, 1) = pt.y;    

		pt = m_RightKey[m_Matches[i].trainIdx].pt;    
		p2.at<float>(i, 0) = pt.x;    
		p2.at<float>(i, 1) = pt.y;    
	}    

	// 用RANSAC方法计算F     
	Mat m_Fundamental;    
	vector<uchar> m_RANSACStatus;       // 这个变量用于存储RANSAC后每个点的状态     
	findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);    

	// 计算野点个数     

	int OutlinerCount = 0;    
	for (int i=0; i<ptCount; i++)    
	{    
		if (m_RANSACStatus[i] == 0)    // 状态为0表示野点     
		{    
			OutlinerCount++;    
		}    
	}    
	int InlinerCount = ptCount - OutlinerCount;   // 计算内点     
	//cout<<"内点数为："<<InlinerCount<<endl;    


	// 这三个变量用于保存内点和匹配关系     
	vector<Point2f> m_LeftInlier;    
	vector<Point2f> m_RightInlier;    
	vector<DMatch> m_InlierMatches;    

	m_InlierMatches.resize(InlinerCount);    
	m_LeftInlier.resize(InlinerCount);    
	m_RightInlier.resize(InlinerCount);    
	InlinerCount=0;    
	float inlier_minRx=img_infrare_edge.cols;        //用于存储内点中右图最小横坐标，以便后续融合     

	for (int i=0; i<ptCount; i++)    
	{    
		if (m_RANSACStatus[i] != 0)    
		{    
			m_LeftInlier[InlinerCount].x = p1.at<float>(i, 0);    
			m_LeftInlier[InlinerCount].y = p1.at<float>(i, 1);    
			m_RightInlier[InlinerCount].x = p2.at<float>(i, 0);    
			m_RightInlier[InlinerCount].y = p2.at<float>(i, 1);    
			m_InlierMatches[InlinerCount].queryIdx = InlinerCount;    
			m_InlierMatches[InlinerCount].trainIdx = InlinerCount;    

			if(m_RightInlier[InlinerCount].x<inlier_minRx) inlier_minRx=m_RightInlier[InlinerCount].x;   //存储内点中右图最小横坐标     

			InlinerCount++;    
		}    
	}    

	// 把内点转换为drawMatches可以使用的格式     
	vector<KeyPoint> key1(InlinerCount);    
	vector<KeyPoint> key2(InlinerCount);    
	KeyPoint::convert(m_LeftInlier, key1);    
	KeyPoint::convert(m_RightInlier, key2);    

	// 显示计算F过后的内点匹配     
	//Mat OutImage;    
	//drawMatches(img1, key1, img2, key2, m_InlierMatches, OutImage);    
	//cvNamedWindow( "Match features", 1);    
	//cvShowImage("Match features", &IplImage(OutImage));    
	//waitKey(10);    

	//cvDestroyAllWindows();    

	//矩阵H用以存储RANSAC得到的单应矩阵     
	Mat H = findHomography( m_LeftInlier, m_RightInlier, RANSAC );   

	//释放内存
	img_infrare.release();
	img_light.release();
	img_infrare_edge.release();
	img_light_edge.release();
	descriptors1.release();
	descriptors2.release(); 
	p1.release();
	p2.release();
	m_Fundamental.release();
	vector<KeyPoint>().swap(m_LeftKey);
	vector<KeyPoint>().swap(m_RightKey);
	vector<DMatch>().swap(matches);
	vector<DMatch>().swap(goodMatches); 
	vector<DMatch>().swap(m_Matches); 
	vector<uchar>().swap(m_RANSACStatus); 
	vector<Point2f>().swap(m_LeftInlier);    
	vector<Point2f>().swap(m_RightInlier);    
	vector<DMatch>().swap(m_InlierMatches); 
	vector<KeyPoint>().swap(key1);    
	vector<KeyPoint>().swap(key2);  

	return H; //返回变换矩阵

}

//获取配准后的图像
IplImage* CNewSubstationDetectionDlg::PeiZhun(IplImage* frame_light,vector<Point2f> firstType,vector<Point2f> secondType, vector<Point2f> thirdType){

	if (!firstType.empty())
	{

		if (H.empty())
		{
			AfxMessageBox(_T("H矩阵为空"));
		}

		std::vector<Point2f> firstType_1;
		perspectiveTransform( firstType, firstType_1, H);
		for (int i=0;i<firstType_1.size();i++)
		{
			changeColor(frame_light,firstType_1[i].y,firstType_1[i].x,0,255,0);
		}
	}

	if (!secondType.empty())
	{
		std::vector<Point2f> secondType_1;
		perspectiveTransform( secondType, secondType_1, H);
		for (int i=0;i<secondType_1.size();i++)
		{
			changeColor(frame_light,secondType_1[i].y,secondType_1[i].x,62,178,102);
		}
	}

	if (!thirdType.empty())
	{
		std::vector<Point2f> thirdType_1;
		perspectiveTransform( secondType, thirdType_1, H);
		for (int i=0;i<thirdType_1.size();i++)
		{
			changeColor(frame_light,thirdType_1[i].y,thirdType_1[i].x,0,97,56);
		}
	}

	return frame_light;
}


