// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2004 - Tim Kemp and Stefan Kueng

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

#include "stdafx.h"
#include "TortoiseProc.h"
#include "messagebox.h"
#include "LogPromptDlg.h"
#include "UnicodeUtils.h"
#include ".\logpromptdlg.h"
#include "DirFileEnum.h"
#include "SVNConfig.h"
#include "SVNProperties.h"

// CLogPromptDlg dialog
BOOL	CLogPromptDlg::m_bAscending = FALSE;
int		CLogPromptDlg::m_nSortedColumn = -1;

IMPLEMENT_DYNAMIC(CLogPromptDlg, CResizableDialog)
CLogPromptDlg::CLogPromptDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CLogPromptDlg::IDD, pParent)
	, m_sLogMessage(_T(""))
	, m_nTotal(0)
	, m_nSelected(0)
	, m_bRecursive(FALSE)
	, m_nTargetCount(0)
	, m_bShowUnversioned(FALSE)
	, m_bBlock(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_templist.RemoveAll();
}

CLogPromptDlg::~CLogPromptDlg()
{
	for (int i=0; i<m_templist.GetCount(); i++)
	{
		DeleteFile(m_templist.GetAt(i));
	}
	m_templist.RemoveAll();
	for (int i=0; i<m_arData.GetCount(); i++)
	{
		Data * data = m_arData.GetAt(i);
		delete data;
	} // for (int i=0; i<m_arData.GetCount(); i++) 
	m_arData.RemoveAll();
	for (int i=0; i<m_arAllData.GetCount(); i++)
	{
		Data * data = m_arAllData.GetAt(i);
		delete data;
	} // for (int i=0; i<m_arAllData.GetCount(); i++) 
	m_arAllData.RemoveAll();
}

void CLogPromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOGMESSAGE, m_sLogMessage);
	DDX_Control(pDX, IDC_FILELIST, m_ListCtrl);
	DDX_Control(pDX, IDC_LOGMESSAGE, m_LogMessage);
	DDX_Check(pDX, IDC_SHOWUNVERSIONED, m_bShowUnversioned);
	DDX_Control(pDX, IDC_SELECTALL, m_SelectAll);
}


BEGIN_MESSAGE_MAP(CLogPromptDlg, CResizableDialog)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SETCURSOR()
	ON_NOTIFY(NM_DBLCLK, IDC_FILELIST, OnNMDblclkFilelist)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILELIST, OnLvnItemchangedFilelist)
	ON_BN_CLICKED(IDC_SELECTALL, OnBnClickedSelectall)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnHdnItemclickFilelist)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDHELP, OnBnClickedHelp)
	ON_BN_CLICKED(IDC_SHOWUNVERSIONED, OnBnClickedShowunversioned)
	ON_EN_CHANGE(IDC_LOGMESSAGE, OnEnChangeLogmessage)
END_MESSAGE_MAP()


