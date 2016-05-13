
// BusHoundCompareDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BusHoundCompare.h"
#include "BusHoundCompareDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBusHoundCompareDlg 对话框

UINT  AFX_CDECL BusHoundDecodeThread(LPVOID lpParam)
{
	CBusHoundCompareDlg *  lpDlg = (CBusHoundCompareDlg *)lpParam;

	if (lpDlg)
	{
		return lpDlg->DecodeThread();
	}

	return 0;
}

UINT  AFX_CDECL BusHoundCompareThread(LPVOID lpParam)
{
	CBusHoundCompareDlg *  lpDlg = (CBusHoundCompareDlg *)lpParam;

	if (lpDlg)
	{
		return lpDlg->CompareThread();
	}

	return 0;
}

CBusHoundCompareDlg::CBusHoundCompareDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BUSHOUNDCOMPARE_DIALOG, pParent),
	m_lpDecodeThread(NULL),
	m_lpCompareThread(NULL),
	m_lpSrcMapAddress(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBusHoundCompareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DATAPATH, m_editDataPath);
	DDX_Control(pDX, IDC_LIST_SHOWSTATUS, m_listShowStatus);
	DDX_Control(pDX, IDC_PROGRESS_DECODE, m_progDecode);
}

BEGIN_MESSAGE_MAP(CBusHoundCompareDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CBusHoundCompareDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CBusHoundCompareDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_SELECTPATH, &CBusHoundCompareDlg::OnBnClickedBtnSelectpath)
	ON_BN_CLICKED(IDC_BTN_COMPARE, &CBusHoundCompareDlg::OnBnClickedBtnCompare)
	ON_WM_CLOSE()
//	ON_WM_CREATE()
ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CBusHoundCompareDlg 消息处理程序

BOOL CBusHoundCompareDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码	
	InitialParam();
	m_progDecode.SetRange(0, 1000);
	m_progDecode.SetStep(1);

	for (int i = 0; i < MAX_DMA_NUM; i++)
	{
		m_lpucSecotrData[i] = new BYTE[MAX_TRANS_SEC_NUM*SECTOR];
	}

	m_lpucSysArea = new BYTE[SYSTEM_AREA_SIZE];
	m_lpucDataArea = new BYTE[DATA_AREA_SIZE];
	m_DataAreaMap = new vector<DWORD>;
	(*m_DataAreaMap).reserve(DATA_AREA_MAP_SIZE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 函数名称: InitialParam
// 函数功能: 初始化参数
void CBusHoundCompareDlg::InitialParam()
{
	m_bRun = FALSE;
	m_bEnd = TRUE;
	m_bStop = TRUE;
	m_bCompareStart = FALSE;
	m_bStartWriteFlag = FALSE;

	m_nDataStartPoint = 0;
	m_nDataLen = 0;
	m_strDataPath.Empty();
	m_Granularity = GetAllocationGranularity();
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CBusHoundCompareDlg::OnPaint()
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CBusHoundCompareDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString CBusHoundCompareDlg::GetCurrentPath()
{
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	TCHAR drive[MAX_PATH], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
	_tsplitpath_s(szPath, drive, dir, fname, ext);
	_tcscpy_s(szPath, MAX_PATH - 1, drive);
	_tcscat_s(szPath, MAX_PATH - 1, dir);
	CString strPath = szPath;
	return strPath;
}

void CBusHoundCompareDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
}


void CBusHoundCompareDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CBusHoundCompareDlg::OnBnClickedBtnSelectpath()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE,
		NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("All Files (*.*)|*.*||"),
		NULL);

	if (dlg.DoModal() == IDOK)
	{
		m_strDataPath = dlg.GetPathName();
		m_editDataPath.SetWindowText(m_strDataPath);
	}
}


