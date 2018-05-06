
// NewSubstationDetectionDlg.cpp : ʵ���ļ�
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

UINT VideoProcess_frare(LPVOID lpParameter);//�̺߳���������
UINT PictureProcess_frare(LPVOID lpParameter);//�̺߳���������

UINT message_socket(LPVOID lpParameter);//�̺߳���������


CWinThread* FrareThread;		//��������Ƶ�߳�
CWinThread* socketThread;
CWinThread* FrareThread_deal;	//����֡ͼƬ�߳�


IplImage *frame_frare; //��Ƶ֡
IplImage *frame_light; //��Ƶ֡
CvCapture *capture_frare;
CvCapture *capture_light;

//int detectStartRow = 120;  //���ü��������ʵ��Ӧ����Ӧ����Ϊȫͼ
//int detectEndRow = 220;
//int detectStatCol = 40;
//int detectEndCol = 80;
//int detectStatCol = 200;
//int detectEndCol = 300;

int detectStartRow = 60;  //���ü��������ʵ��Ӧ����Ӧ����Ϊȫͼ
int detectEndRow = 120;
int detectStatCol = 10;
int detectEndCol = 80;

int firstValue = 225;
int secondValue = 500;
int thirdValue = 500;

static int flag[240][320] = {1}; //�洢���ص�ı�־��0��δ���� 1���ѷ���
int AbnormalDimension[3] = {1}; //�洢�쳣��������

int maxGrayValue = 0; //�쳣��������Ҷ�ֵ
int total_grayValue = 0;

int center_x = -1;
int center_y = -1;

//���������ٶ�
float last_firstTemp  = -1;
float last_secondTemp = -1;
float last_thirdTemp  = -1;

//����
string deviceType = "breaker";
float areaMaxTemp = 0;  //����¶�
float areaAvgTemp = 0;  //ƽ���¶�
float referDiffTemp = 0; //����²�
float enviTemp = 25;   //�����¶�
float referTemp = 26;  //�ο��¶�
float tempUpSpeed = -1000;


//��׼����
int PeiZhun_flag = 1;
Mat H ;//�任����

IplImage* transColorImage;

vector<Point2f> firstType,secondType,thirdType;

WinsockMatTransmissionClient socketMat;   //�����쳣��Ϣ
WinsockMatTransmissionClient_1 socketMat_transVideo;  //������Ƶ


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CNewSubstationDetectionDlg �Ի���

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


// CNewSubstationDetectionDlg ��Ϣ�������

