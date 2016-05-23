// PropertyList.cpp : implementation file
//

#include "stdafx.h"
//#include <afxdllx.h>
#include "PropertyList.h"
#include "SelectClass.h"
#include "SelectBits.h"
#include "SelectResource.h"

#include "Standard/Color.h"
#include "File/Path.h"
#include "Factory/Widget.h"

#include "Resource.h"

#define IDC_EDIT_BUTTON			(901)
#define IDC_OPEN_BUTTON			(902)

//---------------------------------------------------------------------------------------------------

#undef RGB
static COLORREF RGB( unsigned char r, unsigned char g, unsigned char b )
{
	return( r | (g << 8) | (b << 16) );
}

//---------------------------------------------------------------------------------------------------

enum {

	PL_INSET             = 10,
	PL_FONTSIZE          = 12,
	PL_FONTWEIGHT        = 400,

	// PropertyList Image constants
	PLI_OPEN					= 0,
	PLI_CLOSE				= 1,
	PLI_ADD					= 2,
	PLI_DELETE				= 3,

	PLI_WIDTH				= 16,
	PLI_HEIGHT				= 16,
	PLI_SPACING				= 4,			// spacing between images (horz)
};

//---------------------------------------------------------------------------------------------------

const char *      PL_FONT_FACE = "Verdana";

const int			DEFAULT_ITEM_HEIGHT = 25;
const COLORREF		BACKGROUND_COLOR = RGB(192,192,192);
const COLORREF		HIGH_COLOR = RGB(0,192,192);
const COLORREF		BORDER_COLOR = RGB(80,80,80);

//---------------------------------------------------------------------------------------------------

struct PropertyListItem
{
	PropertyListItem() :pParent( NULL ), nHeight( DEFAULT_ITEM_HEIGHT ), nOffset( 0 ), 
		pClass( NULL ), pProperty( NULL ), bIsElement( false ), nElement( 0 ), bCanOpen( false ), 
		bOpen( false ), pEditControl( NULL ), pEditButton( NULL ), pOpenButton( NULL )
	{}

	PropertyListItem *	pParent;						// parent list item
	int					nHeight;						// height of this item
	int					nOffset;						// offset in pixels from the left
	PropertyClass *		pClass;						// the class this is a property for..
	Property *			pProperty;					// property, may be NULL

	bool                bIsElement;             // true if property is a member of an array
	int					nElement;					// element index

	bool				bCanOpen;					// true if this item can be opened
	bool				bOpen;						// is list open or close
	CRect				rOpenButton;				// open button rect
	CRect               rRight;                 // right side for this item

	CWnd *              pEditControl;           // control being used to edit this value if any
	CButton *           pEditButton;			// button to edit this value
	CButton *			pOpenButton;			// button to open this value
};

//---------------------------------------------------------------------------------------------------

CPropertyList::CPropertyList() 
: m_bDivIsSet( FALSE ), m_nDivider( 0 ), m_bTracking( FALSE ), m_nCurSel( -1 ), m_nPrevSel( -1 ),
m_pEditBox( NULL ), m_pComboBox( NULL ), m_pEditButton( NULL ), m_pOpenButton( NULL )
{}

CPropertyList::~CPropertyList()
{
	if ( m_pEditBox )
		delete m_pEditBox;
	if ( m_pComboBox )
		delete m_pComboBox;
	if ( m_pEditButton )
		delete m_pEditButton;
	if ( m_pOpenButton )
		delete m_pOpenButton;
}

//---------------------------------------------------------------------------------------------------

bool CPropertyList::CanClearContained( int nItem ) const
{
	if ( nItem < 0 || nItem >= GetCount() )
		return false;
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );
	return pItem->pProperty->isDynamic() && !pItem->bIsElement;
}

bool CPropertyList::CanDeleteFromContainer( int nItem ) const
{
	if ( nItem < 0 || nItem >= GetCount() )
		return false;
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );
	if ( pItem->pParent == NULL || !pItem->bIsElement )
		return false;
	return pItem->pParent->pProperty->isDynamic();
}

bool CPropertyList::CanAddToContainer( int nItem ) const
{
	if ( nItem < 0 || nItem >= GetCount() )
		return false;
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );
	return pItem->pProperty->isDynamic() && !pItem->bIsElement;
}

