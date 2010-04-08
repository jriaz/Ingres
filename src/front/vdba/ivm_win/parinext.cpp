/*
**  Copyright (C) 2005-2006 Ingres Corporation. All Rights Reserved.
** 
**    Source   : parinext.cpp , Implementation File
**    Project  : Ingres II / Visual Manager
**    Author   : Sotheavut UK (uk$so01)
**    Purpose  : Extra Parameter Page for Installation branch
**
**  History:
**	25-May-1999 (uk$so01)
**	    created
**	29-Feb-2000 (uk$so01)
**	    Bug #100621
**	    Cancel the edit control, if it is being editing and the
**	    OnUpdatedate is called.
**	01-Mar-2000 (uk$so01)
**	    SIR #100635, Add the Functionality of Find operation.
**	03-Mars-2000 (uk$so01)
**	    BUG #100707
**	    ingsetenv NAME VALUE, If value contains space then quote the
**	    VALUE.
**	31-Mar-2000 (uk$so01)
**	    BUG 101126
**	    Put the find dialog at top. When open the modal dialog, 
**	    if the find dialog is opened, then close it.
**	31-May-2000 (uk$so01)
**	    bug 99242 Handle DBCS
**	27-Sep-2001 (noifr01)
**	    (sir 105798) increased the width of the "description" column
**	    (the new description being very long)
**	01-nov-2001 (somsa01)
**	    Cleaned up 64-bit compiler warnings.
**  17-Oct-2002 (noifr01)
**      restored } character that had been lost in previous propagation
** 27-Feb-2003 (uk$so01)
**    SIR #109718, Avoiding the duplicated source files by integrating the libraries.
** 04-Apr-2003 (uk$so01)
**    SIR #109718, Avoiding the duplicated source files by integrating the libraries.
*/

#include "stdafx.h"
#include "ivm.h"
#include "parinext.h"
#include "ivmdml.h"
#include "ivmsgdml.h"
#include "dlgenvar.h"
#include "findinfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
static int CALLBACK CompareSubItem (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

/////////////////////////////////////////////////////////////////////////////
// CuDlgParameterInstallationExtra dialog


CuDlgParameterInstallationExtra::CuDlgParameterInstallationExtra(CWnd* pParent /*=NULL*/)
	: CDialog(CuDlgParameterInstallationExtra::IDD, pParent)
{
	//{{AFX_DATA_INIT(CuDlgParameterInstallationExtra)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_sortparam.m_bAsc = TRUE;
}


void CuDlgParameterInstallationExtra::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CuDlgParameterInstallationExtra)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CuDlgParameterInstallationExtra, CDialog)
	//{{AFX_MSG_MAP(CuDlgParameterInstallationExtra)
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, OnColumnclickList1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	//}}AFX_MSG_MAP
	ON_MESSAGE (WMUSRMSG_ENABLE_CONTROL, OnEnableControl)

	ON_MESSAGE (WMUSRMSG_LISTCTRL_EDITING, OnListCtrlEditing)
	ON_MESSAGE (WM_LAYOUTEDITDLG_OK, OnListCtrlEditOK)
	ON_MESSAGE (WMUSRMSG_VARIABLE_UNSET, OnUnsetVariable)
	ON_MESSAGE (WMUSRMSG_IVM_PAGE_UPDATING, OnUpdateData)
	ON_MESSAGE (WMUSRMSG_ADD_OBJECT, OnAddParameter)
	ON_MESSAGE (WMUSRMSG_FIND_ACCEPTED, OnFindAccepted)
	ON_MESSAGE (WMUSRMSG_FIND, OnFind)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CuDlgParameterInstallationExtra message handlers


void CuDlgParameterInstallationExtra::PostNcDestroy() 
{
	while (!m_listParameter.IsEmpty())
		delete m_listParameter.RemoveHead();
	delete this;
	CDialog::PostNcDestroy();
}