void CBusHoundCompareDlg::OnBnClickedBtnCompare()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_strDataPath.IsEmpty())
		MessageBox(_T("请先选取BusHound数据文件!"), _T("警告"), MB_ICONWARNING | MB_OK);
	else
	{
		// 创建工作线程
		if(!GetRunFlag())
			CreateWorkThread();
	}
}

VOID CBusHoundCompareDlg::CreateWorkThread()
{
	SetRunFlag(TRUE);
	SetEndFlag(FALSE);
	SetStopFlag(FALSE);

	// 开启解析数据文件线程
	CreateDecodeThread();

	//开启比较数据线程
	//CreateCompareThread();
}

HANDLE CBusHoundCompareDlg::CreateUserFileMapping(CString strPath, __int64 &fileSize)
{
	// 创建文件对象
	HANDLE hFile = CreateFile(strPath,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		MessageBox(_T("创建文件对象失败!"));
		return INVALID_HANDLE_VALUE;
	}

	// 创建文件映射对象
	HANDLE hFileMap = CreateFileMapping(hFile,
		NULL,
		PAGE_READWRITE,
		0,
		0,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hFileMap)
	{
		MessageBox(_T("创建文件对象失败!"));
		return INVALID_HANDLE_VALUE;
	}

	// 得到文件尺寸
	DWORD dwFileSizeHigh;
	fileSize = GetFileSize(hFile, &dwFileSizeHigh);
	fileSize |= (((__int64)dwFileSizeHigh) << 32);

	// 关闭文件对象
	CloseHandle(hFile);

	return hFileMap;
}

// 函数名称: GetAllocationGranularity
// 函数功能: 获取系统分配粒度
DWORD CBusHoundCompareDlg::GetAllocationGranularity()
{
	// 得到系统分配粒度
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	return SysInfo.dwAllocationGranularity;
}

// 函数名称: GetMappingBlkSize
// 函数功能: 获取映射块大小
// 输入参数: fileSize(文件大小)
DWORD CBusHoundCompareDlg::GetMappingBlkSize(__int64 fileSize)
{
	return (fileSize < BLOCK_UNIT_SIZE * m_Granularity) ? ((DWORD)fileSize) : (BLOCK_UNIT_SIZE * m_Granularity);
}

// 函数名称: CreateDecodeThread
// 函数功能: 开启解析数据文件线程
DWORD CBusHoundCompareDlg::CreateDecodeThread()
{
	DestroyDecodeThread();

	m_lpDecodeThread = AfxBeginThread(BusHoundDecodeThread,				//AFX_THREADPROC pfnThreadProc,
		(LPVOID)this,						//LPVOID pParam,
		THREAD_PRIORITY_NORMAL,    		    //int nPriority = THREAD_PRIORITY_NORMAL,
		NULL,								//UINT nStackSize = 0,
		0,					                //DWORD dwCreateFlags = 0, 创建后直接启动线程
		NULL								//LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL 
	);

	return (DWORD)m_lpDecodeThread;
}

void CBusHoundCompareDlg::DestroyDecodeThread()
{
	if (m_lpDecodeThread)
	{
		Sleep(100);

		m_lpDecodeThread = NULL;
	}
}

// 函数名称: MappingVirtualMemory
// 函数功能: 映射虚拟内存
// 输入参数:
// 输出参数:
// 返回值  :
BOOL CBusHoundCompareDlg::MappingVirtualMemory()
{
	return FALSE;
}

// 函数名称: CreateCompareThread
// 函数功能: 开启比较数据线程
// 输入参数:
// 输出参数:
// 返回值  :
DWORD CBusHoundCompareDlg::CreateCompareThread()
{
	DestroyCompareThread();

	m_lpCompareThread = AfxBeginThread(BusHoundCompareThread,				//AFX_THREADPROC pfnThreadProc,
		(LPVOID)this,						//LPVOID pParam,
		THREAD_PRIORITY_NORMAL,    		    //int nPriority = THREAD_PRIORITY_NORMAL,
		NULL,								//UINT nStackSize = 0,
		0,					                //DWORD dwCreateFlags = 0, 创建后直接启动线程
		NULL								//LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL 
	);

	return (DWORD)m_lpCompareThread;
}