BOOL CNewSubstationDetectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	//���ӷ�����
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

	


	//list control�ؼ���ʼ��
	CRect rect;   

	// ��ȡ��������б���ͼ�ؼ���λ�úʹ�С   
	m_list.GetClientRect(&rect);   

	// Ϊ�б���ͼ�ؼ����ȫ��ѡ�к�դ����   
	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);   

	// Ϊ�б���ͼ�ؼ��������   
	m_list.InsertColumn(0, _T("���"), LVCFMT_CENTER, rect.Width()/24, 0);  
	m_list.InsertColumn(1, _T("�豸����"), LVCFMT_CENTER, rect.Width()/12, 1);  
	m_list.InsertColumn(2, _T("�쳣����"), LVCFMT_CENTER, rect.Width()/12, 2);   
	m_list.InsertColumn(3, _T("�쳣���"), LVCFMT_CENTER, rect.Width()/12, 3); 
	m_list.InsertColumn(4, _T("����¶�(��C)"), LVCFMT_CENTER, rect.Width()/12, 4);
	m_list.InsertColumn(5, _T("ƽ���¶�(��C)"), LVCFMT_CENTER, rect.Width()/12, 5);
	m_list.InsertColumn(6, _T("�ο��¶�(��C)"), LVCFMT_CENTER, rect.Width()/12, 6);
	m_list.InsertColumn(7, _T("�����¶�(��C)"), LVCFMT_CENTER, rect.Width()/12, 7);
	m_list.InsertColumn(8, _T("����²�(%)"), LVCFMT_CENTER, rect.Width()/12, 8);
	m_list.InsertColumn(9, _T("�����ٶ�(��C/s)"), LVCFMT_CENTER, rect.Width()/12, 9);
	m_list.InsertColumn(10, _T("�쳣����"), LVCFMT_CENTER, rect.Width()/12, 10);
	m_list.InsertColumn(11, _T("���ʱ��"), LVCFMT_CENTER, rect.Width()*3/24, 11);

	////�����������߳� 
	FrareThread = AfxBeginThread(VideoProcess_frare,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
	//socketThread = AfxBeginThread(message_socket,this,THREAD_PRIORITY_NORMAL,0,0,NULL);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);

		
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//�̺߳�����������Ƶ
UINT VideoProcess_frare(LPVOID lpParameter)//��������ΪUINT����
{

	CNewSubstationDetectionDlg *Dlg = (CNewSubstationDetectionDlg*) lpParameter;

	capture_frare = cvCreateFileCapture ("video/FLIR0095.mp4");  //��ȡ������Ƶ
	capture_light = cvCreateFileCapture ("video/FLIR0095.mp4");  //��ȡ�ɼ�����Ƶ
	
	if(capture_frare==NULL) {
		AfxMessageBox(L"NO frare capture");
		return 1;
	};  

	if(capture_light==NULL) {
		AfxMessageBox(L"NO light capture");
		return 1;
	}; 

	int picNumber = 0;
	MSG msg;   //����һ��MSG�ı���msg��������Ϣ
	while(frame_frare = cvQueryFrame( capture_frare ))     
	{
		frame_light = cvQueryFrame( capture_light ); //��ȡ�ɼ�����Ƶ֡

		Dlg -> DrawPicToHDC(frame_frare, IDC_frare);
		Dlg -> DrawPicToHDC(frame_light, IDC_light); //��ʾ������Ƶ֡�Ϳɼ�����Ƶ֡
		
		//�������Ϳɼ�����Ƶ
		socketMat_transVideo.transmit(IplImageToMat(frame_frare),IplImageToMat(frame_light));

		picNumber ++;
		if (picNumber % 80 == 0) //ÿ80֡ͼ����1֡
		{
			FrareThread_deal = AfxBeginThread(PictureProcess_frare,Dlg,THREAD_PRIORITY_NORMAL,0,0,NULL);
		}else if (picNumber>=400)
		{
			picNumber = 0;
		}

		
		Sleep(35); 

		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){      //����Ϣ���������Ϣ�������msg

			if(msg.message == WM_QUIT){     //����յ���ֹ��Ϣ���˳�

				//TODO�����ڶ���ı���Ҫ�������ֶ�����
				return 0;      //�߳��������أ����ͷžֲ��������ڴ���Դ
			}
			else{
				DispatchMessage(&msg);//������˼��������
			}
		}
	} 

	cvReleaseCapture(&capture_frare);
	//�ͷ�����
	socketMat.socketDisconnect();
	socketMat_transVideo.socketDisconnect();
	return 0;
}



