#if !defined(AFX_PORTWIZARDAW_H__BFA4A02A_7D92_11D2_B451_00C04F6FF8BD__INCLUDED_)
#define AFX_PORTWIZARDAW_H__BFA4A02A_7D92_11D2_B451_00C04F6FF8BD__INCLUDED_

// PortWizardaw.h : header file
//

class CDialogChooser;

// All function calls made by mfcapwz.dll to this custom AppWizard (except for
//  GetCustomAppWizClass-- see PortWizard.cpp) are through this class.  You may
//  choose to override more of the CCustomAppWiz virtual functions here to
//  further specialize the behavior of this custom AppWizard.
class CPortWizardAppWiz : public CCustomAppWiz
{
public:
	virtual CAppWizStepDlg* Next(CAppWizStepDlg* pDlg);
		
	virtual void InitCustomAppWiz();
	virtual void ExitCustomAppWiz();
	virtual void CustomizeProject(IBuildProject* pProject);
};

// This declares the one instance of the CPortWizardAppWiz class.  You can access
//  m_Dictionary and any other public members of this class through the
//  global PortWizardaw.  (Its definition is in PortWizardaw.cpp.)
extern CPortWizardAppWiz PortWizardaw;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTWIZARDAW_H__BFA4A02A_7D92_11D2_B451_00C04F6FF8BD__INCLUDED_)
