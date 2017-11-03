// File64.h: interface for the CFile64 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILE64_H__26F9292D_8289_4D28_93D5_34484F7E6EFF__INCLUDED_)
#define AFX_FILE64_H__26F9292D_8289_4D28_93D5_34484F7E6EFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct CFileStatus64
{
	CTime m_ctime;          // creation date/time of file
	CTime m_mtime;          // last modification date/time of file
	CTime m_atime;          // last access date/time of file
	ULONGLONG m_size;            // logical size of file in bytes
	BYTE m_attribute;       // logical OR of CFile::Attribute enum values
	BYTE _m_padding;        // pad the structure to a WORD
	TCHAR m_szFullName[_MAX_PATH]; // absolute path name
	
#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
#endif
};

class CFile64 : public CFile  
{
public:
	CFile64();
	virtual ~CFile64();

	// Attributes
	ULONGLONG GetPosition();

	// Overridables
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
	virtual void SetLength(ULONGLONG dwNewLen);
	ULONGLONG GetLength() ;

	virtual void LockRange(ULONGLONG dwPos, ULONGLONG dwCount);
	virtual void UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount);
	static  BOOL GetStatus(LPCTSTR lpszFileName, CFileStatus64& rStatus);
};

#endif // !defined(AFX_FILE64_H__26F9292D_8289_4D28_93D5_34484F7E6EFF__INCLUDED_)
