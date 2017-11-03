// CDBurn.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CDBurn.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#include "DirObject.h"
#include <direct.h>
// #include <error.h>

/////////////////////////////////////////////////////////////////////////////
// CCDBurnApp

/////////////////////////////////////////////////////////////////////////////
// CCDBurnApp construction

CCDBurnApp::CCDBurnApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_pCurDiscRecorder = NULL;
	m_cstrFilePath = _T("E:\\百胜20090902\\1-产品介绍-Word");
}

CCDBurnApp::~CCDBurnApp()
{
	::CoUninitialize();
}



/////////////////////////////////////////////////////////////////////////////
// The one and only CCDBurnApp object

CCDBurnApp theApp;

#ifndef EEXIST
#define  EEXIST 17
#endif

/////////////////////////////////////////////////////////////////////////////
// CCDBurnApp initialization

BOOL CCDBurnApp::InitInstance()
{
	HANDLE g_hMutex = NULL; //互斥量 句柄
	BOOL  bRecFile = FALSE; //刻录文件是否成功
	BOOL bNeedToOpenProcess = TRUE;//是否开启子进程
	m_bPopupDrivers = TRUE;

	::CoInitializeEx(NULL,COINIT_MULTITHREADED);

	m_bLogFile = TRUE; 

	CString cstrLogFile;
	CString strPath;
	CTime cTime = CTime::GetCurrentTime();
	strPath.Format("C:\\RecDataLog");
	if(mkdir(strPath) != 0)
	{
		DWORD dwErrId = GetLastError();
		if(dwErrId != 17 && dwErrId != 183)
		{
			return FALSE;
		}
	}

	cstrLogFile.Format("%s\\%s.txt",(LPCTSTR)strPath,cTime.Format("%Y%m%d%H%M%S"));
	if(!m_cLogFile.Open((LPCTSTR)cstrLogFile
		,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite|CFile::shareDenyNone))
	{
		m_bLogFile = FALSE;	
	}

	if(InitRecorder())
	{
			//ReadDisc();
	}
	else
	{
		::AfxMessageBox(_T("没有找到可用光驱"));
		PostQuitMessage(0);
	}
	RecorderFile(m_cstrFilePath,m_cstrCurDriver); 
	return FALSE;
}

BOOL CCDBurnApp::ReadDisc()
{
	CDiscFormatData discFormatData; 
	if(discFormatData.Initialize(m_pCurDiscRecorder,_T("CDBurn")))
	{

	}
	return true;
}
BOOL CCDBurnApp::InitRecorder()
{
	int i ;
	CDiscMaster m_discMaster;
	CDiscRecorder *pDiscRecorder = NULL;


	if(!m_discMaster.Initialize())  // 加载失败，推出程序
	{
		if(m_bLogFile)
		{
			m_cLogFile.Write(m_discMaster.GetErrorMessage(),m_discMaster.GetErrorMessage().GetLength());
		}
		return FALSE; 
	}

	long nDeviceCount = m_discMaster.GetTotalDevices();
	for(i = 0 ; i < nDeviceCount ; i++)
	{
		CString cstrDevID = m_discMaster.GetDeviceUniqueID(i);
		if(cstrDevID.IsEmpty())
		{
			// 			CString strErr; 
			// 			strErr.Format(_T("无法获取设备ID"));
			// 			if(m_bLogFile)
			// 			{
			// 				m_cLogFile.Write(strErr,strErr.GetLength());
			// 			}
			continue ; 
		}
		pDiscRecorder = new CDiscRecorder;

		if(!pDiscRecorder->Initialize(cstrDevID))
		{
			// 			CString strErr ; 
			// 			strErr.Format(_T("初始化光驱异常,错误号：0x%x"),pDiscRecorder->GetHresult());
			// 			if(m_bLogFile)
			// 			{
			// 				m_cLogFile.Write(strErr,strErr.GetLength());
			// 			}

			delete pDiscRecorder;
			continue;
		}


		CString volumeList;
		ULONG totalVolumePaths = pDiscRecorder->GetTotalVolumePaths();
		for (ULONG volIndex = 0; volIndex < totalVolumePaths; volIndex++)
		{
			if (volIndex)
				volumeList += _T(",");
			volumeList += pDiscRecorder->GetVolumePath(volIndex);
		}

		CString productId = pDiscRecorder->GetProductID();
		CString strName;
		strName.Format(_T("%s [%s]"), (LPCTSTR)volumeList, (LPCTSTR)productId);
		CDiscFormatData discFormatData ; 
		if(!discFormatData.Initialize(pDiscRecorder,_T("Burn")))
		{
			if(m_bLogFile)
			{
				CString strErr ; 
				strErr = discFormatData.GetErrorMessage();
				m_cLogFile.Write(strErr,strErr.GetLength()); 
			}
			delete pDiscRecorder;
			continue; 		
		}
		if(m_bLogFile)
		{
			CString strTemp ;
			strTemp.Format(_T("找到可用光驱：%s") , strName);
			m_cLogFile.Write(strTemp,strTemp.GetLength());
		}
		pDiscRecorder->m_cstrRecorderName = strName;
		m_cstrCurDriver = strName;
		m_listDiscRecorder.AddTail(pDiscRecorder); 

	}

	if(m_listDiscRecorder.GetCount() == 0)
	{
		if(m_bLogFile)
		{
			CString strErr ; 
			strErr = _T("没有搜索到光驱");
			m_cLogFile.Write(strErr,strErr.GetLength());
		}
		return FALSE;
	}
	return TRUE;
}