//�̺߳�����������Ƶ
UINT message_socket(LPVOID lpParameter)//��������ΪUINT����
{

	CNewSubstationDetectionDlg *Dlg = (CNewSubstationDetectionDlg*) lpParameter;

	MSG msg;   //����һ��MSG�ı���msg��������Ϣ
	while(1)     
	{
		char buff[256];
		char szText[256] ;
		int ad = 1;
		       
		//�ӷ������˽�������
		cout << " �ӷ������˽�������: " ;
		int nRecv = ::recv(socketMat.sockClient, buff, 256, 0);
		if(nRecv > 0)
		{
		    buff[nRecv] = '\0';
		    printf("���յ����ݣ�%s\n", buff);
			if (buff[0] == 'q')
			{
				//AfxMessageBox(L"�û��������벻ƥ��");
				Dlg->DrawPicToHDC(frame_light,IDC_show);
				//socketMat_transVideo.transmit(frame_light,ad);
				memset(buff,0,256);
			}
		}

		//Dlg -> DrawPicToHDC(frame_light, IDC_light);
		Sleep(35);

		while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){      //����Ϣ���������Ϣ�������msg

			if(msg.message == WM_QUIT){     //����յ���ֹ��Ϣ���˳�

				//TODO�����ڶ���ı���Ҫ�������ֶ�����
				return 0;      //�߳��������أ����ͷžֲ��������ڴ���Դ
			}
			else{
				DispatchMessage(&msg);//������˼��������
			}
		}
	} 
	cvReleaseCapture(&capture_light);
	return 0;
}


//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CNewSubstationDetectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//ͼ����
UINT PictureProcess_frare(LPVOID lpParameter)//��������ΪUINT����
{
	CNewSubstationDetectionDlg *Dlg = (CNewSubstationDetectionDlg*) lpParameter;

	vector<float> tempture;
	Mat image = IplImageToMat(frame_frare);

	Mat destImg = Mat_colorToGray(image);  //��ɫת�Ҷ�


	 IplImage* colorImage_detect = cvCloneImage(frame_frare);
	 IplImage* lightImage = cvCloneImage(frame_light);

	 IplImage* grayImage = IplImage_colorToGray(colorImage_detect);
	 IplImage* colorImage = cvCloneImage(colorImage_detect);


	 CTime tm = CTime::GetCurrentTime();
	 CString Time = tm.Format("%Y-%m-%d-%H-%M-%S");
	 //��ȡʱ�䣬�������ݿ�
	 //int m_nYear = tm.GetYear(); ///��
	 //int m_nMonth = tm.GetMonth(); ///��  
	 //int m_nDay = tm.GetDay(); ///��
	 //int m_nHour = tm.GetHour(); //ʱ
	 //int m_nMin = tm.GetMinute(); //��
	 //int m_nSec = tm.GetSecond(); //��
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
	
	 //�ò�ͬ����ɫ��ʶ���쳣����ı�Ե
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
	
	 
	 //����쳣����
	 Dlg->detectAbnormalArea(grayImage,colorImage,colorImage_detect,lightImage);

	 //�ͷ���Դ
	 image.release();
	 destImg.release();
	 cvReleaseImage(&colorImage_detect);
	 cvReleaseImage(&lightImage);
	 cvReleaseImage(&grayImage);
	 cvReleaseImage(&colorImage);
	 
	return 0 ;
}



