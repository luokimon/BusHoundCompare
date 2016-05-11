
// BusHoundCompareDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#define BLOCK_UNIT_SIZE			(0x400)			// 块单元个数
#define FAT_MAX_UINT_SIZE		(0x400)			// FAT表预计单元个数     

UINT  AFX_CDECL BusHoundDecodeThread(LPVOID lpParam);
UINT  AFX_CDECL BusHoundCompareThread(LPVOID lpParam);

// CBusHoundCompareDlg 对话框
class CBusHoundCompareDlg : public CDialogEx
{
// 构造
public:
	CBusHoundCompareDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BUSHOUNDCOMPARE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CEdit m_editDataPath;
	afx_msg void OnBnClickedBtnSelectpath();
	afx_msg void OnBnClickedBtnCompare();

public:
	CWinThread	*m_lpDecodeThread;
	CWinThread	*m_lpCompareThread;

	UINT m_nDataStartPoint;
	UINT m_nDataEndPoint;

public:
	DWORD	DecodeThread();
	DWORD   CompareThread();

private:
	CString m_strDataPath;
	HANDLE  m_hSrcFileMap;
	CMutex m_Mutex;
	UINT m_err;
	DWORD m_dwBlkSize;
	DWORD m_Granularity;
	
	BOOL m_bRun;
	BOOL m_bEnd;
	__int64 m_nSrcFileSize;
private:
	CString GetCurrentPath();
	BOOL CompareData();

	BOOL SetErrCode(UINT uErr);
	UINT GetErrCode();
	BOOL GetRunFlag();
	BOOL SetRunFlag(BOOL  runFlag);
	BOOL GetEndFlag();
	BOOL SetEndFlag(BOOL  endFlag);

	HANDLE CreateUserFileMapping(CString strPath, __int64 &fileSize);

	DWORD GetMappingBlkSize(__int64 fileSize);
	DWORD CreateDecodeThread();
	void DestroyDecodeThread();
	BOOL MappingVirtualMemory();
	DWORD CreateCompareThread();
	void DestroyCompareThread();
	VOID CreateWorkThread();
	DWORD GetAllocationGranularity();
	void DisplayWindowInfo();
	void InitialParam();

	void GetDataOffset(__int64 &fileOffset);
	BOOL AddDisplay(LPCTSTR str);



public:
	CListBox m_listShowStatus;
	CEdit m_editGranularity;
	CEdit m_editBlkUnitSize;
	CEdit m_editFATUnitSize;
};