// CLogPromptDlg message handlers
// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void CLogPromptDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CResizableDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR CLogPromptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CLogPromptDlg::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	LOGFONT LogFont;
	LogFont.lfHeight         = -MulDiv((DWORD)CRegDWORD(_T("Software\\TortoiseSVN\\LogFontSize"), 8), GetDeviceCaps(this->GetDC()->m_hDC, LOGPIXELSY), 72);
	LogFont.lfWidth          = 0;
	LogFont.lfEscapement     = 0;
	LogFont.lfOrientation    = 0;
	LogFont.lfWeight         = 400;
	LogFont.lfItalic         = 0;
	LogFont.lfUnderline      = 0;
	LogFont.lfStrikeOut      = 0;
	LogFont.lfCharSet        = DEFAULT_CHARSET;
	LogFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
	LogFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
	LogFont.lfQuality        = DRAFT_QUALITY;
	LogFont.lfPitchAndFamily = FF_DONTCARE | FIXED_PITCH;
	_tcscpy(LogFont.lfFaceName, (LPCTSTR)(CString)CRegString(_T("Software\\TortoiseSVN\\LogFontName"), _T("Courier New")));
	m_logFont.CreateFontIndirect(&LogFont);
	GetDlgItem(IDC_LOGMESSAGE)->SetFont(&m_logFont);

	m_regAddBeforeCommit = CRegDWORD(_T("Software\\TortoiseSVN\\AddBeforeCommit"), TRUE);
	m_bShowUnversioned = m_regAddBeforeCommit;

	UpdateData(FALSE);

	OnEnChangeLogmessage();

	CString temp = m_sPath;

	//set the listcontrol to support checkboxes
	m_ListCtrl.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_ListCtrl.DeleteAllItems();
	int c = ((CHeaderCtrl*)(m_ListCtrl.GetDlgItem(0)))->GetItemCount()-1;
	while (c>=0)
		m_ListCtrl.DeleteColumn(c--);
	temp.LoadString(IDS_LOGPROMPT_FILE);
	m_ListCtrl.InsertColumn(0, temp);
	temp.LoadString(IDS_LOGPROMPT_STATUS);
	m_ListCtrl.InsertColumn(1, temp);

	m_ListCtrl.SetRedraw(false);
	int mincol = 0;
	int maxcol = ((CHeaderCtrl*)(m_ListCtrl.GetDlgItem(0)))->GetItemCount()-1;
	int col;
	for (col = mincol; col <= maxcol; col++)
	{
		m_ListCtrl.SetColumnWidth(col,LVSCW_AUTOSIZE_USEHEADER);
	}

	//first start a thread to obtain the file list with the status without
	//blocking the dialog
	DWORD dwThreadId;
	if ((m_hThread = CreateThread(NULL, 0, &StatusThread, this, 0, &dwThreadId))==0)
	{
		CMessageBox::Show(this->m_hWnd, IDS_ERR_THREADSTARTFAILED, IDS_APPNAME, MB_OK | MB_ICONERROR);
	}
	m_bBlock = TRUE;
	m_ListCtrl.UpdateData(FALSE);

	m_tooltips.Create(this);
	m_SelectAll.SetCheck(BST_INDETERMINATE);
	GetDlgItem(IDC_LOGMESSAGE)->SetFocus();

	AddAnchor(IDC_COMMITLABEL, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_COMMIT_TO, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_LOGMESSAGE, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_FILELIST, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_SHOWUNVERSIONED, BOTTOM_LEFT);
	AddAnchor(IDC_SELECTALL, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_HINTLABEL, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_STATISTICS, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDHELP, BOTTOM_RIGHT);
	EnableSaveRestore(_T("LogPromptDlg"));
	CenterWindow(CWnd::FromHandle(hWndExplorer));

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CLogPromptDlg::OnLvnItemchangedFilelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (m_bBlock)
		return;
	if ((pNMLV->uNewState==0)||(pNMLV->uNewState & LVIS_SELECTED))
		return;
	m_bBlock = TRUE;
	Data * data = m_arData.GetAt(pNMLV->iItem);
	//was the item checked?
	if (m_ListCtrl.GetCheck(pNMLV->iItem))
	{
		//yes, is it an unversioned item?
		if (data->status == svn_wc_status_unversioned)
		{
			CMessageBox::ShowCheck(this->m_hWnd, 
									IDS_LOGPROMPT_ASKADD, 
									IDS_APPNAME, 
									MB_OK | MB_ICONQUESTION, 
									_T("AddUnversionedFilesOnCommitMsgBox"), 
									IDS_MSGBOX_DONOTSHOW);
			//we need to check the parent folder too
			CString folderpath = data->path;
			for (int i=0; i<m_ListCtrl.GetItemCount(); i++)
			{
				if (!m_ListCtrl.GetCheck(i))
				{
					Data * d = m_arData.GetAt(i);
					CString t = d->path;
					if (CUtils::PathIsParent(t, folderpath))
					{
						m_ListCtrl.SetCheck(i, TRUE);
						d->checked = TRUE;
						m_nSelected++;
					} // if (CUtils::PathIsParent(t, folderpath)) 
				}
			} // for (int i=0; i<m_addListCtrl.GetItemCount(); i++)
		} // if (data->status == svn_wc_status_unversioned)
		//is it a deleted item?
		if (data->status == svn_wc_status_deleted)
		{
			CString folderpath = data->path;
			if (PathIsDirectory(folderpath))
			{
				//check all children of this folder
				for (int j=0; j<m_ListCtrl.GetItemCount(); j++)
				{
					if (!m_ListCtrl.GetCheck(j))
					{
						Data * dd = m_arData.GetAt(j);
						CString tt = dd->path;
						if (folderpath.CompareNoCase(tt.Left(folderpath.GetLength()))==0)
						{
							m_ListCtrl.SetCheck(j, TRUE);
							dd->checked = TRUE;
							m_nSelected++;
						} // if (t.CompareNoCase(tt.Left(t.GetLength()))==0) 
					} // if (!m_ListCtrl.GetCheck(j)) 
				} // for (j=0; j<m_ListCtrl.GetItemCount(); j++) 
			} // if (PathIsDirectory(folderpath)) 

			//we need to check the parent folder if it is deleted and all children of that
			//parent folder too!
			for (int i=0; i<m_ListCtrl.GetItemCount(); i++)
			{
				if (!m_ListCtrl.GetCheck(i))
				{
					Data * d = m_arData.GetAt(i);
					CString t = d->path;
					if (CUtils::PathIsParent(t, folderpath))
					{
						if (d->status == svn_wc_status_deleted)
						{
							m_ListCtrl.SetCheck(i, TRUE);
							d->checked = TRUE;
							m_nSelected++;
							//now we need to check all children of this parent folder
							for (int j=0; j<m_ListCtrl.GetItemCount(); j++)
							{
								if (!m_ListCtrl.GetCheck(j))
								{
									Data * dd = m_arData.GetAt(j);
									CString tt = dd->path;
									if (t.CompareNoCase(tt.Left(t.GetLength()))==0)
									{
										m_ListCtrl.SetCheck(j, TRUE);
										dd->checked = TRUE;
										m_nSelected++;
									} // if (t.CompareNoCase(tt.Left(t.GetLength()))==0) 
								} // if (!m_ListCtrl.GetCheck(j)) 
							} // for (j=0; j<m_ListCtrl.GetItemCount(); j++) 
						} // if (d->status == svn_wc_status_deleted) 
					} // if (CUtils::PathIsParent(t, folderpath)) 
				} // if (!m_ListCtrl.GetCheck(i)) 
			} // for (int i=0; i<m_addListCtrl.GetItemCount(); i++)
		} // if (data->status == svn_wc_status_deleted) 
		data->checked = TRUE;
		m_nSelected++;
	} // if (m_ListCtrl.GetCheck(pNMLV->iItem)) 
	else
	{
		//item was unchecked
		if (PathIsDirectory(data->path))
		{
			//disable all files within that folder
			CString folderpath = data->path;
			for (int i=0; i<m_ListCtrl.GetItemCount(); i++)
			{
				if (m_ListCtrl.GetCheck(i))
				{
					Data * d = m_arData.GetAt(i);
					CString t = d->path;
					if (CUtils::PathIsParent(folderpath, t))
					{
						m_ListCtrl.SetCheck(i, FALSE);
						d->checked = FALSE;
						m_nSelected--;
					} // if (CUtils::PathIsParent(folderpath, t)) 
				} // if (m_ListCtrl.GetCheck(i)) 
			} // for (int i=0; i<m_addListCtrl.GetItemCount(); i++)
		} // if (PathIsDirectory(m_arFileList.GetAt(index)))
		else if (data->status == svn_wc_status_deleted)
		{
			//a "deleted" file was unchecked, so uncheck all parent folders
			//and all children of those parents
			CString folderpath = data->path;
			for (int i=0; i<m_ListCtrl.GetItemCount(); i++)
			{
				if (m_ListCtrl.GetCheck(i))
				{
					Data * d = m_arData.GetAt(i);
					CString t = d->path;
					if (CUtils::PathIsParent(t, folderpath))
					{
						if (d->status == svn_wc_status_deleted)
						{
							m_ListCtrl.SetCheck(i, FALSE);
							d->checked = FALSE;
							m_nSelected--;
							//now we need to check all children of this parent folder
							t += _T("\\");
							for (int j=0; j<m_ListCtrl.GetItemCount(); j++)
							{
								if (m_ListCtrl.GetCheck(j))
								{
									Data * dd = m_arData.GetAt(j);
									CString tt = dd->path;
									if (t.CompareNoCase(tt.Left(t.GetLength()))==0)
									{
										m_ListCtrl.SetCheck(j, FALSE);
										dd->checked = FALSE;
										m_nSelected--;
									} // if (t.CompareNoCase(tt.Left(t.GetLength()))==0) 
								} // if (m_ListCtrl.GetCheck(j)) 
							} // for (j=0; j<m_ListCtrl.GetItemCount(); j++) 
						} // if (d->status == svn_wc_status_deleted) 
					} // if (CUtils::PathIsParent(t, folderpath)) 
				} // if (m_ListCtrl.GetCheck(i))  
			} // for (int i=0; i<m_addListCtrl.GetItemCount(); i++)
		} 
		data->checked = FALSE;
		m_nSelected--;
	} 
	m_bBlock = FALSE;
	CString sStats;
	sStats.Format(IDS_LOGPROMPT_STATISTICSFORMAT, m_nSelected, m_nTotal);
	GetDlgItem(IDC_STATISTICS)->SetWindowText(sStats);
	if (m_nSelected == 0)
		m_SelectAll.SetCheck(BST_UNCHECKED);
	else if (m_nSelected != m_nTotal)
		m_SelectAll.SetCheck(BST_INDETERMINATE);
	else
		m_SelectAll.SetCheck(BST_CHECKED);
}

void CLogPromptDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd == &m_ListCtrl)
	{
		int selIndex = m_ListCtrl.GetSelectionMark();
		if ((point.x == -1) && (point.y == -1))
		{
			CRect rect;
			m_ListCtrl.GetItemRect(selIndex, &rect, LVIR_LABEL);
			m_ListCtrl.ClientToScreen(&rect);
			point.x = rect.left + rect.Width()/2;
			point.y = rect.top + rect.Height()/2;
		}
		if (selIndex >= 0)
		{
			Data * data = m_arData.GetAt(selIndex);
			CString filepath = data->path;
			svn_wc_status_kind wcStatus = data->status;
			//entry is selected, now show the popup menu
			CMenu popup;
			if (popup.CreatePopupMenu())
			{
				CString temp;
				if ((wcStatus > svn_wc_status_normal)&&(wcStatus != svn_wc_status_added))
				{
					temp.LoadString(IDS_LOG_COMPAREWITHBASE);
					popup.AppendMenu(MF_STRING | MF_ENABLED, ID_COMPARE, temp);
					popup.SetDefaultItem(ID_COMPARE, FALSE);
					temp.LoadString(IDS_MENUREVERT);
					popup.AppendMenu(MF_STRING | MF_ENABLED, ID_REVERT, temp);
				} // if ((wcStatus > svn_wc_status_normal)&&(wcStatus != svn_wc_status_added))
				if ((wcStatus > svn_wc_status_normal)&&(wcStatus != svn_wc_status_deleted))
				{
					temp.LoadString(IDS_REPOBROWSE_OPEN);
					popup.AppendMenu(MF_STRING | MF_ENABLED, ID_OPEN, temp);
				} // if ((wcStatus > svn_wc_status_normal)&&(wcStatus != svn_wc_status_deleted))
				if (wcStatus == svn_wc_status_unversioned)
				{
					temp.LoadString(IDS_REPOBROWSE_DELETE);
					popup.AppendMenu(MF_STRING | MF_ENABLED, ID_DELETE, temp);
					temp.LoadString(IDS_MENUIGNORE);
					popup.AppendMenu(MF_STRING | MF_ENABLED, ID_IGNORE, temp);
				}
				temp.LoadString(IDS_MENUREFRESH);
				popup.AppendMenu(MF_STRING | MF_ENABLED, ID_REFRESH, temp);
				int cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_NONOTIFY, point.x, point.y, this, 0);
				m_bBlock = TRUE;
				theApp.DoWaitCursor(1);
				switch (cmd)
				{
				case ID_REVERT:
					{
						if (CMessageBox::Show(this->m_hWnd, IDS_PROC_WARNREVERT, IDS_APPNAME, MB_YESNO | MB_ICONQUESTION)==IDYES)
						{
							SVN svn;
							if (!svn.Revert(filepath, FALSE))
							{
								CMessageBox::Show(this->m_hWnd, svn.GetLastErrorMessage(), _T("TortoiseSVN"), MB_ICONERROR);
							}
							else
							{
								//since the entry got reverted we need to remove
								//it from the list too
								m_nTotal--;
								if (m_ListCtrl.GetCheck(selIndex))
									m_nSelected--;
								m_ListCtrl.DeleteItem(selIndex);
								m_arData.RemoveAt(selIndex);
							}
						} // if (CMessageBox::Show(this->m_hWnd, IDS_PROC_WARNREVERT, IDS_APPNAME, MB_YESNO | MB_ICONQUESTION)==IDYES)  
					} 
					break;
				case ID_COMPARE:
					{
						StartDiff(selIndex);
					}
					break;
				case ID_REFRESH:
					{
						Refresh();
					}
					break;
				case ID_OPEN:
					{
						ShellExecute(this->m_hWnd, _T("open"), filepath, NULL, NULL, SW_SHOW);
					}
					break;
				case ID_DELETE:
					{
						TCHAR buf[MAX_PATH];
						ZeroMemory(buf, sizeof(buf));
						_tcsncpy(buf, filepath, MAX_PATH);
						SHFILEOPSTRUCT fileop;
						fileop.hwnd = this->m_hWnd;
						fileop.wFunc = FO_DELETE;
						fileop.pFrom = buf;
						fileop.pTo = _T("");
						fileop.fFlags = FOF_ALLOWUNDO | FOF_NO_CONNECTED_ELEMENTS;
						fileop.lpszProgressTitle = _T("deleting file");
						SHFileOperation(&fileop);

						if (! fileop.fAnyOperationsAborted)
						{
							if (m_ListCtrl.GetCheck(selIndex))
								m_nSelected--;
							m_nTotal--;
							m_ListCtrl.DeleteItem(selIndex);
							m_arData.RemoveAt(selIndex);
						} // if (! fileop.fAnyOperationsAborted) 
					}
					break;
				case ID_IGNORE:
					{
						filepath.Replace('\\', '/');
						CString name = filepath.Mid(filepath.ReverseFind('/')+1);
						CString parentfolder = filepath.Left(filepath.ReverseFind('/'));
						SVNProperties props(parentfolder);
						CStringA value;
						for (int i=0; i<props.GetCount(); i++)
						{
							CString propname(props.GetItemName(i).c_str());
							if (propname.CompareNoCase(_T("svn:ignore"))==0)
							{
								stdstring stemp;
								stdstring tmp = props.GetItemValue(i);
								//treat values as normal text even if they're not
								value = (char *)tmp.c_str();
							}
						}
						if (value.IsEmpty())
							value = name;
						else
						{
							value = value.Trim("\n\r");
							value += "\n";
							value += name;
							value.Remove('\r');
						}
						if (!props.Add(_T("svn:ignore"), value))
						{
							CString temp;
							temp.Format(IDS_ERR_FAILEDIGNOREPROPERTY, name);
							CMessageBox::Show(this->m_hWnd, temp, _T("TortoiseSVN"), MB_ICONERROR);
						} // if (!props.Add(_T("svn:ignore"), value))
						Refresh();
					}
					break;
				default:
					m_bBlock = FALSE;
					break;
				} // switch (cmd)
				m_bBlock = FALSE;
				theApp.DoWaitCursor(-1);
			} // if (popup.CreatePopupMenu())
		} // if (selIndex >= 0)
	}
	CString sStats;
	sStats.Format(IDS_LOGPROMPT_STATISTICSFORMAT, m_nSelected, m_nTotal);
	GetDlgItem(IDC_STATISTICS)->SetWindowText(sStats);
}

