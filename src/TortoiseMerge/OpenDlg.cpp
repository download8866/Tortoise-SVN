// TortoiseMerge - a Diff/Patch program

// Copyright (C) 2006-2010 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "TortoiseMerge.h"
#include "BrowseFolder.h"
#include ".\opendlg.h"
#include "auto_buffer.h"
#include "SelectFileFilter.h"
#include "registry.h"

// COpenDlg dialog

IMPLEMENT_DYNAMIC(COpenDlg, CStandAloneDialog)
COpenDlg::COpenDlg(CWnd* pParent /*=NULL*/)
	: CStandAloneDialog(COpenDlg::IDD, pParent)
	, m_sBaseFile(_T(""))
	, m_sTheirFile(_T(""))
	, m_sYourFile(_T(""))
	, m_sUnifiedDiffFile(_T(""))
	, m_sPatchDirectory(_T(""))
	, m_bFromClipboard(FALSE)
	, m_cFormat(0)
	, m_nextViewer(NULL)
{
}

COpenDlg::~COpenDlg()
{
}

void COpenDlg::DoDataExchange(CDataExchange* pDX)
{
	CStandAloneDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_BASEFILEEDIT, m_sBaseFile);
	DDX_Text(pDX, IDC_THEIRFILEEDIT, m_sTheirFile);
	DDX_Text(pDX, IDC_YOURFILEEDIT, m_sYourFile);
	DDX_Text(pDX, IDC_DIFFFILEEDIT, m_sUnifiedDiffFile);
	DDX_Text(pDX, IDC_DIRECTORYEDIT, m_sPatchDirectory);
	DDX_Control(pDX, IDC_BASEFILEEDIT, m_cBaseFileEdit);
	DDX_Control(pDX, IDC_THEIRFILEEDIT, m_cTheirFileEdit);
	DDX_Control(pDX, IDC_YOURFILEEDIT, m_cYourFileEdit);
	DDX_Control(pDX, IDC_DIFFFILEEDIT, m_cDiffFileEdit);
	DDX_Control(pDX, IDC_DIRECTORYEDIT, m_cDirEdit);
	DDX_Check(pDX, IDC_PATCHFROMCLIPBOARD, m_bFromClipboard);
}

BEGIN_MESSAGE_MAP(COpenDlg, CStandAloneDialog)
	ON_BN_CLICKED(IDC_BASEFILEBROWSE, OnBnClickedBasefilebrowse)
	ON_BN_CLICKED(IDC_THEIRFILEBROWSE, OnBnClickedTheirfilebrowse)
	ON_BN_CLICKED(IDC_YOURFILEBROWSE, OnBnClickedYourfilebrowse)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnBnClickedHelp)
	ON_BN_CLICKED(IDC_DIFFFILEBROWSE, OnBnClickedDifffilebrowse)
	ON_BN_CLICKED(IDC_DIRECTORYBROWSE, OnBnClickedDirectorybrowse)
	ON_BN_CLICKED(IDC_MERGERADIO, OnBnClickedMergeradio)
	ON_BN_CLICKED(IDC_APPLYRADIO, OnBnClickedApplyradio)
	ON_WM_CHANGECBCHAIN()
	ON_WM_DRAWCLIPBOARD()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_PATCHFROMCLIPBOARD, &COpenDlg::OnBnClickedPatchfromclipboard)
END_MESSAGE_MAP()

