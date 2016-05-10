
// BusHoundCompareDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#define BLOCK_UNIT_SIZE			(0x400)			// 块单元个数
#define FAT_MAX_UINT_SIZE		(0x400)			// FAT表预计单元个数        
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

private:
	CString m_strDataPath;
	HANDLE  m_hSrcFileMap;
	CMutex m_Mutex;
	UINT m_err;
	DWORD m_dwBlkSize;

	__int64 m_nSrcFileSize;
private:
	CString GetCurrentPath();
	BOOL CompareData();
	VOID SetErrCode(UINT uErr);
	UINT GetErrCode();
	HANDLE CreateUserFileMapping(CString strPath, __int64 &fileSize);

	BOOL MappingDataFile();
	DWORD GetMappingBlkSize(__int64 srcFileSize);
	VOID CreateDecodeThread();
	BOOL MappingVirtualMemory();
	VOID CreateCompareThread();

};