CString GetMediaStatusErrorCode(long hr)
{
	CString m_cstrErr;
	switch(hr)
	{
	case E_INVALIDARG:
		m_cstrErr = _T("参数无效");
		break;
	case E_POINTER:
		m_cstrErr = _T("指针无效");
		break ;
	case E_FAIL:
		m_cstrErr = _T("获取状态失败");
		break ;
	case E_OUTOFMEMORY:
		m_cstrErr = _T("内存溢出");
		break ;
	case E_IMAPI_RECORDER_COMMAND_TIMEOUT:
		m_cstrErr = _T("超时");
		break ;
	case E_IMAPI_RECORDER_INVALID_RESPONSE_FROM_DEVICE:
		m_cstrErr = _T("异常或无效数据");
		break ;
	case E_IMAPI_RECORDER_MEDIA_UPSIDE_DOWN:
		m_cstrErr = _T("光盘正反面可能颠倒");
		break ;
	case E_IMAPI_RECORDER_MEDIA_BECOMING_READY:
		m_cstrErr = _T("光驱正在准备，请稍后再继续操作");
		break ;
	case E_IMAPI_RECORDER_MEDIA_NO_MEDIA:
		m_cstrErr = _T("光驱中无光盘或者光驱已弹出");
		break ;
	case E_IMAPI_RECORDER_MEDIA_FORMAT_IN_PROGRESS:
		m_cstrErr = _T("光盘正在格式化");
		break;
	case E_IMAPI_RECORDER_MEDIA_BUSY:
		m_cstrErr = _T("光驱工作时间过长，需要暂停工作");
		break ;
	case E_IMAPI_LOSS_OF_STREAMING:
		m_cstrErr = _T("写操作失败，因为长时间没有提供写入的数据");
		break;
	case E_IMAPI_RECORDER_MEDIA_INCOMPATIBLE:
		m_cstrErr = _T("不兼容或者格式未知");
		break ;
	case E_IMAPI_RECORDER_DVD_STRUCTURE_NOT_PRESENT:
		m_cstrErr = _T("DVD未正常工作，可能是不兼容的媒体格式");
		break ;
	case E_IMAPI_RECORDER_NO_SUCH_MODE_PAGE:
		m_cstrErr = _T("The requested mode page (and type) is not present");
		break ;
	case E_IMAPI_RECORDER_INVALID_MODE_PARAMETERS:
		m_cstrErr = _T("The drive reported that the combination of parameters provided in the mode page for a MODE SELECT command were not supported.");
		break ;
	case E_IMAPI_RECORDER_MEDIA_WRITE_PROTECTED:
		m_cstrErr = _T("光盘已经写保护");
		break ;
	case E_IMAPI_RECORDER_MEDIA_SPEED_MISMATCH:
		m_cstrErr = _T("刻录的速度和光驱的不匹配");
		break ; 
	case ERROR_INVALID_HANDLE:
		m_cstrErr = _T("The specified handle is invalid");
		break ;
	case ERROR_DEV_NOT_EXIST:
		m_cstrErr = _T("光驱可能被卸载");
		break ; 
	case E_IMAPI_RECORDER_LOCKED:
		m_cstrErr = _T("不能进行并行操作");
		break ; 
	}

	CString strTemp;
	strTemp.Format(_T("错误号：%x\r\n"),hr);
	m_cstrErr += strTemp;
	// 	m_cstrErr.AppendFormat(_T("  错误号：0x%x"),hr);
	return m_cstrErr;
}

