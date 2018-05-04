
// NewSubstationDetectionDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "cv.h"
#include "highgui.h"
#include "CvvImage.h"
#include "afxcmn.h"

#include "opencv2/nonfree/nonfree.hpp"  


//���ݿ����
#include "mysql.h"

using namespace std; 
using namespace cv; 


// CNewSubstationDetectionDlg �Ի���
class CNewSubstationDetectionDlg : public CDialogEx
{
// ����
public:
	CNewSubstationDetectionDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_NEWSUBSTATIONDETECTION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_frare;
	CStatic m_light;
	CListCtrl m_list;
	virtual void findArea(IplImage* colorImage,IplImage* grayImage,int index,int AbnormalCount,int x,int y);
	virtual void insertIntoList(CString deviceType,int x,int y,int AbnormalDimension,float areaMaxTemp,float areaAvgTemp,CString referTemp,CString enviTemp, float referDiffTemp,float speed,int abnormalLevel,CString time);
	virtual void DrawPicToHDC(IplImage *img, UINT ID);
	virtual void detectAbnormalArea(IplImage* grayImage,IplImage* colorImage,IplImage* colorImage_2,IplImage* lightImage);
	virtual Mat  getH(IplImage* &colorImage,IplImage* &frame_light);
	virtual IplImage* PeiZhun(IplImage* frame_light,vector<Point2f> firstType,vector<Point2f> secondType,vector<Point2f> thirdType);

	CStatic m_show;
};