void CLogPromptDlg::OnNMDblclkFilelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (m_bBlock)
		return;

	StartDiff(pNMLV->iItem);
}

void CLogPromptDlg::StartDiff(int fileindex)
{
	if (fileindex < 0)
		return;
	Data * data = m_arData.GetAt(fileindex);
	if (data->status == svn_wc_status_added)
		return;		//we don't compare an added file to itself
	if (data->status == svn_wc_status_deleted)
		return;		//we don't compare a deleted file (nothing) with something
	if (PathIsDirectory(data->path))
		return;		//we also don't compare folders
	CString path1;
	CString path2 = SVN::GetPristinePath(data->path);

	if ((!CRegDWORD(_T("Software\\TortoiseSVN\\DontConvertBase"), TRUE))&&(SVN::GetTranslatedFile(path1, data->path)))
	{
		m_templist.Add(path1);
	}
	else
	{
		path1 = data->path;
	}

	CString name = CUtils::GetFileNameFromPath(data->path);
	CString ext = CUtils::GetFileExtFromPath(data->path);
	CString n1, n2;
	n1.Format(IDS_DIFF_WCNAME, name);
	n2.Format(IDS_DIFF_BASENAME, name);
	CUtils::StartDiffViewer(path2, path1, FALSE, n2, n1, ext);
}

void CLogPromptDlg::OnOK()
{
	if (m_bBlock)
		return;
	m_bBlock = TRUE;
	CDWordArray arDeleted;
	//first add all the unversioned files the user selected
	//and check if all versioned files are selected
	int nUnchecked = 0;
	for (int j=0; j<m_ListCtrl.GetItemCount(); j++)
	{
		Data * data = m_arData.GetAt(j);
		if (m_ListCtrl.GetCheck(j))
		{
			if (data->status == svn_wc_status_unversioned)
			{
				SVN svn;
				svn.Add(data->path, FALSE);
			} // if (m_arFileStatus.GetAt(j) == svn_wc_status_unversioned)
			if (data->status == svn_wc_status_missing)
			{
				SVN svn;
				svn.Remove(data->path, TRUE);
			} // if (data->status == svn_wc_status_missing)
			if (data->status == svn_wc_status_deleted)
			{
				arDeleted.Add(j);
			}
		} // if (m_ListCtrl.GetCheck(j)) 
		else
		{
			if ((data->status != svn_wc_status_unversioned)	&&
				(data->status != svn_wc_status_ignored))
				nUnchecked++;
		}
	} // for (int j=0; j<m_ListCtrl.GetItemCount(); j++)

	if ((nUnchecked == 0)&&(m_nTargetCount == 1))
	{
		m_bRecursive = TRUE;
	}
	else
	{
		m_bRecursive = FALSE;

		//the next step: find all deleted files and check if they're 
		//inside a deleted folder. If that's the case, then remove those
		//files from the list since they'll get deleted by the parent
		//folder automatically.
		for (int i=0; i<arDeleted.GetCount(); i++)
		{
			if (m_ListCtrl.GetCheck(arDeleted.GetAt(i)))
			{
				CString path = ((Data *)(m_arData.GetAt(arDeleted.GetAt(i))))->path;
				path += _T("/");
				if (PathIsDirectory(path))
				{
					//now find all children of this directory
					for (int j=0; j<arDeleted.GetCount(); j++)
					{
						if (i!=j)
						{
							if (m_ListCtrl.GetCheck(arDeleted.GetAt(j)))
							{
								if (path.CompareNoCase(((Data *)(m_arData.GetAt(arDeleted.GetAt(j))))->path.Left(path.GetLength()))==0)
								{
									m_ListCtrl.SetCheck(arDeleted.GetAt(j), FALSE);
								}
							}
						} // if (i!=j) 
					} // for (int j=0; j<arDeleted.GetCount(); j++) 
				} // if (PathIsDirectory(path)) 
			} // if (m_ListCtrl.GetCheck(i)) 
		} // for (int i=0; i<arDeleted.GetCount(); i++) 

		//save only the files the user has selected into the temporary file
		try
		{
			CStdioFile file(m_sPath, CFile::typeBinary | CFile::modeReadWrite | CFile::modeCreate);
			for (int i=0; i<m_ListCtrl.GetItemCount(); i++)
			{
				if (m_ListCtrl.GetCheck(i))
				{
					Data * data = m_arData.GetAt(i);
					file.WriteString(data->path + _T("\n"));
				}
			} // for (int i=0; i<m_ListCtrl.GetItemCount(); i++) 
			file.Close();
		}
		catch (CFileException* pE)
		{
			TRACE(_T("CFileException in Commit!\n"));
			pE->Delete();
		}
	}
	UpdateData();
	m_regAddBeforeCommit = m_bShowUnversioned;
	m_bBlock = FALSE;
	CResizableDialog::OnOK();
}

