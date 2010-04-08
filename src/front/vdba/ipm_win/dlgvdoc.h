/*
**  Copyright (C) 2005-2006 Ingres Corporation. All Rights Reserved.		       
*/

/*
**    Source   : dlgvdoc.h, Header file.
**    Project  : INGRES II/ Monitoring.
**    Author   : UK Sotheavut (uk$so01)
**    Purpose  : Document for CFormView, Scrollable Dialog box of Detail Page
**
** History:
**
** xx-Mar-1997 (uk$so01)
**    Created
*/

#ifndef DLGVDOC_HEADER
#define DLGVDOC_HEADER

class CuDlgViewDoc : public CDocument
{
	DECLARE_DYNCREATE(CuDlgViewDoc)
public:
	CuDlgViewDoc(); 


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CuDlgViewDoc)
	public:
	virtual void Serialize(CArchive& ar); // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CuDlgViewDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CuDlgViewDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif
