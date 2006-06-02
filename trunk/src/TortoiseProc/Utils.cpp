﻿// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2006 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
#include "StdAfx.h"
#include "resource.h"
#include "PathUtils.h"
#include "utils.h"
#include "MessageBox.h"
#include "Registry.h"
#include "TSVNPath.h"


CUtils::CUtils(void)
{
}

CUtils::~CUtils(void)
{
}

BOOL CUtils::StartExtMerge(const CTSVNPath& basefile, const CTSVNPath& theirfile, const CTSVNPath& yourfile, const CTSVNPath& mergedfile,
						   		const CString& basename, const CString& theirname, const CString& yourname, const CString& mergedname, bool bReadOnly)
{

	CRegString regCom = CRegString(_T("Software\\TortoiseSVN\\Merge"));
	CString ext = mergedfile.GetFileExtension();
	CString com = regCom;
	bool bInternal = false;
	if (ext != "")
	{
		// is there an extension specific merge tool?
		CRegString mergetool(_T("Software\\TortoiseSVN\\MergeTools\\") + ext.MakeLower());
		if (CString(mergetool) != "")
		{
			com = mergetool;
		}
	}
	
	if (com.IsEmpty()||(com.Left(1).Compare(_T("#"))==0))
	{
		// use TortoiseMerge
		bInternal = true;
		CRegString tortoiseMergePath(_T("Software\\TortoiseSVN\\TMergePath"), _T(""), false, HKEY_LOCAL_MACHINE);
		com = tortoiseMergePath;
		if (com.IsEmpty())
		{
			com = CPathUtils::GetAppDirectory();
			com += _T("TortoiseMerge.exe");
		}
		com = _T("\"") + com + _T("\"");
		com = com + _T(" /base:%base /theirs:%theirs /mine:%mine /merged:%merged");
		com = com + _T(" /basename:%bname /theirsname:%tname /minename:%yname /mergedname:%mname");
	}
	// check if the params are set. If not, just add the files to the command line
	if ((com.Find(_T("%base"))<0)&&(com.Find(_T("%theirs"))<0)&&(com.Find(_T("%mine"))<0))
	{
		com += _T(" \"")+basefile.GetWinPathString()+_T("\"");
		com += _T(" \"")+theirfile.GetWinPathString()+_T("\"");
		com += _T(" \"")+yourfile.GetWinPathString()+_T("\"");
		com += _T(" \"")+mergedfile.GetWinPathString()+_T("\"");
	}
	if (basefile.IsEmpty())
	{
		com.Replace(_T("/base:%base"), _T(""));
		com.Replace(_T("%base"), _T(""));		
	}
	else
		com.Replace(_T("%base"), _T("\"") + basefile.GetWinPathString() + _T("\""));
	if (theirfile.IsEmpty())
	{
		com.Replace(_T("/theirs:%theirs"), _T(""));
		com.Replace(_T("%theirs"), _T(""));
	}
	else
		com.Replace(_T("%theirs"), _T("\"") + theirfile.GetWinPathString() + _T("\""));
	if (yourfile.IsEmpty())
	{
		com.Replace(_T("/mine:%mine"), _T(""));
		com.Replace(_T("%mine"), _T(""));
	}
	else
		com.Replace(_T("%mine"), _T("\"") + yourfile.GetWinPathString() + _T("\""));
	if (mergedfile.IsEmpty())
	{
		com.Replace(_T("/merged:%merged"), _T(""));
		com.Replace(_T("%merged"), _T(""));
	}
	else
		com.Replace(_T("%merged"), _T("\"") + mergedfile.GetWinPathString() + _T("\""));
	if (basename.IsEmpty())
	{
		if (basefile.IsEmpty())
		{
			com.Replace(_T("/basename:%bname"), _T(""));
			com.Replace(_T("%bname"), _T(""));
		}
		else
		{
			com.Replace(_T("%bname"), _T("\"") + basefile.GetFileOrDirectoryName() + _T("\""));
		}
	}
	else
		com.Replace(_T("%bname"), _T("\"") + basename + _T("\""));
	if (theirname.IsEmpty())
	{
		if (theirfile.IsEmpty())
		{
			com.Replace(_T("/theirsname:%tname"), _T(""));
			com.Replace(_T("%tname"), _T(""));
		}
		else
		{
			com.Replace(_T("%tname"), _T("\"") + theirfile.GetFileOrDirectoryName() + _T("\""));
		}
	}
	else
		com.Replace(_T("%tname"), _T("\"") + theirname + _T("\""));
	if (yourname.IsEmpty())
	{
		if (yourfile.IsEmpty())
		{
			com.Replace(_T("/minename:%yname"), _T(""));
			com.Replace(_T("%yname"), _T(""));
		}
		else
		{
			com.Replace(_T("%yname"), _T("\"") + yourfile.GetFileOrDirectoryName() + _T("\""));
		}
	}
	else
		com.Replace(_T("%yname"), _T("\"") + yourname + _T("\""));
	if (mergedname.IsEmpty())
	{
		if (mergedfile.IsEmpty())
		{
			com.Replace(_T("/mergedname:%mname"), _T(""));
			com.Replace(_T("%mname"), _T(""));
		}
		else
		{
			com.Replace(_T("%mname"), _T("\"") + mergedfile.GetFileOrDirectoryName() + _T("\""));
		}
	}
	else
		com.Replace(_T("%mname"), _T("\"") + mergedname + _T("\""));

	if ((bReadOnly)&&(bInternal))
		com += _T(" /readonly");

	if(!LaunchApplication(com, IDS_ERR_EXTMERGESTART, false))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CUtils::StartExtPatch(const CTSVNPath& patchfile, const CTSVNPath& dir, const CString& sOriginalDescription, const CString& sPatchedDescription, BOOL bReversed, BOOL bWait)
{
	CString viewer;
	// use TortoiseMerge
	viewer = CPathUtils::GetAppDirectory();
	viewer += _T("TortoiseMerge.exe");

	viewer = _T("\"") + viewer + _T("\"");
	viewer = viewer + _T(" /diff:\"") + patchfile.GetWinPathString() + _T("\"");
	viewer = viewer + _T(" /patchpath:\"") + dir.GetWinPathString() + _T("\"");
	if (bReversed)
		viewer += _T(" /reversedpatch");
	if (!sOriginalDescription.IsEmpty())
		viewer = viewer + _T(" /patchoriginal:\"") + sOriginalDescription + _T("\"");
	if (!sPatchedDescription.IsEmpty())
		viewer = viewer + _T(" /patchpatched:\"") + sPatchedDescription + _T("\"");
	if(!LaunchApplication(viewer, IDS_ERR_DIFFVIEWSTART, !!bWait))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CUtils::StartExtDiff(const CTSVNPath& file1, const CTSVNPath& file2, const CString& sName1, const CString& sName2, BOOL bWait, BOOL bBlame)
{
	CString viewer;
	CRegString diffexe(_T("Software\\TortoiseSVN\\Diff"));
	CRegDWORD blamediff(_T("Software\\TortoiseSVN\\DiffBlamesWithTortoiseMerge"), FALSE);
	bool bUseTMerge = !!(DWORD)blamediff;
	viewer = diffexe;
	if (!file2.GetFileExtension().IsEmpty())
	{
		// is there an extension specific diff tool?
		CRegString difftool(_T("Software\\TortoiseSVN\\DiffTools\\") + file2.GetFileExtension().MakeLower());
		if (CString(difftool) != "")
		{
			viewer = difftool;
		}
		else
		{
			// check if we maybe should use TortoiseIDiff
			CString sExtension = file2.GetFileExtension();
			if ((sExtension.CompareNoCase(_T(".jpg"))==0)||
				(sExtension.CompareNoCase(_T(".jpeg"))==0)||
				(sExtension.CompareNoCase(_T(".bmp"))==0)||
				(sExtension.CompareNoCase(_T(".gif"))==0)||
				(sExtension.CompareNoCase(_T(".png"))==0)||
				(sExtension.CompareNoCase(_T(".ico"))==0)||
				(sExtension.CompareNoCase(_T(".dib"))==0)||
				(sExtension.CompareNoCase(_T(".emf"))==0))
			{
				viewer = CPathUtils::GetAppDirectory();
				viewer += _T("TortoiseIDiff.exe");
				viewer = _T("\"") + viewer + _T("\"");
				viewer = viewer + _T(" /left:%base /right:%mine /lefttitle:%bname /righttitle:%yname");
			}
		}
	}
	if (bUseTMerge||viewer.IsEmpty()||(viewer.Left(1).Compare(_T("#"))==0))
	{
		//no registry entry (or commented out) for a diff program
		//use TortoiseMerge
		viewer = CPathUtils::GetAppDirectory();
		viewer += _T("TortoiseMerge.exe");
		viewer = _T("\"") + viewer + _T("\"");
		viewer = viewer + _T(" /base:%base /mine:%mine /basename:%bname /minename:%yname");
		if (bBlame)
			viewer += _T(" /blame");
	}
	// check if the params are set. If not, just add the files to the command line
	if ((viewer.Find(_T("%base"))<0)&&(viewer.Find(_T("%mine"))<0))
	{
		viewer += _T(" \"")+file1.GetWinPathString()+_T("\"");
		viewer += _T(" \"")+file2.GetWinPathString()+_T("\"");
	}
	if (viewer.Find(_T("%base")) >= 0)
	{
		viewer.Replace(_T("%base"),  _T("\"")+file1.GetWinPathString()+_T("\""));
	}
	if (viewer.Find(_T("%mine")) >= 0)
	{
		viewer.Replace(_T("%mine"),  _T("\"")+file2.GetWinPathString()+_T("\""));
	}

	if (sName1.IsEmpty())
		viewer.Replace(_T("%bname"), _T("\"") + file1.GetFileOrDirectoryName() + _T("\""));
	else
		viewer.Replace(_T("%bname"), _T("\"") + sName1 + _T("\""));

	if (sName2.IsEmpty())
		viewer.Replace(_T("%yname"), _T("\"") + file2.GetFileOrDirectoryName() + _T("\""));
	else
		viewer.Replace(_T("%yname"), _T("\"") + sName2 + _T("\""));

	if(!LaunchApplication(viewer, IDS_ERR_EXTDIFFSTART, !!bWait))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CUtils::StartUnifiedDiffViewer(const CTSVNPath& patchfile, BOOL bWait)
{
	CString viewer;
	CRegString v = CRegString(_T("Software\\TortoiseSVN\\DiffViewer"));
	viewer = v;
	if (viewer.IsEmpty() || (viewer.Left(1).Compare(_T("#"))==0))
	{
		//first try the default app which is associated with diff files
		CRegString diff = CRegString(_T(".diff\\"), _T(""), FALSE, HKEY_CLASSES_ROOT);
		viewer = diff;
		viewer = viewer + _T("\\Shell\\Open\\Command\\");
		CRegString diffexe = CRegString(viewer, _T(""), FALSE, HKEY_CLASSES_ROOT);
		viewer = diffexe;
		if (viewer.IsEmpty())
		{
			CRegString txt = CRegString(_T(".txt\\"), _T(""), FALSE, HKEY_CLASSES_ROOT);
			viewer = txt;
			viewer = viewer + _T("\\Shell\\Open\\Command\\");
			CRegString txtexe = CRegString(viewer, _T(""), FALSE, HKEY_CLASSES_ROOT);
			viewer = txtexe;
		}
		DWORD len = ExpandEnvironmentStrings(viewer, NULL, 0);
		TCHAR * buf = new TCHAR[len+1];
		ExpandEnvironmentStrings(viewer, buf, len);
		viewer = buf;
		delete buf;
	}
	if (viewer.Find(_T("%1"))>=0)
	{
		if (viewer.Find(_T("\"%1\"")) >= 0)
			viewer.Replace(_T("%1"), patchfile.GetWinPathString());
		else
			viewer.Replace(_T("%1"), _T("\"") + patchfile.GetWinPathString() + _T("\""));
	}
	else
		viewer += _T(" \"") + patchfile.GetWinPathString() + _T("\"");

	if(!LaunchApplication(viewer, IDS_ERR_DIFFVIEWSTART, !!bWait))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CUtils::StartTextViewer(CString file)
{
	CString viewer;
	CRegString txt = CRegString(_T(".txt\\"), _T(""), FALSE, HKEY_CLASSES_ROOT);
	viewer = txt;
	viewer = viewer + _T("\\Shell\\Open\\Command\\");
	CRegString txtexe = CRegString(viewer, _T(""), FALSE, HKEY_CLASSES_ROOT);
	viewer = txtexe;

	DWORD len = ExpandEnvironmentStrings(viewer, NULL, 0);
	TCHAR * buf = new TCHAR[len+1];
	ExpandEnvironmentStrings(viewer, buf, len);
	viewer = buf;
	delete buf;
	len = ExpandEnvironmentStrings(file, NULL, 0);
	buf = new TCHAR[len+1];
	ExpandEnvironmentStrings(file, buf, len);
	file = buf;
	delete buf;
	file = _T("\"")+file+_T("\"");
	if (viewer.IsEmpty())
	{
		OPENFILENAME ofn;		// common dialog box structure
		TCHAR szFile[MAX_PATH];  // buffer for file name. Explorer can't handle paths longer than MAX_PATH.
		ZeroMemory(szFile, sizeof(szFile));
		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		//ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;		//to stay compatible with NT4
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile)/sizeof(TCHAR);
		CString sFilter;
		sFilter.LoadString(IDS_PROGRAMSFILEFILTER);
		TCHAR * pszFilters = new TCHAR[sFilter.GetLength()+4];
		_tcscpy_s (pszFilters, sFilter.GetLength()+4, sFilter);
		// Replace '|' delimeters with '\0's
		TCHAR *ptr = pszFilters + _tcslen(pszFilters);  //set ptr at the NULL
		while (ptr != pszFilters)
		{
			if (*ptr == '|')
				*ptr = '\0';
			ptr--;
		} // while (ptr != pszFilters) 
		ofn.lpstrFilter = pszFilters;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		CString temp;
		temp.LoadString(IDS_UTILS_SELECTTEXTVIEWER);
		CUtils::RemoveAccelerators(temp);
		ofn.lpstrTitle = temp;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

		// Display the Open dialog box. 

		if (GetOpenFileName(&ofn)==TRUE)
		{
			delete [] pszFilters;
			viewer = CString(ofn.lpstrFile);
		} // if (GetOpenFileName(&ofn)==TRUE)
		else
		{
			delete [] pszFilters;
			return FALSE;
		}
	}
	if (viewer.Find(_T("\"%1\"")) >= 0)
	{
		viewer.Replace(_T("\"%1\""), file);
	}
	else if (viewer.Find(_T("%1")) >= 0)
	{
		viewer.Replace(_T("%1"),  file);
	}
	else
	{
		viewer += _T(" ");
		viewer += file;
	}

	if(!LaunchApplication(viewer, IDS_ERR_DIFFVIEWSTART, false))
	{
		return FALSE;
	}
	return TRUE;
}

void CUtils::Unescape(char * psz)
{
	char * pszSource = psz;
	char * pszDest = psz;

	static const char szHex[] = "0123456789ABCDEF";

	// Unescape special characters. The number of characters
	// in the *pszDest is assumed to be <= the number of characters
	// in pszSource (they are both the same string anyway)

	while (*pszSource != '\0' && *pszDest != '\0')
	{
		if (*pszSource == '%')
		{
			// The next two chars following '%' should be digits
			if ( *(pszSource + 1) == '\0' ||
				 *(pszSource + 2) == '\0' )
			{
				// nothing left to do
				break;
			}

			char nValue = '?';
			const char * pszLow = NULL;
			const char * pszHigh = NULL;
			pszSource++;

			*pszSource = (char) toupper(*pszSource);
			pszHigh = strchr(szHex, *pszSource);

			if (pszHigh != NULL)
			{
				pszSource++;
				*pszSource = (char) toupper(*pszSource);
				pszLow = strchr(szHex, *pszSource);

				if (pszLow != NULL)
				{
					nValue = (char) (((pszHigh - szHex) << 4) +
									(pszLow - szHex));
				}
			} // if (pszHigh != NULL) 
			*pszDest++ = nValue;
		} 
		else
			*pszDest++ = *pszSource;
			
		pszSource++;
	}

	*pszDest = '\0';
}

static const char iri_escape_chars[256] = {
		1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,

		/* 128 */
		0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0
};

const char uri_autoescape_chars[256] = {
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 1, 0, 0,

		/* 64 */
		1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
		0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,

		/* 128 */
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,

		/* 192 */
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

static const char uri_char_validity[256] = {
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 1, 0, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 1, 0, 0,

		/* 64 */
		1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
		0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,

		/* 128 */
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,

		/* 192 */
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

CStringA CUtils::PathEscape(const CStringA& path)
{
	CStringA ret2;
	int c;
	int i;
	for (i=0; path[i]; ++i)
	{
		c = (unsigned char)path[i];
		if (iri_escape_chars[c])
		{
			// no escaping needed for that char
			ret2 += (unsigned char)path[i];
		}
		else
		{
			// char needs escaping
			CStringA temp;
			temp.Format("%%%02X", (unsigned char)c);
			ret2 += temp;
		}
	}
	CStringA ret;
	for (i=0; ret2[i]; ++i)
	{
		c = (unsigned char)ret2[i];
		if (uri_autoescape_chars[c])
		{
			// no escaping needed for that char
			ret += (unsigned char)ret2[i];
		}
		else
		{
			// char needs escaping
			CStringA temp;
			temp.Format("%%%02X", (unsigned char)c);
			ret += temp;
		}
	}

	ret.Replace(("file:///%5C"), ("file:///\\"));
	ret.Replace(("file:////%5C"), ("file:////\\"));

	return ret;
}

CString CUtils::GetVersionFromFile(const CString & p_strDateiname)
{
	struct TRANSARRAY
	{
		WORD wLanguageID;
		WORD wCharacterSet;
	};

	CString strReturn;
	DWORD dwReserved,dwBufferSize;
	dwBufferSize = GetFileVersionInfoSize((LPTSTR)(LPCTSTR)p_strDateiname,&dwReserved);

	if (dwBufferSize > 0)
	{
		LPVOID pBuffer = (void*) malloc(dwBufferSize);

		if (pBuffer != (void*) NULL)
		{
			UINT        nInfoSize = 0,
			nFixedLength = 0;
			LPSTR       lpVersion = NULL;
			VOID*       lpFixedPointer;
			TRANSARRAY* lpTransArray;
			CString     strLangProduktVersion;

			GetFileVersionInfo((LPTSTR)(LPCTSTR)p_strDateiname,
			dwReserved,
			dwBufferSize,
			pBuffer);

			// Abfragen der aktuellen Sprache
			VerQueryValue(	pBuffer,
							_T("\\VarFileInfo\\Translation"),
							&lpFixedPointer,
							&nFixedLength);
			lpTransArray = (TRANSARRAY*) lpFixedPointer;

			strLangProduktVersion.Format(_T("\\StringFileInfo\\%04x%04x\\ProductVersion"),
			lpTransArray[0].wLanguageID, lpTransArray[0].wCharacterSet);

			VerQueryValue(pBuffer,
			(LPTSTR)(LPCTSTR)strLangProduktVersion,
			(LPVOID *)&lpVersion,
			&nInfoSize);
			strReturn = (LPCTSTR)lpVersion;
			free(pBuffer);
		}
	} 

	return strReturn;
}


BOOL CUtils::CheckForEmptyDiff(const CTSVNPath& sDiffPath)
{
	DWORD length = 0;
	HANDLE hFile = ::CreateFile(sDiffPath.GetWinPath(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return TRUE;
	length = ::GetFileSize(hFile, NULL);
	::CloseHandle(hFile);
	if (length < 4)
		return TRUE;
	return FALSE;

}

void CUtils::RemoveAccelerators(CString& text)
{
	int pos = 0;
	while ((pos=text.Find('&',pos))>=0)
	{
		if (text.GetLength() > (pos-1))
		{
			if (text.GetAt(pos+1)!=' ')
				text.Delete(pos);
		}
		pos++;
	}
}


bool CUtils::WriteAsciiStringToClipboard(const CStringA& sClipdata, HWND hOwningWnd)
{
	if (OpenClipboard(hOwningWnd))
	{
		EmptyClipboard();
		HGLOBAL hClipboardData;
		hClipboardData = GlobalAlloc(GMEM_DDESHARE, sClipdata.GetLength()+1);
		if (hClipboardData)
		{
			char * pchData;
			pchData = (char*)GlobalLock(hClipboardData);
			if (pchData)
			{
				strcpy_s(pchData, sClipdata.GetLength()+1, (LPCSTR)sClipdata);
				if (GlobalUnlock(hClipboardData))
				{
					if (SetClipboardData(CF_TEXT,hClipboardData)==NULL)
					{
						CloseClipboard();
						return false;
					}
				}
				else
				{
					CloseClipboard();
					return false;
				}
			}
			else
			{
				CloseClipboard();
				return false;
			}
		}
		else
		{
			CloseClipboard();
			return false;
		}
		CloseClipboard();
		return true;
	}
	return false;
}

void CUtils::CreateFontForLogs(CFont& fontToCreate)
{
	LOGFONT logFont;
	HDC hScreenDC = ::GetDC(NULL);
	logFont.lfHeight         = -MulDiv((DWORD)CRegDWORD(_T("Software\\TortoiseSVN\\LogFontSize"), 8), GetDeviceCaps(hScreenDC, LOGPIXELSY), 72);
	::ReleaseDC(NULL, hScreenDC);
	logFont.lfWidth          = 0;
	logFont.lfEscapement     = 0;
	logFont.lfOrientation    = 0;
	logFont.lfWeight         = FW_NORMAL;
	logFont.lfItalic         = 0;
	logFont.lfUnderline      = 0;
	logFont.lfStrikeOut      = 0;
	logFont.lfCharSet        = DEFAULT_CHARSET;
	logFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
	logFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
	logFont.lfQuality        = DRAFT_QUALITY;
	logFont.lfPitchAndFamily = FF_DONTCARE | FIXED_PITCH;
	_tcscpy_s(logFont.lfFaceName, 32, (LPCTSTR)(CString)CRegString(_T("Software\\TortoiseSVN\\LogFontName"), _T("Courier New")));
	VERIFY(fontToCreate.CreateFontIndirect(&logFont));
}

bool CUtils::LaunchApplication(const CString& sCommandLine, UINT idErrMessageFormat, bool bWaitForStartup)
{
	STARTUPINFO startup;
	PROCESS_INFORMATION process;
	memset(&startup, 0, sizeof(startup));
	startup.cb = sizeof(startup);
	memset(&process, 0, sizeof(process));

	CString cleanCommandLine(sCommandLine);

	if (CreateProcess(NULL, const_cast<TCHAR*>((LPCTSTR)cleanCommandLine), NULL, NULL, FALSE, 0, 0, 0, &startup, &process)==0)
	{
		if(idErrMessageFormat != 0)
		{
			LPVOID lpMsgBuf;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);
			CString temp;
			temp.Format(idErrMessageFormat, lpMsgBuf);
			CMessageBox::Show(NULL, temp, _T("TortoiseSVN"), MB_OK | MB_ICONINFORMATION);
			LocalFree( lpMsgBuf );
		}
		return false;
	}

	if (bWaitForStartup)
	{
		WaitForInputIdle(process.hProcess, 10000);
	}

	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
	return true;
}

/**
* Launch the external blame viewer
*/
bool CUtils::LaunchTortoiseBlame(const CString& sBlameFile, const CString& sLogFile, const CString& sOriginalFile)
{
	CString viewer = CPathUtils::GetAppDirectory();
	viewer += _T("TortoiseBlame.exe");
	viewer += _T(" \"") + sBlameFile + _T("\"");
	viewer += _T(" \"") + sLogFile + _T("\"");
	viewer += _T(" \"") + sOriginalFile + _T("\"");
	
	return LaunchApplication(viewer, IDS_ERR_EXTDIFFSTART, false);
}

void CUtils::ResizeAllListCtrlCols(CListCtrl * pListCtrl)
{
	int maxcol = ((CHeaderCtrl*)(pListCtrl->GetDlgItem(0)))->GetItemCount()-1;
	int nItemCount = pListCtrl->GetItemCount();
	TCHAR textbuf[MAX_PATH];
	for (int col = 0; col <= maxcol; col++)
	{
		HDITEM hdi = {0};
		hdi.mask = HDI_TEXT;
		hdi.pszText = textbuf;
		hdi.cchTextMax = sizeof(textbuf);
		((CHeaderCtrl*)(pListCtrl->GetDlgItem(0)))->GetItem(col, &hdi);
		int cx = pListCtrl->GetStringWidth(hdi.pszText)+20; // 20 pixels for col separator and margin
		for (int index = 0; index<nItemCount; ++index)
		{
			// get the width of the string and add 12 pixels for the column separator and margins
			int linewidth = pListCtrl->GetStringWidth(pListCtrl->GetItemText(index, col)) + 12;
			if (index == 0)
			{
				// add the image size
				CImageList * pImgList = pListCtrl->GetImageList(LVSIL_SMALL);
				if ((pImgList)&&(pImgList->GetImageCount()))
				{
					IMAGEINFO imginfo;
					pImgList->GetImageInfo(0, &imginfo);
					linewidth += (imginfo.rcImage.right - imginfo.rcImage.left);
					linewidth += 3;	// 3 pixels between icon and text
				}
			}
			if (cx < linewidth)
				cx = linewidth;
		}
		pListCtrl->SetColumnWidth(col, cx);
	}
}

#define IsCharNumeric(C) (!IsCharAlpha(C) && IsCharAlphaNumeric(C))

int CUtils::CompareNumerical(LPCTSTR x_str, LPCTSTR y_str)
{
  LPCTSTR num_x_begin = x_str, num_y_begin = y_str;
  DWORD num_x_cnt = 0, num_y_cnt = 0;
  int cs_cmp_result = 2;

  // skip same chars and remember last numeric part of strings
  while ((*x_str || *y_str) && CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREWIDTH, x_str, 1, y_str, 1) == 2 /* equal */ )
  {
    if (IsCharNumeric(*x_str))
    {
      ++num_x_cnt;
      ++num_y_cnt;
    }
    else
    {
      num_x_begin = CharNext(x_str);
      num_y_begin = CharNext(y_str);
      num_x_cnt = 0;
      num_y_cnt = 0;
	  if (cs_cmp_result == 2)
		  cs_cmp_result = CompareString(LOCALE_USER_DEFAULT, NORM_IGNOREWIDTH, x_str, 1, y_str, 1);
    }
    x_str = CharNext(x_str);
    y_str = CharNext(y_str);
  }

  // parse numeric part of first arg
  if (num_x_cnt || IsCharNumeric(*x_str))
  {
    LPCTSTR x_str_tmp = x_str;
    while (IsCharNumeric(*x_str_tmp))
    {
      ++num_x_cnt;
      x_str_tmp = CharNext(x_str_tmp);
    }

    // parse numeric part of second arg
    if (num_y_cnt || IsCharNumeric(*y_str))
    {
      LPCTSTR y_str_tmp = y_str;
      while (IsCharNumeric(*y_str_tmp))
      {
        ++num_y_cnt;
        y_str_tmp = CharNext(y_str_tmp);
      }

      DWORD num_x_cnt_with_zeros = num_x_cnt, num_y_cnt_with_zeros = num_y_cnt;

      while (num_x_cnt < num_y_cnt)
      {
        if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNOREWIDTH, num_y_begin, 1, TEXT("0"), 1) != 2 /* not equal to '0' */ )
          return -1;
        num_y_begin = CharNext(num_y_begin);
        --num_y_cnt;
      }

      while (num_x_cnt > num_y_cnt)
      {
        if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNOREWIDTH, num_x_begin, 1, TEXT("0"), 1) != 2 /* not equal to '0' */ )
          return 1;
        num_x_begin = CharNext(num_x_begin);
        --num_x_cnt;
      }

      // here num_x_cnt == num_y_cnt
      int cmp_result = CompareString(LOCALE_USER_DEFAULT, NORM_IGNOREWIDTH, num_x_begin, num_x_cnt, num_y_begin, num_y_cnt);

      if (cmp_result != 2)
        return cmp_result - 2;
      if (num_x_cnt_with_zeros != num_y_cnt_with_zeros)
        return num_x_cnt_with_zeros < num_y_cnt_with_zeros ? -1 : 1;
	  if (cs_cmp_result != 2)
        return cs_cmp_result - 2;
    }
  }

  // otherwise, compare literally
  int cmp_result = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREWIDTH, x_str, -1, y_str, -1);
  if (cmp_result != 2)
	  return cmp_result - 2;
  if (cs_cmp_result == 2)
	  cs_cmp_result = CompareString(LOCALE_USER_DEFAULT, NORM_IGNOREWIDTH, x_str, -1, y_str, -1);
  return cs_cmp_result - 2;
}
