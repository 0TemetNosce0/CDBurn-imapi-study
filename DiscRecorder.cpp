///////////////////////////////////////////////////////////////////////
// DiscRecorder.cpp
//
// Wrapper for IDiscRecorder2 Interface
//
// Written by Eric Haddan
//
#include "StdAfx.h"
#include "DiscRecorder.h"

#ifndef CDROM_EXCLUSIVE_CALLER_LENGTH
#define CDROM_EXCLUSIVE_CALLER_LENGTH 64
#endif

CDiscRecorder::CDiscRecorder(void)
: m_discRecorder(NULL)
, m_volumePathNames(NULL)
{
}

CDiscRecorder::~CDiscRecorder(void)
{
	if (m_discRecorder != NULL)
	{
		m_discRecorder->Release();
		m_discRecorder = NULL;
	}
}


///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::Initialize()
//
// Description:
//		Creates and initializes the IDiscRecorder2 interface
//
// Parameters:
//		recorderUniqueId	The recorder's unique id retrieved from
//							the IDiscMaster2 interface.
//
bool CDiscRecorder::Initialize(const CString& recorderUniqueId)
{
	m_recorderUniqueId = recorderUniqueId;

	//
	// Create an IDiscRecorder2
	//
	m_hResult = CoCreateInstance(__uuidof(MsftDiscRecorder2), NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IDiscRecorder2), (void**)&m_discRecorder);
	ASSERT(SUCCEEDED(m_hResult));
	if (FAILED(m_hResult))
	{
		return false;
	}

	m_hResult = m_discRecorder->InitializeDiscRecorder(recorderUniqueId.AllocSysString());
	if (FAILED(m_hResult))
	{
		return false;
	}

	//
	// Get the volume name paths
	//

	/**********���½�����ʾ��֧�ֿ�¼�Ĺ���

	//SAFEARRAY *pSafeArray;
	//m_discRecorder->get_SupportedProfiles(&pSafeArray);

	//if(pSafeArray == NULL)
	//{
	//	return false;
	//}

	//ULONG i = 0; 

	//for( i = 0; i < pSafeArray->rgsabound[0].cElements; i++)
	//{
	//	int nData = ((VARIANT*)pSafeArray->pvData)[i].intVal;
	//	if(nData == IMAPI_MEDIA_TYPE_CDRW || nData == IMAPI_MEDIA_TYPE_DVDPLUSRW || nData == IMAPI_MEDIA_TYPE_DVDDASHRW 
	//		|| nData == IMAPI_MEDIA_TYPE_DISK || nData == IMAPI_MEDIA_TYPE_DVDPLUSRW_DUALLAYER || nData == IMAPI_MEDIA_TYPE_BDRE)
	//	{
	//		break;
	//	}
	//}

	//if(i >= pSafeArray->rgsabound[0].cElements)
	//{
	//	return false;
	//}
	************/

	m_hResult = m_discRecorder->get_VolumePathNames(&m_volumePathNames);
	//BSTR bstr;
	//m_discRecorder->get_VolumeName(&bstr);  // �˺�����ù�����ID�����Ǵ����̷���
	if(!SUCCEEDED(m_hResult))
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::AcquireExclusiveAccess()
//
// Description:
//		Acquires exclusive access to this device
//
// Parameters:
//		force		forces exclusive access whether or not the device
//					can be dismounted
//		clientName	The name of the client application
//
bool CDiscRecorder::AcquireExclusiveAccess(bool force, const CString& clientName)
{
	int length = clientName.GetLength();
	ASSERT(!clientName.IsEmpty());
	ASSERT(length < CDROM_EXCLUSIVE_CALLER_LENGTH);

	if (length == 0 || length >= CDROM_EXCLUSIVE_CALLER_LENGTH)
	{
		return false;
	}

	for (int index = 0; index < length; index++)
	{
		TCHAR ch = clientName[index];
		if (_istalnum(ch) || ch == _T(' ') || ch == _T('.') || ch == _T(',') ||
			ch == _T(':') || ch == _T(';') || ch == _T('-') || ch == _T('_'))
		{
			continue;
		}

		//
		// Client name does not meet specification
		//
		ASSERT(FALSE);
		return false;
	}

	if (m_discRecorder != NULL)
	{
		m_hResult = m_discRecorder->AcquireExclusiveAccess(
			force ? VARIANT_TRUE : VARIANT_FALSE,
			clientName.AllocSysString());
		if (SUCCEEDED(m_hResult))
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::ReleaseExclusiveAccess()
//
// Description:
//		Releases exclusive access to this device
//
bool CDiscRecorder::ReleaseExclusiveAccess()
{
	if (m_discRecorder != NULL)
	{
		m_hResult = m_discRecorder->ReleaseExclusiveAccess();
		if (SUCCEEDED(m_hResult))
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::ExclusiveAccessOwner()
//
// Description:
//		Returns the name of the client who has exclusive access to this
//		device.
//
CString CDiscRecorder::ExclusiveAccessOwner()
{
	if (m_discRecorder != NULL)
	{
		BSTR owner = NULL;

		m_hResult = m_discRecorder->get_ExclusiveAccessOwner(&owner);
		if (SUCCEEDED(m_hResult))
		{
			return owner;
		}
	}

	return _T("");
}


///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::EjectMedia()
//
// Description:
//		Elects the media on this device
//
bool CDiscRecorder::EjectMedia()
{
	if (m_discRecorder != NULL)
	{
		m_hResult = m_discRecorder->EjectMedia();
		if (SUCCEEDED(m_hResult))
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::CloseTray()
//
// Description:
//		Closes the tray on this device
//
bool CDiscRecorder::CloseTray()
{
	if (m_discRecorder != NULL)
	{
		m_hResult = m_discRecorder->CloseTray();
		if (SUCCEEDED(m_hResult))
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::EnableMcn()
//
// Description:
//		Enables the Media Change Notification on this device
//
bool CDiscRecorder::EnableMcn()
{
	if (m_discRecorder != NULL)
	{
		m_hResult = m_discRecorder->EnableMcn();
		if (SUCCEEDED(m_hResult))
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::DisableMcn()
//
// Description:
//		Disables the Media Change Notification on this device
//
bool CDiscRecorder::DisableMcn()
{
	if (m_discRecorder != NULL)
	{
		m_hResult = m_discRecorder->DisableMcn();
		if (SUCCEEDED(m_hResult))
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::LegacyDeviceNumber()
//
// Description:
//		Returns the legacy device number
//
LONG CDiscRecorder::GetLegacyDeviceNumber()
{
	LONG deviceNumber = 0;
	if (m_discRecorder != NULL)
	{
		m_discRecorder->get_LegacyDeviceNumber(&deviceNumber);
	}
	return deviceNumber;
}


///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::ProductRevision()
//
// Description:
//		Returns the product id for this device
//
CString CDiscRecorder::GetProductID()
{
	BSTR productId = NULL;
	if (m_discRecorder != NULL)
	{
		m_discRecorder->get_ProductId(&productId);
	}
	return productId;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::ProductRevision()
//
// Description:
//		Returns the product revision for this device
//
CString CDiscRecorder::GetProductRevision()
{
	BSTR productRevision = NULL;
	if (m_discRecorder != NULL)
	{
		m_discRecorder->get_ProductRevision(&productRevision);
	}
	return productRevision;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::VendorId()
//
// Description:
//		Returns the vendor id for this device
//
CString CDiscRecorder::GetVendorId()
{
	BSTR vendorId = NULL;
	if (m_discRecorder != NULL)
	{
		m_discRecorder->get_VendorId(&vendorId);
	}
	return vendorId;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscRecorder::VolumeName()
//
// Description:
//		Returns the unique volume name associated with this device
//
CString CDiscRecorder::GetVolumeName()
{
	BSTR volumeName = NULL;
	if (m_discRecorder != NULL)
	{
		m_discRecorder->get_VolumeName(&volumeName);
	}
	return volumeName;
}

ULONG CDiscRecorder::GetTotalVolumePaths()
{
	if (m_volumePathNames != NULL)
	{
		return m_volumePathNames->rgsabound[0].cElements;
	}

	return 0;
}

CString CDiscRecorder::GetVolumePath(ULONG volumePathIndex)
{
	ASSERT(volumePathIndex < m_volumePathNames->rgsabound[0].cElements);
	if (volumePathIndex >= m_volumePathNames->rgsabound[0].cElements)
	{
		return _T("");
	}

	return ((VARIANT*)(m_volumePathNames->pvData))[volumePathIndex].bstrVal;
}