BOOL COpenDlg::OnInitDialog()
{
	CStandAloneDialog::OnInitDialog();

	ExtendFrameIntoClientArea(IDC_MERGEGROUP, IDC_MERGEGROUP, IDC_MERGEGROUP, IDC_UNIDIFFGROUP);
	m_aeroControls.SubclassControl(GetDlgItem(IDC_VERSIONSTRING)->GetSafeHwnd());
	m_aeroControls.SubclassControl(GetDlgItem(IDC_MERGERADIO)->GetSafeHwnd());
	m_aeroControls.SubclassControl(GetDlgItem(IDC_APPLYRADIO)->GetSafeHwnd());
	m_aeroControls.SubclassControl(GetDlgItem(IDOK)->GetSafeHwnd());
	m_aeroControls.SubclassControl(GetDlgItem(IDCANCEL)->GetSafeHwnd());
	m_aeroControls.SubclassControl(GetDlgItem(IDC_HELPBUTTON)->GetSafeHwnd());

	CRegDWORD lastRadioButton(_T("Software\\TortoiseMerge\\OpenRadio"), IDC_MERGERADIO);
	if (((DWORD)lastRadioButton != IDC_MERGERADIO)&&((DWORD)lastRadioButton != IDC_APPLYRADIO))
		lastRadioButton = IDC_MERGERADIO;
	GroupRadio((DWORD)lastRadioButton);
	CheckRadioButton(IDC_MERGERADIO, IDC_APPLYRADIO, (DWORD)lastRadioButton);

	// turn on auto completion for the edit controls
	HWND hwndEdit;
	GetDlgItem(IDC_BASEFILEEDIT, &hwndEdit);
	if (hwndEdit)
		SHAutoComplete(hwndEdit, SHACF_AUTOSUGGEST_FORCE_ON | SHACF_AUTOAPPEND_FORCE_ON | SHACF_FILESYSTEM);
	GetDlgItem(IDC_THEIRFILEEDIT, &hwndEdit);
	if (hwndEdit)
		SHAutoComplete(hwndEdit, SHACF_AUTOSUGGEST_FORCE_ON | SHACF_AUTOAPPEND_FORCE_ON | SHACF_FILESYSTEM);
	GetDlgItem(IDC_YOURFILEEDIT, &hwndEdit);
	if (hwndEdit)
		SHAutoComplete(hwndEdit, SHACF_AUTOSUGGEST_FORCE_ON | SHACF_AUTOAPPEND_FORCE_ON | SHACF_FILESYSTEM);
	GetDlgItem(IDC_DIFFFILEEDIT, &hwndEdit);
	if (hwndEdit)
		SHAutoComplete(hwndEdit, SHACF_AUTOSUGGEST_FORCE_ON | SHACF_AUTOAPPEND_FORCE_ON | SHACF_FILESYSTEM);
	GetDlgItem(IDC_DIRECTORYEDIT, &hwndEdit);
	if (hwndEdit)
		SHAutoComplete(hwndEdit, SHACF_AUTOSUGGEST_FORCE_ON | SHACF_AUTOAPPEND_FORCE_ON | SHACF_FILESYSTEM);

	m_cFormat = RegisterClipboardFormat(_T("TSVN_UNIFIEDDIFF"));
	m_nextViewer = SetClipboardViewer();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// COpenDlg message handlers

void COpenDlg::OnBnClickedBasefilebrowse()
{
	CString temp;
	UpdateData();
	temp.LoadString(IDS_SELECTFILE);
	BrowseForFile(m_sBaseFile, temp);
	UpdateData(FALSE);
}

void COpenDlg::OnBnClickedTheirfilebrowse()
{
	CString temp;
	UpdateData();
	temp.LoadString(IDS_SELECTFILE);
	BrowseForFile(m_sTheirFile, temp);
	UpdateData(FALSE);
}

void COpenDlg::OnBnClickedYourfilebrowse()
{
	CString temp;
	UpdateData();
	temp.LoadString(IDS_SELECTFILE);
	BrowseForFile(m_sYourFile, temp);
	UpdateData(FALSE);
}

void COpenDlg::OnBnClickedHelp()
{
	this->OnHelp();
}

BOOL COpenDlg::BrowseForFile(CString& filepath, CString title, UINT nFileFilter)
{
	OPENFILENAME ofn = {0};			// common dialog box structure
	TCHAR szFile[MAX_PATH] = {0};	// buffer for file name
	if (!filepath.IsEmpty())
	{
		_tcscpy_s(szFile, filepath);
	}
	// Initialize OPENFILENAME
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile)/sizeof(TCHAR);
	CSelectFileFilter fileFilter(nFileFilter);
	ofn.lpstrFilter = fileFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = title;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn)==TRUE)
	{
		filepath = CString(ofn.lpstrFile);
		return TRUE;
	}
	return FALSE;			//user canceled the dialog
}

void COpenDlg::OnBnClickedDifffilebrowse()
{
	CString temp;
	UpdateData();
	temp.LoadString(IDS_SELECTFILE);
	BrowseForFile(m_sUnifiedDiffFile, temp, IDS_PATCHFILEFILTER);
	UpdateData(FALSE);
}

void COpenDlg::OnBnClickedDirectorybrowse()
{
	CBrowseFolder folderBrowser;
	UpdateData();
	folderBrowser.m_style = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	folderBrowser.Show(GetSafeHwnd(), m_sPatchDirectory, m_sPatchDirectory);
	UpdateData(FALSE);
}

void COpenDlg::OnBnClickedMergeradio()
{
	GroupRadio(IDC_MERGERADIO);
}

void COpenDlg::OnBnClickedApplyradio()
{
	GroupRadio(IDC_APPLYRADIO);
}

void COpenDlg::GroupRadio(UINT nID)
{
	BOOL bMerge = FALSE;
	BOOL bUnified = FALSE;
	if (nID == IDC_MERGERADIO)
		bMerge = TRUE;
	if (nID == IDC_APPLYRADIO)
		bUnified = TRUE;

	GetDlgItem(IDC_BASEFILEEDIT)->EnableWindow(bMerge);
	GetDlgItem(IDC_BASEFILEBROWSE)->EnableWindow(bMerge);
	GetDlgItem(IDC_THEIRFILEEDIT)->EnableWindow(bMerge);
	GetDlgItem(IDC_THEIRFILEBROWSE)->EnableWindow(bMerge);
	GetDlgItem(IDC_YOURFILEEDIT)->EnableWindow(bMerge);
	GetDlgItem(IDC_YOURFILEBROWSE)->EnableWindow(bMerge);

	GetDlgItem(IDC_DIFFFILEEDIT)->EnableWindow(bUnified);
	GetDlgItem(IDC_DIFFFILEBROWSE)->EnableWindow(bUnified);
	GetDlgItem(IDC_DIRECTORYEDIT)->EnableWindow(bUnified);
	GetDlgItem(IDC_DIRECTORYBROWSE)->EnableWindow(bUnified);

	CheckAndEnableClipboardChecker();
}