bool CPropertyList::CanOpenClose( int nItem ) const
{
	if ( nItem < 0 || nItem >= GetCount() )
		return false;
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );
	if (! pItem->bCanOpen )
		return false;
	return true;
}

//---------------------------------------------------------------------------------------------------

int CPropertyList::AddClass( PropertyClass * pClass, int nInsert /*= -1*/, int nParent /*= -1*/ )
{
	if ( pClass == NULL )
		return 0;
	PropertyListItem * pParent = NULL;
	if ( nParent >= 0 )
		pParent = (PropertyListItem *)GetItemDataPtr( nParent );

	int nCount = 0;
	if ( nInsert < 0 )
		nInsert = GetCount();

	PropertyList * pList = pClass->propertyList();
	for(int i=0;i<pList->propertyCount();i++)
	{
		Property * pProperty = pList->property( i );
		if ( pProperty->flags() & PF_NOEDIT )
			continue;		// skip, no edit flag...

		int nItem = InsertString( nInsert + nCount, _T("") );

		PropertyListItem * pItem = new PropertyListItem();
		pItem->pParent = pParent;
		pItem->nHeight = DEFAULT_ITEM_HEIGHT;
		pItem->nOffset = pParent != NULL ? pParent->nOffset + PL_INSET : 0;
		pItem->pClass = pClass;
		pItem->pProperty = pProperty;

		pItem->bIsElement = false;
		pItem->nElement = 0;

		pItem->bCanOpen = pProperty->type()->propertyList() != NULL ||
			pProperty->isDynamic() || pProperty->count( pClass ) > 1;

		SetItemDataPtr( nItem, pItem );
		nCount++;
	}

	return nInsert;
}

void CPropertyList::DeleteAll()
{
	while( GetCount() > 0 )
		RecursiveDelete( 0 );
}

bool CPropertyList::ClearContained( int nItem )
{
	if ( nItem < 0 || nItem >= GetCount() )
		return false;
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );
	if (! pItem->pProperty->isDynamic() || pItem->bIsElement )
		return false;

	pItem->pProperty->clear( pItem->pClass );
	pItem->bOpen = false;
	RecursiveDelete( nItem, false );

	return true;
}

bool CPropertyList::DeleteFromContainer( int nItem )
{
	if ( nItem < 0 || nItem >= GetCount() )
		return false;
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );
	if ( pItem->pParent == NULL )
		return false;
	if (! pItem->pParent->pProperty->isDynamic() )
		return false;

	// check to see if the item is a pointer, if so then we need to release the object correctly to avoid a memory leak..
	Type * pType = pItem->pProperty->type();
	if ( pType->isPointer() )
	{
		PropertyList * pList = pType->propertyList();
		if ( pList != NULL )
		{
			void ** pPointer = (void **)pItem->pProperty->pointer( pItem->pClass, pItem->nElement );
			void * pObject = *pPointer;

			if ( pObject != NULL )
			{
				if ( pType->isReference() )
					((PropertyClass *)pObject)->releaseReference();
				else 
					pType->free( pObject );

				*pPointer = NULL;
			}
		}
	}

	// update the element index for other siblings
	int nCount = GetCount();
	for(int j=nItem + 1;j<nCount;j++)
	{
		PropertyListItem * pSibling = (PropertyListItem *)GetItemDataPtr( j );
		if ( pSibling->pParent == pItem->pParent )
			pSibling->nElement--;
	}

	// now actually remove the item..
	if (! pItem->pParent->pProperty->remove( pItem->pParent->pClass, pItem->nElement ) )
		return false;

	RecursiveDelete( nItem, true );

	return false;
}

