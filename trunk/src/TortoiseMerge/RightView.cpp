#include "StdAfx.h"
#include "Resource.h"
#include ".\rightview.h"

IMPLEMENT_DYNCREATE(CRightView, CBaseView)

CRightView::CRightView(void)
{
	m_pwndRight = this;
	m_nStatusBarID = ID_INDICATOR_RIGHTVIEW;
}

CRightView::~CRightView(void)
{
}

BOOL CRightView::IsStateSelectable(CDiffData::DiffStates state)
{
	//The right view is not visible in one-way diff...
	if (!this->IsWindowVisible())
	{
		return FALSE;
	}

	//The right view is always "Yours" in both two and three-way diff
	switch (state)
	{
	case CDiffData::DIFFSTATE_WHITESPACE:
	case CDiffData::DIFFSTATE_ADDED:
	case CDiffData::DIFFSTATE_REMOVED:
	case CDiffData::DIFFSTATE_CONFLICTED:
	case CDiffData::DIFFSTATE_CONFLICTEMPTY:
	case CDiffData::DIFFSTATE_CONFLICTADDED:
	case CDiffData::DIFFSTATE_EMPTY:
		return TRUE;
	default:
		return FALSE;
	} // switch (state) 
	return FALSE;
}

void CRightView::OnContextMenu(CPoint point, int nLine)
{
	CMenu popup;
	if (popup.CreatePopupMenu())
	{
#define ID_USEBLOCK 1
		UINT uEnabled = MF_ENABLED;
		if ((m_nSelBlockStart == -1)||(m_nSelBlockEnd == -1))
			uEnabled |= MF_DISABLED | MF_GRAYED;
		CString temp;
		if (m_pwndBottom->IsWindowVisible())
		{
			temp.LoadString(IDS_VIEWCONTEXTMENU_USETHISBLOCK);
		}
		else
			temp.LoadString(IDS_VIEWCONTEXTMENU_USETHEIRBLOCK);
		popup.AppendMenu(MF_STRING | uEnabled, ID_USEBLOCK, temp);

		int cmd = popup.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_NONOTIFY, point.x, point.y, this, 0);
		switch (cmd)
		{
		case ID_USEBLOCK:
			{
				if (m_pwndBottom->IsWindowVisible())
				{
					for (int i=m_nSelBlockStart; i<=m_nSelBlockEnd; i++)
					{
						m_pwndBottom->m_arDiffLines->SetAt(i, m_arDiffLines->GetAt(i));
						m_pwndBottom->m_arLineStates->SetAt(i, m_arLineStates->GetAt(i));
					} // for (int i=m_nSelBlockStart; i<=m_nSelBlockEnd; i++) 
				} // if (m_pwndBottom->IsWindowVisible()) 
				else
				{
					for (int i=m_nSelBlockStart; i<=m_nSelBlockEnd; i++)
					{
						m_arDiffLines->SetAt(i, m_pwndLeft->m_arDiffLines->GetAt(i));
						m_arLineStates->SetAt(i, m_pwndLeft->m_arLineStates->GetAt(i));
					} // for (int i=m_nSelBlockStart; i<=m_nSelBlockEnd; i++) 
				}
			} 
		break;
		} // switch (cmd) 
	} // if (popup.CreatePopupMenu()) 
}
