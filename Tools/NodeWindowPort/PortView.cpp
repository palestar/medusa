// PortView.cpp : implementation file
//

#define NODEWINDOWPORT_DLL
#include "stdafx.h"
#include "PortView.h"
#include "Port.h"

#include "Tools/ResourcerDoc/SelectClass.h"
#include "Tools/ResourcerDoc/SelectResource.h"
#include "Tools/WindowStylePort/Port.h"

#include "File/Path.h"



/////////////////////////////////////////////////////////////////////////////
// CPortView

IMPLEMENT_DYNCREATE(CPortView, CFormView)

CPortView::CPortView()
	: CFormView(CPortView::IDD)
{
	//{{AFX_DATA_INIT(CPortView)
	m_Class = _T("");
	m_Name = _T("");
	m_Visible = FALSE;
	m_Alpha = 0.0f;
	m_Left = 0;
	m_Top = 0;
	m_Right = 0;
	m_Bottom = 0;
	m_Docked = FALSE;
	m_CanMove = FALSE;
	m_CanSize = FALSE;
	m_CanScroll = FALSE;
	m_EffectFadeIn = FALSE;
	m_EffectHighlight = FALSE;
	m_EffectRollUV = FALSE;
	m_HasBack = FALSE;
	m_CanDock = FALSE;
	m_DockX = FALSE;
	m_DockY = FALSE;
	m_WindowStyle = _T("");
	m_Document = FALSE;
	m_NoClip = FALSE;
	m_HCenter = FALSE;
	m_VCenter = FALSE;
	m_DockZ = FALSE;
	m_DockOutside = FALSE;
	m_Height = 0;
	m_Width = 0;
	m_ParentWidth = FALSE;
	m_ParentHeight = FALSE;
	m_CanFocus = FALSE;
	m_User1 = FALSE;
	m_User2 = FALSE;
	m_User3 = FALSE;
	m_User4 = FALSE;
	m_User5 = FALSE;
	m_User6 = FALSE;
	m_User7 = FALSE;
	m_User8 = FALSE;
	m_SmoothUL = FALSE;
	m_SmoothUR = FALSE;
	m_SmoothLR = FALSE;
	m_SmoothLL = FALSE;
	m_AnimBeam = FALSE;
	m_AnimBack = FALSE;
	m_AnimFlare = FALSE;
	m_Help = _T("");
	//}}AFX_DATA_INIT
}

CPortView::~CPortView()
{
}

