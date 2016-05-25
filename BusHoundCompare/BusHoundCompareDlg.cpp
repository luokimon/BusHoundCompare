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

CBusHoundCompareDlg::CBusHoundCompareDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BUSHOUNDCOMPARE_DIALOG, pParent),
	m_lpDecodeThread(NULL),
	m_lpSrcMapAddress(NULL),
    m_lpDstMapAddress(NULL),
    m_SmallAreaIdx(0),
    m_DstFileIdx(0),
	m_BlkIdx(INFINITE)
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
	ON_WM_DROPFILES()
	//	ON_WM_CREATE()
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

	// 利用未公开 API 放行文件拖拽消息
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);	// 0x0049 - WM_COPYGLOBALDATA

	for (int i = 0; i < MAX_DMA_NUM; i++)
	{
		m_lpucSecotrData[i] = new BYTE[MAX_TRANSFER_LEN];
	}

	m_lpucSysArea = new BYTE[SYSTEM_AREA_SIZE];
	m_lpucDataArea = new BYTE[DATA_AREA_SIZE];
	m_DataAreaMap = new vector<DWORD>;
	(*m_DataAreaMap).reserve(DATA_AREA_MAP_SIZE);

    m_lpSmallSecArea = new BYTE[SMALL_AREA_SIZE];

    m_lpSmallAreaMap = new vector<WORD>;
    (*m_lpSmallAreaMap).reserve(SMALL_AREA_SIZE/SECTOR);


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

	m_nDataLen = 0;
	m_strSrcPath.Empty();
    m_strDstPath.Empty();
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
		TEXT("All Files (*.*)|*.*||"),
		NULL);

	if (dlg.DoModal() == IDOK)
	{
		m_strSrcPath = dlg.GetPathName();
		m_editDataPath.SetWindowText(m_strSrcPath);
	}
}

void CBusHoundCompareDlg::OnBnClickedBtnCompare()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_strSrcPath.IsEmpty())
		MessageBox(TEXT("请先选取BusHound数据文件!"), TEXT("警告"), MB_ICONWARNING | MB_OK);
	else
	{
		// 创建工作线程
		if (!GetRunFlag())
		{
			if (GetFileAttribute())
			{
				CreateWorkThread();
			}
		}
	}
}

// 函数名称: GetFileAttribute
// 函数功能: 获取数据所在列及长度
BOOL CBusHoundCompareDlg::GetFileAttribute()
{
	// 创建源文件映射并获取文件长度
	m_hSrcFileMap = CreateUserFileMapping(m_strSrcPath, m_nSrcFileSize);

	// 设置源映射块大小
	m_dwSrcBlkSize = GetMappingBlkSize(m_nSrcFileSize);

	// 根据数据文件计算数据偏移
	if (!GetDataOffset(0, 0))
	{
		m_listShowStatus.AddString(TEXT("解析数据失败!"));
		return FALSE;
	}
    else
    {
        if (CreateDstFile())
        {
            // 创建目标文件映射并获取文件长度
            m_hDstFileMap = CreateUserFileMapping(m_strDstPath, m_nDstFileSize);

            // 设置目标映射块大小
            m_dwDstBlkSize = GetMappingBlkSize(m_nDstFileSize);
        }
        else
        {
            m_listShowStatus.AddString(TEXT("生成目标文件失败!"));
            return FALSE;
        }
    }

	return TRUE;
}

BOOL CBusHoundCompareDlg::CreateDstFile()
{
    m_strDstPath = GetCurrentPath() + TEXT("PseudoDev.bin");

    HANDLE hFile = CreateFile(m_strDstPath, 
        GENERIC_READ | GENERIC_WRITE, 
        0, 
        NULL, 
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, 
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(TEXT("创建文件失败!"));
        return  FALSE;
    }

    LARGE_INTEGER liDistanceToMove;
    liDistanceToMove.QuadPart = m_nDstFileSize; //设置成这个大，单位字节
    if (!SetFilePointerEx(hFile, liDistanceToMove, NULL, FILE_BEGIN))
    {
        MessageBox(TEXT("移动文件指针失败!"));
        return FALSE;
    }
    if (!SetEndOfFile(hFile))
    {
        MessageBox(TEXT("设置文件尾失败!"));
        return FALSE;
    }
    CloseHandle(hFile);

    return TRUE;
}

