#if !defined(AFX_NODEPORTWIZARDAW_H__D8D093B0_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
#define AFX_NODEPORTWIZARDAW_H__D8D093B0_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_

// NodePortWizardaw.h : header file
//

class CDialogChooser;

// All function calls made by mfcapwz.dll to this custom AppWizard (except for
//  GetCustomAppWizClass-- see NodePortWizard.cpp) are through this class.  You may
//  choose to override more of the CCustomAppWiz virtual functions here to
//  further specialize the behavior of this custom AppWizard.
class CNodePortWizardAppWiz : public CCustomAppWiz
{
public:
	virtual CAppWizStepDlg* Next(CAppWizStepDlg* pDlg);
		
	virtual void InitCustomAppWiz();
	virtual void ExitCustomAppWiz();
	virtual void CustomizeProject(IBuildProject* pProject);
};

// This declares the one instance of the CNodePortWizardAppWiz class.  You can access
//  m_Dictionary and any other public members of this class through the
//  global NodePortWizardaw.  (Its definition is in NodePortWizardaw.cpp.)
extern CNodePortWizardAppWiz NodePortWizardaw;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEPORTWIZARDAW_H__D8D093B0_A0AD_11D2_A4ED_00C04F6FF8CE__INCLUDED_)