void CPortView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPortView)
	DDX_Control(pDX, IDC_LIST1, m_ResourceList);
	DDX_Text(pDX, IDC_EDIT7, m_Class);
	DDX_Text(pDX, IDC_EDIT11, m_Name);
	DDX_Check(pDX, IDC_CHECK2, m_Visible);
	DDX_Text(pDX, IDC_EDIT13, m_Alpha);
	DDX_Text(pDX, IDC_EDIT1, m_Left);
	DDX_Text(pDX, IDC_EDIT2, m_Top);
	DDX_Text(pDX, IDC_EDIT3, m_Right);
	DDX_Text(pDX, IDC_EDIT4, m_Bottom);
	DDX_Check(pDX, IDC_CHECK3, m_Docked);
	DDX_Check(pDX, IDC_CHECK7, m_CanMove);
	DDX_Check(pDX, IDC_CHECK8, m_CanSize);
	DDX_Check(pDX, IDC_CHECK9, m_CanScroll);
	DDX_Check(pDX, IDC_CHECK11, m_EffectFadeIn);
	DDX_Check(pDX, IDC_CHECK12, m_EffectHighlight);
	DDX_Check(pDX, IDC_CHECK14, m_EffectRollUV);
	DDX_Check(pDX, IDC_CHECK10, m_HasBack);
	DDX_Check(pDX, IDC_CHECK15, m_CanDock);
	DDX_Check(pDX, IDC_CHECK4, m_DockX);
	DDX_Check(pDX, IDC_CHECK5, m_DockY);
	DDX_Text(pDX, IDC_EDIT6, m_WindowStyle);
	DDX_Check(pDX, IDC_CHECK6, m_Document);
	DDX_Check(pDX, IDC_CHECK1, m_NoClip);
	DDX_Check(pDX, IDC_CHECK16, m_HCenter);
	DDX_Check(pDX, IDC_CHECK18, m_VCenter);
	DDX_Check(pDX, IDC_CHECK17, m_DockZ);
	DDX_Check(pDX, IDC_CHECK19, m_DockOutside);
	DDX_Text(pDX, IDC_EDIT8, m_Height);
	DDX_Text(pDX, IDC_EDIT5, m_Width);
	DDX_Check(pDX, IDC_CHECK20, m_ParentWidth);
	DDX_Check(pDX, IDC_CHECK21, m_ParentHeight);
	DDX_Check(pDX, IDC_CHECK22, m_CanFocus);
	DDX_Check(pDX, IDC_CHECK23, m_User1);
	DDX_Check(pDX, IDC_CHECK24, m_User2);
	DDX_Check(pDX, IDC_CHECK25, m_User3);
	DDX_Check(pDX, IDC_CHECK26, m_User4);
	DDX_Check(pDX, IDC_CHECK27, m_User5);
	DDX_Check(pDX, IDC_CHECK28, m_User6);
	DDX_Check(pDX, IDC_CHECK29, m_User7);
	DDX_Check(pDX, IDC_CHECK30, m_User8);
	DDX_Check(pDX, IDC_CHECK31, m_SmoothUL);
	DDX_Check(pDX, IDC_CHECK32, m_SmoothUR);
	DDX_Check(pDX, IDC_CHECK33, m_SmoothLR);
	DDX_Check(pDX, IDC_CHECK34, m_SmoothLL);
	DDX_Check(pDX, IDC_CHECK35, m_AnimBeam);
	DDX_Check(pDX, IDC_CHECK36, m_AnimBack);
	DDX_Check(pDX, IDC_CHECK37, m_AnimFlare);
	DDX_Text(pDX, IDC_EDIT9, m_Help);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPortView, CFormView)
	//{{AFX_MSG_MAP(CPortView)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnUpdateNode)
	ON_BN_CLICKED(IDC_BUTTON1, OnBrowseClass)
	ON_BN_CLICKED(IDC_BUTTON7, OnAddResource)
	ON_BN_CLICKED(IDC_BUTTON8, OnRemoveResource)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnEditResource)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, OnRenameResource)
	ON_BN_CLICKED(IDC_BUTTON2, OnOpenWindowStyle)
	ON_BN_CLICKED(IDC_BUTTON3, OnBrowseWindowStyle)
	ON_EN_UPDATE(IDC_EDIT5, OnUpdateWidthHeight)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnUpdateNode)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT11, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT13, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK11, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK12, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK14, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK15, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK2, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK3, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK4, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK5, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK7, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK8, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK9, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK10, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK6, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK16, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK18, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK17, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK19, OnUpdateNode)
	ON_EN_UPDATE(IDC_EDIT8, OnUpdateWidthHeight)
	ON_BN_CLICKED(IDC_CHECK20, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK21, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK22, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK23, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK24, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK25, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK26, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK27, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK28, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK29, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK30, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK31, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK32, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK33, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK34, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK35, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK36, OnUpdateNode)
	ON_BN_CLICKED(IDC_CHECK37, OnUpdateNode)
	ON_EN_KILLFOCUS(IDC_EDIT9, OnUpdateNode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPortView diagnostics

#ifdef _DEBUG
void CPortView::AssertValid() const
{
	CFormView::AssertValid();
}

void CPortView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPortView message handlers

void CPortView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// modify the style of our frame to prevent user from resizing
	GetParentFrame()->ModifyStyle(WS_THICKFRAME | WS_MAXIMIZEBOX,0);
	// resize the parent frame to the size of the dialog box
    ResizeParentToFit(FALSE);

	m_TextColor.SubclassDlgItem(IDC_TEXT_COLOR,this);

	// setup the resource list
	RECT rect;
	m_ResourceList.GetWindowRect(&rect);
	
	int columnWidth = (rect.right - rect.left) / 2;
	m_ResourceList.InsertColumn(0,"ID", LVCFMT_LEFT, columnWidth );
	m_ResourceList.InsertColumn(1,"PORT", LVCFMT_LEFT, columnWidth );

	// put data to dialog
	OnUpdate( NULL, false, NULL );
}

#pragma warning(disable:4800)