VOID CBusHoundCompareDlg::CreateWorkThread()
{
	SetRunFlag(TRUE);
	SetEndFlag(FALSE);
	SetStopFlag(FALSE);

	// 开启解析数据文件线程
	CreateDecodeThread();
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
		MessageBox(TEXT("创建文件对象失败!"));
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
		MessageBox(TEXT("创建文件对象失败!"));
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
		bRet = m_DMAMask&((DWORD)1 << dmaIdx);
		m_Mutex.Unlock();
	}

	return bRet;
}

BOOL CBusHoundCompareDlg::SetDMAIdxMask(DWORD dmaIdx, BOOL maskFlag)
{
	if (m_Mutex.Lock())
	{
		if (maskFlag)
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

BOOL CBusHoundCompareDlg::CreateMapAddr(HANDLE hFileMap, __int64 fileOffset, DWORD blkSize, LPBYTE &mapAddr)
{
	DistroyMapAddr(mapAddr);

	// 映射视图
	mapAddr = (LPBYTE)MapViewOfFile(hFileMap,
		FILE_MAP_READ | FILE_MAP_WRITE,
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
BOOL CBusHoundCompareDlg::GetDataOffset(__int64 fileOffset, UINT blkOffset)
{
	CString strLine;
	m_nDataLen = 0;
	m_nDataStartPoint = 0;
	m_nPhaseStartPoint = 0;
	m_nCmdPhaseOfsPoint = 0;

	// 创建文件映射
	if (!CreateMapAddr(m_hSrcFileMap, fileOffset, m_dwSrcBlkSize, m_lpSrcMapAddress))
		return FALSE;

	while (blkOffset < m_dwSrcBlkSize)
	{
		strLine = FindLine(m_lpSrcMapAddress, blkOffset, m_dwSrcBlkSize);

		if (m_nDataStartPoint)
			CheckDataStartPoint(strLine);
		else
			GetDataStartPoint(strLine);

        if (m_nDataLen)
        {
            // 预估大小
            if(GetDstFileSize(strLine))
			    break;
        }
	}

	// 撤销文件映射
	DistroyMapAddr(m_lpSrcMapAddress);

	return (BOOL)m_nDataLen;
}

void	CBusHoundCompareDlg::GetDataStartPoint(CString &strLine)
{
	int offset = strLine.Find(TEXT(" Data "));

	if (EOF != offset)
	{
		m_nDataStartPoint = offset + 1;

		int phaseOffset = strLine.Find(TEXT("Phase"));
		int cmdPhaseOfs = strLine.Find(TEXT("Cmd.Phase.Ofs(rep)"));
		if ((EOF != phaseOffset) && (EOF != cmdPhaseOfs))
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

void	CBusHoundCompareDlg::CheckDataStartPoint(CString &strLine)
{
	int dataOff = strLine.Find(TEXT("----"), m_nDataStartPoint);
	int phaseOff = strLine.Find(TEXT("-----  "), m_nPhaseStartPoint);
	int ofsOff = strLine.Find(TEXT("------------------  "), m_nCmdPhaseOfsPoint);

	if ((m_nDataStartPoint == dataOff) && (m_nPhaseStartPoint == phaseOff) && (m_nCmdPhaseOfsPoint == ofsOff))
	{
		dataOff = strLine.Find(TEXT(" "), m_nDataStartPoint);
		m_nDataLen = dataOff - m_nDataStartPoint;
	}

	else
	{
		GetDataStartPoint(strLine);
	}
}

// 函数名称: GetDstFileSize
// 函数功能: 获取预估目标文件大小
// 输入参数: strLine(行数据)
BOOL CBusHoundCompareDlg::GetDstFileSize(CString &strLine)
{
    int lineLen = strLine.GetLength();
    int linedataNum = ((m_nDataLen + 2) / 13) * 4;      // 开始与结尾加一个字符空格,每4个 BYTE 一个单元
    int srcDesRatio = (lineLen / linedataNum) * 2;      // 行与数据比例以及读写各占1/2

    m_nDstFileSize = m_nSrcFileSize / srcDesRatio;
    return TRUE;
}

DWORD   CBusHoundCompareDlg::DecodeThread()
{
	__int64 qwFileOffset = 0;
	UINT    uiBlkOffset = 0;

	m_strResidualData.Empty();
    m_progDecode.SetPos(0);

	AddDisplay(TEXT("解析数据开始!"));

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
		// 创建源文件位置映射
		if (!CreateMapAddr(m_hSrcFileMap, qwFileOffset, m_dwSrcBlkSize, m_lpSrcMapAddress))
			break;

		// 获取命令及数据
		uiBlkOffset = 0;
		while (uiBlkOffset < m_dwSrcBlkSize)
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

			strLine = FindLine(m_lpSrcMapAddress, uiBlkOffset, m_dwSrcBlkSize);

			if (!m_strResidualData.IsEmpty())
				continue;

			int cmdIdx = strLine.Find(TEXT("CMD"), m_nPhaseStartPoint);
			int inIdx = strLine.Find(TEXT("IN"), m_nPhaseStartPoint);
			int outIdx = strLine.Find(TEXT("OUT"), m_nPhaseStartPoint);
			int spaceIdx = strLine.Find(TEXT(" "), m_nPhaseStartPoint);

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
				if ((inIdx == m_nPhaseStartPoint) || (outIdx == m_nPhaseStartPoint))
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

		qwFileOffset += m_dwSrcBlkSize;

		if (qwFileOffset > m_nSrcFileSize)
		{
			SetRunFlag(FALSE);
			break;
		}

		// 文件结尾长度不能越界
		if ((m_nSrcFileSize - qwFileOffset) < m_dwSrcBlkSize)
		{
			m_dwSrcBlkSize = (DWORD)(m_nSrcFileSize - qwFileOffset);
		}
	}
	DistroyMapAddr(m_lpSrcMapAddress);
	DistroyMapAddr(m_lpDstMapAddress);

	if (!GetStopFlag())
	{
		m_progDecode.SetPos(1000);
		AddDisplay(TEXT("解析数据结束!"));
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
#if DATA_FILE_EX
                if (!PseudoWriteData_Ex(addr, secCnt, dmaIdx, cmdPhaseOfs))
#else
                if (!PseudoWriteData(addr, secCnt, dmaIdx))
#endif
				{
					return FALSE;
				}
			}
			else
			{
				// 读入数据
#if DATA_FILE_EX
                if (!PseudoReadData_Ex(addr, secCnt, dmaIdx, cmdPhaseOfs))
#else
                if (!PseudoReadData(addr, secCnt, dmaIdx, cmdPhaseOfs))
#endif
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
BOOL CBusHoundCompareDlg::PseudoWriteData(DWORD addr, WORD secCnt, DWORD dmaIdx)
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

BOOL CBusHoundCompareDlg::PseudoWriteData_Ex(DWORD addr, WORD secCnt, DWORD dmaIdx, TCHAR *cmdPhaseOfs)
{
    ASSERT(secCnt < 0x81);      // 单次传输限制在0x80个sector(64K)

    __int64 qwFileOffset;
    __int64 qwDstMapIdx;
    DWORD dwMapLowAddr;
    DWORD dwMapHighAddr;
    map<DWORD, WORD>::iterator iterLow, iterHigh;

    

    if (m_DstFileMap.empty())
    {
        if (!AddNewMapData(addr, secCnt, dmaIdx))
            return FALSE;
    }
    else
    {
        // 寻找 MAP 文件区间中高位部分
        iterHigh = m_DstFileMap.upper_bound(addr);
        if (iterHigh == m_DstFileMap.end())
        {
            iterLow  = iterHigh;
             iterLow--;

            dwMapLowAddr = (*iterLow).first;
            if ((addr + secCnt) <= (dwMapLowAddr + MAX_TRANS_SEC_NUM))
            {
                if (AdjustFileMap(m_DstFileMap[dwMapLowAddr], qwFileOffset))
                {
                    qwDstMapIdx = m_DstFileMap[dwMapLowAddr] * MAX_TRANSFER_LEN + (addr - dwMapLowAddr)*SECTOR - qwFileOffset;
                    memcpy(&m_lpDstMapAddress[qwDstMapIdx], m_lpucSecotrData[dmaIdx], secCnt*SECTOR);

                    m_DstSecMap[dwMapLowAddr] = (m_DstSecMap[dwMapLowAddr] > (addr + secCnt - dwMapLowAddr)) ? m_DstSecMap[dwMapLowAddr] : (WORD)(addr + secCnt - dwMapLowAddr);
                }               
            }
            else
            {
                if (addr < (dwMapLowAddr + m_DstSecMap[dwMapLowAddr]))
                {
                    ShowMissInfo(addr, cmdPhaseOfs);
                }
                else
                {
                    if (!AddNewMapData(addr, secCnt, dmaIdx))
                        return FALSE;
                }
            }

        }
        else
        {
            iterLow = iterHigh;               // 取巧找文件区间中低位部分方法
            iterLow--;
            dwMapHighAddr = (*iterHigh).first;
            if (iterLow == m_DstFileMap.end())
            {
                if (dwMapHighAddr <= (addr + secCnt))
                {
                    ShowMissInfo(addr, cmdPhaseOfs);
                }
                else
                {
                    if (!AddNewMapData(addr, secCnt, dmaIdx))
                        return FALSE;
                }
            }
            else
            {
                dwMapLowAddr = (*iterLow).first;
                if ((addr + secCnt) <= (dwMapLowAddr + MAX_TRANS_SEC_NUM))
                {
                    if (AdjustFileMap(m_DstFileMap[dwMapLowAddr], qwFileOffset))
                    {
                        qwDstMapIdx = m_DstFileMap[dwMapLowAddr] * MAX_TRANSFER_LEN + (addr - dwMapLowAddr)*SECTOR - qwFileOffset;
                        memcpy(&m_lpDstMapAddress[qwDstMapIdx], m_lpucSecotrData[dmaIdx], secCnt*SECTOR);

                        m_DstSecMap[dwMapLowAddr] = (m_DstSecMap[dwMapLowAddr] > (addr + secCnt - dwMapLowAddr)) ? m_DstSecMap[dwMapLowAddr] : (WORD)(addr + secCnt - dwMapLowAddr);
                    }
                }
                else
                {
                    if ((addr < (dwMapLowAddr + m_DstSecMap[dwMapLowAddr]))||(addr+secCnt > dwMapHighAddr))
                    {
                        ShowMissInfo(addr, cmdPhaseOfs);
                    }
                    else
                    {
                        if (!AddNewMapData(addr, secCnt, dmaIdx))
                            return FALSE;
                    }
                }
            }
        }

    }

    SetDMAIdxMask(dmaIdx, FALSE);
    m_CommandInfo.pop();

    return TRUE;
}

BOOL CBusHoundCompareDlg::AdjustFileMap(WORD idx, __int64 &qwFileOffset)
{
	qwFileOffset = (__int64)idx*MAX_TRANSFER_LEN;

	if (m_BlkIdx != (qwFileOffset / m_dwDstBlkSize))
	{
		DWORD blkSize = (DWORD)(((m_nDstFileSize - (qwFileOffset/ m_dwDstBlkSize)*m_dwDstBlkSize) >= m_dwDstBlkSize) ? m_dwDstBlkSize : (m_nDstFileSize - (qwFileOffset / m_dwDstBlkSize)*m_dwDstBlkSize));

		m_BlkIdx = (DWORD)(qwFileOffset / m_dwDstBlkSize);

		// 创建目标文件位置映射
		if (!CreateMapAddr(m_hDstFileMap, m_dwDstBlkSize*m_BlkIdx, blkSize, m_lpDstMapAddress))
			return FALSE;

	}

    qwFileOffset = m_BlkIdx*m_dwDstBlkSize;

	return TRUE;
}

BOOL CBusHoundCompareDlg::AddNewMapData(DWORD addr, WORD secCnt, DWORD dmaIdx)
{
    __int64 qwFileOffset;

    m_DstFileMap.insert(pair<DWORD, WORD>(addr, m_DstFileIdx++));
    m_DstSecMap.insert(pair<DWORD, WORD>(addr, secCnt));

    if (AdjustFileMap(m_DstFileMap[addr], qwFileOffset))
    {
        memcpy(&m_lpDstMapAddress[m_DstFileMap[addr] * MAX_TRANSFER_LEN - qwFileOffset], m_lpucSecotrData[dmaIdx], secCnt*SECTOR);
        return TRUE;
    }

    return FALSE;
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

BOOL CBusHoundCompareDlg::PseudoReadData_Ex(DWORD addr, WORD secCnt, DWORD dmaIdx, TCHAR *cmdPhaseOfs)
{
    ASSERT(secCnt < 0x81);      // 单次传输限制在0x80个sector(64K)

    BOOL bMatch = TRUE;
    __int64 qwFileOffset;
    __int64 qwDstMapIdx;
    DWORD dwMapLowAddr;
    DWORD dwMapHighAddr;
    map<DWORD, WORD>::iterator iterLow, iterHigh;

    if (m_DstFileMap.empty())
    {
        ShowMissInfo(addr, cmdPhaseOfs);
    }
    else
    {
        // 寻找 MAP 文件区间中高位部分
        iterHigh = m_DstFileMap.upper_bound(addr);
        if (iterHigh == m_DstFileMap.end())
        {
            iterLow = iterHigh;
            iterLow--;
            dwMapLowAddr = (*iterLow).first;
            if ((addr + secCnt) > dwMapLowAddr + m_DstSecMap[dwMapLowAddr])
            {
                ShowMissInfo(addr, cmdPhaseOfs);
            }
            else
            {
                if (AdjustFileMap(m_DstFileMap[dwMapLowAddr], qwFileOffset))
                {
                    qwDstMapIdx = m_DstFileMap[dwMapLowAddr] * MAX_TRANSFER_LEN + (addr - dwMapLowAddr)*SECTOR - qwFileOffset;
                    if (0 != memcmp(&m_lpDstMapAddress[qwDstMapIdx], m_lpucSecotrData[dmaIdx], secCnt*SECTOR))
                    {
                        ShowErrInfo(addr, cmdPhaseOfs);
                    }
                }
            }
        }
        else
        {
            iterLow = iterHigh;               // 取巧找文件区间中低位部分方法
            iterLow--;
            dwMapHighAddr = (*iterHigh).first;
            if (iterLow == m_DstFileMap.end())
            {
                ShowMissInfo(addr, cmdPhaseOfs);
            }
            else
            {
                dwMapLowAddr = (*iterLow).first;
                DWORD tmpAddr = addr;
                WORD tmpSecCnt = secCnt;
                if (dwMapLowAddr + m_DstSecMap[dwMapLowAddr] == dwMapHighAddr)
                {
                    while (tmpSecCnt)
                    {
                        DWORD mapAddr = (*iterLow++).first;
                        if (AdjustFileMap(m_DstFileMap[mapAddr], qwFileOffset))
                        {
                            DWORD dstMapAddr = (DWORD)(m_DstFileMap[mapAddr] * MAX_TRANSFER_LEN - qwFileOffset + (tmpAddr - mapAddr)*SECTOR);
                            WORD transSec = (WORD)(m_DstSecMap[mapAddr] + mapAddr - tmpAddr) < tmpSecCnt ? (WORD)(m_DstSecMap[mapAddr] + mapAddr - tmpAddr): tmpSecCnt;
                            if (0 != memcmp(&m_lpDstMapAddress[dstMapAddr], &m_lpucSecotrData[dmaIdx][(tmpAddr - addr)*SECTOR], transSec*SECTOR))
                            {
                                bMatch = FALSE;
                                break;
                            }
                            tmpAddr += transSec;
                            tmpSecCnt -= transSec;
                        }
                    }
                    if(!bMatch)
                        ShowErrInfo(addr, cmdPhaseOfs);
                }
                else
                {
                    //ShowMissInfo(addr, cmdPhaseOfs);
                    if (AdjustFileMap(m_DstFileMap[dwMapLowAddr], qwFileOffset))
                    {
                        qwDstMapIdx = m_DstFileMap[dwMapLowAddr] * MAX_TRANSFER_LEN + (addr - dwMapLowAddr)*SECTOR - qwFileOffset;
                        if (0 != memcmp(&m_lpDstMapAddress[qwDstMapIdx], m_lpucSecotrData[dmaIdx], secCnt*SECTOR))
                        {
                            ShowErrInfo(addr, cmdPhaseOfs);
                        }
                    }
                }
            }
        }
    }

    SetDMAIdxMask(dmaIdx, FALSE);
    m_CommandInfo.pop();

    return TRUE;
}

void CBusHoundCompareDlg::ShowErrInfo(DWORD addr, TCHAR *cmdPhaseOfs)
{
	CString strShow;
	strShow.Format(TEXT("Error Address: 0x%-10X, Error Phase Offset: %-31s"), addr, cmdPhaseOfs);
	AddDisplay(strShow);
}

void CBusHoundCompareDlg::ShowMissInfo(DWORD addr, TCHAR *cmdPhaseOfs)
{
	CString strShow;
	strShow.Format(TEXT("Miss Address: 0x%-10X, Miss Phase Offset: %-31s"), addr, cmdPhaseOfs);
	AddDisplay(strShow);
}

void CBusHoundCompareDlg::ShowOverflowInfo(DWORD addr, TCHAR *cmdPhaseOfs)
{
    CString strShow;
    strShow.Format(TEXT("OverFlow Address: 0x%-10X, OverFlow Phase Offset: %-31s"), addr, cmdPhaseOfs);
    AddDisplay(strShow);
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

    if (m_lpSmallSecArea)
    {
        delete m_lpSmallSecArea;
        m_lpSmallSecArea = NULL;
    }
    //if (!m_strDstPath.IsEmpty())
    //{
    //    // 删除临时生成文件
    //    DeleteFile(m_strDstPath.GetBuffer());
    //}
    

	CDialogEx::OnClose();
}

void CBusHoundCompareDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (1 == DragQueryFile(hDropInfo, INFINITE, NULL, NULL))
	{
		DragQueryFile(hDropInfo, NULL, m_strSrcPath.GetBufferSetLength(MAX_PATH), MAX_PATH);
		m_editDataPath.SetWindowText(m_strSrcPath);
		//m_editDataPath.UpdateWindow();
	}
	else
	{
		MessageBox(TEXT("请单独选择需要解析的文件!"), NULL, MB_ICONERROR | MB_OK);
	}
	DragFinish(hDropInfo);

	CDialogEx::OnDropFiles(hDropInfo);
}

//int CBusHoundCompareDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//	DragAcceptFiles(TRUE);
//
//	return CDialogEx::OnCreate(lpCreateStruct);
//}