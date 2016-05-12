
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
	DDX_Control(pDX, IDC_EDIT_GRANULARITY, m_editGranularity);
	DDX_Control(pDX, IDC_EDIT_BLKUNITSIZE, m_editBlkUnitSize);
	DDX_Control(pDX, IDC_EDIT_FATUNITSIZE, m_editFATUnitSize);
}

BEGIN_MESSAGE_MAP(CBusHoundCompareDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CBusHoundCompareDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CBusHoundCompareDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_SELECTPATH, &CBusHoundCompareDlg::OnBnClickedBtnSelectpath)
	ON_BN_CLICKED(IDC_BTN_COMPARE, &CBusHoundCompareDlg::OnBnClickedBtnCompare)
	ON_WM_CLOSE()
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
	DisplayWindowInfo();

	for (int i = 0; i < MAX_DMA_NUM; i++)
	{
		m_lpucSecotrData[i] = new BYTE[MAX_TRANS_SEC_NUM*SECTOR];
	}

	m_lpucSysArea = new BYTE[SYSTEM_AREA_SIZE];
	m_lpucDataArea = new BYTE[DATA_AREA_SIZE];

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 函数名称: InitialParam
// 函数功能: 初始化参数
void CBusHoundCompareDlg::InitialParam()
{
	m_bRun = FALSE;
	m_bEnd = FALSE;
	m_bCompareStart = FALSE;

	m_nDataStartPoint = 0;
	m_nDataLen = 0;
	m_strDataPath.Empty();
	m_Granularity = GetAllocationGranularity();
}

// 函数名称: DisplayWindowInfo
// 函数功能: 显示主界面信息
void CBusHoundCompareDlg::DisplayWindowInfo()
{
	CString str;

	str.Format(_T("0x%X"), m_Granularity);
	m_editGranularity.SetWindowText(str);

	str.Format(_T("0x%X"), BLOCK_UNIT_SIZE);
	m_editBlkUnitSize.SetWindowText(str);

	str.Format(_T("0x%X"), FAT_MAX_UINT_SIZE);
	m_editFATUnitSize.SetWindowText(str);
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
		CreateWorkThread();
	}
}