BOOL CuDlgParameterInstallationExtra::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	const int GLAYOUT_NUMBER = 3;
	VERIFY (m_cListCtrl.SubclassDlgItem (IDC_LIST1, this));
	LONG style = GetWindowLong (m_cListCtrl.m_hWnd, GWL_STYLE);
	style |= WS_CLIPCHILDREN;
	SetWindowLong (m_cListCtrl.m_hWnd, GWL_STYLE, style);
	m_ImageList.Create (IDB_EVIRONMENT_VARIABLE, 16, 0, RGB(255,0,255));
	m_cListCtrl.SetImageList (&m_ImageList, LVSIL_SMALL);

	//
	// Initalize the Column Header of CListCtrl
	LV_COLUMN lvcolumn;
	LSCTRLHEADERPARAMS2 lsp[GLAYOUT_NUMBER] =
	{
		{IDS_HEADER_PARAMETER,    140,  LVCFMT_LEFT, FALSE},
		{IDS_HEADER_VALUE,        160,  LVCFMT_LEFT, FALSE},
		{IDS_HEADER_DESCRIPTION, 1880,  LVCFMT_LEFT, FALSE}
	};

	CString strHeader;
	memset (&lvcolumn, 0, sizeof (LV_COLUMN));
	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	for (int i=0; i<GLAYOUT_NUMBER; i++)
	{
		strHeader.LoadString (lsp[i].m_nIDS);
		lvcolumn.fmt      = lsp[i].m_fmt;
		lvcolumn.pszText  = (LPTSTR)(LPCTSTR)strHeader;
		lvcolumn.iSubItem = i;
		lvcolumn.cx       = lsp[i].m_cxWidth;
		m_cListCtrl.InsertColumn (i, &lvcolumn, lsp[i].m_bUseCheckMark);
	}
	RefreshParameter();
	(GetParent()->GetParent())->PostMessage (WMUSRMSG_ENABLE_CONTROL, 0, 0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CuDlgParameterInstallationExtra::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if (!IsWindow (m_cListCtrl.m_hWnd))
		return;
	CRect r;
	GetClientRect (r);
	m_cListCtrl.MoveWindow (r);
}



void CuDlgParameterInstallationExtra::RefreshParameter()
{
	try
	{
		int nTopIndex = m_cListCtrl.GetTopIndex();
		m_cListCtrl.HideProperty(FALSE);
		m_cListCtrl.DeleteAllItems();
		while (!m_listParameter.IsEmpty())
			delete m_listParameter.RemoveHead();

		if (theApp.m_ingresVariable.QueryEnvironment (m_listParameter, FALSE, TRUE))
		{
			//
			// Sort the parameter:
			CaEnvironmentParameter* pParam = NULL;
			CObArray a;
			int i, indx, nCount = 0;
			if (!m_listParameter.IsEmpty())
			{
				a.SetSize(m_listParameter.GetCount());
				POSITION pos = m_listParameter.GetHeadPosition();

				while (pos != NULL)
				{
					pParam = (CaEnvironmentParameter*)m_listParameter.GetNext(pos);
					if (pParam->GetName().CompareNoCase(_T("II_NUM_OF_PROCESSORS"))==0)
						pParam->LoadDescription(IDS_II_NUM_OF_PROCESSORS);
					else
					if (pParam->GetName().CompareNoCase(_T("II_MAX_SEM_LOOPS"))==0)
						pParam->LoadDescription(IDS_II_MAX_SEM_LOOPS);

					a[nCount] = pParam;
					nCount++;
				}
				if (a.GetSize() > 1)
					IVM_DichotomySort(a, CompareSubItem, (LPARAM)&m_sortparam, NULL);
			}
			else
				return;

			for (i=0; i< a.GetSize(); i++)
			{
				pParam = (CaEnvironmentParameter*)a[i];
				nCount = m_cListCtrl.GetItemCount();
				indx = m_cListCtrl.InsertItem (nCount, pParam->GetName(), IM_ENV_SYSTEM);
				if (indx != -1)
				{
					m_cListCtrl.SetItemText (indx, 1, pParam->GetValue());
					m_cListCtrl.SetItemText (indx, 2, pParam->GetDescription());
					m_cListCtrl.SetItemData (indx, (DWORD_PTR)pParam);
				}
			}

			if (nTopIndex != -1)
			{
				CRect r;
				m_cListCtrl.GetItemRect (0, r, LVIR_BOUNDS);
				m_cListCtrl.Scroll (CSize(0, nTopIndex*r.Height()));
			}
		}
	}
	catch (...)
	{
		AfxMessageBox (_T("System error (CuDlgParameterInstallationExtra::RefreshParameter): \nRefresh parameter failed"));
	}
}

