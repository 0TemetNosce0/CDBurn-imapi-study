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
	m_hResult = m_discFormatData->get_SupportedMediaTypes(&m_mediaTypesArray); // ��ȡ��ϵͳ�����й���֧�ֵ�ý���ʽ ������������������������������ý���ʽ
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscFormat2Data->get_SupportedMediaTypes Failed - Error:0x%08x"), m_hResult);
		return false;
	}

	//SAFEARRAY*	pSafeArray;
	m_hResult = pDiscRecorder->GetInterface()->get_SupportedProfiles(&m_mediaTypesArray);  // ��ʱ�ǻ�ȡһ��������֧��ý���ʽ
	if(!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscRecorder2->get_SupportedProfiles Failed - Error:0x%08x"), m_hResult);
		return false;
	}


	VARIANT_BOOL isSupported = VARIANT_FALSE;
	m_hResult = m_discFormatData->IsRecorderSupported(pDiscRecorder->GetInterface(), &isSupported);  // ���庬�岻�����ֻ֪���������֧�ֿ�¼�ģ�������Supported������֧��
	if (isSupported == VARIANT_FALSE)
	{
		m_errorMessage = _T("Recorder not supported\r\n");  // maybe ������¼���Ƿ�֧�ֿ�¼ ������ ��ʽ������ 
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
		cstrWriteError = _T("��ʱ");
		break;
	case 0xC0AA02FF:
		cstrWriteError = _T("�쳣����������Ч");
		break;
	case 0xC0AA0204:
		cstrWriteError = _T("���̷ŵߵ���");
		break;
	case 0xC0AA0205:
		cstrWriteError = _T("׼���У��Ժ������Ҫ��������Write����");
		break;
	case 0xC0AA0202:
		cstrWriteError = _T("�������޹���");
		break;
	case 0xC0AA0206:
		cstrWriteError = _T("media���ڸ�ʽ�������Ե�");
		break;
	case 0xC0AA0207:
		cstrWriteError = _T("���������Ѿ���ʱ�乤����������Ҫֹͣ����һ��");
		break;
	case 0xC0AA0203:
		cstrWriteError = _T("���̲����ݻ���δ֪�������ʽ");
		break;
	case 0xC0AA0201:
		cstrWriteError = _T("����Ҫ���ҳģʽ��Ӧ�ó���δ�ṩ");
		break;
	case 0xC0AA0208:
		cstrWriteError = _T("ģʽҳ��֧��");
		break;
	case 0xC0AA0209:
		cstrWriteError = _T("����д����");
		break;
	case 0xC0AA020F:
		cstrWriteError = _T("д����ٶȲ�ƥ��");
		break;
	case 6:
		cstrWriteError = _T("�����Ч");
		break;
	case 55:
		cstrWriteError = _T("�����쳣���߹�����ж��");
		break;
	case 0xC0AA0210:
		cstrWriteError = _T("�������ڱ�������д���ռ");  // 
		break;
	case 0xC0AA0301:
		cstrWriteError = _T("���������쳣");
		break;
	case 0xC0AA0003:
		cstrWriteError = _T("д�붯��û��ָ������");
		break;
	case 0x00AA0005:
		cstrWriteError = _T("Ҫ�����תʱ��¼��������֧��");
		break;
	case 0x00AA0004:
		cstrWriteError = _T("Ҫ��Ŀ�¼�ٶȹ�����֧�֣����������е�����¼�ٶ�");
		break;
	case 0x00AA0006:
		cstrWriteError = _T("Ҫ�����ת��¼�Ϳ�¼�ٶȲ�֧�֣������Ѿ�����ƥ��");
		break;
	case 0xC0AA0407:
		cstrWriteError = _T("������֧�ֹ��̵ĸ�ʽ");
		break;
	case 0xC0AA0002:
		cstrWriteError = _T("����ȡ��");
		break;
	case 0xC0AA0400:
		cstrWriteError = _T("��һ��д�������˲�����ͻ"); //
		break;
	case 0xC0AA0403:
		cstrWriteError = _T("�ṩ��IStream��С��Ч.The size must be a multiple of the sector size, 2048.");
		break;
	case 0x80070057:
		cstrWriteError = _T("һ������������Ч");
		break;
	case 0x80004003:
		cstrWriteError = _T("ָ����Ч");
		break;
	case 0x80004005:
		cstrWriteError = _T("δ֪��ʧ��");
		break;
	case 0x8007000E:
		cstrWriteError = _T("�ڴ治��");
		break;
	case 0x80004001:
		cstrWriteError = _T("δ֪");
		break;
	case 0xC0AA0300:
		cstrWriteError = _T("The write failed because the drive did not receive data quickly enough to continue writing");
		break;
	case 0xC0AA020E:
		cstrWriteError = _T("DVD�豸δ�ҵ�");
		break;
	}
	CString strTemp; 
	strTemp .Format(_T(" ����ţ�0x%x,"),hr);
	cstrWriteError += strTemp;
	return cstrWriteError;
}
