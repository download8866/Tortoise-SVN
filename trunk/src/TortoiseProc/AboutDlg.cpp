// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2005 - Stefan Kueng

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
#include "AboutDlg.h"
#include "svn_version.h"
#include "..\version.h"
#include "Utils.h"

#ifdef UNICODE
#	define STRINGWIDTH  "UNICODE"
#else
#	define STRINGWIDTH	"MBCS"
#endif

// CAboutDlg dialog

IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)
CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutDlg::IDD, pParent)
{
}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WEBLINK, m_cWebLink);
	DDX_Control(pDX, IDC_SUPPORTLINK, m_cSupportLink);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_UPDATE, OnBnClickedUpdate)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//set the version string
	CString temp, boxtitle;
	boxtitle.Format(IDS_ABOUTVERSIONBOX, TSVN_VERMAJOR, TSVN_VERMINOR, TSVN_VERMICRO, TSVN_VERBUILD);
	GetDlgItem(IDC_VERSIONBOX)->SetWindowText(boxtitle);
	const svn_version_t * svnver = svn_client_version();

	temp.Format(IDS_ABOUTVERSION, TSVN_VERMAJOR, TSVN_VERMINOR, TSVN_VERMICRO, TSVN_VERBUILD, _T(STRINGWIDTH),
		svnver->major, svnver->major, svnver->patch, CString(svnver->tag), 
		APR_MAJOR_VERSION, APR_MINOR_VERSION, APR_PATCH_VERSION,
		API_MAJOR_VERSION, API_MINOR_VERSION, API_PATCH_VERSION,
		APU_MAJOR_VERSION, APU_MINOR_VERSION, APU_PATCH_VERSION,
		DB_VERSION_MAJOR, DB_VERSION_MINOR, DB_VERSION_PATCH,
		_T(NEON_VERSION),
		_T(OPENSSL_VERSION_TEXT),
		_T(ZLIB_VERSION));
	GetDlgItem(IDC_VERSIONABOUT)->SetWindowText(temp);
	temp.Format(_T("%s, %s version"), _T("TortoiseSVN"), _T(STRINGWIDTH));
	this->SetWindowText(temp);

	CPictureHolder tmpPic;
	tmpPic.CreateFromBitmap(IDB_LOGOFLIPPED);
	m_renderSrc.Create32BitFromPicture(&tmpPic,468,64);
	m_renderDest.Create32BitFromPicture(&tmpPic,468,64);

	m_waterEffect.Create(468,64);
	SetTimer(ID_EFFECTTIMER, 40, NULL);
	SetTimer(ID_DROPTIMER, 300, NULL);

	m_cWebLink.SetURL(_T("http://www.tortoisesvn.org"));
	m_cSupportLink.SetURL(_T("http://tortoisesvn.tigris.org/donate.html"));

	CenterWindow(CWnd::FromHandle(hWndExplorer));
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == ID_EFFECTTIMER)
	{
		m_waterEffect.Render((DWORD*)m_renderSrc.GetDIBits(), (DWORD*)m_renderDest.GetDIBits());
		CClientDC dc(this);
		CPoint ptOrigin(15,20);
		m_renderDest.Draw(&dc,ptOrigin);
	}
	if (nIDEvent == ID_DROPTIMER)
	{
		CRect r;
		r.left = 15;
		r.top = 20;
		r.right = r.left + m_renderSrc.GetWidth();
		r.bottom = r.top + m_renderSrc.GetHeight();
		m_waterEffect.Blob(random(r.left,r.right), random(r.top, r.bottom), 2, 400, m_waterEffect.m_iHpage);
	}
	CDialog::OnTimer(nIDEvent);
}

void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect r;
	r.left = 15;
	r.top = 20;
	r.right = r.left + m_renderSrc.GetWidth();
	r.bottom = r.top + m_renderSrc.GetHeight();

	if(r.PtInRect(point) == TRUE)
	{
		// dibs are drawn upside down...
		point.y -= 20;
		point.y = 64-point.y;

		if (nFlags & MK_LBUTTON)
			m_waterEffect.Blob(point.x -15,point.y,5,1600,m_waterEffect.m_iHpage);
		else
			m_waterEffect.Blob(point.x -15,point.y,2,50,m_waterEffect.m_iHpage);

	}


	CDialog::OnMouseMove(nFlags, point);
}

void CAboutDlg::OnBnClickedUpdate()
{
	TCHAR com[MAX_PATH+100];
	GetModuleFileName(NULL, com, MAX_PATH);
	_tcscat(com, _T(" /command:updatecheck /visible"));

	CUtils::LaunchApplication(com, 0, false);
}