void CuDlgParameterInstallationExtra::OnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iSubItem != 0)
		return;
	CWaitCursor doWaitCursor;
	m_sortparam.m_bAsc = !m_sortparam.m_bAsc;
	m_cListCtrl.SortItems(CompareSubItem, (LPARAM)&m_sortparam);
	
	*pResult = 0;
}

LONG CuDlgParameterInstallationExtra::OnListCtrlEditOK(WPARAM w, LPARAM l)
{
	SetUnsetVariable (FALSE); // SET
	return 0;
}

LONG CuDlgParameterInstallationExtra::OnUnsetVariable(WPARAM w, LPARAM l)
{
	SetUnsetVariable (TRUE); // UNSET
	return 0;
}

static int CALLBACK CompareSubItem (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int nC;
	CaEnvironmentParameter* lpItem1 = (CaEnvironmentParameter*)lParam1;
	CaEnvironmentParameter* lpItem2 = (CaEnvironmentParameter*)lParam2;
	SORTPARAMS*   pSr = (SORTPARAMS*)lParamSort;
	BOOL bAsc = pSr? pSr->m_bAsc: TRUE;

	if (lpItem1 && lpItem2)
	{
		nC = bAsc? 1 : -1;
		return nC * lpItem1->GetName().CompareNoCase (lpItem2->GetName());
	}
	else
	{
		nC = lpItem1? 1: lpItem2? -1: 0;
		return bAsc? nC: (-1)*nC;
	}

	return 0;
}

void CuDlgParameterInstallationExtra::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	UINT nState = ST_ADD;
	if (pNMListView->iItem >= 0 && pNMListView->uNewState > 0 && (pNMListView->uNewState&LVIS_SELECTED))
	{
		int iNameCol  = 0;
		int iValueCol = 1;
		int nSelected = pNMListView->iItem;
		CString strName  = m_cListCtrl.GetItemText (nSelected, iNameCol);
		CString strValue = m_cListCtrl.GetItemText (nSelected, iValueCol);

		strValue.TrimLeft();
		strValue.TrimRight();

		CaEnvironmentParameter* pParameter = (CaEnvironmentParameter*)m_cListCtrl.GetItemData(nSelected);
		if (pParameter)
		{
			nState |= ST_MODIFY;
			if (strValue.CompareNoCase (theApp.m_strVariableNotSet) != 0)
				nState |= ST_UNSET;
		}
		(GetParent()->GetParent())->SendMessage (WMUSRMSG_ENABLE_CONTROL, (WPARAM)nState, 0);
	}
	*pResult = 0;
}


void CuDlgParameterInstallationExtra::SetUnsetVariable (BOOL bUnset)
{
	CWaitCursor doWaitCursor;
	int iNameCol  = 0;
	int iValueCol = 1;
	int nSelected = m_cListCtrl.GetNextItem (-1, LVNI_SELECTED);
	if (nSelected == -1)
		return ;
	CString strName  = m_cListCtrl.GetItemText (nSelected, iNameCol);
	CString strValue = m_cListCtrl.GetItemText (nSelected, iValueCol);

	strValue.TrimLeft();
	strValue.TrimRight();
	if (strValue.CompareNoCase (theApp.m_strVariableNotSet) == 0)
		return;
	
	BOOL bSystem = TRUE;
	if (bUnset)
	{
		// _T("Do you wish to unset the parameter %s ?"), (LPCTSTR)strName);
		CString strMsg;
		AfxFormatString1(strMsg, IDS_MSG_UNSET_PARAMETER_X, (LPCTSTR)strName);
		if (AfxMessageBox (strMsg, MB_ICONEXCLAMATION|MB_YESNO) != IDYES)
			return;
	}
	IVM_SetIngresVariable (strName, strValue, bSystem, bUnset);
	OnUpdateData(0, 0);
	if (bUnset)
	{
		LVFINDINFO lvfindinfo;
		memset (&lvfindinfo, 0, sizeof (LVFINDINFO));
		lvfindinfo.flags = LVFI_STRING;
		lvfindinfo.psz   = strName;
		nSelected = m_cListCtrl.FindItem (&lvfindinfo);

		if (nSelected != -1)
			m_cListCtrl.SetItemState (nSelected, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}

	OnEnableControl(0, 0);
}


LONG CuDlgParameterInstallationExtra::OnListCtrlEditing (WPARAM w, LPARAM l)
{
	CRect r, rCell;
	int iValueCol = 1;
	int nSelected = m_cListCtrl.GetNextItem (-1, LVNI_SELECTED);
	if (nSelected == -1)
		return 0;

	m_cListCtrl.GetItemRect (nSelected, r, LVIR_BOUNDS);
	BOOL bOk = m_cListCtrl.GetCellRect (r, iValueCol, rCell);
	if (bOk)
	{
		rCell.top    -= 2;
		rCell.bottom += 2;
		m_cListCtrl.EditValue (nSelected, iValueCol, rCell);
	}
	return 0;
}


LONG CuDlgParameterInstallationExtra::OnEnableControl(WPARAM w, LPARAM l)
{
	UINT nState = ST_ADD;
	int iNameCol  = 0;
	int iValueCol = 1;
	int nSelected = m_cListCtrl.GetNextItem (-1, LVNI_SELECTED);
	if (nSelected != -1)
	{
		CaEnvironmentParameter* pParameter = (CaEnvironmentParameter*)m_cListCtrl.GetItemData(nSelected);
		if (pParameter)
		{
			nState |= ST_MODIFY;
			CString strName  = m_cListCtrl.GetItemText (nSelected, iNameCol);
			CString strValue = m_cListCtrl.GetItemText (nSelected, iValueCol);

			strValue.TrimLeft();
			strValue.TrimRight();
		
			if (strValue.CompareNoCase (theApp.m_strVariableNotSet) != 0)
				nState |= ST_UNSET;
		}
	}

	(GetParent()->GetParent())->SendMessage (WMUSRMSG_ENABLE_CONTROL, (WPARAM)nState, 0);
	return 0;
}


LONG CuDlgParameterInstallationExtra::OnUpdateData (WPARAM wParam, LPARAM lParam)
{
	RefreshParameter();
	return 0;
}

LONG CuDlgParameterInstallationExtra::OnAddParameter (WPARAM wParam, LPARAM lParam)
{
	CaModalUp modalUp;
	CxEnvironmentVariable dlg;
	if (dlg.DoModal() == IDOK)
	{
		CString strName  = dlg.m_strName;
		CString strValue = dlg.m_strValue;
		strValue.TrimLeft();
		strValue.TrimRight();

		if (strName.IsEmpty() || strValue.IsEmpty())
			return 0L;
	
		BOOL bSystem = TRUE; // Use ingsetenv
		BOOL bUnset  = FALSE;// Set
		IVM_SetIngresVariable (strName, strValue, bSystem, bUnset);
		OnUpdateData(0, 0);
	}
	return 0L;
}


LONG CuDlgParameterInstallationExtra::OnFindAccepted (WPARAM wParam, LPARAM lParam)
{
	return (LONG)TRUE;
}


LONG CuDlgParameterInstallationExtra::OnFind (WPARAM wParam, LPARAM lParam)
{
	TRACE0 ("CuDlgParameterInstallationExtra::OnFind \n");
	return FIND_ListCtrlParameter((WPARAM)&m_cListCtrl, lParam);
}