bool CPropertyList::AddToContainer( int nItem )
{
	if ( nItem < 0 || nItem >= GetCount() )
		return false;
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );
	if (! pItem->pProperty->isDynamic() || pItem->bIsElement )
		return false;

	// check to see if the item is a pointer, if so then we need to create an object...
	Type * pType = pItem->pProperty->type();
	if ( pType->isPointer() )
	{
		PropertyList * pList = pType->propertyList();
		if ( pList != NULL )
		{
			CSelectClass dialog;
			
			Factory * pFactory = Factory::findFactoryByTypeId( pList->type()->typeId() );
			if ( pFactory != NULL )
                dialog.m_Base = pFactory->classKey();

			if ( dialog.DoModal() == IDOK )
			{
				int nElement = pItem->pProperty->add( pItem->pClass );
				if ( nElement < 0 )
					return false;

				void ** pPointer = (void **)pItem->pProperty->pointer( pItem->pClass, nElement );
				void * pObject = Factory::createWidget( dialog.m_Selected );
				if ( pType->isReference() && pObject != NULL )
					((Referenced *)pObject)->grabReference();
				*pPointer = pObject;
			}
		}
	}
	else
	{
		// add a new item to the container...
		pItem->pProperty->add( pItem->pClass );
	}

	// close then reopen the item to display the new array member...
	CloseItem( nItem );
	OpenItem( nItem );

	return true;
}

int CPropertyList::RecursiveDelete( int nItem, bool bDeleteTop )
{
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );

	int nCount = 0;
	int nNext = nItem + 1;
	while( nNext < GetCount() )
	{
		PropertyListItem * pChild = (PropertyListItem *)GetItemDataPtr( nNext );
		if ( pChild->pParent != pItem )
			break;      // not a child item, stop trying to delete..

		nCount += RecursiveDelete( nNext, true );
	}

	if ( bDeleteTop )
	{
		nCount++;

		if ( nItem == m_nCurSel )
			SetSelected( nItem - 1 );
		if ( nItem <= m_nPrevSel )
			m_nPrevSel--;
		if ( nItem <= m_nCurSel )
			m_nCurSel--;

		if ( pItem->pEditControl != NULL )
		{
			pItem->pEditControl->ShowWindow( SW_HIDE );
			pItem->pEditControl = NULL;
		}

		delete pItem;
		DeleteString( nItem );
	}

	return nCount;
}