void COpenDlg::OnOK()
{
	UpdateData(TRUE);

	bool bUDiffOnClipboard = false;
	if (OpenClipboard())
	{
		UINT enumFormat = 0;
		do 
		{
			if (enumFormat == m_cFormat)
			{
				bUDiffOnClipboard = true;
			}
		} while((enumFormat = EnumClipboardFormats(enumFormat))!=0);
		CloseClipboard();
	}

	if (GetDlgItem(IDC_BASEFILEEDIT)->IsWindowEnabled())
	{
		m_sUnifiedDiffFile.Empty();
		m_sPatchDirectory.Empty();
	}
	else
	{
		m_sBaseFile.Empty();
		m_sYourFile.Empty();
		m_sTheirFile.Empty();
	}
	UpdateData(FALSE);
	CString sFile;
	if (!m_sUnifiedDiffFile.IsEmpty())
		if (!PathFileExists(m_sUnifiedDiffFile))
			sFile = m_sUnifiedDiffFile;
	if (!m_sPatchDirectory.IsEmpty())
		if (!PathFileExists(m_sPatchDirectory))
			sFile = m_sPatchDirectory;
	if (!m_sBaseFile.IsEmpty())
		if (!PathFileExists(m_sBaseFile))
			sFile = m_sBaseFile;
	if (!m_sYourFile.IsEmpty())
		if (!PathFileExists(m_sYourFile))
			sFile = m_sYourFile;
	if (!m_sTheirFile.IsEmpty())
		if (!PathFileExists(m_sTheirFile))
			sFile = m_sTheirFile;

	if (bUDiffOnClipboard && m_bFromClipboard)
	{
		if (OpenClipboard()) 
		{ 
			HGLOBAL hglb = GetClipboardData(m_cFormat); 
			LPCSTR lpstr = (LPCSTR)GlobalLock(hglb); 

			DWORD len = GetTempPath(0, NULL);
			auto_buffer<TCHAR> path(len+1);
			auto_buffer<TCHAR> tempF(len+100);
			GetTempPath (len+1, path);
			GetTempFileName (path, TEXT("tsm"), 0, tempF);
			CString sTempFile = CString(tempF);

			FILE * outFile;
			size_t patchlen = strlen(lpstr);
			_tfopen_s(&outFile, sTempFile, _T("wb"));
			if(outFile)
			{
				size_t size = fwrite(lpstr, sizeof(char), patchlen, outFile);
				if (size < patchlen)
					bUDiffOnClipboard = false;
				else
				{
					m_sUnifiedDiffFile = sTempFile;
					UpdateData(FALSE);
					sFile.Empty();
				}
				fclose(outFile);
			}
			GlobalUnlock(hglb); 
			CloseClipboard(); 
		} 

	}

	if (!sFile.IsEmpty())
	{
		CString sErr;
		sErr.Format(IDS_ERR_PATCH_INVALIDPATCHFILE, (LPCTSTR)sFile);
		MessageBox(sErr, NULL, MB_ICONERROR);
		return;
	}
	CRegDWORD lastRadioButton(_T("Software\\TortoiseMerge\\OpenRadio"), IDC_MERGERADIO);
	lastRadioButton = GetCheckedRadioButton(IDC_MERGERADIO, IDC_APPLYRADIO);
	CStandAloneDialog::OnOK();
}

void COpenDlg::OnChangeCbChain(HWND hWndRemove, HWND hWndAfter)
{
	CStandAloneDialog::OnChangeCbChain(hWndRemove, hWndAfter);
}

bool COpenDlg::CheckAndEnableClipboardChecker()
{
	int radio = GetCheckedRadioButton(IDC_MERGERADIO, IDC_APPLYRADIO);
	bool bUDiffOnClipboard = false;
	if (radio == IDC_APPLYRADIO)
	{
		if (OpenClipboard())
		{
			UINT enumFormat = 0;
			do 
			{
				if (enumFormat == m_cFormat)
				{
					bUDiffOnClipboard = true;
				}
			} while((enumFormat = EnumClipboardFormats(enumFormat))!=0);
			CloseClipboard();
		}
	}

	DialogEnableWindow(IDC_PATCHFROMCLIPBOARD, bUDiffOnClipboard);
	return bUDiffOnClipboard;
}

void COpenDlg::OnDrawClipboard()
{
	CheckAndEnableClipboardChecker();
	CStandAloneDialog::OnDrawClipboard();
}

void COpenDlg::OnDestroy()
{
	ChangeClipboardChain(m_nextViewer);
	CStandAloneDialog::OnDestroy();
}

void COpenDlg::OnBnClickedPatchfromclipboard()
{
	UpdateData();
	DialogEnableWindow(IDC_DIFFFILEEDIT, !m_bFromClipboard);
	DialogEnableWindow(IDC_DIFFFILEBROWSE, !m_bFromClipboard);
}
