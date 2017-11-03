///////////////////////////////////////////////////////////////////////
// DiscFormatData.cpp
//
// Wrapper for IDiscFormat2Data Interface
//
// Written by Eric Haddan
//

#include "StdAfx.h"
#include "DiscFormatData.h"
#include "DiscRecorder.h"
#include "DiscFormatDataEvent.h"

CDiscFormatData::CDiscFormatData(void)
: m_discFormatData(NULL)
, m_mediaTypesArray(NULL)
, m_hResult(0)
// , m_hNotificationWnd(NULL)
, m_closeMedia(true)
{
}

CDiscFormatData::~CDiscFormatData(void)
{
	if (m_discFormatData != NULL)
	{
		m_discFormatData->Release();
		m_discFormatData = NULL;
	}
}

///////////////////////////////////////////////////////////////////////
//
// CDiscFormatData::Initialize()
//
// Description:
//		Creates and initializes the IDiscFormat2Data interface
//
bool CDiscFormatData::Initialize(CDiscRecorder* pDiscRecorder, const CString& clientName)
{
	ASSERT(m_discFormatData == NULL);
	ASSERT(pDiscRecorder != NULL);
	if (pDiscRecorder == NULL)
	{
		m_errorMessage = _T("Error - CDiscFormatData::Initialize - pDiscRecorder is NULL");
		return false;
	}

	//
	// Initialize the IDiscFormat2Data Interface
	//
	m_hResult = CoCreateInstance(__uuidof(MsftDiscFormat2Data), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiscFormat2Data), (void**)&m_discFormatData);
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("Unable to Initialize IDiscFormat2Data - Error:0x%08x"), m_hResult);
		return false;
	}

	//
	// Setup the Disc Format Information
	//
	m_hResult = m_discFormatData->get_SupportedMediaTypes(&m_mediaTypesArray); // 获取本系统内所有光驱支持的媒体格式 两个光驱则是罗列两个光驱的所有媒体格式
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscFormat2Data->get_SupportedMediaTypes Failed - Error:0x%08x"), m_hResult);
		return false;
	}

	//SAFEARRAY*	pSafeArray;
	m_hResult = pDiscRecorder->GetInterface()->get_SupportedProfiles(&m_mediaTypesArray);  // 此时是获取一个光驱的支持媒体格式
	if(!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscRecorder2->get_SupportedProfiles Failed - Error:0x%08x"), m_hResult);
		return false;
	}


	VARIANT_BOOL isSupported = VARIANT_FALSE;
	m_hResult = m_discFormatData->IsRecorderSupported(pDiscRecorder->GetInterface(), &isSupported);  // 具体含义不清楚，只知道，如果是支持刻录的，将返回Supported，否则不支持
	if (isSupported == VARIANT_FALSE)
	{
		m_errorMessage = _T("Recorder not supported\r\n");  // maybe 检索刻录机是否支持刻录 数据流 形式的数据 
		return false;
	}


	m_hResult = m_discFormatData->put_Recorder(pDiscRecorder->GetInterface());
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscFormat2Data->put_Recorder Failed - Error:0x%08x"), m_hResult);
		DWORD dwLastErr = GetLastError();
		return false;
	}

	m_hResult = m_discFormatData->put_ClientName(clientName.AllocSysString());
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscFormat2Data->put_ClientName Failed - Error:0x%08x"), m_hResult);
		return false;
	}


	return true;
}

ULONG CDiscFormatData::GetTotalSupportedMediaTypes()
{
	if (m_mediaTypesArray == NULL)
		return 0;

	return m_mediaTypesArray->rgsabound[0].cElements;
}

int CDiscFormatData::GetSupportedMediaType(ULONG index)
{
	ASSERT(index < GetTotalSupportedMediaTypes());
	if (index < GetTotalSupportedMediaTypes())
	{
		if (m_mediaTypesArray)
		{
			return ((VARIANT*)(m_mediaTypesArray->pvData))[index].intVal;
		}
	}

	return 0;
}