void CPortView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	NodeWindowPort * pDoc = dynamic_cast<NodeWindowPort *>( GetDocument() );
	if ( ! pDoc )
		return;

	if ( lHint )
	{
		UpdateData( true );

		pDoc->m_Flags = 0;
		pDoc->m_Flags |= m_Visible ? NodeWindow::VISIBLE : 0;
		pDoc->m_Flags |= m_NoClip ? NodeWindow::NOCLIP : 0;
		pDoc->m_Flags |= m_Docked ? NodeWindow::DOCKED : 0;
		pDoc->m_Flags |= m_DockX ? NodeWindow::DOCK_X : 0;
		pDoc->m_Flags |= m_DockY ? NodeWindow::DOCK_Y : 0;
		pDoc->m_Flags |= m_DockZ ? NodeWindow::DOCK_Z : 0;
		pDoc->m_Flags |= m_DockOutside ? NodeWindow::DOCK_OUTSIDE : 0;
		pDoc->m_Flags |= m_HCenter ? NodeWindow::HCENTER : 0;
		pDoc->m_Flags |= m_VCenter ? NodeWindow::VCENTER : 0;
		pDoc->m_Flags |= m_ParentWidth ? NodeWindow::PARENT_WIDTH : 0;
		pDoc->m_Flags |= m_ParentHeight ? NodeWindow::PARENT_HEIGHT : 0;

		pDoc->m_Style = 0;
		pDoc->m_Style |= m_HasBack ? NodeWindow::HAS_BACK : 0;
		pDoc->m_Style |= m_CanMove ? NodeWindow::CAN_MOVE : 0;
		pDoc->m_Style |= m_CanSize ? NodeWindow::CAN_SIZE : 0;
		pDoc->m_Style |= m_CanScroll ? NodeWindow::CAN_SCROLL : 0;
		pDoc->m_Style |= m_CanDock ? NodeWindow::CAN_DOCK : 0;
		pDoc->m_Style |= m_EffectFadeIn ? NodeWindow::EFFECT_FADEIN : 0;
		pDoc->m_Style |= m_EffectHighlight ? NodeWindow::EFFECT_HIGHLIGHT : 0;
		pDoc->m_Style |= m_EffectRollUV ? NodeWindow::EFFECT_ROLLUV : 0;
		pDoc->m_Style |= m_Document ? NodeWindow::DOCUMENT : 0;
		pDoc->m_Style |= m_CanFocus ? NodeWindow::CAN_FOCUS : 0;

		pDoc->m_Style |= m_SmoothUL ? NodeWindow::SMOOTH_UL : 0;
		pDoc->m_Style |= m_SmoothUR ? NodeWindow::SMOOTH_UR : 0;
		pDoc->m_Style |= m_SmoothLR ? NodeWindow::SMOOTH_LR : 0;
		pDoc->m_Style |= m_SmoothLL ? NodeWindow::SMOOTH_LL : 0;
		pDoc->m_Style |= m_AnimBeam ? NodeWindow::ANIM_BEAM : 0;
		pDoc->m_Style |= m_AnimBack ? NodeWindow::ANIM_BACK : 0;
		pDoc->m_Style |= m_AnimFlare ? NodeWindow::ANIM_FLARE : 0;
		
		pDoc->m_Style |= m_User1 ? NodeWindow::USER1 : 0;
		pDoc->m_Style |= m_User2 ? NodeWindow::USER2 : 0;
		pDoc->m_Style |= m_User3 ? NodeWindow::USER3 : 0;
		pDoc->m_Style |= m_User4 ? NodeWindow::USER4 : 0;
		pDoc->m_Style |= m_User5 ? NodeWindow::USER5 : 0;
		pDoc->m_Style |= m_User6 ? NodeWindow::USER6 : 0;
		pDoc->m_Style |= m_User7 ? NodeWindow::USER7 : 0;
		pDoc->m_Style |= m_User8 ? NodeWindow::USER8 : 0;

		pDoc->m_Alpha = m_Alpha;

		pDoc->m_Name = (const char *)m_Name;
		pDoc->m_Window.set( m_Left, m_Top, m_Right, m_Bottom );
		pDoc->m_Help = (const char *)m_Help;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
	else
	{
		m_Visible = (pDoc->m_Flags & NodeWindow::VISIBLE) ? true : false;
		m_NoClip = (pDoc->m_Flags & NodeWindow::NOCLIP) ? true : false;
		m_Docked = (pDoc->m_Flags & NodeWindow::DOCKED) ? true : false;
		m_DockX = (pDoc->m_Flags & NodeWindow::DOCK_X) ? true : false;
		m_DockY = (pDoc->m_Flags & NodeWindow::DOCK_Y) ? true : false;
		m_DockZ = (pDoc->m_Flags & NodeWindow::DOCK_Z) ? true : false;
		m_DockOutside = (pDoc->m_Flags & NodeWindow::DOCK_OUTSIDE) ? true : false;
		m_HCenter = (pDoc->m_Flags & NodeWindow::HCENTER) ? true : false;
		m_VCenter = (pDoc->m_Flags & NodeWindow::VCENTER) ? true : false;
		m_ParentWidth = (pDoc->m_Flags & NodeWindow::PARENT_WIDTH) ? true : false;
		m_ParentHeight = (pDoc->m_Flags & NodeWindow::PARENT_HEIGHT) ? true : false;

		m_HasBack = (pDoc->m_Style & NodeWindow::HAS_BACK) ? true : false;
		m_CanMove = (pDoc->m_Style & NodeWindow::CAN_MOVE) ? true : false;
		m_CanSize = (pDoc->m_Style & NodeWindow::CAN_SIZE) ? true : false;
		m_CanScroll = (pDoc->m_Style & NodeWindow::CAN_SCROLL) ? true : false;
		m_CanDock = (pDoc->m_Style & NodeWindow::CAN_DOCK) ? true : false;
		m_EffectFadeIn = (pDoc->m_Style & NodeWindow::EFFECT_FADEIN) ? true : false;
		m_EffectHighlight = (pDoc->m_Style & NodeWindow::EFFECT_HIGHLIGHT) ? true : false;
		m_EffectRollUV = (pDoc->m_Style & NodeWindow::EFFECT_ROLLUV) ? true : false;
		m_Document = (pDoc->m_Style & NodeWindow::DOCUMENT) ? true : false;
		m_CanFocus = (pDoc->m_Style & NodeWindow::CAN_FOCUS) ? true : false;

		m_SmoothUL = (pDoc->m_Style & NodeWindow::SMOOTH_UL) ? true : false;
		m_SmoothUR = (pDoc->m_Style & NodeWindow::SMOOTH_UR) ? true : false;
		m_SmoothLR = (pDoc->m_Style & NodeWindow::SMOOTH_LR) ? true : false;
		m_SmoothLL = (pDoc->m_Style & NodeWindow::SMOOTH_LL) ? true : false;
		m_AnimBeam = (pDoc->m_Style & NodeWindow::ANIM_BEAM) ? true : false;
		m_AnimBack = (pDoc->m_Style & NodeWindow::ANIM_BACK) ? true : false;
		m_AnimFlare = (pDoc->m_Style & NodeWindow::ANIM_FLARE) ? true : false;

		m_User1 = (pDoc->m_Style & NodeWindow::USER1) ? true : false;
		m_User2 = (pDoc->m_Style & NodeWindow::USER2) ? true : false;
		m_User3 = (pDoc->m_Style & NodeWindow::USER3) ? true : false;
		m_User4 = (pDoc->m_Style & NodeWindow::USER4) ? true : false;
		m_User5 = (pDoc->m_Style & NodeWindow::USER5) ? true : false;
		m_User6 = (pDoc->m_Style & NodeWindow::USER6) ? true : false;
		m_User7 = (pDoc->m_Style & NodeWindow::USER7) ? true : false;
		m_User8 = (pDoc->m_Style & NodeWindow::USER8) ? true : false;

		m_Alpha = pDoc->m_Alpha;

		m_Name = pDoc->m_Name;
		m_Help = pDoc->m_Help;
		m_Class = (const char *)Factory::className( pDoc->m_Class );
		m_Left = pDoc->m_Window.left;
		m_Top = pDoc->m_Window.top;
		m_Right = pDoc->m_Window.right;
		m_Bottom = pDoc->m_Window.bottom;
		m_Width = (m_Right - m_Left) + 1;
		m_Height = (m_Bottom - m_Top) + 1;
		m_WindowStyle = pDoc->m_WindowStyle;

		UpdateData( false );

		ASSERT( pDoc->m_ResourceId.size() == pDoc->m_ResourcePort.size() );
		m_ResourceList.DeleteAllItems();
		for(int i=0;i<pDoc->m_ResourceId.size();i++)
		{
			int listItem = m_ResourceList.InsertItem( 0, pDoc->m_ResourceId[i] );
			m_ResourceList.SetItemText( listItem, 1, pDoc->m_ResourcePort[i] );
			m_ResourceList.SetItemData( listItem, i );
		}
	}
}

