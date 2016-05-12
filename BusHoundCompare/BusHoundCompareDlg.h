
// BusHoundCompareDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include <queue>
#include <map>
using namespace std;

#define BLOCK_UNIT_SIZE			(0x400)			// 块单元个数
#define FAT_MAX_UINT_SIZE		(0x400)			// FAT表预计单元个数

#define CBW_MAX_LEN             (0x1E)			// Command Block Wrapper 长度
#define SYSTEM_AREA_SIZE        (0x2000000)		// 32M 供系统区使用
#define DATA_AREA_SIZE			(0x1000000)		// 16M 供数据区使用
#define MAX_TRANS_SEC_NUM       (0x80)			// 单次最大传输扇区个数
#define SECTOR                  (0x200)         // 扇区大小
#define MAX_DMA_NUM				(0x10)			// DMA个数
#define BYTE_STRING_LEN         (2)             // 每个字符表示 单字节 所占长度

#define CMD_PHASE_OFS_LEN		(0x12)			// 相位差所占长度

UINT  AFX_CDECL BusHoundDecodeThread(LPVOID lpParam);
UINT  AFX_CDECL BusHoundCompareThread(LPVOID lpParam);

struct COMMAND_INFO
{
	UINT dmaIdx;
	WORD sectorCnt;
	DWORD addr;
	BOOL direction;		// 0:IN/1:OUT
	TCHAR cmdPhaseOfs[CMD_PHASE_OFS_LEN+1];
};

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

	UINT m_nCmdPhaseOfsPoint;
	UINT m_nPhaseStartPoint;
	UINT m_nDataStartPoint;
	UINT m_nDataLen;

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
	BOOL m_bCompareStart;	

	__int64 m_nSrcFileSize;	
	CString m_strResidualData;

	LPBYTE m_lpSrcMapAddress;

	BYTE m_ucCmdData[CBW_MAX_LEN];
	BYTE *m_lpucSecotrData[MAX_DMA_NUM];
	BYTE *m_lpucSysArea;							// 32M 供系统区使用
	BYTE *m_lpucDataArea;							// 16M 供数据区使用
	UINT m_DataFlag;

	map<DWORD, WORD> m_DataAreaMap;
	queue<COMMAND_INFO> m_CommandInfo;
private:
	CString GetCurrentPath();

	BOOL SetErrCode(UINT uErr);
	UINT GetErrCode();
	BOOL GetRunFlag();
	BOOL SetRunFlag(BOOL  runFlag);
	BOOL GetEndFlag();
	BOOL SetEndFlag(BOOL  endFlag);
	BOOL GetCompareStartFlag();
	BOOL SetCompareStartFlag(BOOL  startFlag);
	UINT GetDataFlag();
	BOOL SetDataFlag(UINT  dataFlag);

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

	BOOL CreateMapAddr(HANDLE hFileMap, __int64 &fileOffset, DWORD blkSize, LPBYTE &mapAddr);
	BOOL DistroyMapAddr(LPBYTE &mapAddr);
	BOOL GetDataOffset(__int64 &fileOffset, UINT &blkOffset);
	BOOL AddDisplay(LPCTSTR str);

	CString  FindLine(LPBYTE  pByte, UINT & uiIndex, UINT uiLen);
	BYTE  StringToByte(CString strChar);
	DWORD ReverseDWORD(DWORD InData);
	WORD ReverseWORD(WORD InData);



public:
	CListBox m_listShowStatus;
	CEdit m_editGranularity;
	CEdit m_editBlkUnitSize;
	CEdit m_editFATUnitSize;
	afx_msg void OnClose();
};
