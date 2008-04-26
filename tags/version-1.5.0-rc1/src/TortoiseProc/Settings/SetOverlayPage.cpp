// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2008 - TortoiseSVN

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
#include "TortoiseProc.h"
#include "SetOverlayPage.h"
#include "SetOverlayIcons.h"
#include "Globals.h"
#include "ShellUpdater.h"
#include "..\TSVNCache\CacheInterface.h"
#include ".\setoverlaypage.h"
#include "MessageBox.h"


IMPLEMENT_DYNAMIC(CSetOverlayPage, ISettingsPropPage)
CSetOverlayPage::CSetOverlayPage()
	: ISettingsPropPage(CSetOverlayPage::IDD)
	, m_bRemovable(FALSE)
	, m_bNetwork(FALSE)
	, m_bFixed(FALSE)
	, m_bCDROM(FALSE)
	, m_bRAM(FALSE)
	, m_bUnknown(FALSE)
	, m_bOnlyExplorer(FALSE)
	, m_sExcludePaths(_T(""))
	, m_sIncludePaths(_T(""))
	, m_bUnversionedAsModified(FALSE)
	, m_bFloppy(FALSE)
	, m_bShowExcludedAsNormal(TRUE)
	, m_bShowIgnoredOverlay(FALSE)
	, m_bShowUnversionedOverlay(FALSE)
{
	m_regOnlyExplorer = CRegDWORD(_T("Software\\TortoiseSVN\\LoadDllOnlyInExplorer"), FALSE);
	m_regDriveMaskRemovable = CRegDWORD(_T("Software\\TortoiseSVN\\DriveMaskRemovable"));
	m_regDriveMaskFloppy = CRegDWORD(_T("Software\\TortoiseSVN\\DriveMaskFloppy"));
	m_regDriveMaskRemote = CRegDWORD(_T("Software\\TortoiseSVN\\DriveMaskRemote"));
	m_regDriveMaskFixed = CRegDWORD(_T("Software\\TortoiseSVN\\DriveMaskFixed"), TRUE);
	m_regDriveMaskCDROM = CRegDWORD(_T("Software\\TortoiseSVN\\DriveMaskCDROM"));
	m_regDriveMaskRAM = CRegDWORD(_T("Software\\TortoiseSVN\\DriveMaskRAM"));
	m_regDriveMaskUnknown = CRegDWORD(_T("Software\\TortoiseSVN\\DriveMaskUnknown"));
	m_regExcludePaths = CRegString(_T("Software\\TortoiseSVN\\OverlayExcludeList"));
	m_regIncludePaths = CRegString(_T("Software\\TortoiseSVN\\OverlayIncludeList"));
	m_regCacheType = CRegDWORD(_T("Software\\TortoiseSVN\\CacheType"), GetSystemMetrics(SM_REMOTESESSION) ? 2 : 1);
	m_regUnversionedAsModified = CRegDWORD(_T("Software\\TortoiseSVN\\UnversionedAsModified"), FALSE);
	m_regShowExcludedAsNormal = CRegDWORD(_T("Software\\TortoiseSVN\\ShowExcludedAsNormal"), TRUE);
	m_regShowIgnoredOverlay = CRegDWORD(_T("Software\\TortoiseSVN\\ShowIgnoredOverlay"), TRUE);
	m_regShowUnversionedOverlay = CRegDWORD(_T("Software\\TortoiseSVN\\ShowUnversionedOverlay"), TRUE);

	m_bOnlyExplorer = m_regOnlyExplorer;
	m_bRemovable = m_regDriveMaskRemovable;
	m_bFloppy = m_regDriveMaskFloppy;
	m_bNetwork = m_regDriveMaskRemote;
	m_bFixed = m_regDriveMaskFixed;
	m_bCDROM = m_regDriveMaskCDROM;
	m_bRAM = m_regDriveMaskRAM;
	m_bUnknown = m_regDriveMaskUnknown;
	m_bUnversionedAsModified = m_regUnversionedAsModified;
	m_bShowIgnoredOverlay = m_regShowIgnoredOverlay;
	m_bShowUnversionedOverlay = m_regShowUnversionedOverlay;
	m_bShowExcludedAsNormal = m_regShowExcludedAsNormal;
	m_sExcludePaths = m_regExcludePaths;
	m_sExcludePaths.Replace(_T("\n"), _T("\r\n"));
	m_sIncludePaths = m_regIncludePaths;
	m_sIncludePaths.Replace(_T("\n"), _T("\r\n"));
	m_dwCacheType = m_regCacheType;
}