//��������Ƿ�Ϊ��Ե����
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
			if (i>=0 && i<=x_range && j>=0 && j<=y_range) //����±��Ƿ�Խ��
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
	//ͼ����׼
	if (PeiZhun_flag == 1)
	{
		//��ȡ����Ϳɼ���ı任����
		IplImage* infraredImage_H = cvLoadImage("infraredImage_H.jpg");
		IplImage* lightImage_H = cvLoadImage("lightImage_H.jpg");

		while(H.empty())
		{
			H = getH(infraredImage_H,lightImage_H);
			//AfxMessageBox(_T("H����Ϊ��"));
		}

		PeiZhun_flag = 2;

		cvReleaseImage(&infraredImage_H);
		cvReleaseImage(&lightImage_H);

		//H = getH(colorImage,frame_light);
		//PeiZhun_flag = 2;
	}
	IplImage* lightImage_1 = PeiZhun(lightImage,firstType,secondType,thirdType);

	//cvSaveImage("lightImage.jpg",lightImage_1);

	//��Ҫ���͵ĺ���ԭͼ
	transColorImage = cvCloneImage(colorImage);

	int grayValue = 0;
	//��������쳣��ÿһ���쳣�����ʶһ����ɫ���������ͳ���쳣����
	for (int i=detectStartRow;i<=detectEndRow;i++)
	{
		for (int j=detectStatCol;j<=detectEndCol;j++)
		{
			grayValue = getGray(grayImage,i,j);
			if (grayValue >= firstValue && grayValue <= secondValue)
			{
				changeColor(colorImage,i,j,255,0,0); //��ɫ

			}else if (grayValue >= secondValue && grayValue <= thirdValue)
			{
				changeColor(colorImage,i,j,0,255,0); //��ɫ

			}else if (grayValue > thirdValue){

				changeColor(colorImage,i,j,0,0,255); //��ɫ

			}else{
				changeColor(colorImage,i,j,0,0,0);
			}
		}
	}

	//cvSaveImage("colorImage_3.jpg",colorImage);

	
	//���ü��������
	for (int i=detectStartRow;i<=detectEndRow;i++)
	{
		for (int j=detectStatCol;j<=detectEndCol;j++)
		{
			flag[i][j] = 0;
		}
	}

	//���Դ���
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

	//��ȡʱ�䣬�������ݿ�
	//int year = tm.GetYear(); ///��
	//int month = tm.GetMonth(); ///��  
	//int day = tm.GetDay(); ///��
	//int hour = tm.GetHour(); //ʱ
	//int minute = tm.GetMinute(); //��
	//int second = tm.GetSecond(); //��

	//CString detectTime = intToCString(year) + "-" +intToCString(month)+ "-" +intToCString(day) 
	//	+ "-" +intToCString(hour)+ "-" +intToCString(minute)+ "-" +intToCString(second);


	
	//ͳ���쳣����
	for (int i=detectStartRow;i<=detectEndRow;i++)
	{
		for (int j=detectStatCol;j<=detectEndCol;j++)
		{
			if (flag[i][j] == 0)
			{
				rgb = getRGB(colorImage,i,j);
				if (rgb[0] == 255)
				{
					maxGrayValue = getGray(grayImage,i,j); //�洢����������Ҷ�ֵ
					total_grayValue = getGray(grayImage,i,j); //�洢��������ܻҶ�ֵ
					rgb.clear();
					flag[i][j] = 1;
					findArea(colorImage,grayImage,0,firstAbnormalCount,i,j);
					//�����Ͻ���ʾfirstAbnormalCount
					if (AbnormalDimension[0]>20)
					{
						firstAbnormalCount ++;
						areaMaxTemp = maxGrayValue; //��ȡ��������¶�,�����Ҷ�ֵ���棬��������
						areaAvgTemp = total_grayValue/AbnormalDimension[0]; //��ȡ����ƽ���¶�
						referTemp = 10;
						//referDiffTemp = (T1 - T2)/(T1 - T0) T1:���� T2:���� T0:����
						referDiffTemp = (areaMaxTemp - referTemp)/(areaMaxTemp - enviTemp);
						if (tempUpSpeed >= -100)
						{
							tempUpSpeed = areaMaxTemp - last_firstTemp;
						}else{
							tempUpSpeed = 0;
						}
						
						//insertIntoList(i,j,AbnormalDimension[0],areaMinTemp,areaMaxTemp,areaAvgTemp,m_abnormalDeviceTemp,referDiffTemp,0);
						insertIntoList(stringToCString(deviceType), center_x,center_y,AbnormalDimension[0],areaMaxTemp,areaAvgTemp,Save2Float(referTemp),Save2Float(enviTemp), referDiffTemp,tempUpSpeed,0, detectDate + _T("-") + detectTime);
						//��������
						//�����ֱ�Ϊ������ԭͼ �����쳣���ͼ �ɼ�����׼ͼ ���λ�� ��������x ��������y ������� ����¶� ����¶� ƽ���¶� �ο��¶� ����²� �����ٶ� ȱ�ݵȼ�
						socketMat.transmit(IplImageToMat(transColorImage),IplImageToMat(colorImage_detect),IplImageToMat(lightImage_1),deviceType,center_x,center_y,AbnormalDimension[0],areaMaxTemp,areaAvgTemp,referTemp,enviTemp,referDiffTemp,tempUpSpeed,0,CStringToString(detectDate), CStringToString(detectTime)); 
						//AfxMessageBox(intToCString(i)+_T(" ")+intToCString(j));
						//AfxMessageBox(intToCString(i)+_T(" ")+intToCString(j));
						//AfxMessageBox(_T("AbnormalDimension[0]")+intToCString(AbnormalDimension[0])); 
					}
					
					AbnormalDimension[0] = 0;
					total_grayValue = 0;
					
					
				}else if (rgb[1] == 255)
				{
					maxGrayValue = getGray(grayImage,i,j); //�洢����������Ҷ�ֵ
					total_grayValue = getGray(grayImage,i,j); //�洢��������ܻҶ�ֵ
					rgb.clear();
					flag[i][j] = 1;
					findArea(colorImage,grayImage,0,firstAbnormalCount,i,j);
					//�����Ͻ���ʾfirstAbnormalCount
					if (AbnormalDimension[0]>20)
					{
						firstAbnormalCount ++;
						areaMaxTemp = maxGrayValue; //��ȡ��������¶�,�����Ҷ�ֵ���棬��������
						areaAvgTemp = total_grayValue/AbnormalDimension[0]; //��ȡ����ƽ���¶�
						referTemp = 10;
						//referDiffTemp = (T1 - T2)/(T1 - T0) T1:���� T2:���� T0:����
						referDiffTemp = (areaMaxTemp - referTemp)/(areaMaxTemp - enviTemp);
						if (tempUpSpeed >= -100)
						{
							tempUpSpeed = areaMaxTemp - last_firstTemp;
						}else{
							tempUpSpeed = 0;
						}

						//insertIntoList(i,j,AbnormalDimension[0],areaMinTemp,areaMaxTemp,areaAvgTemp,m_abnormalDeviceTemp,referDiffTemp,0);
						insertIntoList(stringToCString(deviceType), center_x,center_y,AbnormalDimension[0],areaMaxTemp,areaAvgTemp,Save2Float(referTemp),Save2Float(enviTemp), referDiffTemp,tempUpSpeed,1,detectTime);
						//��������
						//�����ֱ�Ϊ������ԭͼ �����쳣���ͼ �ɼ�����׼ͼ ���λ�� ��������x ��������y ������� ����¶� ����¶� ƽ���¶� �ο��¶� ����²� �����ٶ� ȱ�ݵȼ�
						socketMat.transmit(IplImageToMat(transColorImage),IplImageToMat(colorImage_detect),IplImageToMat(lightImage_1),deviceType,center_x,center_y,AbnormalDimension[1],areaMaxTemp,areaAvgTemp,referTemp,enviTemp,referDiffTemp,tempUpSpeed,1,CStringToString(detectDate), CStringToString(detectTime)); 
						//socketMat.transmit(IplImageToMat(transColorImage),IplImageToMat(colorImage_2),IplImageToMat(lightImage_1),1,"breaker",i,j,AbnormalDimension[0],areaMinTemp,areaMaxTemp,areaAvgTemp,25.0,referDiffTemp,0.2,1); 
						//AfxMessageBox(intToCString(i)+_T(" ")+intToCString(j));
						//AfxMessageBox(_T("AbnormalDimension[1]")+intToCString(AbnormalDimension[1]));
					}

					AbnormalDimension[1] = 0;
					total_grayValue = 0;
					
				}else if (rgb[2] == 255)
				{
					maxGrayValue = getGray(grayImage,i,j); //�洢����������Ҷ�ֵ
					total_grayValue = getGray(grayImage,i,j); //�洢��������ܻҶ�ֵ
					rgb.clear();
					flag[i][j] = 1;
					findArea(colorImage,grayImage,0,firstAbnormalCount,i,j);
					//�����Ͻ���ʾfirstAbnormalCount
					if (AbnormalDimension[0]>20)
					{
						firstAbnormalCount ++;
						areaMaxTemp = maxGrayValue; //��ȡ��������¶�,�����Ҷ�ֵ���棬��������
						areaAvgTemp = total_grayValue/AbnormalDimension[0]; //��ȡ����ƽ���¶�
						referTemp = 10;
						//referDiffTemp = (T1 - T2)/(T1 - T0) T1:���� T2:���� T0:����
						referDiffTemp = (areaMaxTemp - referTemp)/(areaMaxTemp - enviTemp);
						if (tempUpSpeed >= -100)
						{
							tempUpSpeed = areaMaxTemp - last_firstTemp;
						}else{
							tempUpSpeed = 0;
						}

						//insertIntoList(i,j,AbnormalDimension[0],areaMinTemp,areaMaxTemp,areaAvgTemp,m_abnormalDeviceTemp,referDiffTemp,0);
						insertIntoList(stringToCString(deviceType), center_x,center_y,AbnormalDimension[0],areaMaxTemp,areaAvgTemp,Save2Float(referTemp),Save2Float(enviTemp), referDiffTemp,tempUpSpeed,2,detectTime);
						//��������
						//�����ֱ�Ϊ������ԭͼ �����쳣���ͼ �ɼ�����׼ͼ  ��������x ��������y �쳣��� ����¶�  ƽ���¶� �ο��¶�  �����¶� ����²� �����ٶ� �쳣���� ���ʱ��
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

//���쳣��Ϣ���뵽�б���
void CNewSubstationDetectionDlg::insertIntoList(CString deviceType,int x,int y,int AbnormalDimension,float areaMaxTemp,float areaAvgTemp,CString referTemp,CString enviTemp, float referDiffTemp,float speed,int abnormalLevel,CString time){

	int nIndex = m_list.GetItemCount() + 1; //����������ţ���1��ʼ
	CString t;
	t.Format(_T("%d"), nIndex);
	//LV_ITEM lvItem;
	//lvItem.mask = LVIF_TEXT;
	//lvItem.iItem = nIndex;     //����
	//lvItem.iSubItem = 0;
	//lvItem.pszText = (LPWSTR)(LPCWSTR)t;
	//m_video_list.InsertItem(&lvItem);
	////����������

	CString positon = _T("(") + intToCString(x) + _T(",") + intToCString(y) + _T(")");
	CString abnormalType;

	if (abnormalLevel == 0)
	{
		abnormalType = _T("һ��ȱ��");
	}else if (abnormalLevel == 1)
	{
		abnormalType = _T("����ȱ��");
	}else if (abnormalLevel == 2)
	{
		abnormalType = _T("��ͬ����ȱ��");
	}



	//�����б�����
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

	//��ȡ�쳣�������
	

void CNewSubstationDetectionDlg::findArea(IplImage* colorImage,IplImage* grayImage,int index,int AbnormalCount,int x,int y){

	vector<int> rgb; //��ʱ�洢rgbֵ
	int temp_grayValue = 0;  //��ʱ�洢�Ҷ�ֵ

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

					if (temp_grayValue > maxGrayValue) //��ȡ��������¶�ֵ
					{
						maxGrayValue = temp_grayValue;
						center_x = i;
						center_y = j;
					}

					total_grayValue += temp_grayValue; //��ȡ�ܻҶ�ֵ

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

//����Ƶ֡��ʾ��pictureControl
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

//��ȡ�任����
Mat CNewSubstationDetectionDlg::getH(IplImage* &frame_infrare,IplImage* &frame_light){

	initModule_nonfree();//��ʼ��ģ�飬ʹ��SIFT��SURFʱ�õ�      
	Ptr<FeatureDetector> detector = FeatureDetector::create( "SURF" );//����SIFT������������ɸĳ�SURF/ORB     
	Ptr<DescriptorExtractor> descriptor_extractor = DescriptorExtractor::create( "SURF" );//���������������������ɸĳ�SURF/ORB     
	Ptr<DescriptorMatcher> descriptor_matcher = DescriptorMatcher::create( "BruteForce" );//��������ƥ����       
	if( detector.empty() || descriptor_extractor.empty() )    
		AfxMessageBox(_T("��׼�����У����� detector ʧ��!")); 
		//cout<<"fail to create detector!";      

	//����ͼ��     
	//DWORD start_time=GetTickCount();

	Mat img_infrare = IplImageToMat(frame_infrare);
	Mat img_light = IplImageToMat(frame_light);
 
	//�ҶȻ�
	cvtColor(img_infrare,img_infrare,CV_BGR2GRAY);  
	cvtColor(img_light,img_light,CV_BGR2GRAY); 

	//����

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

	//��������       
	double t = getTickCount();//��ǰ�δ���       
	vector<KeyPoint> m_LeftKey,m_RightKey;      
	detector->detect( img_infrare_edge, m_LeftKey );//���img1�е�SIFT�����㣬�洢��m_LeftKey��       
	detector->detect( img_light_edge, m_RightKey );      
/*	cout<<"ͼ��1���������:"<<m_LeftKey.size()<<endl;      
	cout<<"ͼ��2���������:"<<m_RightKey.size()<<endl;   */   

	//����������������������Ӿ��󣬼�������������       
	Mat descriptors1,descriptors2;      
	descriptor_extractor->compute( img_infrare_edge, m_LeftKey, descriptors1 );      
	descriptor_extractor->compute( img_light_edge, m_RightKey, descriptors2 );      
	t = ((double)getTickCount() - t)/getTickFrequency();      
	/*	cout<<"SIFT�㷨��ʱ��"<<t<<"��"<<endl;      

	std::cout<<"ͼ��1�������������С��"<< descriptors1.size      
	<<"����������������"<<descriptors1.rows<<"��ά����"<<descriptors1.cols<<endl;      
	cout<<"ͼ��2�������������С��"<<descriptors2.size      
	<<"����������������"<<descriptors2.rows<<"��ά����"<<descriptors2.cols<<endl;  */    

	//����������       
	/*	Mat img_m_LeftKey,img_m_RightKey;      
	drawKeypoints(img1,m_LeftKey,img_m_LeftKey,Scalar::all(-1),0);      
	drawKeypoints(img2,m_RightKey,img_m_RightKey,Scalar::all(-1),0);      
	imshow("Src1",img_m_LeftKey);       
	imshow("Src2",img_m_RightKey);  */     

	//����ƥ��       
	vector<DMatch> matches;//ƥ����       
	descriptor_matcher->match( descriptors1, descriptors2, matches );//ƥ������ͼ�����������       
	//cout<<"Match������"<<matches.size()<<endl;      

	//����ƥ�����о����������Сֵ       
	//������ָ���������������ŷʽ���룬�������������Ĳ��죬ֵԽС��������������Խ�ӽ�       
	double max_dist = 0;      
	double min_dist = 100;      
	for(int i=0; i<matches.size(); i++)      
	{      
		double dist = matches[i].distance;      
		if(dist < min_dist) min_dist = dist;      
		if(dist > max_dist) max_dist = dist;      
	}      
	/*	cout<<"�����룺"<<max_dist<<endl;      
	cout<<"��С���룺"<<min_dist<<endl; */     

	//ɸѡ���Ϻõ�ƥ���       
	vector<DMatch> goodMatches;      
	for(int i=0; i<matches.size(); i++)      
	{      
		if(matches[i].distance < 0.2 * max_dist)      
		{      
			goodMatches.push_back(matches[i]);      
		}      
	}      
	//cout<<"goodMatch������"<<goodMatches.size()<<endl;      

	//����ƥ����       
	//Mat img_matches;      
	////��ɫ���ӵ���ƥ���������ԣ���ɫ��δƥ���������       
	//drawMatches(img1,m_LeftKey,img2,m_RightKey,goodMatches,img_matches,      
	//	Scalar::all(-1)/*CV_RGB(255,0,0)*/,CV_RGB(0,255,0),Mat(),2);      

	//imshow("MatchSIFT",img_matches);      
	//IplImage result=img_matches;    

	//waitKey(10);    


	//RANSACƥ�����     
	vector<DMatch> m_Matches=goodMatches;    
	// ����ռ�     
	int ptCount = (int)m_Matches.size();    

	if ( ptCount<30)  
	{         
		cout<<"û���ҵ��㹻��ƥ���"<<endl;  
		waitKey(0);  
		return H;  
	}  
	Mat p1(ptCount, 2, CV_32F);    
	Mat p2(ptCount, 2, CV_32F);    

	// ��Keypointת��ΪMat     
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

	// ��RANSAC��������F     
	Mat m_Fundamental;    
	vector<uchar> m_RANSACStatus;       // ����������ڴ洢RANSAC��ÿ�����״̬     
	findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC);    

	// ����Ұ�����     

	int OutlinerCount = 0;    
	for (int i=0; i<ptCount; i++)    
	{    
		if (m_RANSACStatus[i] == 0)    // ״̬Ϊ0��ʾҰ��     
		{    
			OutlinerCount++;    
		}    
	}    
	int InlinerCount = ptCount - OutlinerCount;   // �����ڵ�     
	//cout<<"�ڵ���Ϊ��"<<InlinerCount<<endl;    


	// �������������ڱ����ڵ��ƥ���ϵ     
	vector<Point2f> m_LeftInlier;    
	vector<Point2f> m_RightInlier;    
	vector<DMatch> m_InlierMatches;    

	m_InlierMatches.resize(InlinerCount);    
	m_LeftInlier.resize(InlinerCount);    
	m_RightInlier.resize(InlinerCount);    
	InlinerCount=0;    
	float inlier_minRx=img_infrare_edge.cols;        //���ڴ洢�ڵ�����ͼ��С�����꣬�Ա�����ں�     

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

			if(m_RightInlier[InlinerCount].x<inlier_minRx) inlier_minRx=m_RightInlier[InlinerCount].x;   //�洢�ڵ�����ͼ��С������     

			InlinerCount++;    
		}    
	}    

	// ���ڵ�ת��ΪdrawMatches����ʹ�õĸ�ʽ     
	vector<KeyPoint> key1(InlinerCount);    
	vector<KeyPoint> key2(InlinerCount);    
	KeyPoint::convert(m_LeftInlier, key1);    
	KeyPoint::convert(m_RightInlier, key2);    

	// ��ʾ����F������ڵ�ƥ��     
	//Mat OutImage;    
	//drawMatches(img1, key1, img2, key2, m_InlierMatches, OutImage);    
	//cvNamedWindow( "Match features", 1);    
	//cvShowImage("Match features", &IplImage(OutImage));    
	//waitKey(10);    

	//cvDestroyAllWindows();    

	//����H���Դ洢RANSAC�õ��ĵ�Ӧ����     
	Mat H = findHomography( m_LeftInlier, m_RightInlier, RANSAC );   

	//�ͷ��ڴ�
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

	return H; //���ر任����

}

//��ȡ��׼���ͼ��
IplImage* CNewSubstationDetectionDlg::PeiZhun(IplImage* frame_light,vector<Point2f> firstType,vector<Point2f> secondType, vector<Point2f> thirdType){

	if (!firstType.empty())
	{

		if (H.empty())
		{
			AfxMessageBox(_T("H����Ϊ��"));
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