void CPortView::OnUpdateNode() 
{
	OnUpdate(NULL,true,NULL);
}

void CPortView::OnBrowseClass() 
{
	NodeWindowPort * pDoc = dynamic_cast<NodeWindowPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectClass dialog( pDoc->m_Type );
	if ( dialog.DoModal() == IDOK )
	{
		pDoc->m_Class = dialog.m_Selected;

		pDoc->UpdateAllViews( NULL );
		pDoc->setUpdate();
		pDoc->updateParent();
	}
}

void CPortView::OnAddResource() 
{
	NodeWindowPort * pDoc = dynamic_cast<NodeWindowPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY( Port ) );
	if ( dialog.DoModal() == IDOK)
	{
		Path path( (const char *)dialog.m_Selected );
		String id( path.name() );
		id.upper();

		pDoc->m_ResourceId.push( id );
		pDoc->m_ResourcePort.push( path.path() );

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
}

void CPortView::OnRemoveResource() 
{
	NodeWindowPort * pDoc = dynamic_cast<NodeWindowPort *>( GetDocument() );
	ASSERT( pDoc );

	int removeItem = m_ResourceList.GetNextItem(-1,LVNI_SELECTED);
	if ( removeItem >= 0 )
	{
		int resourceIndex = m_ResourceList.GetItemData( removeItem );
		
		pDoc->m_ResourceId.remove( resourceIndex );
		pDoc->m_ResourcePort.remove( resourceIndex );

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
}

void CPortView::OnEditResource(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NodeWindowPort * pDoc = dynamic_cast<NodeWindowPort *>( GetDocument() );
	ASSERT( pDoc );

	int selectedItem = m_ResourceList.GetNextItem(-1,LVNI_SELECTED);
	if ( selectedItem >= 0 )
	{
		CSelectResource dialog( CLASS_KEY( Port ) );
		if ( dialog.DoModal() == IDOK)
		{
			int resourceIndex = m_ResourceList.GetItemData( selectedItem );
			pDoc->m_ResourcePort[ resourceIndex ] = (const char *)dialog.m_Selected;

			pDoc->UpdateAllViews(NULL);
			pDoc->setUpdate();
			pDoc->updateParent();
		}
	}

	*pResult = 0;
}

void CPortView::OnRenameResource(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	if ( pDispInfo->item.pszText != NULL)
	{
		m_ResourceList.SetItemText( pDispInfo->item.iItem, 0, pDispInfo->item.pszText );

		NodeWindowPort * pDoc = dynamic_cast<NodeWindowPort *>( GetDocument() );
		ASSERT( pDoc );

		int resourceIndex = m_ResourceList.GetItemData( pDispInfo->item.iItem );
		pDoc->m_ResourceId[ resourceIndex ] = pDispInfo->item.pszText;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}

	*pResult = 0;
}

void CPortView::OnOpenWindowStyle() 
{
	AfxGetApp()->OpenDocumentFile( m_WindowStyle );
}

void CPortView::OnBrowseWindowStyle() 
{
	NodeWindowPort * pDoc = dynamic_cast<NodeWindowPort *>( GetDocument() );
	ASSERT( pDoc );

	CSelectResource dialog( CLASS_KEY( WindowStylePort ) );
	if ( dialog.DoModal() == IDOK)
	{
		pDoc->m_WindowStyle = (const char *)dialog.m_Selected;

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
}

void CPortView::OnUpdateWidthHeight() 
{
	NodeWindowPort * pDoc = dynamic_cast<NodeWindowPort *>( GetDocument() );
	ASSERT( pDoc );

	if ( UpdateData( true ) )
	{
		m_Right = (m_Left + m_Width) - 1;
		m_Bottom = (m_Top + m_Height) - 1;
		
		// update the node
		pDoc->m_Window.set( m_Left, m_Top, m_Right, m_Bottom );

		pDoc->UpdateAllViews(NULL);
		pDoc->setUpdate();
		pDoc->updateParent();
	}
}
