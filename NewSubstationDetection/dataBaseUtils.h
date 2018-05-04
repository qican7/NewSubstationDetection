#ifndef DATABASEUTILS_H
#define DATABASEUTILS_H

#include<stdio.h>  
#include <iostream> 
#include "mysql.h"
#include "afxdialogex.h"
#include "stdafx.h"

#include<vector>

using namespace std;

extern void DataBaseConnect();   //连接数据库
extern MYSQL_RES* selectData(CString m_sql);  //查询数据

extern CString getCameraIdFromCamerName(CString cameraName); //通过cameraName获取cameraId
extern vector<CString> getEnviParamFromCameraName(char* cameraName); //通过cameraName获取enviParam
extern CString getPositionFromCameraName(char* cameraName); //通过cameraName获取position

extern vector<CString> getCameraFromCameraName(char* cameraName);//通过cameraName获取camera

extern void saveDataToDataBase(CString cameraId,char* failureType,float MaxTemp,  float referTemp, float diffTemp,CString path,int year, int month,int day,int hour, int minu,int sec); //保存数据到abnormalPic

extern vector<CString>  getAbnormalDataFromTime(int Year,int Month,int Day,int Hour,int Min,int Sec); //根据时间戳查询异常信息

extern vector<vector<CString>> getAbnormalInfoFromTime(int year,int month,int day); //根据时间获取异常信息，查询界面获取信息

extern vector<CString> getCameraFromCameraId(CString cameraId); //通过cameraId获取camera

extern CString getFailureTypeFromFailureLevel(CString failureLevel); //通过failureLevel获取FailureType

extern CString  getFailureLevelFromFailureType(CString failureType); //通过FailureType获取failureLevel

extern CString  getCameraIdFromPositionAndDeviceType(CString position,CString deviceType); //通过Position DeviceType获取CameraId

extern vector<vector<CString>> searchDlg_searchData(CString cameraId,CString failureLevel,CString year,CString month,CString day); //查询符合条件的数据

extern vector<CString>  getDeviceFromDeviceType(CString deviceType); //根据deviceType获取device

#endif