void CBusHoundCompareDlg::DestroyCompareThread()
{
	if (m_lpCompareThread)
	{
		Sleep(100);

		m_lpCompareThread = NULL;
	}
}

BOOL CBusHoundCompareDlg::SetErrCode(UINT uErr)
{
	if (m_Mutex.Lock())
	{
		m_err = uErr;
		m_Mutex.Unlock();
		return TRUE;
	}

	return FALSE;
}

UINT CBusHoundCompareDlg::GetErrCode()
{
	UINT uErr = 0;

	if (m_Mutex.Lock())
	{
		uErr = m_err;
		m_Mutex.Unlock();
	}

	return uErr;
}

BOOL CBusHoundCompareDlg::GetRunFlag()
{
	BOOL  bRet = FALSE;

	if (m_Mutex.Lock())
	{
		bRet = m_bRun;
		m_Mutex.Unlock();
	}

	return bRet;

}

BOOL CBusHoundCompareDlg::SetRunFlag(BOOL  runFlag)
{
	if (m_Mutex.Lock())
	{
		m_bRun = runFlag;
		m_Mutex.Unlock();
		return TRUE;
	}

	return FALSE;
}

BOOL CBusHoundCompareDlg::GetEndFlag()
{
	BOOL  bRet = FALSE;

	if (m_Mutex.Lock())
	{
		bRet = m_bEnd;
		m_Mutex.Unlock();
	}

	return bRet;
}

BOOL CBusHoundCompareDlg::SetEndFlag(BOOL  endFlag)
{
	if (m_Mutex.Lock())
	{
		m_bEnd = endFlag;
		m_Mutex.Unlock();
		return TRUE;
	}

	return FALSE;
}

BOOL CBusHoundCompareDlg::GetStopFlag()
{
	BOOL  bRet = FALSE;

	if (m_Mutex.Lock())
	{
		bRet = m_bStop;
		m_Mutex.Unlock();
	}

	return bRet;

}

BOOL CBusHoundCompareDlg::SetStopFlag(BOOL  stopFlag)
{
	if (m_Mutex.Lock())
	{
		m_bStop = stopFlag;
		m_Mutex.Unlock();
		return TRUE;
	}

	return FALSE;
}

BOOL CBusHoundCompareDlg::GetCompareStartFlag()
{
	BOOL  bRet = FALSE;

	if (m_Mutex.Lock())
	{
		bRet = m_bCompareStart;
		m_Mutex.Unlock();
	}

	return bRet;
}

BOOL CBusHoundCompareDlg::SetCompareStartFlag(BOOL  startFlag)
{
	if (m_Mutex.Lock())
	{
		m_bCompareStart = startFlag;
		m_Mutex.Unlock();
		return TRUE;
	}

	return FALSE;
}

UINT CBusHoundCompareDlg::GetDataFlag()
{
	UINT  uiRet = 0;

	if (m_Mutex.Lock())
	{
		uiRet = m_DataFlag;
		m_Mutex.Unlock();
	}

	return uiRet;
}

BOOL CBusHoundCompareDlg::SetDataFlag(UINT  dataFlag)
{
	if (m_Mutex.Lock())
	{
		m_DataFlag &= dataFlag;
		m_Mutex.Unlock();
		return TRUE;
	}

	return FALSE;
}

BOOL CBusHoundCompareDlg::GetDMAIdxMask(DWORD dmaIdx)
{
	BOOL  bRet = TRUE;		// 默认被占用

	if (m_Mutex.Lock())
	{
		bRet = m_DMAMask&((DWORD)1<< dmaIdx);
		m_Mutex.Unlock();
	}

	return bRet;
}