CSetOverlayPage::~CSetOverlayPage()
{
}

void CSetOverlayPage::DoDataExchange(CDataExchange* pDX)
{
	ISettingsPropPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_REMOVABLE, m_bRemovable);
	DDX_Check(pDX, IDC_NETWORK, m_bNetwork);
	DDX_Check(pDX, IDC_FIXED, m_bFixed);
	DDX_Check(pDX, IDC_CDROM, m_bCDROM);
	DDX_Check(pDX, IDC_RAM, m_bRAM);
	DDX_Check(pDX, IDC_UNKNOWN, m_bUnknown);
	DDX_Check(pDX, IDC_ONLYEXPLORER, m_bOnlyExplorer);
	DDX_Text(pDX, IDC_EXCLUDEPATHS, m_sExcludePaths);
	DDX_Text(pDX, IDC_INCLUDEPATHS, m_sIncludePaths);
	DDX_Check(pDX, IDC_UNVERSIONEDASMODIFIED, m_bUnversionedAsModified);
	DDX_Check(pDX, IDC_FLOPPY, m_bFloppy);
	DDX_Check(pDX, IDC_SHOWEXCLUDEDASNORMAL, m_bShowExcludedAsNormal);
	DDX_Check(pDX, IDC_SHOWIGNOREDOVERLAY, m_bShowIgnoredOverlay);
	DDX_Check(pDX, IDC_SHOWUNVERSIONEDOVERLAY, m_bShowUnversionedOverlay);
}

BEGIN_MESSAGE_MAP(CSetOverlayPage, ISettingsPropPage)
	ON_BN_CLICKED(IDC_REMOVABLE, OnChange)
	ON_BN_CLICKED(IDC_FLOPPY, &CSetOverlayPage::OnChange)
	ON_BN_CLICKED(IDC_NETWORK, OnChange)
	ON_BN_CLICKED(IDC_FIXED, OnChange)
	ON_BN_CLICKED(IDC_CDROM, OnChange)
	ON_BN_CLICKED(IDC_UNKNOWN, OnChange)
	ON_BN_CLICKED(IDC_RAM, OnChange)
	ON_BN_CLICKED(IDC_ONLYEXPLORER, OnChange)
	ON_EN_CHANGE(IDC_EXCLUDEPATHS, OnChange)
	ON_EN_CHANGE(IDC_INCLUDEPATHS, OnChange)
	ON_BN_CLICKED(IDC_CACHEDEFAULT, &CSetOverlayPage::OnChange)
	ON_BN_CLICKED(IDC_CACHESHELL, &CSetOverlayPage::OnChange)
	ON_BN_CLICKED(IDC_CACHENONE, &CSetOverlayPage::OnChange)
	ON_BN_CLICKED(IDC_UNVERSIONEDASMODIFIED, &CSetOverlayPage::OnChange)
	ON_BN_CLICKED(IDC_SHOWEXCLUDEDASNORMAL, &CSetOverlayPage::OnChange)
	ON_BN_CLICKED(IDC_SHOWIGNOREDOVERLAY, &CSetOverlayPage::OnChange)
	ON_BN_CLICKED(IDC_SHOWUNVERSIONEDOVERLAY, &CSetOverlayPage::OnChange)
END_MESSAGE_MAP()

