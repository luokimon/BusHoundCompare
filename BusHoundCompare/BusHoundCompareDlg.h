
// BusHoundCompareDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include <queue>
#include <vector>
#include "afxcmn.h"
#include <map>
using namespace std;

#define DATA_FILE_EX			(1)				// 新解码流程

#define SECTOR                  (0x200)         // 扇区大小
#define MAX_TRANSFER_LEN        (0x10000)       // 单次最大传输长度(64K)
#define BLOCK_UNIT_SIZE			(0x400)			// 块单元个数
#define FAT_MAX_UINT_SIZE		(0x400)			// FAT表预计单元个数

#define CBW_MAX_LEN             (0x1E)			// Command Block Wrapper 长度
#define SYSTEM_AREA_SIZE        (0x2000000)		// 32M 供系统区使用
#define DATA_AREA_SIZE			(0x1000000)		// 16M 供数据区使用
#define DATA_AREA_MAP_SIZE      (DATA_AREA_SIZE/SECTOR)		// 数据区映射文件长度 
#define MAX_TRANS_SEC_NUM       (0x80)			// 单次最大传输扇区个数

#define MAX_DMA_NUM				(0x10)			// DMA个数
#define BYTE_STRING_LEN         (2)             // 每个字符表示 单字节 所占长度

#define CMD_PHASE_OFS_LEN		(0x12)			// 相位差所占长度

#define CMD_BLK_CMDIDX			(0)				// 命令块中命令位置
#define CMD_BLK_ADDRIDX			(2)				// 命令块中地址开始位置
#define CMD_BLK_LENIDX			(7)				// 命令块中传输长度开始位置


#define SMALL_AREA_SIZE         (0x40000)                   // 256K 供小扇区使用

UINT  AFX_CDECL BusHoundDecodeThread(LPVOID lpParam);

struct COMMAND_INFO
{
	DWORD dmaIdx;
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
	afx_msg void OnClose();
	afx_msg void OnDropFiles(HDROP hDropInfo);
//	afx_msg int OnCreate(LPCREATESTRUCT);


public:
	CWinThread	*m_lpDecodeThread;

	UINT m_nCmdPhaseOfsPoint;		// 命令状态偏移位置
	UINT m_nPhaseStartPoint;		// 状态开始位置
	UINT m_nDataStartPoint;			// 数据开始位置
	UINT m_nDataLen;				// 数据在原始文件中所占长度

public:
    DWORD	DecodeThread();

private:
	// 源文件相关参数
	CString m_strSrcPath;
	HANDLE  m_hSrcFileMap;
	DWORD m_dwSrcBlkSize;
	LPBYTE m_lpSrcMapAddress;

	// 目标文件相关参数
    CString m_strDstPath;
    HANDLE  m_hDstFileMap;
    DWORD m_dwDstBlkSize;
    LPBYTE m_lpDstMapAddress;
	DWORD m_BlkIdx;


	CMutex m_Mutex;
	UINT m_err;
	DWORD m_Granularity;
	
	BOOL m_bRun;		// 线程启动标记
	BOOL m_bEnd;		// 线程结束标记
	BOOL m_bStop;		// 线程停止标记
	BOOL m_bCompareStart;	

	__int64 m_nSrcFileSize;	
    __int64 m_nDstFileSize;
	CString m_strResidualData;


	BYTE m_ucCmdData[CBW_MAX_LEN];
	BYTE *m_lpucSecotrData[MAX_DMA_NUM];
	DWORD m_DMAMask;								// DMA 掩码
	BYTE *m_lpucSysArea;							// 32M 供系统区使用
	BYTE *m_lpucDataArea;							// 16M 供数据区使用
	UINT m_DataFlag;

    BYTE *m_lpSmallSecArea;                         // 供小扇区使用

	UINT m_PhaseType;							// 0:其他状态/1:命令状态/2:数据状态
	UINT m_DmaIdx;								// DMA 索引位置
	UINT m_DataIdx;								// 数据索引位置
	UINT m_CBWIdx;								// 命令索引位置