CString GetCurrentMediaStatusCode(IMAPI_FORMAT2_DATA_MEDIA_STATE mStates , bool *bWrite)
{
	CString strStates;

	*bWrite = TRUE;

	if(mStates == IMAPI_FORMAT2_DATA_MEDIA_STATE_UNKNOWN)
	{
		strStates = _T("未知的媒体状态");
		*bWrite = false;
	}
	//if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_INFORMATIONAL_MASK) == IMAPI_FORMAT2_DATA_MEDIA_STATE_INFORMATIONAL_MASK)
	//{
	//	strStates += _T("  正在读取光盘状态，可能还需要再次调用接口以正确获取状态");
	//}
	if(((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MASK) == IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MASK) || ((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MEDIA) == IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MEDIA))
	{
		strStates += _T("  不支持的媒体状态");
		*bWrite = false;
	}
	if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_OVERWRITE_ONLY) == IMAPI_FORMAT2_DATA_MEDIA_STATE_OVERWRITE_ONLY)
	{
		strStates += _T("  写操作可以在光盘已写过的地方,即可以覆盖写");
	}
	if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_BLANK) == IMAPI_FORMAT2_DATA_MEDIA_STATE_BLANK)
	{
		strStates += _T("  光盘是空的或者已经被擦出过");
	}
	if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_APPENDABLE) == IMAPI_FORMAT2_DATA_MEDIA_STATE_APPENDABLE)
	{
		strStates += _T("  光盘可追加写入");
	}
	if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_FINAL_SESSION) == IMAPI_FORMAT2_DATA_MEDIA_STATE_FINAL_SESSION)
	{
		strStates += _T("  光盘可以被刻录一次，以后就不可以往光盘上增加刻录其他数据");
	}
	if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_DAMAGED) == IMAPI_FORMAT2_DATA_MEDIA_STATE_DAMAGED)
	{
		strStates += _T("  接口不可使用此光盘，光盘可能需要其他的工具覆写或者擦出");
		*bWrite = false;
	}
	if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_ERASE_REQUIRED) == IMAPI_FORMAT2_DATA_MEDIA_STATE_ERASE_REQUIRED)
	{
		strStates += _T("  光盘需要先擦出再使用,此接口可擦出光盘");
		*bWrite = false;
	}
	if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_NON_EMPTY_SESSION) == IMAPI_FORMAT2_DATA_MEDIA_STATE_NON_EMPTY_SESSION)
	{
		strStates += _T("  光盘已经被写过了，不可以再次写入");
		*bWrite = false;
	}
	if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_WRITE_PROTECTED) == IMAPI_FORMAT2_DATA_MEDIA_STATE_WRITE_PROTECTED)
	{
		strStates += _T("  光盘或者光驱已经写保护");
		*bWrite = false;
	}
	if((mStates & IMAPI_FORMAT2_DATA_MEDIA_STATE_FINALIZED) == IMAPI_FORMAT2_DATA_MEDIA_STATE_FINALIZED)
	{
		strStates += _T("  光盘无法写入（定稿）");
		*bWrite = false;
	}

	strStates += "\r\n";

	return strStates;
}
BOOL CCDBurnApp::RecorderFile(CString FileName, CString CurRecorderName)
{
	IStream *pFileStream = NULL; 
	CDiscRecorder *pDiscRecorder = NULL ;
	POSITION pos = m_listDiscRecorder.GetHeadPosition(); 
	while(pos)
	{
		pDiscRecorder = m_listDiscRecorder.GetNext(pos); 
		if(pDiscRecorder->m_cstrRecorderName == CurRecorderName)
			break; 
		pDiscRecorder = NULL; 
	}

	if(pDiscRecorder == NULL)
	{
		if(m_bLogFile )
		{
			CString strErr = _T("程序出现异常，未找到光驱"); 
			m_cLogFile.Write(strErr,strErr.GetLength()); 
		}
		return false;
	}

	CDiscFormatData discFormatData; 
	if(!discFormatData.Initialize(pDiscRecorder,_T("CDBurn")))
	{
		if(m_bLogFile)
		{
			m_cLogFile.Write(discFormatData.GetErrorMessage(),discFormatData.GetErrorMessage().GetLength()); 
		}
		return false; 
	}

	bool bWrite;
	CString strErr;
	CString strStates;
	HRESULT hr; 

	IMAPI_FORMAT2_DATA_MEDIA_STATE iState = IMAPI_FORMAT2_DATA_MEDIA_STATE_UNKNOWN;

	// 	SAFEARRAY *pSafeArrayModePages = NULL;
	// 	pDiscRecorder->GetInterface()->get_SupportedModePages(&pSafeArrayModePages); // 没有读到任何属性


	while(TRUE)
	{
		hr = discFormatData.GetInterface()->get_CurrentMediaStatus( &iState);  // 

		if(!SUCCEEDED(hr))
		{
			strErr = GetMediaStatusErrorCode(hr);
			if(m_bLogFile)
			{
				m_cLogFile.Write(strErr,strErr.GetLength()); 
			}
			if(hr == E_IMAPI_RECORDER_MEDIA_UPSIDE_DOWN || hr == E_IMAPI_RECORDER_MEDIA_BECOMING_READY || 
				hr == E_IMAPI_RECORDER_MEDIA_NO_MEDIA || hr == E_IMAPI_RECORDER_MEDIA_FORMAT_IN_PROGRESS)
			{
				Sleep(10000); 
				continue;  // 此处的异常，可以通过等待以实现以后的继续刻录
			}
			return false; 
			//ASSERT(false);
		}
		else
		{
			if(iState == IMAPI_FORMAT2_DATA_MEDIA_STATE_UNKNOWN || iState == IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MASK || 
				iState == IMAPI_FORMAT2_DATA_MEDIA_STATE_UNSUPPORTED_MEDIA || iState == IMAPI_FORMAT2_DATA_MEDIA_STATE_DAMAGED || 
				iState == IMAPI_FORMAT2_DATA_MEDIA_STATE_ERASE_REQUIRED || iState == IMAPI_FORMAT2_DATA_MEDIA_STATE_WRITE_PROTECTED ||
				iState == IMAPI_FORMAT2_DATA_MEDIA_STATE_FINALIZED || iState == IMAPI_FORMAT2_DATA_MEDIA_STATE_NON_EMPTY_SESSION)
			{
				strStates = GetCurrentMediaStatusCode(iState,&bWrite);
				if(m_bLogFile)
				{
					m_cLogFile.Write(strStates,strStates.GetLength());
				}
				return false; 
			}
			if((iState & IMAPI_FORMAT2_DATA_MEDIA_STATE_OVERWRITE_ONLY) == IMAPI_FORMAT2_DATA_MEDIA_STATE_OVERWRITE_ONLY)
			{
				strStates = _T("光盘已经刻录过\r\n");
				if(m_bLogFile)
				{
					m_cLogFile.Write(strStates,strStates.GetLength());
				}
				return false;
			}
			break;
		}
	}


	IMAPI_MEDIA_PHYSICAL_TYPE mediaType = IMAPI_MEDIA_TYPE_UNKNOWN; 
	discFormatData.GetInterface()->get_CurrentPhysicalMediaType(&mediaType); // 光盘的格式

	IFileSystemImage*		image = NULL;
	IFileSystemImageResult*	imageResult = NULL;
	IFsiDirectoryItem*		rootItem = NULL;
	CString					message;
	bool					returnVal = false;

	hr = CoCreateInstance(CLSID_MsftFileSystemImage,
		NULL, CLSCTX_ALL, __uuidof(IFileSystemImage), (void**)&image);
	if (FAILED(hr) || (image == NULL))
	{
		CString strErr; 
		strErr = _T("刻录文件创建流时失败");
		if(m_bLogFile)
		{
			m_cLogFile.Write(strErr,strErr.GetLength()); 
		}
		return false;
	}


	hr = image->put_FileSystemsToCreate((FsiFileSystems)(FsiFileSystemJoliet|FsiFileSystemISO9660));
	if(!SUCCEEDED(hr))
	{
		CString strErr ; 
		strErr.Format(_T("Create file system failed! error code : 0x%d"),hr);
		if(m_bLogFile)
		{
			m_cLogFile.Write(strErr,strErr.GetLength()); 
		}
		return false; 
	}
	CDirObject fileObj(FileName); 
	CString strFileName = fileObj.GetName();
	//	*(char*)strrchr(strFileName.GetBuffer(0),'\.') = '\0';

	//    image->put_VolumeName(strFileName.AllocSysString()); // 镜像文件名称
	//  image->ChooseImageDefaultsForMediaType(mediaType);

	SAFEARRAY* pSafeArrayMulSession = NULL;
	VARIANT_BOOL vbBlank;
	IMultisession *pMultisession = NULL;   

	discFormatData.GetInterface()->get_MediaHeuristicallyBlank(&vbBlank);
	hr = image->put_FileSystemsToCreate((FsiFileSystems)(FsiFileSystemJoliet|FsiFileSystemISO9660));
	if(!SUCCEEDED(hr))
	{
		return FALSE; 
	}
	image->ChooseImageDefaultsForMediaType(mediaType);

	if(vbBlank == VARIANT_TRUE) 
	{  
		//	image->put_FileSystemsToCreate((FsiFileSystems)(FsiFileSystemJoliet|FsiFileSystemISO9660));  
		//	image->put_VolumeName(pThis->m_volumeLabel.AllocSysString());  
		BSTR bstrFileName = strFileName.AllocSysString();
		image->put_VolumeName(bstrFileName); 
		SysFreeString(bstrFileName);		
	}  else  
	{  	
		SAFEARRAY *pSafeArrayMulSection = NULL;
		HRESULT hrTemp = discFormatData.GetInterface()->get_MultisessionInterfaces(&pSafeArrayMulSection);
		if(!SUCCEEDED(hrTemp))
		{			
			return FALSE; 
		}
		long vale = 0;
		hrTemp = discFormatData.GetInterface()->get_NextWritableAddress(&vale);
		if(SUCCEEDED(hrTemp))
		{
			hrTemp = image->put_MultisessionInterfaces(pSafeArrayMulSection);
			VARIANT *va = NULL ;
			SafeArrayAccessData(pSafeArrayMulSection,(VOID**)&va); 
			if(va)
			{
				IMultisession *pMultisession = NULL ; 
				pMultisession = (IMultisession*)va[0].pdispVal;
				hr = pMultisession->put_InUse(VARIANT_TRUE); 
				if(!SUCCEEDED(hr))
				{
					return FALSE;
				}
			}
			//	FsiFileSystems fs; 
			//	hrTemp = image->GetDefaultFileSystemForImport((FsiFileSystems)(FsiFileSystemJoliet|FsiFileSystemISO9660),&fs); 
			FsiFileSystems filesystem ; 
			//encounter IMAPI_E_IMPORT_SEEK_FAILURE or IMAPI_E_NO_SUPPORTED_FILE_SYSTEM
			hrTemp = image->ImportFileSystem(&filesystem);
			if(!SUCCEEDED(hrTemp))
			{
				SafeArrayDestroy(pSafeArrayMulSection); 

				return FALSE; 
			}
		}// get_multisession OK
		SafeArrayDestroy(pSafeArrayMulSection); 
	}
	LONG lFreeSectorOnMedia = 0;
	if(SUCCEEDED(hr))
	{
		hr = discFormatData.GetInterface()->get_FreeSectorsOnMedia(&lFreeSectorOnMedia); 
		if(SUCCEEDED(hr))
		{
			hr = image->put_FreeMediaBlocks(lFreeSectorOnMedia); 
		}
	}

	//	image->get_FreeMediaBlocks(&m_nFreeBlock); // 获取光盘的容积
	m_ullFree = (ULONGLONG)lFreeSectorOnMedia * 2048 ; // 转换成字节数

	ULONGLONG ullFileSize = fileObj.GetSizeOnDisc();

	if(ullFileSize > m_ullFree - 1024000)  // 文件大小必须要小于光盘近 1M 大小
	{
		CString strErr; 
		strErr = _T("文件太大，刻录可能导致光盘损坏，以后无法读取数据"); 
		if(m_bLogFile)
		{
			m_cLogFile.Write(strErr,strErr.GetLength()); 
		}
		return false; 
	}

	// 到此 光盘有足够空间可以容纳指定的文件
	hr = image->get_Root(&rootItem);
	if(SUCCEEDED(hr))
	{
		//	IStream* fileStream = fileObj.GetStream();
		CString  strFilePath = FileName;
		hr = rootItem->AddTree(strFilePath.AllocSysString(),VARIANT_TRUE); 
		if(!SUCCEEDED(hr))
		{
			// IMAPI_E_IMAGE_SIZE_LIMIT 0xc0aab120
			CString strErr; 
			strErr = _T("Create file to stream failed!"); 
			if(m_bLogFile)
			{
				m_cLogFile.Write(strErr,strErr.GetLength()); 
			}
			return  false; 
		}

		hr = image->CreateResultImage(&imageResult);
		if(SUCCEEDED(hr))
		{
			hr = imageResult->get_ImageStream(&pFileStream);
			if(!SUCCEEDED(hr))
			{
				CString strErr; 
				strErr.Format(_T("Create image stream failed! Error code：0x%d"),hr);
				if(m_bLogFile)
				{
					m_cLogFile.Write(strErr,strErr.GetLength()); 
				}
				return false; 
			}

		}
		else
		{
			CString strErr; 
			strErr.Format(_T("Create image stream failed! Error code：0x%d"),hr);
			if(m_bLogFile)
			{
				m_cLogFile.Write(strErr,strErr.GetLength()); 
			}
			return false; 
		}
	}
	else
	{
		if(hr == 0x8007000E)
		{
			CString strErr = _T("Failed IFileSystemImage->getRoot.Failed to allocate the required memory. ");
		}
		else if( hr == E_POINTER)
		{
			CString strErr = _T("Failed IFileSystemImage->getRoot.Pointer is not valid. ");
		}

		if(m_bLogFile)
		{
			m_cLogFile.Write(strErr,strErr.GetLength()); 
		}
		return false; 
	}

	if(image != NULL)
	{
		image->Release();
	}
	if (imageResult != NULL)
	{
		imageResult->Release();
	}
	if (rootItem != NULL)
	{
		rootItem->Release();
	}

	discFormatData.SetCloseMedia(TRUE); 

	if(m_bLogFile)
	{
		CString strLog; 
		strLog = _T("开始刻录数据 \n");
		m_cLogFile.Write(strLog,strLog.GetLength());
	}

	if(discFormatData.Burn(NULL,pFileStream))
	{
		if(m_bLogFile)
		{
			CString strTemp; 
			strTemp = _T("刻录完成");
			m_cLogFile.Write(strTemp,strTemp.GetLength()); 
		}
	}
	else if(m_bLogFile)
	{
		m_cLogFile.Write(discFormatData.GetErrorMessage(),discFormatData.GetErrorMessage().GetLength());
		AfxMessageBox(_T("刻录失败")); 

	}
	pFileStream->Release();
	if(m_bPopupDrivers)
	{
		pDiscRecorder->EjectMedia();
	}

	return TRUE;
}