VOID CBusHoundCompareDlg::CreateWorkThread()
{
	SetRunFlag(TRUE);
	SetEndFlag(FALSE);

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

			if (-1 != offset)
			{
				m_nDataStartPoint = offset + 1;

				// 获取状态码偏移位置
				int phaseOffset = strLine.Find(_T("Phase"));
				int cmdPhaseOfs = strLine.Find(_T("Cmd.Phase.Ofs(rep)"));
				if (-1 != phaseOffset)
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
	UINT dmaIdx = 0;
	UINT dataIdx = 0;
	UINT cbwIdx = 0;
	UINT phaseType = 0;  // 0:其他状态/1:命令状态/2:数据状态
	COMMAND_INFO cmdInfo;
	BOOL bStartWriteFlag = FALSE;
	BOOL bOutOfRange = FALSE;

	m_DataAreaMap.clear();

	while (GetRunFlag())
	{
		//SetCompareStartFlag(TRUE); 暂时关闭

		// 获取命令及数据
		while (uiBlkOffset < m_dwBlkSize)
		{
			// 判断越界条件
			if ((qwFileOffset + uiBlkOffset) >= m_nSrcFileSize)
			{
				bOutOfRange = TRUE;
				break;
			}

			strLine = FindLine(m_lpSrcMapAddress, uiBlkOffset, m_dwBlkSize);       // 5ms

			int cmdIdx = strLine.Find(_T("CMD"), m_nPhaseStartPoint);
			int inIdx = strLine.Find(_T("IN"), m_nPhaseStartPoint);
			int outIdx = strLine.Find(_T("OUT"), m_nPhaseStartPoint);
			int spaceIdx = strLine.Find(_T(" "), m_nPhaseStartPoint);

			if ((cmdIdx == m_nPhaseStartPoint) || ((spaceIdx == m_nPhaseStartPoint) && (1 == phaseType)))
			{
				// 处理两条命令靠近的情况
				if ((cmdIdx == m_nPhaseStartPoint))
					cbwIdx = 0;

				dataIdx = 0;
				phaseType = 1;
				// 获取命令
				strData = strLine.Mid(m_nDataStartPoint, m_nDataLen);
				strData.TrimRight();
					

				while (strData.GetLength())
				{
					strData.TrimLeft();
					m_ucCmdData[cbwIdx++] = StringToByte(strData);
					strData = strData.Mid(BYTE_STRING_LEN);
				}
				if ((m_ucCmdData[0] != 0x28) && (m_ucCmdData[0] != 0x2a))
					continue;
				else
				{
					CString strPhaseOfs = strLine.Mid(m_nCmdPhaseOfsPoint, CMD_PHASE_OFS_LEN);
					strPhaseOfs.Trim();

					if (-1 != strPhaseOfs.Find(_T("2270.1.0")))
					{
						strPhaseOfs.Trim();
					}

					_tcscpy_s(cmdInfo.cmdPhaseOfs, strPhaseOfs);
					//memcpy(cmdInfo.cmdPhaseOfs, strPhaseOfs, strPhaseOfs.GetLength() + 1);
					cmdInfo.addr = ReverseDWORD(*((DWORD *)&m_ucCmdData[2]));
					cmdInfo.sectorCnt = ReverseWORD(*((WORD *)&m_ucCmdData[7]));
					cmdInfo.dmaIdx = dmaIdx++;
					cmdInfo.direction = (m_ucCmdData[0] == 0x2a);
					m_CommandInfo.push(cmdInfo);

					if(cmdInfo.direction)
						bStartWriteFlag = TRUE;
					//ASSERT(m_CommandInfo.size() < 2);
					continue;
				}
			}
			else if (((inIdx == m_nPhaseStartPoint) || (outIdx == m_nPhaseStartPoint)) || ((spaceIdx == m_nPhaseStartPoint) && (2 == phaseType)))
			{
				// 处理两条数据靠近的情况
				if((inIdx == m_nPhaseStartPoint) || (outIdx == m_nPhaseStartPoint))
					dataIdx = 0;

				phaseType = 2;
				cbwIdx = 0;

				// 未开始写之前不比较数据
				if (!bStartWriteFlag)
				{
					if (!m_CommandInfo.empty())
					{
						m_CommandInfo.pop();
						dmaIdx = 0;
					}
					continue;
				}
					
						
					

				// 获取数据
				if (!m_CommandInfo.empty())
				{
					
					strData = strLine.Mid(m_nDataStartPoint, m_nDataLen);    // 6ms
					strData.TrimRight();


					while (strData.GetLength())
					{
						strData.TrimLeft();
						m_lpucSecotrData[m_CommandInfo.front().dmaIdx][dataIdx++] = StringToByte(strData);
						strData = strData.Mid(BYTE_STRING_LEN);
					}

					//ASSERT(dataIdx <= (UINT)m_CommandInfo.front().sectorCnt*SECTOR);
					if (dataIdx == m_CommandInfo.front().sectorCnt*SECTOR)
					{
						if (m_CommandInfo.front().direction)
						{
							if (m_CommandInfo.front().addr < (SYSTEM_AREA_SIZE / SECTOR))
							{
								memcpy(&m_lpucSysArea[m_CommandInfo.front().addr*SECTOR], m_lpucSecotrData[m_CommandInfo.front().dmaIdx], m_CommandInfo.front().sectorCnt*SECTOR);
							}
							else
							{
								memcpy(&m_lpucDataArea[m_DataAreaMap.size()*SECTOR], m_lpucSecotrData[m_CommandInfo.front().dmaIdx], m_CommandInfo.front().sectorCnt*SECTOR);
								for (int i = 0; i < m_CommandInfo.front().sectorCnt; i++)
								{
									m_DataAreaMap.insert(make_pair(m_CommandInfo.front().addr, m_DataAreaMap.size()));
								}
								
							}
							dmaIdx--;
							m_CommandInfo.pop();
						}
						else
						{
							if (bStartWriteFlag)
							{
								if (m_CommandInfo.front().addr < (SYSTEM_AREA_SIZE / SECTOR))
								{
									if (0 != memcmp(&m_lpucSysArea[m_CommandInfo.front().addr*SECTOR], m_lpucSecotrData[m_CommandInfo.front().dmaIdx], m_CommandInfo.front().sectorCnt*SECTOR))
									{
										CString strShow;
										strShow.Format(_T("Error Address: 0x%8X, Error Phase Offset: %s"), m_CommandInfo.front().addr, m_CommandInfo.front().cmdPhaseOfs);
										AddDisplay(strShow);
									}
								}
								else
								{
									for (int i = 0; i < m_CommandInfo.front().sectorCnt; i++)
									{
										if (0 != memcmp(&m_lpucDataArea[m_DataAreaMap[m_CommandInfo.front().addr+i]*SECTOR], &m_lpucSecotrData[m_CommandInfo.front().dmaIdx][i*SECTOR], SECTOR))
										{
											CString strShow;
											strShow.Format(_T("Error Address: 0x%8X, Error Phase Offset: %s"), m_CommandInfo.front().addr, m_CommandInfo.front().cmdPhaseOfs);
											AddDisplay(strShow);
										}
									}
								}
							}
							dmaIdx--;
							m_CommandInfo.pop();
						}
					}
				}
			}
			else
			{
				dataIdx = 0;
				cbwIdx = 0;
				phaseType = 0;
			}
		}
		// 判断越界则跳出
		if (bOutOfRange)
			break;

		qwFileOffset += m_dwBlkSize;

		
		if (qwFileOffset > m_nSrcFileSize)
			break;

		// 创建前会销毁原来的映射
		if (!CreateMapAddr(m_hSrcFileMap, qwFileOffset, m_dwBlkSize, m_lpSrcMapAddress))
			return FALSE;

		uiBlkOffset = 0;

	}
	DistroyMapAddr(m_lpSrcMapAddress);
	//SetRunFlag(FALSE);
	SetEndFlag(TRUE);
	AddDisplay(_T("解析数据结束!"));
	return TRUE;
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

BYTE  CBusHoundCompareDlg::StringToByte(CString strChar)
{
	BYTE  bRet = 0;
	//int iLen = strChar.GetLength();
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

	CDialogEx::OnClose();
}
