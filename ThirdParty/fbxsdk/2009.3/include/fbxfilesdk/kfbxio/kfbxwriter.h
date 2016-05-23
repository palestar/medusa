/*!  \file kfbxwriter.h
 */

#ifndef FBXFILESDK_KFBXIO_KFBXWRITER_H
#define FBXFILESDK_KFBXIO_KFBXWRITER_H

/**************************************************************************************

 Copyright © 2001 - 2008 Autodesk, Inc. and/or its licensors.
 All Rights Reserved.

 The coded instructions, statements, computer programs, and/or related material 
 (collectively the "Data") in these files contain unpublished information 
 proprietary to Autodesk, Inc. and/or its licensors, which is protected by 
 Canada and United States of America federal copyright law and by international 
 treaties. 
 
 The Data may not be disclosed or distributed to third parties, in whole or in
 part, without the prior written consent of Autodesk, Inc. ("Autodesk").

 THE DATA IS PROVIDED "AS IS" AND WITHOUT WARRANTY.
 ALL WARRANTIES ARE EXPRESSLY EXCLUDED AND DISCLAIMED. AUTODESK MAKES NO
 WARRANTY OF ANY KIND WITH RESPECT TO THE DATA, EXPRESS, IMPLIED OR ARISING
 BY CUSTOM OR TRADE USAGE, AND DISCLAIMS ANY IMPLIED WARRANTIES OF TITLE, 
 NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE OR USE. 
 WITHOUT LIMITING THE FOREGOING, AUTODESK DOES NOT WARRANT THAT THE OPERATION
 OF THE DATA WILL BE UNINTERRUPTED OR ERROR FREE. 
 
 IN NO EVENT SHALL AUTODESK, ITS AFFILIATES, PARENT COMPANIES, LICENSORS
 OR SUPPLIERS ("AUTODESK GROUP") BE LIABLE FOR ANY LOSSES, DAMAGES OR EXPENSES
 OF ANY KIND (INCLUDING WITHOUT LIMITATION PUNITIVE OR MULTIPLE DAMAGES OR OTHER
 SPECIAL, DIRECT, INDIRECT, EXEMPLARY, INCIDENTAL, LOSS OF PROFITS, REVENUE
 OR DATA, COST OF COVER OR CONSEQUENTIAL LOSSES OR DAMAGES OF ANY KIND),
 HOWEVER CAUSED, AND REGARDLESS OF THE THEORY OF LIABILITY, WHETHER DERIVED
 FROM CONTRACT, TORT (INCLUDING, BUT NOT LIMITED TO, NEGLIGENCE), OR OTHERWISE,
 ARISING OUT OF OR RELATING TO THE DATA OR ITS USE OR ANY OTHER PERFORMANCE,
 WHETHER OR NOT AUTODESK HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS
 OR DAMAGE. 

**************************************************************************************/

#include <fbxfilesdk/components/kbaselib/kaydaradef_h.h>
#include <fbxfilesdk/components/kbaselib/klib/kerror.h>
#include <fbxfilesdk/components/kbaselib/klib/kstring.h>
#include <fbxfilesdk/components/kbaselib/klib/karrayul.h>

#include <fbxfilesdk/components/kbaselib/kbaselib_forward.h>
#include <fbxfilesdk/kfbxplugins/kfbxrenamingstrategy.h>

#include <fbxfilesdk/fbxfilesdk_nsbegin.h>

class KFbxNode;
class KFbxDocument;
class KFbxScene;
class KFbxSdkManager;
class KFbxExporter;
class KFbxStreamOptions;
class KFbxIOSettings;
class KFbxObject;

class KFBX_DLL KFbxWriter
{
public:
    KFbxWriter(KFbxSdkManager& pManager, int pID);
    virtual ~KFbxWriter();

    //! Error codes
    typedef enum
    {
        eFILE_CORRUPTED,
        eFILE_NOT_OPENED,
        eFILE_NOT_CREATED,
        eOUT_OF_DISK_SPACE,
        eSTREAM_OPTIONS_NOT_SET,
        eINVALID_DOCUMENT_HANDLE,
        eDOCUMENT_NOT_SUPPORTED,
        eUNIDENTIFIED_ERROR,
        eEMBEDDED_OUT_OF_SPACE,
        eERROR_COUNT
    } EError;

    enum KInfoRequest {
        eInfoExtension, // return a null terminated char const* const*
        eInfoDescriptions, // return a null terminated char const* const*
        eInfoVersions, // return a null terminated char const* const*
        eReserved1 = 0xFBFB,
    };

    typedef KFbxWriter*				(*CreateFuncType)(KFbxSdkManager& pManager, KFbxExporter& pExporter, int pSubID, int pPluginID);
    typedef void					(*IOSettingsFillerFuncType)(KFbxIOSettings& pIOS);
    typedef void*					(*GetInfoFuncType)(KInfoRequest pRequest, int pWriterTypeId);

    virtual bool					FileCreate(char* pFileName) = 0;
    virtual bool					FileClose() = 0;
    virtual bool					IsFileOpen() = 0;

    virtual KFbxStreamOptions*		GetWriteOptions() = 0;
    virtual bool					Write(KFbxDocument* pDocument, KFbxStreamOptions* pStreamOptions) = 0;

    virtual bool					PreprocessScene(KFbxScene &pScene) = 0;
    virtual bool					PostprocessScene(KFbxScene &pScene) = 0;
    virtual KFbxNode*				FindRootNode(KFbxScene& pScene);
    virtual bool					CheckSpaceInNodeNameRecursive(KFbxNode* pNode, KString& pNodeNameList);

	virtual void					PluginWriteSettings(KFbxObject& pSettings){}

    bool							SetFileExportVersion(KString pVersion);
    void							SetRenamingMode(KFbxSceneRenamer::ERenamingMode pRenamingMode){mRenamingMode = pRenamingMode;}
    inline void						SetResamplingRate(double pResamplingRate){mResamplingRate = pResamplingRate;}
	bool							IsGenuine();

    KError&							GetError();
    EError							GetLastErrorID() const;
    const char*						GetLastErrorString() const;
    void							GetMessage(KString &pMessage) const;
    KString&						GetMessage();
    void							ClearMessage();

protected:
	void							PluginsWriteBegin(KFbxScene& pScene);
	void							PluginsWrite(KFbx& pFbx);
	void							PluginsWriteEnd(KFbxScene& pScene);

    KFbxWriter&						operator=(KFbxWriter const&) { return *this; }

    KFbxSdkManager& 				mManager;
    KString							mFileVersion;
    double							mResamplingRate;
    KFbxSceneRenamer::ERenamingMode	mRenamingMode;

private:
    KError							mError;
    KString         				mMessage;
	int								mInternalID;
};

#include <fbxfilesdk/fbxfilesdk_nsend.h>

#endif // FBXFILESDK_KFBXIO_KFBXWRITER_H

