// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2007 - Stefan Kueng

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
#pragma once
#include "RevisionGraph.h"
#include "ProgressDlg.h"
#include "Colors.h"

#define REVGRAPH_PREVIEW_WIDTH 100
#define REVGRAPH_PREVIEW_HEIGTH 200

enum NodeShape
{
	TSVNRectangle,
	TSVNRoundRect,
	TSVNOctangle,
	TSVNEllipse
};
#define STARTPOINT_PT		(CPoint(5, 5))
#define SHADOW_OFFSET_PT	(CPoint(4, 4))
#define ROUND_RECT			(12)

#define RGB_DEF_SEL				RGB(160, 160, 160)
#define RGB_DEF_SHADOW			RGB(128, 128, 128)
#define RGB_DEF_HEADER			RGB(255, 0, 0)
#define RGB_DEF_TAG				RGB(0, 0, 0)
#define RGB_DEF_BRANCH			RGB(0, 0, 255)
#define RGB_DEF_NODE			RGB(0, 0, 255)

#define NODE_RECT_WIDTH			200
#define NODE_SPACE_LEFT			12
#define NODE_SPACE_RIGHT		100
#define NODE_SPACE_LINE			20
#define NODE_RECT_HEIGTH		60
#define NODE_SPACE_TOP			20
#define NODE_SPACE_BOTTOM		20

#define MAXFONTS				4
#define	MAX_TT_LENGTH			10000



/**
 * \ingroup TortoiseProc
 * Window class showing a revision graph.
 *
 * The analyzation of the log data is done in the child class CRevisionGraph.
 * Here, we handle the window notifications.
 */
class CRevisionGraphWnd : public CWnd, public CRevisionGraph
{
public:
	CRevisionGraphWnd();   // standard constructor
	virtual ~CRevisionGraphWnd();
	enum { IDD = IDD_REVISIONGRAPH };


	CString			m_sPath;
	volatile LONG	m_bThreadRunning;
	CProgressDlg* 	m_pProgress;

	void			InitView();
	void			Init(CWnd * pParent, LPRECT rect);
	void			SaveGraphAs(CString sSavePath);
protected:
	BOOL			m_bNoGraph;
	DWORD			m_dwTicks;
	CRect			m_ViewRect;
	CRect			m_GraphRect;
	CRect			m_OverviewPosRect;
	CRect			m_OverviewRect;
	CPtrArray		m_arConnections;
	CDWordArray		m_arVertPositions;
	BOOL			m_bShowOverview;
	
	std::multimap<source_entry*, CRevisionEntry*>		m_targetsbottom;
	std::multimap<source_entry*, CRevisionEntry*>		m_targetsright;

	
	CRevisionEntry * m_SelectedEntry1;
	CRevisionEntry * m_SelectedEntry2;
	LOGFONT			m_lfBaseFont;
	CFont *			m_apFonts[MAXFONTS];
	int				m_nFontSize;
	CToolTipCtrl *	m_pDlgTip;
	char			m_szTip[MAX_TT_LENGTH+1];
	wchar_t			m_wszTip[MAX_TT_LENGTH+1];

	int				m_node_rect_width;
	int				m_node_space_left;
	int				m_node_space_right;
	int				m_node_space_line;
	int				m_node_rect_heigth;
	int				m_node_space_top;
	int				m_node_space_bottom;
	int				m_nIconSize;
	CPoint			m_RoundRectPt;
	float			m_fZoomFactor;
	CColors			m_Colors;
	bool			m_bFetchLogs;
	bool			m_bShowAll;
	bool			m_bArrangeByPath;
	bool			m_bIsRubberBand;
	CPoint			m_ptRubberStart;
	CPoint			m_ptRubberEnd;

	CBitmap			m_Preview;
	
	virtual BOOL	ProgressCallback(CString text, CString text2, DWORD done, DWORD total);
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void	OnPaint();
	afx_msg BOOL	OnEraseBkgnd(CDC* pDC);
	afx_msg void	OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void	OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg INT_PTR	OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL	OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL	OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void	OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL	OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	DECLARE_MESSAGE_MAP()
private:
	void			CompareRevs(bool bHead);
	void			UnifiedDiffRevs(bool bHead);
	CTSVNPath		DoUnifiedDiff(bool bHead, CString& sRoot, bool& bIsFolder);
	CRevisionEntry* GetRevisionEntry(LONG rev) const;
	CRevisionEntry* GetRevisionEntry(source_entry * sentry) const;
	INT_PTR			GetIndexOfRevision(source_entry * sentry);
	void			SetScrollbars(int nVert = 0, int nHorz = 0, int oldwidth = 0, int oldheight = 0);
	CRect *			GetViewSize();
	CRect *			GetGraphSize();
	CFont*			GetFont(BOOL bItalic = FALSE, BOOL bBold = FALSE);

	void			DrawOctangle(CDC * pDC, const CRect& rect);
	void			DrawNode(CDC * pDC, const CRect& rect,
							COLORREF contour, CRevisionEntry *rentry,
							NodeShape shape, BOOL isSel, HICON hIcon, int penStyle = PS_SOLID);
	void			DrawGraph(CDC* pDC, const CRect& rect, int nVScrollPos, int nHScrollPos, bool bDirectDraw);

	void			BuildConnections();
	void			ClearEntryConnections();
	void			CountEntryConnections();
	void			MarkSpaceLines(source_entry * entry, int level, svn_revnum_t startrev, svn_revnum_t endrev);
	void			DecrementSpaceLines(source_entry * reventry);
	void			DrawConnections(CDC* pDC, const CRect& rect, int nVScrollPos, int nHScrollPos, INT_PTR start, INT_PTR end);
	int				GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	void			DoZoom(float nZoomFactor);
	void			DrawRubberBand();

	void			BuildPreview();
friend class CRevisionGraphDlg;
};