	BOOL m_bStartWriteFlag;						// 是否已写入标记

    vector<WORD> *m_lpSmallAreaMap;             // 极限映射范围 32M
    vector<WORD> *m_lpDstFileMap;               // 

	vector<DWORD> *m_DataAreaMap;
	queue<COMMAND_INFO> m_CommandInfo;


    WORD    m_SmallAreaIdx;
    WORD    m_DstFileIdx;
    map<DWORD, WORD>    m_SmallAreaMap;
    map<DWORD, WORD>    m_DstFileMap;
    map<DWORD, WORD>    m_DstSecMap;

private:
	CString GetCurrentPath();

	BOOL SetErrCode(UINT uErr);
	UINT GetErrCode();
	BOOL GetRunFlag();
	BOOL SetRunFlag(BOOL  runFlag);
	BOOL GetEndFlag();
	BOOL SetEndFlag(BOOL  endFlag);
	BOOL GetStopFlag();
	BOOL SetStopFlag(BOOL  stopFlag);
	BOOL GetCompareStartFlag();
	BOOL SetCompareStartFlag(BOOL  startFlag);
	UINT GetDataFlag();
	BOOL SetDataFlag(UINT  dataFlag);
	BOOL GetDMAIdxMask(DWORD dmaIdx);
	BOOL SetDMAIdxMask(DWORD dmaIdx, BOOL maskFlag);

	HANDLE CreateUserFileMapping(CString strPath, __int64 &fileSize);

	DWORD GetMappingBlkSize(__int64 fileSize);
	DWORD CreateDecodeThread();
	void DestroyDecodeThread();
	BOOL MappingVirtualMemory();
	VOID CreateWorkThread();
	DWORD GetAllocationGranularity();
	void InitialParam();

	BOOL	CreateMapAddr(HANDLE hFileMap, __int64 &fileOffset, DWORD blkSize, LPBYTE &mapAddr);
	BOOL	DistroyMapAddr(LPBYTE &mapAddr);
	BOOL	GetDataOffset(__int64 fileOffset, UINT blkOffset);
	void	GetDataStartPoint(CString &strLine);
	void	CheckDataStartPoint(CString &strLine);
    BOOL    GetDstFileSize(CString &strLine);
	BOOL	AddDisplay(LPCTSTR str);

	CString  FindLine(LPBYTE  pByte, UINT & uiIndex, UINT uiLen);
	BYTE  StringToByte(CString &strChar);
	DWORD ReverseDWORD(DWORD InData);
	WORD ReverseWORD(WORD InData);

	BOOL CommandDecodeFlow(CString &strLine);
	BOOL DataDecodeFlow(CString &strLine);
	BOOL ExistedWriteFlag();
    BOOL PseudoWriteData(DWORD addr, WORD secCnt, DWORD dmaIdx);
    BOOL PseudoWriteData_Ex(DWORD addr, WORD secCnt, DWORD dmaIdx, TCHAR *cmdPhaseOfs);
    BOOL PseudoReadData(DWORD addr, WORD secCnt, DWORD dmaIdx, TCHAR *cmdPhaseOfs);
    BOOL PseudoReadData_Ex(DWORD addr, WORD secCnt, DWORD dmaIdx, TCHAR *cmdPhaseOfs);
	void ShowErrInfo(DWORD addr, TCHAR *cmdPhaseOfs);
	void ShowMissInfo(DWORD addr, TCHAR *cmdPhaseOfs);
    void ShowOverflowInfo(DWORD addr, TCHAR *cmdPhaseOfs);

	BOOL    GetFileAttribute();
	BOOL    CreateDstFile();
	BOOL	AdjustFileMap(WORD idx, __int64 &qwFileOffset);
    BOOL    AddNewMapData(DWORD addr, WORD secCnt, DWORD dmaIdx);

public:
	CListBox m_listShowStatus;	
	CProgressCtrl m_progDecode;
};