void CPropertyList::SetSelected( int nSelected )
{
	m_nCurSel = nSelected;

	PropertyListItem * pOldItem = (m_nPrevSel >= 0 && m_nPrevSel < GetCount()) ? (PropertyListItem *)GetItemDataPtr( m_nPrevSel ) : NULL;
	if ( pOldItem != NULL )
	{
		if ( pOldItem->pEditControl != NULL  )
		{
			pOldItem->pEditControl->ShowWindow( SW_HIDE );
			pOldItem->pEditControl = NULL;
		}
		if ( pOldItem->pEditButton != NULL )
		{
			pOldItem->pEditButton->ShowWindow( SW_HIDE );
			pOldItem->pEditButton = NULL;
		}
		if ( pOldItem->pOpenButton != NULL )
		{
			pOldItem->pOpenButton->ShowWindow( SW_HIDE );
			pOldItem->pOpenButton = NULL;
		}
	}

	if ( m_nPrevSel != m_nCurSel )
	{
		CRect r;
		if ( m_nPrevSel >= 0 )
		{
			GetItemRect( m_nPrevSel, r );
			InvalidateRect( r );
		}
		if ( m_nCurSel >= 0 )
		{
			GetItemRect( m_nCurSel, r );
			InvalidateRect( r );
		}
		m_nPrevSel = m_nCurSel;
	}

	// set the current selection in the ListBox control object to match...
	SetCurSel( m_nCurSel );

	PropertyListItem * pItem = m_nCurSel >= 0 ? (PropertyListItem *)GetItemDataPtr( m_nCurSel ) : NULL;
	if ( pItem == NULL )
		return;
	if ( pItem->bCanOpen )
		return;        // this item is an array or class object, it cannot be edited then...

	CRect rRight( pItem->rRight );
	if ( pItem->pProperty->flags() & (PF_FILENAME|PF_COLOR|PF_BITS|PF_CLASSKEY|PF_WIDGETKEY|PF_PORT) )
	{
		CRect rButton( rRight );
		if ( rButton.Width() > 25 )
		{
			rButton.left = rButton.right - 25;
			rRight.right = rButton.left;
		}

		if ( m_pEditButton == NULL )
		{
			m_pEditButton = new CButton();
			m_pEditButton->Create( _T("..."), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, rButton, this, IDC_EDIT_BUTTON );
			m_pEditButton->SetFont( &m_Font );
		}
		m_pEditButton->MoveWindow( rButton );
		m_pEditButton->ShowWindow( SW_SHOW );
		m_pEditButton->SetFocus();

		pItem->pEditButton = m_pEditButton;

		if ( pItem->pProperty->flags() & (PF_FILENAME|PF_PORT) )
		{
			CRect rOpenButton( rButton );
			rOpenButton.OffsetRect( -25, 0 );

			if (! m_pOpenButton )
			{
				m_pOpenButton = new CButton();
				m_pOpenButton->Create( _T("*"), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, rOpenButton, this, IDC_OPEN_BUTTON );
				m_pOpenButton->SetFont( &m_Font );
			}

			m_pOpenButton->MoveWindow( rOpenButton );
			m_pOpenButton->ShowWindow( SW_SHOW );
			m_pOpenButton->SetFocus();

			pItem->pEditButton = m_pEditButton;
		}

		// don't display combo/edit box for the following property types...
		if ( pItem->pProperty->flags() & (PF_BITS|PF_CLASSKEY|PF_WIDGETKEY|PF_PORT) )
			return;
	}

	if ( pItem->pProperty->optionCount() > 0 )
	{
		CRect r( rRight );
		r.bottom += 250;

		// we have options, so display a combo box
		if ( m_pComboBox == NULL )
		{
			m_pComboBox = new CComboBox();
			m_pComboBox->Create( CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT | WS_VISIBLE | WS_CHILD, r, this, IDC_PROPCMBBOX );
			m_pComboBox->SetFont( &m_Font );
		}

		// get the current value
		CharString sValue;
		if (! pItem->pClass->get( pItem->pProperty, pItem->nElement, sValue ) )
			return;

		m_pComboBox->ResetContent();
		for(int i=0;i<pItem->pProperty->optionCount();i++)
		{
			// add the options to the combo box
			const Property::Option & option = pItem->pProperty->option( i );
			int nItem = m_pComboBox->AddString( option.sName );
			m_pComboBox->SetItemData( nItem, i );
			if ( option.sName == sValue )
				m_pComboBox->SetCurSel( nItem );
		}

		m_pComboBox->MoveWindow( r );
		m_pComboBox->ShowWindow( SW_SHOW );
		m_pComboBox->SetFocus();

		pItem->pEditControl = m_pComboBox;
	}
	else
	{
		CharString sValue;
		if (! pItem->pClass->get( pItem->pProperty, pItem->nElement, sValue ) )
			return;

		// lastly, display an edit box..
		if ( m_pEditBox == NULL )
		{
			m_pEditBox = new CEdit();
			m_pEditBox->Create( ES_LEFT | ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD,
				rRight, this, IDC_PROPEDITBOX );
			m_pEditBox->SetFont( &m_Font );
		}

		m_pEditBox->SetWindowText( sValue );
		m_pEditBox->MoveWindow( rRight );
		m_pEditBox->ShowWindow( SW_SHOW );
		m_pEditBox->SetFocus();
		m_pEditBox->SetSel( 0, -1 );

		pItem->pEditControl = m_pEditBox;
	}
}

bool CPropertyList::OpenItem( int nItem )
{
	if ( nItem < 0 || nItem >= GetCount() )
		return false;
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );
	if (! pItem->bCanOpen )
		return false;
	if ( pItem->bOpen )
		return true;      // item already open

	Property * pProperty = pItem->pProperty;
	int nCount = pProperty->count( pItem->pClass );
	bool bIsArray = pProperty->isDynamic() || nCount > 1;

	// opening the item, we are either opening an array or a propertyList
	if ( bIsArray && !pItem->bIsElement )
	{
		// opening an array...
		for(int n=0;n<nCount;n++)
		{
			int nSubItem = InsertString( nItem + n + 1, _T("") );

			PropertyListItem * pSubItem = new PropertyListItem();
			pSubItem->pParent = pItem;
			pSubItem->nHeight = DEFAULT_ITEM_HEIGHT;
			pSubItem->nOffset = pItem->nOffset + PL_INSET;
			pSubItem->pClass = pItem->pClass;
			pSubItem->pProperty = pItem->pProperty;
			pSubItem->bIsElement = true;
			pSubItem->nElement = n;
			pSubItem->bCanOpen = pItem->pProperty->type()->propertyList() != NULL;

			SetItemDataPtr( nSubItem, pSubItem );
		}
	}
	else
	{
		if ( pProperty->type()->isPointer() )
		{
			void ** pPointer = (void **)pProperty->pointer( pItem->pClass, pItem->nElement );
			void * pObject = *pPointer;

			// create the object if it doesn't exist..
			if ( pObject == NULL )
			{
				// check to see if the item is a pointer, if so then we need to create an object...
				PropertyList * pList = pProperty->type()->propertyList();
				if ( pList == NULL )
					return false;

				CSelectClass dialog;

				Factory * pFactory = Factory::findFactoryByTypeId( pList->type()->typeId() );
				if ( pFactory != NULL )
					dialog.m_Base = pFactory->classKey();

				if ( dialog.DoModal() != IDOK )
					return false;

				pObject = Factory::createWidget( dialog.m_Selected );
				if ( pObject == NULL )
					return false;

				if ( pProperty->type()->isReference() )
					((Referenced *)pObject)->grabReference();
				*pPointer = pObject;
			}

			AddClass( (PropertyClass *)pObject, nItem + 1, nItem );
		}
		else
		{
			AddClass( (PropertyClass *)pProperty->pointer( pItem->pClass, pItem->nElement), 
				nItem + 1, nItem );
		}
	}

	pItem->bOpen = true;
	return true;
}

bool CPropertyList::CloseItem( int nItem )
{
	if ( nItem < 0 || nItem >= GetCount() )
		return false;
	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nItem );
	if (! pItem->bCanOpen )
		return false;
	if (! pItem->bOpen )
		return true;      // item already closed

	RecursiveDelete( nItem, false );
	pItem->bOpen = false;

	return true;
}

//---------------------------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPropertyList, CListBox)
	//{{AFX_MSG_MAP(CPropertyList)
	ON_WM_CREATE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_PROPCMBBOX, OnSelchangeCmbBox)
	ON_EN_CHANGE(IDC_PROPEDITBOX, OnChangeEditBox)
	ON_BN_CLICKED(IDC_EDIT_BUTTON, OnEditButton)
	ON_BN_CLICKED(IDC_OPEN_BUTTON, OnOpenButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyList message handlers

BOOL CPropertyList::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CListBox::PreCreateWindow(cs))
		return FALSE;

	cs.style &= ~(LBS_OWNERDRAWVARIABLE | LBS_SORT);
	cs.style |= LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_NOTIFY|WS_VSCROLL;

	m_bTracking = FALSE;
	m_nDivider = 0;
	m_bDivIsSet = FALSE;

	return TRUE;
}

void CPropertyList::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	int nIndex = lpMeasureItemStruct->itemID;
	if ( nIndex < GetCount() )
	{
		PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nIndex );
		lpMeasureItemStruct->itemHeight = pItem->nHeight; //pixels
	}
	else
	{
		lpMeasureItemStruct->itemHeight = DEFAULT_ITEM_HEIGHT;
	}
}

void CPropertyList::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	int nIndex = lpDIS->itemID;
	if ( nIndex < 0 || nIndex >= GetCount() )
		return;

	CDC dc;
	dc.Attach(lpDIS->hDC);

	CFont * pOldFont = dc.SelectObject( &m_Font );

	PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nIndex );

	CRect rFull( lpDIS->rcItem );
	if ( m_nDivider == 0 )
		m_nDivider = rFull.Width() / 3;
	CRect rRight( rFull );
	rRight.left = m_nDivider;
	CRect rLeft( rFull );
	rLeft.right = rRight.left - 1;

	Property * pProperty = pItem->pProperty;
	if ( ! pItem->bCanOpen )
	{
		dc.FillSolidRect( rLeft, nIndex == m_nCurSel ? HIGH_COLOR : BACKGROUND_COLOR );
		dc.DrawEdge( rLeft, EDGE_ETCHED, BF_BOTTOMRIGHT );
		dc.DrawEdge( rRight, EDGE_ETCHED, BF_BOTTOMRIGHT );

		rLeft.DeflateRect( 2, 2 );
		rLeft.left += pItem->nOffset;

		dc.SetBkMode(TRANSPARENT);

		CharString sPropertyName = pItem->pProperty->name();
		if ( pItem->bIsElement )
			sPropertyName += CharString().format( "[%d]", pItem->nElement);

		//sPropertyName += StringUtil::Format( " (%s)", pItem->pProperty->type()->name() );
		dc.DrawText( sPropertyName.cstr(), rLeft, DT_LEFT | DT_SINGLELINE);
		rRight.DeflateRect( 2, 2 );

		CharString sValue;
		if ( pItem->pClass->get( pItem->pProperty, pItem->nElement, sValue ) )
			dc.DrawText( sValue.cstr(), rRight, DT_LEFT | DT_SINGLELINE );
		pItem->rRight = rRight;
	}
	else
	{
		dc.FillSolidRect( rFull, nIndex == m_nCurSel ? HIGH_COLOR : BACKGROUND_COLOR );
		dc.DrawEdge( rFull, EDGE_ETCHED, BF_BOTTOMRIGHT );
		rFull.DeflateRect( 2, 2 );

		rFull.left += pItem->nOffset;

		pItem->rOpenButton = rFull;
		pItem->rOpenButton.right = rFull.left + PLI_WIDTH;
		rFull.left += PLI_WIDTH + PLI_SPACING;

		if ( !pItem->bOpen )
			m_Images.Draw( &dc, PLI_OPEN, pItem->rOpenButton.TopLeft(), ILD_NORMAL );
		else
			m_Images.Draw( &dc, PLI_CLOSE, pItem->rOpenButton.TopLeft(), ILD_NORMAL );

		dc.SetBkMode(TRANSPARENT);
		CharString sPropertyName = pItem->pProperty->name();
		if ( pItem->bIsElement )
			sPropertyName += CharString().format( "[%d]", pItem->nElement);
		dc.DrawText( sPropertyName.cstr(), rFull, DT_LEFT | DT_SINGLELINE);

		if ( pItem->pProperty->type()->isPointer() && (pItem->bIsElement || !pItem->pProperty->isDynamic()) )
		{
			void ** pPointer = (void **)pItem->pProperty->pointer( pItem->pClass, pItem->nElement );
			PropertyClass * pObject = (PropertyClass *)(*pPointer);

			if ( pObject != NULL )
			{
				CharString sObject = CharString().format( "%p %s", pObject, 
					pObject->propertyList()->className() );
				sPropertyName += CharString(" (") + sObject + CharString(")");
			}
			else
			{
				sPropertyName += " (NULL)";
			}
		}

		dc.DrawText( sPropertyName.cstr(), rFull, DT_LEFT | DT_SINGLELINE);
		//if ( sObject.length() > 0 )
		//   dc.DrawText( sObject.c_str(), rRight, DT_LEFT | DT_SINGLELINE );
	}

	dc.SelectObject( pOldFont );
	dc.Detach();
}

int CPropertyList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_bDivIsSet = FALSE;
	m_nDivider = 0;
	m_bTracking = FALSE;

	m_hCursorSize = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
	m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);

	//m_SSerif8Font.CreatePointFont(80,_T("MS Sans Serif"));

	return 0;
}

void CPropertyList::OnSelchangeCmbBox()
{
	if ( m_pComboBox != NULL && m_nCurSel >= 0 )
	{
		int nSelected = m_pComboBox->GetCurSel();
		if ( nSelected >= 0 )
		{
			CString sSelected;
			m_pComboBox->GetLBText( nSelected, sSelected );

			PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( m_nCurSel );
			pItem->pClass->set( pItem->pProperty, pItem->nElement, CharString( sSelected ) );
		}
	}
}

void CPropertyList::OnChangeEditBox()
{
	if ( m_pEditBox != NULL && m_nCurSel >= 0 )
	{
		CString sText;
		m_pEditBox->GetWindowText( sText );

		CharString sValue = sText;
		PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( m_nCurSel );
		pItem->pClass->set( pItem->pProperty, pItem->nElement, sValue );
	}
}

void CPropertyList::OnEditButton()
{
	if ( m_pEditButton != NULL && m_nCurSel >= 0 )
	{
		PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( m_nCurSel );
		if ( pItem->pProperty->flags() & PF_FILENAME )
		{
			// get the current property value..
			CharString sValue;
			pItem->pClass->get( pItem->pProperty, pItem->nElement, sValue );

			CFileDialog dialog(TRUE, NULL, NULL, NULL, "Ports (*.prt)|*.prt|Files (*.*)|*.*||");
			dialog.m_ofn.lpstrTitle = "Select file";
			if ( sValue.length() > 0 )
				dialog.m_ofn.lpstrInitialDir = Path( sValue ).directory().cstr();

			if( dialog.DoModal() == IDOK )
			{
				sValue = CResourcerDoc::document()->relative( dialog.GetPathName() );

				pItem->pClass->set( pItem->pProperty, pItem->nElement, sValue );
				InvalidateRect( pItem->rRight );
				SetSelected( m_nCurSel );
			}
		}
		else if ( pItem->pProperty->flags() & PF_COLOR )
		{
			Color cValue;
			pItem->pClass->get( pItem->pProperty, pItem->nElement, cValue );

			CColorDialog dialog( cValue.RGB(), CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT );
			if ( dialog.DoModal() == IDOK )
			{
				Color cSelected( dialog.GetColor() );
				cSelected.alpha = cValue.alpha;

				pItem->pClass->set( pItem->pProperty, pItem->nElement, cSelected );
				InvalidateRect( pItem->rRight );
				SetSelected( m_nCurSel );
			}
		}
		else if ( pItem->pProperty->flags() & PF_BITS )
		{
			dword nValue;
			pItem->pClass->get( pItem->pProperty, pItem->nElement, nValue );

			CSelectBits dialog;
			dialog.m_nBits = nValue;
			dialog.m_pProperty = pItem->pProperty;

			if ( dialog.DoModal() == IDOK )
			{
				pItem->pClass->set( pItem->pProperty, pItem->nElement, dialog.m_nBits );

				InvalidateRect( pItem->rRight );
				SetSelected( m_nCurSel );
			}
		}
		else if ( pItem->pProperty->flags() & PF_CLASSKEY )
		{
			ClassKey nKey( (qword)0 );
			pItem->pClass->get( pItem->pProperty, pItem->nElement, nKey );

			CSelectClass dialog;

			Property::Option * pOption = pItem->pProperty->findOption( "_BASE" );
			if ( pOption != NULL )
			{
				Factory * pFactory = Factory::findFactoryByName( pOption->sValue );
				if ( pFactory != NULL )
					dialog.m_Base = pFactory->classKey();
			}

			if ( dialog.DoModal() == IDOK )
			{
				pItem->pClass->set( pItem->pProperty, pItem->nElement, dialog.m_Selected );

				InvalidateRect( pItem->rRight );
				SetSelected( m_nCurSel );
			}
		}
		else if ( pItem->pProperty->flags() & PF_PORT )
		{
			ClassKey nType( Resource::classKey() );

			Property::Option * pOption = pItem->pProperty->findOption( "_BASE" );
			if ( pOption != NULL )
			{
				Factory * pFactory = Factory::findFactoryByName( pOption->sValue );
				if ( pFactory != NULL )
					nType = pFactory->classKey();
			}

			CSelectResource dialog( nType );
			if ( dialog.DoModal() == IDOK )
			{
				CharString sSelected = dialog.m_Selected;
				pItem->pClass->set( pItem->pProperty, pItem->nElement, sSelected );

				InvalidateRect( pItem->rRight );
				SetSelected( m_nCurSel );
			}
		}
	}
}

VOID CPropertyList::OnOpenButton()
{
	if ( m_pEditButton != NULL && m_nCurSel >= 0 )
	{
		PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( m_nCurSel );
		if ( pItem->pProperty->flags() & (PF_FILENAME|PF_PORT) )
		{
			// get the current property value..
			CharString sValue;
			if ( pItem->pClass->get( pItem->pProperty, pItem->nElement, sValue ) )
			{
				// now open the file 
				CResourcerDoc *pDoc = CResourcerDoc::document();
				ASSERT( pDoc );

				CharString sFileName = CharString( pDoc->currentDirectory() ) + sValue;

				// open up ports using this program, other file types send to windows
				if ( stricmp<char>( Path( sFileName ).extension(), ".PRT" ) == 0 )
					AfxGetApp()->OpenDocumentFile( sFileName );
				else
					ShellExecute( NULL, "open", sFileName, NULL, pDoc->currentDirectory(), SW_SHOWNORMAL );
			}
		}
	}
}

void CPropertyList::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bTracking)
	{
		//if columns were being resized then this indicates
		//that mouse is up so resizing is done.  Need to redraw
		//columns to reflect their new widths.

		m_bTracking = FALSE;
		//if mouse was captured then release it
		if (GetCapture()==this)
			::ReleaseCapture();

		::ClipCursor(NULL);

		CClientDC dc(this);
		InvertLine(&dc,CPoint(point.x,m_nDivTop),CPoint(point.x,m_nDivBtm));
		//set the divider position to the new value
		m_nDivider = point.x;

		//redraw
		Invalidate();
	}
}

void CPropertyList::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ((point.x>=m_nDivider-5) && (point.x<=m_nDivider+5))
	{
		SetSelected( -1 );

		//if mouse clicked on divider line, then start resizing
		::SetCursor(m_hCursorSize);

		CRect windowRect;
		GetWindowRect(windowRect);
		windowRect.left += 10; windowRect.right -= 10;
		//do not let mouse leave the list box boundary
		::ClipCursor(windowRect);

		CRect clientRect;
		GetClientRect(clientRect);

		m_bTracking = TRUE;
		m_nDivTop = clientRect.top;
		m_nDivBtm = clientRect.bottom;
		m_nOldDivX = point.x;

		CClientDC dc(this);
		InvertLine(&dc,CPoint(m_nOldDivX,m_nDivTop),CPoint(m_nOldDivX,m_nDivBtm));

		//capture the mouse
		SetCapture();
	}
	else
	{
		BOOL bOutside = false;

		int nSelected = ItemFromPoint( point, bOutside);
		if ( bOutside )
			nSelected = -1;

		if ( nSelected >= 0 && nSelected < GetCount() )
		{
			PropertyListItem * pItem = (PropertyListItem *)GetItemDataPtr( nSelected );
			if ( pItem->bCanOpen && pItem->rOpenButton.PtInRect( point ) )
			{
				// open/close the item...
				if (! pItem->bOpen )
					OpenItem( nSelected );
				else
					CloseItem( nSelected );
			}
		}

		SetSelected( nSelected );
		m_bTracking = FALSE;
	}
}

void CPropertyList::OnMouseMove(UINT nFlags, CPoint point) 
{	
	if (m_bTracking)
	{
		//move divider line to the mouse pos. if columns are
		//currently being resized
		CClientDC dc(this);
		//remove old divider line
		InvertLine(&dc,CPoint(m_nOldDivX,m_nDivTop),CPoint(m_nOldDivX,m_nDivBtm));
		//draw new divider line
		InvertLine(&dc,CPoint(point.x,m_nDivTop),CPoint(point.x,m_nDivBtm));
		m_nOldDivX = point.x;
	}
	else if ((point.x >= m_nDivider-5) && (point.x <= m_nDivider+5))
		//set the cursor to a sizing cursor if the cursor is over the row divider
		::SetCursor(m_hCursorSize);
	else
		CListBox::OnMouseMove(nFlags, point);
}

void CPropertyList::InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo)
{
	int nOldMode = pDC->SetROP2(R2_NOT);

	pDC->MoveTo(ptFrom);
	pDC->LineTo(ptTo);

	pDC->SetROP2(nOldMode);
}

void CPropertyList::PreSubclassWindow() 
{
	m_hCursorSize = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
	m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);

	// see PropertyListImages, the order should match that enum...
	m_Images.Create( PLI_WIDTH, PLI_HEIGHT, ILC_COLOR32|ILC_MASK, 0, 1 );
	m_Images.Add( AfxGetApp()->LoadIcon( IDI_ARRAY_OPEN ) );
	m_Images.Add( AfxGetApp()->LoadIcon( IDI_ARRAY_CLOSE ) );
	m_Images.Add( AfxGetApp()->LoadIcon( IDI_ARRAY_ADD ) );
	m_Images.Add( AfxGetApp()->LoadIcon( IDI_ARRAY_DELETE ) );

	// create our font for drawing..
	LOGFONT lf;
	memset( &lf, 0, sizeof(lf) );
	strcpy_s( lf.lfFaceName, sizeof(lf.lfFaceName), PL_FONT_FACE );
	lf.lfHeight = PL_FONTSIZE;
	lf.lfWeight = PL_FONTWEIGHT;
	m_Font.CreateFontIndirect( &lf );
}

//---------------------------------------------------------------------------------------------------
// EOF