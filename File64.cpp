// File64.cpp: implementation of the CFile64 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "File64.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFile64::CFile64()
{

}

CFile64::~CFile64()
{

}

ULONGLONG CFile64::Seek(LONGLONG lOff, UINT nFrom)
{
 ASSERT_VALID(this);
 ASSERT((HANDLE)m_hFile != INVALID_HANDLE_VALUE);
 ASSERT(nFrom == begin || nFrom == end || nFrom == current);
 ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);

   LARGE_INTEGER liOff;

   liOff.QuadPart = lOff;
 liOff.LowPart = ::SetFilePointer((HANDLE)m_hFile, liOff.LowPart, &liOff.HighPart,
   (DWORD)nFrom);
 if (liOff.LowPart  == (DWORD)-1)
   if (::GetLastError() != NO_ERROR)
     CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

 return liOff.QuadPart;
}

ULONGLONG CFile64::GetPosition() 
{
 ASSERT_VALID(this);
 ASSERT((HANDLE)m_hFile != INVALID_HANDLE_VALUE);

   LARGE_INTEGER liPos;
   liPos.QuadPart = 0;
 liPos.LowPart = ::SetFilePointer((HANDLE)m_hFile, liPos.LowPart, &liPos.HighPart , FILE_CURRENT);
 if (liPos.LowPart == (DWORD)-1)
   if (::GetLastError() != NO_ERROR)
     CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

 return liPos.QuadPart;
}

void CFile64::LockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
 ASSERT_VALID(this);
 ASSERT((HANDLE)m_hFile != INVALID_HANDLE_VALUE);

   ULARGE_INTEGER liPos;
   ULARGE_INTEGER liCount;

   liPos.QuadPart = dwPos;
   liCount.QuadPart = dwCount;
 if (!::LockFile((HANDLE)m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart, 
   liCount.HighPart))
   {
  CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
   }
}

void CFile64::UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
 ASSERT_VALID(this);
 ASSERT((HANDLE)m_hFile != INVALID_HANDLE_VALUE);

   ULARGE_INTEGER liPos;
   ULARGE_INTEGER liCount;

   liPos.QuadPart = dwPos;
   liCount.QuadPart = dwCount;
 if (!::UnlockFile((HANDLE)m_hFile, liPos.LowPart, liPos.HighPart, liCount.LowPart,
   liCount.HighPart))
   {
  CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
   }
}

void CFile64::SetLength(ULONGLONG dwNewLen)
{
 ASSERT_VALID(this);
 ASSERT((HANDLE)m_hFile != INVALID_HANDLE_VALUE);

 Seek(dwNewLen, (UINT)begin);

 if (!::SetEndOfFile((HANDLE)m_hFile))
  CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
}

ULONGLONG CFile64::GetLength() 
{
 ASSERT_VALID(this);

   ULARGE_INTEGER liSize;
   liSize.LowPart = ::GetFileSize((HANDLE)m_hFile, &liSize.HighPart);
   if (liSize.LowPart == (DWORD)-1)
   if (::GetLastError() != NO_ERROR)
   CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);

 return liSize.QuadPart;
}

bool IsValidFILETIME(FILETIME& fileTime)
{
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime, &localTime))
	{
		return FALSE;
	}
	
	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))
	{
		return FALSE;
	}
	
	return TRUE;

}

BOOL CFile64::GetStatus(LPCTSTR lpszFileName, CFileStatus64& rStatus)
{
	ASSERT( lpszFileName != NULL );

	if ( lpszFileName == NULL ) 
	{
		return FALSE;
	}

	if ( lstrlen(lpszFileName) >= _MAX_PATH )
	{
		ASSERT(FALSE); // MFC requires paths with length < _MAX_PATH
		return FALSE;
	}
	
	// attempt to fully qualify path first
	ZeroMemory(rStatus.m_szFullName,sizeof(rStatus.m_szFullName));
	memcpy(rStatus.m_szFullName,lpszFileName,strlen((char*)lpszFileName));
	//if (!AfxFullPath(rStatus.m_szFullName, lpszFileName))
	//{
	//	rStatus.m_szFullName[0] = '\0';
	//	return FALSE;
	//}

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((LPTSTR)lpszFileName, &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;
	VERIFY(FindClose(hFind));

	// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
	rStatus.m_attribute = (BYTE)
		(findFileData.dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);

	// get just the low DWORD of the file size
	//ASSERT(findFileData.nFileSizeHigh == 0);
	//rStatus.m_size = (LONG)findFileData.nFileSizeLow;
	rStatus.m_size = ((ULONGLONG)findFileData.nFileSizeHigh<<32) + findFileData.nFileSizeLow;

	// convert times as appropriate
	if (IsValidFILETIME(findFileData.ftCreationTime))
	{
		rStatus.m_ctime = CTime(findFileData.ftCreationTime);
	}
	else
	{
		rStatus.m_ctime = CTime();
	}

	if (IsValidFILETIME(findFileData.ftLastAccessTime))
	{
		rStatus.m_atime = CTime(findFileData.ftLastAccessTime);
	}
	else
	{
		rStatus.m_atime = CTime();
	}

	if (IsValidFILETIME(findFileData.ftLastWriteTime))
	{
		rStatus.m_mtime = CTime(findFileData.ftLastWriteTime);
	}
	else
	{
		rStatus.m_mtime = CTime();
	}

	if (rStatus.m_ctime.GetTime() == 0)
		rStatus.m_ctime = rStatus.m_mtime;

	if (rStatus.m_atime.GetTime() == 0)
		rStatus.m_atime = rStatus.m_mtime;

	return TRUE;
}