/*
	PropertyList.h
	(c)2006 Palestar, Richard Lyle
*/

#ifndef CPROPERTYLIST_H
#define CPROPERTYLIST_H

#include "Reflection/PropertyClass.h"
#include "Tools/ResourcerDoc/ResourcerDll.h"

class Window;

//---------------------------------------------------------------------------------------------------

class DLL CPropertyList : public CListBox
{
	// Construction
public:
	CPropertyList();

	// Accessors
	bool		CanClearContained( int nItem ) const;              // true if the item can be cleared
	bool		CanDeleteFromContainer( int nItem ) const;			// true if we can delete the specified item
	bool		CanAddToContainer( int nItem ) const;				   // true if we can add to the specified item
	bool		CanOpenClose( int nItem ) const;

	// Mutators
	int			AddClass( PropertyClass * pClass, int nInsert = -1, int nParent = -1 );
	void		DeleteAll();

	bool		ClearContained( int nItem );                          
	bool		DeleteFromContainer( int nItem );
	bool		AddToContainer( int nItem );

	int			RecursiveDelete( int nItem, 
					bool bDeleteTop = true );                       // deletes the specified item (if bDeleteTop is true) and all it's children
	void		SetSelected( int nSelected );
	bool		OpenItem( int nItem );
	bool		CloseItem( int nItem );

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyList)
public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CPropertyList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg void OnSelchangeCmbBox();
	afx_msg void OnChangeEditBox();
	afx_msg void OnEditButton();
	afx_msg void OnOpenButton();

	DECLARE_MESSAGE_MAP()

	void InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo);

	CComboBox *	m_pComboBox;
	CEdit *		m_pEditBox;
	CButton *	m_pEditButton;
	CButton *	m_pOpenButton;
	CImageList	m_Images;
	CFont       m_Font;

	int			m_nCurSel,m_nPrevSel;
	int			m_nDivider;
	int			m_nDivTop;
	int			m_nDivBtm;
	int			m_nOldDivX;
	int			m_nLastBox;
	BOOL		m_bTracking;
	BOOL		m_bDivIsSet;
	HCURSOR		m_hCursorArrow;
	HCURSOR		m_hCursorSize;
};

#endif

//---------------------------------------------------------------------------------------------------
//EOF
