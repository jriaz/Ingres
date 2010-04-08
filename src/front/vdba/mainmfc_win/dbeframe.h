/****************************************************************************************
//                                                                                     //
//  Copyright (C) 2005-2006 Ingres Corporation. All Rights Reserved.		       //
//                                                                                     //
//    Source   : DbeFrame.h, Header file    (MDI Child Frame)                          //
//                                                                                     //
//                                                                                     //
//    Project  : CA-OpenIngres/Monitoring.                                             //
//    Author   : EMB, Standard Skeleton Generated by MFC Widzard.                      //
//               UK Sotheavut, Detail implementation.                                  //
//                                                                                     //
//    Purpose  : Frame window for the DBEvent Trace.                                   //
//                                                                                     //
****************************************************************************************/
#include "combobar.h"
#include "chboxbar.h"
#include "sbdbebar.h"

#define TOOL_DATABASE_POS       2
#define TOOL_POPUP_POS          4
#define TOOL_MAXLINETEXT_POS    6
#define TOOL_MAXLINE_POS        7
#define COMBOBARWIDTH           120

class CuDlgDBEventPane01;
class CuDlgDBEventPane02;
class CDbeventFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CDbeventFrame)
protected:
	CSplitterWnd m_wndSplitter;
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	CDbeventFrame();           // protected constructor used by dynamic creation

    CuDbeDlgBar     m_DbeDlgBar;

    int  m_iMaxLine;
    int  m_iMaxLineMin;
    int  m_iMaxLineMax;
    BOOL m_bPopup;
    CString m_strCurrentDB;
    // Attributes
public:
    CuComboToolBar  m_ComboDatabase;
    CuCheckBoxBar   m_CheckPopup;
    CStatic         m_StaticMaxLine;
    CEdit*          m_pEditMaxLine;

    CuDlgDBEventPane01* GetPaneRegisteredDBEvent();
    CuDlgDBEventPane02* GetPaneRaisedDBEvent();
    BOOL RequeryDatabase ();
    CuDbeDlgBar* GetDialogBar (){return &m_DbeDlgBar;}
    BOOL IsAllViewCreated(){return m_bAllViewCreated;}
    // Operations
public:

    // Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDbeventFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CDbeventFrame();

	// Generated message map functions
	//{{AFX_MSG(CDbeventFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnUpdateDbeventTraceDatabase(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDbeventTraceMaxline(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDbeventTracePopup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDbeventTraceRefresh(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDbeventTraceReset(CCmdUI* pCmdUI);
	afx_msg void OnDbeventTracePopup();
	afx_msg void OnDbeventTraceRefresh();
	afx_msg void OnDbeventTraceReset();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg void OnSelChangeDatabase();
	afx_msg void OnRequeryDatabase();
    afx_msg void OnSetFocusMaxLine();
    afx_msg void OnKillFocusMaxLine();
    afx_msg void OnEditChangeMaxLine();
    afx_msg LONG OnRefresh (WPARAM wParam, LPARAM lParam);
    afx_msg LONG OnGetNodeHandle(UINT uu, LONG ll);
    afx_msg LONG OnGetMfcDocType(UINT uu, LONG ll);
    afx_msg void OnSystemDBEvent();
    afx_msg void OnClearFirst();
    afx_msg LONG OnDbeventTraceIncoming (WPARAM wParam, LPARAM lParam);

    // toolbar management
    afx_msg LONG OnHasToolbar       (WPARAM wParam, LPARAM lParam);
    afx_msg LONG OnIsToolbarVisible (WPARAM wParam, LPARAM lParam);
    afx_msg LONG OnSetToolbarState  (WPARAM wParam, LPARAM lParam);
    afx_msg LONG OnSetToolbarCaption(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
private:
    BOOL     m_bMaxLineChanged;
    BOOL     m_bAllViewCreated;
	CToolBar m_wndToolBar;
};

/////////////////////////////////////////////////////////////////////////////