BOOL CSetOverlayPage::OnInitDialog()
{
	ISettingsPropPage::OnInitDialog();

	switch (m_dwCacheType)
	{
	case 0:
		CheckRadioButton(IDC_CACHEDEFAULT, IDC_CACHENONE, IDC_CACHENONE);
		break;
	default:
	case 1:
		CheckRadioButton(IDC_CACHEDEFAULT, IDC_CACHENONE, IDC_CACHEDEFAULT);
		break;
	case 2:
		CheckRadioButton(IDC_CACHEDEFAULT, IDC_CACHENONE, IDC_CACHESHELL);
		break;
	}
	GetDlgItem(IDC_UNVERSIONEDASMODIFIED)->EnableWindow(m_dwCacheType == 1);
	GetDlgItem(IDC_FLOPPY)->EnableWindow(m_bRemovable);

	m_tooltips.Create(this);
	m_tooltips.AddTool(IDC_ONLYEXPLORER, IDS_SETTINGS_ONLYEXPLORER_TT);
	m_tooltips.AddTool(IDC_EXCLUDEPATHS, IDS_SETTINGS_EXCLUDELIST_TT);	
	m_tooltips.AddTool(IDC_INCLUDEPATHS, IDS_SETTINGS_INCLUDELIST_TT);
	m_tooltips.AddTool(IDC_CACHEDEFAULT, IDS_SETTINGS_CACHEDEFAULT_TT);
	m_tooltips.AddTool(IDC_CACHESHELL, IDS_SETTINGS_CACHESHELL_TT);
	m_tooltips.AddTool(IDC_CACHENONE, IDS_SETTINGS_CACHENONE_TT);
	m_tooltips.AddTool(IDC_UNVERSIONEDASMODIFIED, IDS_SETTINGS_UNVERSIONEDASMODIFIED_TT);
	m_tooltips.AddTool(IDC_SHOWEXCLUDEDASNORMAL, IDS_SETTINGS_SHOWEXCLUDEDASNORMAL_TT);

	UpdateData(FALSE);

	return TRUE;
}

BOOL CSetOverlayPage::PreTranslateMessage(MSG* pMsg)
{
	m_tooltips.RelayEvent(pMsg);
	return ISettingsPropPage::PreTranslateMessage(pMsg);
}

void CSetOverlayPage::OnChange()
{
	UpdateData();
	int id = GetCheckedRadioButton(IDC_CACHEDEFAULT, IDC_CACHENONE);
	switch (id)
	{
	default:
	case IDC_CACHEDEFAULT:
		m_dwCacheType = 1;
		break;
	case IDC_CACHESHELL:
		m_dwCacheType = 2;
		break;
	case IDC_CACHENONE:
		m_dwCacheType = 0;
		break;
	}
	GetDlgItem(IDC_UNVERSIONEDASMODIFIED)->EnableWindow(m_dwCacheType == 1);
	GetDlgItem(IDC_FLOPPY)->EnableWindow(m_bRemovable);
	SetModified();
}