DWORD WINAPI StatusThread(LPVOID pVoid)
{
	//get the status of all selected file/folders recursively
	//and show the ones which have to be committed to the user
	//in a listcontrol. 
	CLogPromptDlg*	pDlg;
	pDlg = (CLogPromptDlg*)pVoid;
	pDlg->m_bBlock = TRUE;
	pDlg->GetDlgItem(IDCANCEL)->EnableWindow(false);

	pDlg->m_ListCtrl.SetRedraw(false);

	pDlg->m_nTotal = 0;
	pDlg->m_nSelected = 0;
	// Since svn_client_status() returns all files, even those in
	// folders included with svn:externals we need to check if all
	// files we get here belong to the same repository.
	// It is possible to commit changes in an external folder, as long
	// as the folder belongs to the same repository (but another path),
	// but it is not possible to commit all files if the externals are
	// from a different repository.
	//
	// To check if all files belong to the same repository, we compare the
	// UUID's - if they're identical then the files belong to the same
	// repository and can be committed. But if they're different, then
	// tell the user to commit all changes in the external folders
	// first and exit.
	CStringA sUUID;
	BOOL bHasExternalsFromDifferentRepos = FALSE;
	CStringArray arExtPaths;
	SVNConfig config;
	pDlg->m_nTargetCount = 0;
	try
	{
		CStdioFile file(pDlg->m_sPath, CFile::typeBinary | CFile::modeRead);
		CString strLine = _T("");
		TCHAR buf[MAX_PATH];
		const TCHAR * strbuf = NULL;
		// for every selected file/folder
		while (file.ReadString(strLine))
		{
			pDlg->m_nTargetCount++;
			strLine.Replace('\\', '/');
			// remove trailing / characters since they mess up the filename list
			// However "/" and "c:/" will be left alone.
			if (strLine.GetLength() > 1 && strLine.Right(1) == _T("/")) 
			{
				strLine.Delete(strLine.GetLength()-1,1);
			}
			BOOL bIsFolder = PathIsDirectory(strLine);
			SVNStatus status;
			svn_wc_status_t *s;
			s = status.GetFirstFileStatus(strLine, &strbuf);

			// This one fixes a problem with externals: 
			// If a strLine is a file, svn:externals at its parent directory
			// will also be returned by GetXXXFileStatus. Hence, we skip all
			// status info until we found the one matching strLine.

			if (!bIsFolder)
			{
				while (s != 0)
				{
					CString temp = strbuf;
					temp.Replace('\\', '/');
					if (temp == strLine)
						break;

					s = status.GetNextFileStatus(&strbuf);
				} // while (s != 0)
			} // if (!bIsFolder)

			if (s!=0)
			{
				CString temp;
				svn_wc_status_kind stat;
				stat = SVNStatus::GetMoreImportant(s->text_status, s->prop_status);
				if ((s->entry)&&(s->entry->uuid))
				{
					if (sUUID.IsEmpty())
						sUUID = s->entry->uuid;
					else
					{
						if (sUUID.Compare(s->entry->uuid)!=0)
						{
							bHasExternalsFromDifferentRepos = TRUE;
							if (s->entry->kind == svn_node_dir)
								arExtPaths.Add(strLine);
						} // if (sUUID.Compare(s->entry->uuid)!=0) 
					}
				} // if ((s->entry)&&(s->entry->uuid)) 
				if ((s->entry)&&(s->entry->url))
				{
					CUtils::Unescape((char *)s->entry->url);
					CString url = CUnicodeUtils::GetUnicode(s->entry->url);
					pDlg->GetDlgItem(IDC_COMMIT_TO)->SetWindowText(url);
				} // if ((s->entry)&&(s->entry->url))
				temp = strbuf;
				if ((stat == svn_wc_status_unversioned) && (PathIsDirectory(temp)))
				{
					//check if the unversioned folder is maybe versioned. This
					//could happen with nested layouts
					if (SVNStatus::GetAllStatus(temp) != svn_wc_status_unversioned)
						stat = svn_wc_status_normal;	//ignore nested layouts
				} // if ((stat == svn_wc_status_unversioned) && (PathIsDirecory(temp)))
				if (SVNStatus::IsImportant(stat))
				{
					CLogPromptDlg::Data * data = new CLogPromptDlg::Data();
					data->checked = FALSE;
					data->path = strLine;
					data->line = strLine;
					data->status = stat;
					data->textstatus = s->text_status;
					data->propstatus = s->prop_status;
					pDlg->m_arData.Add(data);
					int count = pDlg->m_ListCtrl.GetItemCount();
					pDlg->m_ListCtrl.InsertItem(count, strLine.Right(strLine.GetLength() - strLine.ReverseFind('/') - 1));
					pDlg->m_nTotal++;
					SVNStatus::GetStatusString(AfxGetResourceHandle(), stat, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
					if ((stat == s->prop_status)&&(!SVNStatus::IsImportant(s->text_status)))
						_tcscat(buf, _T("(P only)"));
					pDlg->m_ListCtrl.SetItemText(count, 1, buf);
					pDlg->m_ListCtrl.SetCheck(count);
					pDlg->m_nSelected++;
					data->checked = TRUE;
				} // if (SVNStatus::IsImportant(stat)) 
				if ((stat == svn_wc_status_unversioned)&&(CRegDWORD(_T("Software\\TortoiseSVN\\AddBeforeCommit"), TRUE)))
				{
					CLogPromptDlg::Data * data = new CLogPromptDlg::Data();
					data->checked = FALSE;
					data->path = strLine;
					data->line = strLine;
					data->status = stat;
					data->textstatus = s->text_status;
					data->propstatus = s->prop_status;
					pDlg->m_arData.Add(data);
					int count = pDlg->m_ListCtrl.GetItemCount();
					pDlg->m_ListCtrl.InsertItem(count, strLine.Right(strLine.GetLength() - strLine.ReverseFind('/') - 1));
					pDlg->m_nTotal++;
					SVNStatus::GetStatusString(AfxGetResourceHandle(), stat, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
					pDlg->m_ListCtrl.SetItemText(count, 1, buf);
				} // if ((stat == svn_wc_status_unversioned)&&(CRegDWORD(_T("Software\\TortoiseSVN\\AddBeforeCommit"), TRUE)))   
				CLogPromptDlg::Data * alldata = new CLogPromptDlg::Data();
				alldata->checked = FALSE;
				alldata->path = strLine;
				alldata->line = strLine;
				alldata->status = stat;
				alldata->textstatus = s->text_status;
				alldata->propstatus = s->prop_status;
				pDlg->m_arAllData.Add(alldata);
				while (bIsFolder && ((s = status.GetNextFileStatus(&strbuf)) != NULL))
				{
					temp = strbuf;
					stat = SVNStatus::GetMoreImportant(s->text_status, s->prop_status);
					CLogPromptDlg::Data * alldata = new CLogPromptDlg::Data();
					alldata->checked = FALSE;
					alldata->path = temp;
					alldata->line = strLine;
					alldata->status = stat;
					alldata->textstatus = s->text_status;
					alldata->propstatus = s->prop_status;
					pDlg->m_arAllData.Add(alldata);
					if ((stat == svn_wc_status_unversioned) && (PathIsDirectory(temp)))
					{
						//check if the unversioned folder is maybe versioned. This
						//could happen with nested layouts
						if (SVNStatus::GetAllStatus(temp) != svn_wc_status_unversioned)
							stat = svn_wc_status_normal;	//ignore nested layouts
					} // if ((stat == svn_wc_status_unversioned) && (PathIsDirecory(temp)))
					if (SVNStatus::IsImportant(stat))
					{
						if (s->entry)
						{
							if (s->entry->uuid)
							{
								if (sUUID.IsEmpty())
									sUUID = s->entry->uuid;
								else
								{
									if (sUUID.Compare(s->entry->uuid)!=0)
									{
										bHasExternalsFromDifferentRepos = TRUE;
										if (s->entry->kind == svn_node_dir)
											arExtPaths.Add(temp);
										continue;
									} // if (sUUID.Compare(s->entry->uuid)!=0) 
								}
							} // if (s->entry->uuid)
							else
							{
								// added files don't have an UUID assigned yet, so check if they're
								// below an external folder
								BOOL bMatch = FALSE;
								for (int ix=0; ix<arExtPaths.GetCount(); ix++)
								{
									CString t = arExtPaths.GetAt(ix);
									if (t.CompareNoCase(temp.Left(t.GetLength()))==0)
									{
										bMatch = TRUE;
										break;
									} // if (t.CompareNoCase(temp.Left(t.GetLength()))==0) 
								} // for (int ix=0; ix<arExtPaths.GetCount(); ix++) 
								if (bMatch)
									continue;
							}
						} // if ((s->entry)&&(s->entry->uuid)) 
						CLogPromptDlg::Data * data = new CLogPromptDlg::Data();
						data->checked = FALSE;
						data->path = temp;
						data->line = strLine;
						data->status = stat;
						data->textstatus = s->text_status;
						data->propstatus = s->prop_status;
						pDlg->m_arData.Add(data);
						int count = pDlg->m_ListCtrl.GetItemCount();
						if (bIsFolder)
							pDlg->m_ListCtrl.InsertItem(count, temp.Right(temp.GetLength() - strLine.GetLength() - 1));
						else
							pDlg->m_ListCtrl.InsertItem(count, temp.Right(temp.GetLength() - temp.ReverseFind('/') - 1));
						pDlg->m_nTotal++;
						SVNStatus::GetStatusString(AfxGetResourceHandle(), stat, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
						if ((stat == s->prop_status)&&(!SVNStatus::IsImportant(s->text_status)))
							_tcscat(buf, _T("(P only)"));
						pDlg->m_ListCtrl.SetItemText(count, 1, buf);
						pDlg->m_ListCtrl.SetCheck(count);
						pDlg->m_nSelected++;
						data->checked = TRUE;
					} // if (SVNStatus::IsImportant(stat)) 
					if ((stat == svn_wc_status_unversioned)&&(CRegDWORD(_T("Software\\TortoiseSVN\\AddBeforeCommit"), TRUE))&&(!config.MatchIgnorePattern(temp)))
					{
						CLogPromptDlg::Data * data = new CLogPromptDlg::Data();
						data->checked = FALSE;
						data->path = temp;
						data->line = strLine;
						data->status = stat;
						data->textstatus = s->text_status;
						data->propstatus = s->prop_status;
						pDlg->m_arData.Add(data);
						int count = pDlg->m_ListCtrl.GetItemCount();
						if (PathIsDirectory(temp))
						{
							pDlg->m_ListCtrl.InsertItem(count, temp.Right(temp.GetLength() - strLine.GetLength() - 1));
							pDlg->m_nTotal++;
							//we have an unversioned folder -> get all files in it recursively!
							int count = pDlg->m_ListCtrl.GetItemCount();
							CDirFileEnum filefinder(temp);
							CString filename;
							while (filefinder.NextFile(filename))
							{
								if (!config.MatchIgnorePattern(filename))
								{
									filename.Replace('\\', '/');
									CLogPromptDlg::Data * data = new CLogPromptDlg::Data();
									data->checked = FALSE;
									data->path = filename;
									data->line = strLine;
									data->status = stat;
									data->textstatus = s->text_status;
									data->propstatus = s->prop_status;
									pDlg->m_arData.Add(data);
									pDlg->m_ListCtrl.InsertItem(count, filename.Right(filename.GetLength() - strLine.GetLength() - 1));
									SVNStatus::GetStatusString(AfxGetResourceHandle(), stat, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
									pDlg->m_ListCtrl.SetItemText(count++, 1, buf);
									pDlg->m_nTotal++;
								} // if (!config.MatchIgnorePattern(strLine)) 
							} // while (filefinder.NextFile(filename))
						} // if (bIsFolder) 
						else
						{
							pDlg->m_ListCtrl.InsertItem(count, temp.Right(temp.GetLength() - strLine.GetLength() - 1));
							pDlg->m_nTotal++;
						}
						SVNStatus::GetStatusString(AfxGetResourceHandle(), stat, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
						pDlg->m_ListCtrl.SetItemText(count, 1, buf);
					} // if ((stat == svn_wc_status_unversioned)&&(CRegDWORD(_T("Software\\TortoiseSVN\\AddBeforeCommit"), TRUE)))   
				} // while ((s = status.GetNextFileStatus(&strbuf)) != NULL) 
			} // if (s!=0) 
			else
			{
				CMessageBox::Show(pDlg->m_hWnd, status.GetLastErrorMsg(), _T("TortoiseSVN"), MB_ICONERROR);
			}
		} // while (file.ReadString(strLine)) 
		file.Close();
	}
	catch (CFileException* pE)
	{
		TRACE("CFileException in Commit!\n");
		pE->Delete();
	}


	int mincol = 0;
	int maxcol = ((CHeaderCtrl*)(pDlg->m_ListCtrl.GetDlgItem(0)))->GetItemCount()-1;
	int col;
	for (col = mincol; col <= maxcol; col++)
	{
		pDlg->m_ListCtrl.SetColumnWidth(col,LVSCW_AUTOSIZE_USEHEADER);
	}
	pDlg->m_ListCtrl.SetRedraw(true);

	pDlg->GetDlgItem(IDCANCEL)->EnableWindow(true);
	pDlg->m_bBlock = FALSE;
	if (pDlg->m_ListCtrl.GetItemCount()==0)
	{
		CMessageBox::Show(pDlg->m_hWnd, IDS_LOGPROMPT_NOTHINGTOCOMMIT, IDS_APPNAME, MB_ICONINFORMATION);
		pDlg->EndDialog(0);
		return -1;
	} // if (pDlg->m_ListCtrl.GetItemCount()==0) 
	if (bHasExternalsFromDifferentRepos)
	{
		CMessageBox::Show(pDlg->m_hWnd, IDS_LOGPROMPT_EXTERNALS, IDS_APPNAME, MB_ICONINFORMATION);
	} // if (bHasExternalsFromDifferentRepos) 

	//now gather some statistics
	pDlg->m_nAdded = 0;
	pDlg->m_nDeleted = 0;
	pDlg->m_nModified = 0;
	pDlg->m_nUnversioned = 0;
	pDlg->m_nConflicted = 0;
	for (int i=0; i<pDlg->m_arData.GetCount(); i++)
	{
		CLogPromptDlg::Data * data = pDlg->m_arData.GetAt(i);
		if (data)
		{
			switch (data->status)
			{
			case svn_wc_status_added:
				pDlg->m_nAdded++;
				break;
			case svn_wc_status_missing:
			case svn_wc_status_deleted:
				pDlg->m_nDeleted++;
				break;
			case svn_wc_status_replaced:
			case svn_wc_status_modified:
			case svn_wc_status_merged:
				pDlg->m_nModified++;
				break;
			case svn_wc_status_conflicted:
			case svn_wc_status_obstructed:
				pDlg->m_nConflicted++;
			default:
				{
					pDlg->m_nUnversioned++;
					// check if the unversioned item is just
					// a file differing in case but still versioned
					for (int j=0; j<pDlg->m_arAllData.GetCount(); j++)
					{
						CLogPromptDlg::Data * d = pDlg->m_arAllData.GetAt(j);
						if ((d->status != svn_wc_status_unversioned)&&(data->path.CompareNoCase(d->path)==0))
						{
							// adjust the case of the filename
							MoveFileEx(data->path, d->path, MOVEFILE_REPLACE_EXISTING);
							pDlg->m_ListCtrl.DeleteItem(i);
							pDlg->m_arData.RemoveAt(i);
							delete data;
							i--;
							pDlg->m_nUnversioned--;
							break;
						}
					}
				}
				break;
			} // switch (data->status) 
		} // if (data) 
	} // for (int i=0; i<pDlg->m_arData.GetCount(); i++) 
	CString sAdded, sDeleted, sModified, sConflicted, sUnversioned;
	TCHAR buf[MAX_PATH];
	SVNStatus::GetStatusString(AfxGetResourceHandle(), svn_wc_status_added, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
	sAdded = buf;
	SVNStatus::GetStatusString(AfxGetResourceHandle(), svn_wc_status_deleted, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
	sDeleted = buf;
	SVNStatus::GetStatusString(AfxGetResourceHandle(), svn_wc_status_modified, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
	sModified = buf;
	SVNStatus::GetStatusString(AfxGetResourceHandle(), svn_wc_status_conflicted, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
	sConflicted = buf;
	SVNStatus::GetStatusString(AfxGetResourceHandle(), svn_wc_status_unversioned, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
	sUnversioned = buf;
	CString sToolTip;
	sToolTip.Format(_T("%s = %d\n%s = %d\n%s = %d\n%s = %d\n%s = %d"),
		sUnversioned, pDlg->m_nUnversioned,
		sModified, pDlg->m_nModified,
		sAdded, pDlg->m_nAdded,
		sDeleted, pDlg->m_nDeleted,
		sConflicted, pDlg->m_nConflicted
		);
	pDlg->m_tooltips.AddTool(pDlg->GetDlgItem(IDC_STATISTICS), sToolTip);
	CString sStats;
	sStats.Format(IDS_LOGPROMPT_STATISTICSFORMAT, pDlg->m_nSelected, pDlg->m_nTotal);
	pDlg->GetDlgItem(IDC_STATISTICS)->SetWindowText(sStats);
	POINT pt;
	GetCursorPos(&pt);
	SetCursorPos(pt.x, pt.y);
	return 0;
}

void CLogPromptDlg::OnCancel()
{
	if (m_bBlock)
		return;
	DeleteFile(m_sPath);
	UpdateData(TRUE);
	CResizableDialog::OnCancel();
}

BOOL CLogPromptDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!m_bBlock)
	{
		HCURSOR hCur = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
		SetCursor(hCur);
		return CResizableDialog::OnSetCursor(pWnd, nHitTest, message);
	}
	HCURSOR hCur = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
	SetCursor(hCur);
	return TRUE;
}

void CLogPromptDlg::OnBnClickedSelectall()
{
	UINT state = (m_SelectAll.GetState() & 0x0003);
	if (state == 2)
		return;
	theApp.DoWaitCursor(1);
	m_ListCtrl.SetRedraw(false);
	int itemCount = m_ListCtrl.GetItemCount();
	for (int i=0; i<itemCount; i++)
	{
		m_ListCtrl.SetCheck(i, state == 1);
		Data * data = m_arData.GetAt(i);
		data->checked = (state == 1);
	} // for (int i=0; i<itemCount; i++)
	if (state == 1)
		m_nSelected = m_nTotal;
	else
		m_nSelected = 0;
	m_ListCtrl.SetRedraw(true);

	CString sStats;
	sStats.Format(IDS_LOGPROMPT_STATISTICSFORMAT, m_nSelected, m_nTotal);
	GetDlgItem(IDC_STATISTICS)->SetWindowText(sStats);

	theApp.DoWaitCursor(-1);
}

BOOL CLogPromptDlg::PreTranslateMessage(MSG* pMsg)
{
	if (!m_bBlock)
		m_tooltips.RelayEvent(pMsg);
	if ((pMsg->message == WM_KEYDOWN)&&(pMsg->wParam == VK_F5))//(nChar == VK_F5)
	{
		if (m_bBlock)
			return CResizableDialog::PreTranslateMessage(pMsg);
		Refresh();
	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CLogPromptDlg::Refresh()
{
	m_bBlock = TRUE;
	CString temp;
	for (int i=0; i<m_arData.GetCount(); i++)
	{
		Data * data = m_arData.GetAt(i);
		delete data;
	} // for (int i=0; i<m_arData.GetCount(); i++) 
	m_arData.RemoveAll();
	m_templist.RemoveAll();

	m_ListCtrl.DeleteAllItems();
	int c = ((CHeaderCtrl*)(m_ListCtrl.GetDlgItem(0)))->GetItemCount()-1;
	while (c>=0)
		m_ListCtrl.DeleteColumn(c--);
	temp.LoadString(IDS_LOGPROMPT_FILE);
	m_ListCtrl.InsertColumn(0, temp);
	temp.LoadString(IDS_LOGPROMPT_STATUS);
	m_ListCtrl.InsertColumn(1, temp);

	m_ListCtrl.SetRedraw(false);
	int mincol = 0;
	int maxcol = ((CHeaderCtrl*)(m_ListCtrl.GetDlgItem(0)))->GetItemCount()-1;
	int col;
	for (col = mincol; col <= maxcol; col++)
	{
		m_ListCtrl.SetColumnWidth(col,LVSCW_AUTOSIZE_USEHEADER);
	}

	//first start a thread to obtain the file list with the status without
	//blocking the dialog
	DWORD dwThreadId;
	if ((m_hThread = CreateThread(NULL, 0, &StatusThread, this, 0, &dwThreadId))==0)
	{
		CMessageBox::Show(this->m_hWnd, IDS_ERR_THREADSTARTFAILED, IDS_APPNAME, MB_OK | MB_ICONERROR);
	}
	//m_ListCtrl.UpdateData(FALSE);
}

void CLogPromptDlg::OnHdnItemclickFilelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	if (m_bBlock)
		return;
	if (m_nSortedColumn == phdr->iItem)
		m_bAscending = !m_bAscending;
	else
		m_bAscending = TRUE;
	m_nSortedColumn = phdr->iItem;
	Sort();

	CString temp;
	m_ListCtrl.SetRedraw(FALSE);
	m_ListCtrl.DeleteAllItems();

	//fill in the data for the listcontrol again, but this
	//time sorted.
	TCHAR buf[MAX_PATH];
	m_bBlock = TRUE;
	for (int i=0; i<m_arData.GetCount(); i++)
	{
		Data * data = m_arData.GetAt(i);
		SVNStatus::GetStatusString(AfxGetResourceHandle(), data->status, buf, sizeof(buf)/sizeof(TCHAR), (WORD)CRegStdWORD(_T("Software\\TortoiseSVN\\LanguageID"), GetUserDefaultLangID()));
		if ((data->status == data->propstatus)&&(!SVNStatus::IsImportant(data->textstatus)))
			_tcscat(buf, _T("(P only)"));
		if (data->path.GetLength() == data->line.GetLength())
			m_ListCtrl.InsertItem(i, data->line.Right(data->line.GetLength() - data->line.ReverseFind('/') - 1));
		else
			m_ListCtrl.InsertItem(i, data->path.Right(data->path.GetLength() - data->line.GetLength() - 1));
		m_ListCtrl.SetItemText(i, 1, buf);
		m_ListCtrl.SetCheck(i, data->checked);
	} // for (int i=0; i<m_arData.GetCount(); i++) 
	m_bBlock = FALSE;
	m_ListCtrl.SetRedraw(TRUE);

	*pResult = 0;
}

void CLogPromptDlg::Sort()
{
	qsort(m_arData.GetData(), m_arData.GetSize(), sizeof(Data *), (GENERICCOMPAREFN)SortCompare);
}

int CLogPromptDlg::SortCompare(const void * pElem1, const void * pElem2)
{
	Data * pData1 = *((Data**)pElem1);
	Data * pData2 = *((Data**)pElem2);
	int result = 0;
	switch (m_nSortedColumn)
	{
	case 0:		//path column
		{
			result = pData1->path.Compare(pData2->path);
		}
		break;
	case 1:		//status column
		{
			result = pData1->status - pData2->status;
			if (result == 0)
			{
				result = pData1->path.Compare(pData2->path);
			}
		}
		break;
	default:
		break;
	} // switch (m_nSortedColumn)
	if (!m_bAscending)
		result = -result;
	return result;
}

void CLogPromptDlg::OnBnClickedHelp()
{
	OnHelp();
}

void CLogPromptDlg::OnBnClickedShowunversioned()
{
	UpdateData();
	m_regAddBeforeCommit = m_bShowUnversioned;
	if (!m_bBlock)
		Refresh();
}

void CLogPromptDlg::OnEnChangeLogmessage()
{
	CString sTemp;
	GetDlgItem(IDC_LOGMESSAGE)->GetWindowText(sTemp);
	if (DWORD(sTemp.GetLength()) > CRegDWORD(_T("Software\\TortoiseSVN\\MinLogSize"), 10))
	{
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
}








