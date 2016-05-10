
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



CBusHoundCompareDlg::CBusHoundCompareDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BUSHOUNDCOMPARE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBusHoundCompareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DATAPATH, m_editDataPath);
}

BEGIN_MESSAGE_MAP(CBusHoundCompareDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CBusHoundCompareDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CBusHoundCompareDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_SELECTPATH, &CBusHoundCompareDlg::OnBnClickedBtnSelectpath)
	ON_BN_CLICKED(IDC_BTN_COMPARE, &CBusHoundCompareDlg::OnBnClickedBtnCompare)
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
	m_strDataPath.Empty();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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
	do
	{
		// 映射数据文件
		if (MappingDataFile())
		{
			// 开启解析数据文件线程
			CreateDecodeThread();
		}	
		else
			break;

		// 映射虚拟内存
		if (MappingVirtualMemory())
		{
			//开启比较数据线程
			CreateCompareThread();
		}
					
	} while (false);
	
	//CompareData();
	//CFile tmpFile;
	//tmpFile.Open(_T("tmpData.bin"), CFile::modeCreate | CFile::modeReadWrite);

}

BOOL CBusHoundCompareDlg::CompareData()
{
	m_hSrcFileMap = CreateUserFileMapping(m_strDataPath, m_nSrcFileSize);

	// 得到系统分配粒度
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	DWORD dwGran = SysInfo.dwAllocationGranularity;

	// 偏移地址 
	__int64 qwFileOffset = 0;
	// 块大小(设置为1024整数倍)
	DWORD dwBlockBytes = 0x400 * dwGran;
	if (m_nSrcFileSize < 0x400 * dwGran)
		dwBlockBytes = (DWORD)m_nSrcFileSize;
	/*
	while (m_nSrcFileSize > 0)
	{
		// 映射视图
		LPBYTE lpbMapAddress = (LPBYTE)MapViewOfFile(m_hSrcFileMap, 
			FILE_MAP_READ,
			(DWORD)(qwFileOffset >> 32), 
			(DWORD)(qwFileOffset & 0xFFFFFFFF),
			dwBlockBytes);
		if (NULL == lpbMapAddress)
		{
			TRACE("映射文件映射失败,错误代码:%drn", GetLastError());
			return FALSE;
		}
		// 对映射的视图进行访问
		for (DWORD i = 0; i < dwBlockBytes; i++)
			BYTE temp = *(lpbMapAddress + i);
		// 撤消文件映像
		UnmapViewOfFile(lpbMapAddress);
		// 修正参数
		qwFileOffset += dwBlockBytes;
		m_nSrcFileSize -= dwBlockBytes;
	}
	*/

	// 关闭文件映射对象句柄
	CloseHandle(m_hSrcFileMap);


	return TRUE;
}

HANDLE CBusHoundCompareDlg::CreateUserFileMapping(CString strPath, __int64 &fileSize)
{
	// 创建文件对象
	HANDLE hFile = CreateFile(strPath,
		GENERIC_READ,
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
	HANDLE hSrcFileMap = CreateFileMapping(hFile,
		NULL,
		PAGE_READONLY,
		0,
		0,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hSrcFileMap)
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

	return hSrcFileMap;
}

DWORD CBusHoundCompareDlg::GetMappingBlkSize(__int64 srcFileSize)
{
	// 得到系统分配粒度
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	DWORD dwGran = SysInfo.dwAllocationGranularity;

	if (srcFileSize < BLOCK_UNIT_SIZE * dwGran)
		return((DWORD)srcFileSize);
	else
		return(BLOCK_UNIT_SIZE * dwGran);
}

// 函数名称: MappingDataFile
// 函数功能: 映射数据文件
BOOL CBusHoundCompareDlg::MappingDataFile()
{
	// 创建文件映射并获取文件长度
	m_hSrcFileMap = CreateUserFileMapping(m_strDataPath, m_nSrcFileSize);

	// 设置映射块大小
	m_dwBlkSize = GetMappingBlkSize(m_nSrcFileSize);

	return TRUE;
}

// 函数名称: CreateDecodeThread
// 函数功能: 开启解析数据文件线程
VOID CBusHoundCompareDlg::CreateDecodeThread()
{

}

// 函数名称: MappingVirtualMemory
// 函数功能: 映射虚拟内存
BOOL CBusHoundCompareDlg::MappingVirtualMemory()
{
	return FALSE;
}

// 函数名称: CreateCompareThread
// 函数功能: 开启比较数据线程
VOID CBusHoundCompareDlg::CreateCompareThread()
{

}

VOID CBusHoundCompareDlg::SetErrCode(UINT uErr)
{
	if (m_Mutex.Lock())
	{
		m_err = uErr;
		m_Mutex.Unlock();
	}
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