BOOL CBusHoundCompareDlg::SetDMAIdxMask(DWORD dmaIdx, BOOL maskFlag)
{
	if (m_Mutex.Lock())
	{
		if(maskFlag)
			m_DMAMask |= ((DWORD)1 << dmaIdx);
		else
			m_DMAMask &= ~((DWORD)1 << dmaIdx);
		m_Mutex.Unlock();
		return TRUE;
	}

	return FALSE;
}

// 函数名称: AddDisplay
// 函数功能: 显示信息
BOOL CBusHoundCompareDlg::AddDisplay(LPCTSTR str)
{
	if (m_Mutex.Lock())
	{
		m_listShowStatus.AddString(str);
		m_Mutex.Unlock();
		return TRUE;
	}

	return FALSE;
}

BOOL CBusHoundCompareDlg::CreateMapAddr(HANDLE hFileMap, __int64 &fileOffset, DWORD blkSize, LPBYTE &mapAddr)
{
	DistroyMapAddr(mapAddr);

	// 映射视图
	mapAddr = (LPBYTE)MapViewOfFile(hFileMap,
		FILE_MAP_READ,
		(DWORD)(fileOffset >> 32),
		(DWORD)(fileOffset & 0xFFFFFFFF),
		blkSize);
	if (NULL == mapAddr)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CBusHoundCompareDlg::DistroyMapAddr(LPBYTE &mapAddr)
{
	// 撤消文件映像
	if (mapAddr)
	{
		UnmapViewOfFile(mapAddr);
	}

	return TRUE;
}

// 函数名称: GetDataOffset
// 函数功能: 分析出数据起始与结束位置(只搜索一个块单元大小,未找到则返回失败)
// 输入参数: fileOffset(文件偏移), blkOffset(块内偏移)
BOOL CBusHoundCompareDlg::GetDataOffset(__int64 &fileOffset, UINT &blkOffset)
{
	if (!CreateMapAddr(m_hSrcFileMap, fileOffset, m_dwBlkSize, m_lpSrcMapAddress))
		return FALSE;

	while (blkOffset < m_dwBlkSize)
	{
		CString strLine = FindLine(m_lpSrcMapAddress, blkOffset, m_dwBlkSize);
		if (m_nDataStartPoint)
		{
			int offset = strLine.Find(_T("----"), m_nDataStartPoint);

			if (m_nDataStartPoint == offset)
			{
				offset = strLine.Find(_T(" "), m_nDataStartPoint);
				m_nDataLen = offset - m_nDataStartPoint;
			}

			else
			{
				m_nDataStartPoint = 0;
				m_nPhaseStartPoint = 0;
				m_nCmdPhaseOfsPoint = 0;
			}

		}
		else
		{
			int offset = strLine.Find(_T(" Data "));

			if (EOF != offset)
			{
				m_nDataStartPoint = offset + 1;

				// 获取状态码偏移位置
				int phaseOffset = strLine.Find(_T("Phase"));
				int cmdPhaseOfs = strLine.Find(_T("Cmd.Phase.Ofs(rep)"));
				if (EOF != phaseOffset)
				{
					m_nCmdPhaseOfsPoint = cmdPhaseOfs;
					m_nPhaseStartPoint = phaseOffset;
				}
				else
				{
					m_nDataStartPoint = 0;
					m_nPhaseStartPoint = 0;
					m_nCmdPhaseOfsPoint = 0;
				}
			}

		}

		if (m_nDataLen)
			return TRUE;

	}

	return FALSE;
}

DWORD   CBusHoundCompareDlg::DecodeThread()
{
	__int64 qwFileOffset = 0;
	UINT    uiBlkOffset = 0;

	m_strResidualData.Empty();

	AddDisplay(_T("解析数据开始!"));
	// 创建文件映射并获取文件长度
	m_hSrcFileMap = CreateUserFileMapping(m_strDataPath, m_nSrcFileSize);

	// 设置映射块大小
	m_dwBlkSize = GetMappingBlkSize(m_nSrcFileSize);

	// 根据数据文件计算数据偏移
	if (!GetDataOffset(qwFileOffset, uiBlkOffset))
	{
		AddDisplay(_T("解析数据失败!"));
		return FALSE;
	}

	CString strLine;
	CString strData;
	
	m_DataIdx = 0;
	m_CBWIdx = 0;
	BOOL bOutOfRange = FALSE;

	m_PhaseType = 0;
	m_DmaIdx = 0;

	__int64 stepSize = m_nSrcFileSize / 1000;
	WORD progPos = 0;

	while (GetRunFlag())
	{
		//SetCompareStartFlag(TRUE); 暂时关闭

		// 获取命令及数据
		while (uiBlkOffset < m_dwBlkSize)
		{
			if (((qwFileOffset + uiBlkOffset) / stepSize) > progPos)
			{
				m_progDecode.StepIt();
				progPos++;
			}
			if (GetStopFlag())
			{
				SetRunFlag(FALSE);
				break;
			}

			// 判断越界条件
			if ((qwFileOffset + uiBlkOffset) >= m_nSrcFileSize)
			{
				bOutOfRange = TRUE;
				break;
			}

			strLine = FindLine(m_lpSrcMapAddress, uiBlkOffset, m_dwBlkSize);


			if (!m_strResidualData.IsEmpty())
				continue;

			int cmdIdx = strLine.Find(_T("CMD"), m_nPhaseStartPoint);
			int inIdx = strLine.Find(_T("IN"), m_nPhaseStartPoint);
			int outIdx = strLine.Find(_T("OUT"), m_nPhaseStartPoint);
			int spaceIdx = strLine.Find(_T(" "), m_nPhaseStartPoint);

			if ((cmdIdx == m_nPhaseStartPoint) || ((spaceIdx == m_nPhaseStartPoint) && (1 == m_PhaseType)))
			{
				// 处理两条命令靠近的情况
				if ((cmdIdx == m_nPhaseStartPoint))
					m_CBWIdx = 0;

				CommandDecodeFlow(strLine);

			}
			else if (((inIdx == m_nPhaseStartPoint) || (outIdx == m_nPhaseStartPoint)) || ((spaceIdx == m_nPhaseStartPoint) && (2 == m_PhaseType)))
			{
				// 处理两条数据靠近的情况
				if((inIdx == m_nPhaseStartPoint) || (outIdx == m_nPhaseStartPoint))
					m_DataIdx = 0;

				DataDecodeFlow(strLine);
			}
			else
			{
				m_DataIdx = 0;
				m_CBWIdx = 0;
				m_PhaseType = 0;
			}
		}
		// 判断越界则跳出
		if (bOutOfRange)
			break;

		qwFileOffset += m_dwBlkSize;


		if (qwFileOffset > m_nSrcFileSize)
		{
			SetRunFlag(FALSE);
			break;
		}

		// 文件结尾长度不能越界
		if ((m_nSrcFileSize - qwFileOffset) < m_dwBlkSize)
		{
			m_dwBlkSize = (DWORD)(m_nSrcFileSize - qwFileOffset);
		}

		// 创建前会销毁原来的映射
		if (!CreateMapAddr(m_hSrcFileMap, qwFileOffset, m_dwBlkSize, m_lpSrcMapAddress))
			break;

		uiBlkOffset = 0;

	}
	DistroyMapAddr(m_lpSrcMapAddress);

	if (!GetStopFlag())
	{
		m_progDecode.SetPos(1000);
		AddDisplay(_T("解析数据结束!"));
	}	
	SetEndFlag(TRUE);
	return TRUE;
}

// 函数名称: CommandDecodeFlow
// 函数功能: 命令解析流程
// 输入参数: strLine(命令字符串行)
BOOL CBusHoundCompareDlg::CommandDecodeFlow(CString &strLine)
{
	CString strData;
	COMMAND_INFO cmdInfo;

	m_DataIdx = 0;		// 数据索引清空
	m_PhaseType = 1;	// 状态标记为命令

	// 获取命令字符串
	strData = strLine.Mid(m_nDataStartPoint, m_nDataLen);
	strData.TrimRight();

	// 命令字符串转命令字符数组
	while (strData.GetLength())
	{
		strData.TrimLeft();
		m_ucCmdData[m_CBWIdx++] = StringToByte(strData);
		strData = strData.Mid(BYTE_STRING_LEN);
	}

	// 筛选命令(0x28:读入/0x2A:写出)
	if ((m_ucCmdData[CMD_BLK_CMDIDX] != 0x28) && (m_ucCmdData[CMD_BLK_CMDIDX] != 0x2a))
	{
		return FALSE;
	}
	else
	{
		// 获取命令状态偏移
		CString strCmdPhaseOfs = strLine.Mid(m_nCmdPhaseOfsPoint, CMD_PHASE_OFS_LEN);
		
		
		UINT nextIdx = (m_DmaIdx++) % MAX_DMA_NUM;
		if (!GetDMAIdxMask(nextIdx))
		{
			cmdInfo.dmaIdx = nextIdx;
			SetDMAIdxMask(cmdInfo.dmaIdx, TRUE);
		}
		else
			SetRunFlag(FALSE);		// 结束线程

		// 获取命令字符串
		cmdInfo.addr = ReverseDWORD(*((DWORD *)&m_ucCmdData[CMD_BLK_ADDRIDX]));
		cmdInfo.sectorCnt = ReverseWORD(*((WORD *)&m_ucCmdData[CMD_BLK_LENIDX]));
		cmdInfo.direction = (m_ucCmdData[CMD_BLK_CMDIDX] == 0x2a);
		_tcscpy_s(cmdInfo.cmdPhaseOfs, strCmdPhaseOfs);

		// 设置写入标记
		if (cmdInfo.direction)
			m_bStartWriteFlag = TRUE;
		if (!m_bStartWriteFlag)
		{
			SetDMAIdxMask(cmdInfo.dmaIdx, FALSE);			
		}

		m_CommandInfo.push(cmdInfo);
	}

	return TRUE;
}

// 函数名称: DataDecodeFlow
// 函数功能: 数据解析流程
// 输入参数: strLine(命令字符串行)
BOOL CBusHoundCompareDlg::DataDecodeFlow(CString &strLine)
{
	CString strData;

	m_CBWIdx = 0;			// 命令索引清空
	m_PhaseType = 2;		// 状态标记为数据
		   
	if (!ExistedWriteFlag())
	{
		return FALSE;
	}
	

	DWORD addr = m_CommandInfo.front().addr;
	WORD secCnt = m_CommandInfo.front().sectorCnt;
	UINT dmaIdx = m_CommandInfo.front().dmaIdx;
	TCHAR *cmdPhaseOfs = m_CommandInfo.front().cmdPhaseOfs;

	if (!m_CommandInfo.empty())
	{
		// 获取数据字符串
		strData = strLine.Mid(m_nDataStartPoint, m_nDataLen);
		strData.TrimRight();

		// 数据字符串转换为字符数组
		while (strData.GetLength())
		{
			strData.TrimLeft();
			m_lpucSecotrData[m_CommandInfo.front().dmaIdx][m_DataIdx++] = StringToByte(strData);
			strData = strData.Mid(BYTE_STRING_LEN);
		}

		if (m_DataIdx == m_CommandInfo.front().sectorCnt*SECTOR)
		{
			if (m_CommandInfo.front().direction)
			{
				// 写出数据
				if (!PseudoWriteData(addr, secCnt, dmaIdx))
				{
					return FALSE;
				}
			}
			else
			{
				// 读入数据
				if (!PseudoReadData(addr, secCnt, dmaIdx, cmdPhaseOfs))
				{
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

// 函数名称: ExistedWriteFlag
// 函数功能: 判断是否已发送写命令
BOOL CBusHoundCompareDlg::ExistedWriteFlag()
{
	if (!m_bStartWriteFlag)
	{
		// 清空命令队列
		while (!m_CommandInfo.empty())
		{
			m_CommandInfo.pop();
		}
		//m_DmaIdx = 0;				// DMA 索引清空
		return FALSE;
	}
	return TRUE;
}

// 函数名称: PseudoWriteData
// 函数功能: 模拟 USB 写入数据操作
// 输入参数: addr(写入地址), secCnt(写入扇区数), dmaIdx(DMA 索引)
BOOL CBusHoundCompareDlg::PseudoWriteData(DWORD addr, WORD secCnt, DWORD dmaIdx )
{
	// 根据地址区别对待(暂时分为前顺序 32M 后映射 16M)
	if (addr < (SYSTEM_AREA_SIZE / SECTOR))
	{
		// 写入顺序系统区
		memcpy(&m_lpucSysArea[addr*SECTOR], m_lpucSecotrData[dmaIdx], secCnt*SECTOR);
	}
	else
	{
		for (int i = 0; i < secCnt; i++)
		{
			UINT secIdx;
			vector<DWORD>::iterator result = find((*m_DataAreaMap).begin(), (*m_DataAreaMap).end(), addr + i);
			if (result == (*m_DataAreaMap).end())	// 未找到
				secIdx = (*m_DataAreaMap).size();
			else
				secIdx = *result;

			if (secIdx >= DATA_AREA_MAP_SIZE)
				return FALSE;
			else
				(*m_DataAreaMap).push_back(addr + i);

			// 写入映射数据区
			memcpy(&m_lpucDataArea[secIdx*SECTOR], &m_lpucSecotrData[dmaIdx][i*SECTOR], SECTOR);
		}
	}
	//m_DmaIdx--;	
	SetDMAIdxMask(dmaIdx, FALSE);
	m_CommandInfo.pop();

	return TRUE;
}

// 函数名称: PseudoReadData
// 函数功能: 模拟 USB 读出数据操作
// 输入参数: addr(写入地址), secCnt(写入扇区数), dmaIdx(DMA 索引)
BOOL CBusHoundCompareDlg::PseudoReadData(DWORD addr, WORD secCnt, DWORD dmaIdx, TCHAR *cmdPhaseOfs)
{
	// 根据地址区别对待(暂时分为前顺序 32M 后映射 16M)
	if (m_CommandInfo.front().addr < (SYSTEM_AREA_SIZE / SECTOR))
	{
		if (0 != memcmp(&m_lpucSysArea[addr*SECTOR], m_lpucSecotrData[dmaIdx], secCnt*SECTOR))
		{
			ShowErrInfo(addr, cmdPhaseOfs);
		}
	}
	else
	{
		for (int i = 0; i < secCnt; i++)
		{
			UINT secIdx;
			vector<DWORD>::iterator result = find((*m_DataAreaMap).begin(), (*m_DataAreaMap).end(), addr + i);
			if (result == (*m_DataAreaMap).end())	// 未找到
				return FALSE;
			else
				secIdx = distance((*m_DataAreaMap).begin(), result);

			if (0 != memcmp(&m_lpucDataArea[secIdx*SECTOR], &m_lpucSecotrData[dmaIdx][i*SECTOR], SECTOR))
			{
				ShowErrInfo(addr, cmdPhaseOfs);
			}
		}
	}

	//m_DmaIdx--;
	SetDMAIdxMask(dmaIdx, FALSE);
	m_CommandInfo.pop();

	return TRUE;
}

void CBusHoundCompareDlg::ShowErrInfo(DWORD addr, TCHAR *cmdPhaseOfs)
{
	CString strShow;
	strShow.Format(_T("Error Address: 0x%-10X, Error Phase Offset: %-31s"), addr, cmdPhaseOfs);
	AddDisplay(strShow);
}

DWORD   CBusHoundCompareDlg::CompareThread()
{
	while (GetRunFlag())
	{

		if (GetCompareStartFlag())
		{
			AddDisplay(_T("比较数据开始!"));

		}
		else
		{

		}
	}

	AddDisplay(_T("比较数据结束!"));
	return TRUE;
}

CString  CBusHoundCompareDlg::FindLine(LPBYTE  pByte, UINT & uiIndex, UINT uiLen)
{
	CString    strRet;
	char       szChar[2] = "0";
	char  & ch = szChar[0];

	// 处理上一次残留无结尾符字符串
	if (!m_strResidualData.IsEmpty())
	{
		strRet = m_strResidualData;
		m_strResidualData.Empty();
	}
	for (UINT i = uiIndex; i < uiLen; i++)
	{
		ch = pByte[i];
		if ((0x0d != ch) && (0x0a != ch))
		{

			strRet += szChar;

			if (i == uiLen - 1)
			{
				uiIndex = uiLen;
				m_strResidualData = strRet;
				strRet = "";
				break;
			}
			continue;
		}
		else
		{
			uiIndex = i + 1;

			if (strRet.GetLength() > 0)
				break;
			else
				continue;
		}
	}
	return strRet;
}

BYTE  CBusHoundCompareDlg::StringToByte(CString &strChar)
{
	BYTE  bRet = 0;
	ASSERT(strChar.GetLength() >= BYTE_STRING_LEN);
	strChar.MakeUpper();

	for (int i = 0; i < BYTE_STRING_LEN; i++)
	{
		TCHAR ch = strChar.GetAt(i);
		if ((ch <= _T('9')) && (ch >= _T('0')))
		{
			bRet <<= 4;
			bRet |= (ch - _T('0'));
		}
		else if ((ch <= _T('F')) && (ch >= _T('A')))
		{
			bRet <<= 4;
			bRet |= (ch - _T('A') + 10);
		}
	}
	return bRet;
}

DWORD CBusHoundCompareDlg::ReverseDWORD(DWORD InData)
{
	BYTE   da1 = (BYTE)(InData);
	BYTE   da2 = (BYTE)(InData >> 8);
	BYTE   da3 = (BYTE)(InData >> 16);
	BYTE   da4 = (BYTE)(InData >> 24);

	return ((((DWORD)da1) << 24) | (((DWORD)da2) << 16) | (((DWORD)da3) << 8) | (((DWORD)da4)));
}

WORD CBusHoundCompareDlg::ReverseWORD(WORD InData)
{
	return  ((InData >> 8) | (InData << 8));
}

void CBusHoundCompareDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	// 结束线程
	if (SetStopFlag(TRUE))
	{
		while (!GetEndFlag())
		{
			Sleep(10);
		}
	}

	for (int i = 0; i < MAX_DMA_NUM; i++)
	{
		if (NULL != m_lpucSecotrData[i])
		{
			delete[] m_lpucSecotrData[i];
			m_lpucSecotrData[i] = NULL;
		}
	}

	if (NULL != m_lpucSysArea)
	{
		delete[] m_lpucSysArea;
		m_lpucSysArea = NULL;
	}

	if (NULL != m_lpucDataArea)
	{
		delete[] m_lpucDataArea;
		m_lpucDataArea = NULL;
	}

	if (NULL != m_DataAreaMap)
	{
		delete m_DataAreaMap;
		m_DataAreaMap = NULL;
	}
	CDialogEx::OnClose();
}

void CBusHoundCompareDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	DragQueryFile(hDropInfo, NULL, m_strDataPath.GetBufferSetLength(MAX_PATH), MAX_PATH);
	m_editDataPath.SetWindowText(m_strDataPath);
	//m_editDataPath.UpdateWindow();
	DragFinish(hDropInfo);


	CDialogEx::OnDropFiles(hDropInfo);
}