BOOL CSetOverlayPage::OnApply()
{
	UpdateData();
	m_regOnlyExplorer = m_bOnlyExplorer;
	if (m_regOnlyExplorer.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regOnlyExplorer.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);
	if (DWORD(m_regDriveMaskRemovable) != DWORD(m_bRemovable))
		m_restart = Restart_Cache;
	m_regDriveMaskRemovable = m_bRemovable;
	if (m_regDriveMaskRemovable.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regDriveMaskRemovable.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);

	if (DWORD(m_regDriveMaskFloppy) != DWORD(m_bFloppy))
		m_restart = Restart_Cache;
	m_regDriveMaskFloppy = m_bFloppy;
	if (m_regDriveMaskFloppy.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regDriveMaskFloppy.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);

	if (DWORD(m_regDriveMaskRemote) != DWORD(m_bNetwork))
		m_restart = Restart_Cache;
	m_regDriveMaskRemote = m_bNetwork;
	if (m_regDriveMaskRemote.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regDriveMaskRemote.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);

	if (DWORD(m_regDriveMaskFixed) != DWORD(m_bFixed))
		m_restart = Restart_Cache;
	m_regDriveMaskFixed = m_bFixed;
	if (m_regDriveMaskFixed.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regDriveMaskFixed.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);

	if (DWORD(m_regDriveMaskCDROM) != DWORD(m_bCDROM))
		m_restart = Restart_Cache;
	m_regDriveMaskCDROM = m_bCDROM;
	if (m_regDriveMaskCDROM.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regDriveMaskCDROM.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);

	if (DWORD(m_regDriveMaskRAM) != DWORD(m_bRAM))
		m_restart = Restart_Cache;
	m_regDriveMaskRAM = m_bRAM;
	if (m_regDriveMaskRAM.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regDriveMaskRAM.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);

	if (DWORD(m_regDriveMaskUnknown) != DWORD(m_bUnknown))
		m_restart = Restart_Cache;
	m_regDriveMaskUnknown = m_bUnknown;
	if (m_regDriveMaskUnknown.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regDriveMaskUnknown.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);

	if (m_sExcludePaths.Compare(CString(m_regExcludePaths)))
		m_restart = Restart_Cache;
	m_sExcludePaths.Replace(_T("\r"), _T(""));
	if (m_sExcludePaths.Right(1).Compare(_T("\n"))!=0)
		m_sExcludePaths += _T("\n");
	m_regExcludePaths = m_sExcludePaths;
	if (m_regExcludePaths.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regExcludePaths.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);
	m_sExcludePaths.Replace(_T("\n"), _T("\r\n"));
	m_sIncludePaths.Replace(_T("\r"), _T(""));
	if (m_sIncludePaths.Right(1).Compare(_T("\n"))!=0)
		m_sIncludePaths += _T("\n");
	if (m_sIncludePaths.Compare(CString(m_regIncludePaths)))
		m_restart = Restart_Cache;
	m_regIncludePaths = m_sIncludePaths;
	if (m_regIncludePaths.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regIncludePaths.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);
	m_sIncludePaths.Replace(_T("\n"), _T("\r\n"));

	if (DWORD(m_regUnversionedAsModified) != DWORD(m_bUnversionedAsModified))
		m_restart = Restart_Cache;
	m_regUnversionedAsModified = m_bUnversionedAsModified;
	if (m_regUnversionedAsModified.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regUnversionedAsModified.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);
	m_regShowIgnoredOverlay = m_bShowIgnoredOverlay;
	if (m_regShowIgnoredOverlay.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regShowIgnoredOverlay.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);
	m_regShowUnversionedOverlay = m_bShowUnversionedOverlay;
	if (m_regShowUnversionedOverlay.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regShowUnversionedOverlay.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);
	if (DWORD(m_regShowExcludedAsNormal) != DWORD(m_bShowExcludedAsNormal))
		m_restart = Restart_Cache;
	m_regShowExcludedAsNormal = m_bShowExcludedAsNormal;
	if (m_regShowExcludedAsNormal.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regShowExcludedAsNormal.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);

	m_regCacheType = m_dwCacheType;
	if (m_regCacheType.LastError != ERROR_SUCCESS)
		CMessageBox::Show(m_hWnd, m_regCacheType.getErrorString(), _T("TortoiseSVN"), MB_ICONERROR);
	if (m_dwCacheType != 1)
	{
		// close the possible running cache process
		HWND hWnd = ::FindWindow(TSVN_CACHE_WINDOW_NAME, TSVN_CACHE_WINDOW_NAME);
		if (hWnd)
		{
			::PostMessage(hWnd, WM_CLOSE, NULL, NULL);
		}
		m_restart = Restart_None;
	}
	SetModified(FALSE);
	return ISettingsPropPage::OnApply();
}