bool CDiscFormatData::Burn(HWND hNotificationWnd, IStream* streamData)
{
	if (m_discFormatData == NULL)
		return false;

// 	if (hNotificationWnd == NULL)
// 		return false;

	if (streamData == NULL)
		return false;

	m_streamData = streamData;
// 	m_hNotificationWnd = hNotificationWnd;

	// Create the event sink
	CDiscFormatDataEvent* eventSink = CDiscFormatDataEvent::CreateEventSink();
	if (eventSink == NULL)
	{
		m_errorMessage = _T("Unable to create event sink");
		return false;
	}

	if (!eventSink->ConnectDiscFormatData(this))
	{
		m_errorMessage = _T("Unable to connect event sink with interface");
		return false;
	}

//	eventSink->SetHwnd(m_hNotificationWnd);

//	m_discFormatData->put_ForceMediaToBeClosed( VARIANT_TRUE ); 

	m_hResult = m_discFormatData->Write(m_streamData);

	delete eventSink;

	if (SUCCEEDED(m_hResult))
	{
		return true;
	}

	m_errorMessage = GetWriteErrorMsg(m_hResult);
	m_errorMessage.Format(_T("IDiscFormat2Data->Write Failed! Error:0x%08x"),
		m_hResult);

	return false;

}

CString CDiscFormatData::GetWriteErrorMsg(HRESULT hr)
{
	CString cstrWriteError;
	switch(hr)
	{
	case 0xC0AA020D:
		cstrWriteError = _T("超时");
		break;
	case 0xC0AA02FF:
		cstrWriteError = _T("异常或者数据无效");
		break;
	case 0xC0AA0204:
		cstrWriteError = _T("光盘放颠倒了");
		break;
	case 0xC0AA0205:
		cstrWriteError = _T("准备中，稍后可能需要继续调用Write函数");
		break;
	case 0xC0AA0202:
		cstrWriteError = _T("光驱中无光盘");
		break;
	case 0xC0AA0206:
		cstrWriteError = _T("media正在格式化，请稍等");
		break;
	case 0xC0AA0207:
		cstrWriteError = _T("光驱可能已经长时间工作，现在需要停止工作一会");
		break;
	case 0xC0AA0203:
		cstrWriteError = _T("光盘不兼容或者未知的物理格式");
		break;
	case 0xC0AA0201:
		cstrWriteError = _T("光驱要求的页模式，应用程序未提供");
		break;
	case 0xC0AA0208:
		cstrWriteError = _T("模式页不支持");
		break;
	case 0xC0AA0209:
		cstrWriteError = _T("光盘写保护");
		break;
	case 0xC0AA020F:
		cstrWriteError = _T("写入的速度不匹配");
		break;
	case 6:
		cstrWriteError = _T("句柄无效");
		break;
	case 55:
		cstrWriteError = _T("网络异常或者光驱被卸载");
		break;
	case 0xC0AA0210:
		cstrWriteError = _T("光驱正在被其他的写入独占");  // 
		break;
	case 0xC0AA0301:
		cstrWriteError = _T("光驱报告异常");
		break;
	case 0xC0AA0003:
		cstrWriteError = _T("写入动作没有指定光驱");
		break;
	case 0x00AA0005:
		cstrWriteError = _T("要求的旋转时刻录，光驱不支持");
		break;
	case 0x00AA0004:
		cstrWriteError = _T("要求的刻录速度光驱不支持，光驱已自行调整刻录速度");
		break;
	case 0x00AA0006:
		cstrWriteError = _T("要求的旋转刻录和刻录速度不支持，光驱已经自行匹配");
		break;
	case 0xC0AA0407:
		cstrWriteError = _T("光驱不支持光盘的格式");
		break;
	case 0xC0AA0002:
		cstrWriteError = _T("操作取消");
		break;
	case 0xC0AA0400:
		cstrWriteError = _T("另一个写入操作与此操作冲突"); //
		break;
	case 0xC0AA0403:
		cstrWriteError = _T("提供的IStream大小无效.The size must be a multiple of the sector size, 2048.");
		break;
	case 0x80070057:
		cstrWriteError = _T("一个或多个操作无效");
		break;
	case 0x80004003:
		cstrWriteError = _T("指针无效");
		break;
	case 0x80004005:
		cstrWriteError = _T("未知的失败");
		break;
	case 0x8007000E:
		cstrWriteError = _T("内存不足");
		break;
	case 0x80004001:
		cstrWriteError = _T("未知");
		break;
	case 0xC0AA0300:
		cstrWriteError = _T("The write failed because the drive did not receive data quickly enough to continue writing");
		break;
	case 0xC0AA020E:
		cstrWriteError = _T("DVD设备未找到");
		break;
	}
	CString strTemp; 
	strTemp .Format(_T(" 错误号：0x%x,"),hr);
	cstrWriteError += strTemp;
	return cstrWriteError;
}
