/**************************************************************************************

Copyright (C) 2008 Autodesk, Inc. and/or its licensors.
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

#ifndef _KFBX_RENAMING_STRATEGY_BASE_H_
#define _KFBX_RENAMING_STRATEGY_BASE_H_

#include <fbxfilesdk/kfbxplugins/kfbxscene.h>
#include <fbxfilesdk/components/kbaselib/klib/kname.h>

#include <fbxfilesdk/fbxfilesdk_nsbegin.h>
//Base class for renaming strategy.
class KFBX_DLL KFbxRenamingStrategyBase
{
public:

    //default constructor
    KFbxRenamingStrategyBase();

    //constructor
    KFbxRenamingStrategyBase(char pNameSpaceSymbol);

    //destructor
    virtual ~KFbxRenamingStrategyBase();

    //This put all the names in the scene back to the original values
    //Returns true if some names have been modified.
    virtual bool DecodeScene(KFbxScene* pScene)=0;

    //This renames all the names in the scene
    //Returns true if some names have been modified.
    virtual bool EncodeScene(KFbxScene* pScene)=0;

    //This find the original name of a given string
    //Returns true if the name has been modified.
    virtual bool DecodeString(KName& pString)=0;

    //This find the renaming name of a given string
    //Returns true if the name has been modified.
    virtual bool EncodeString(KName& pString)=0;

    virtual void CleanUp();

    struct NameCell
    {
        NameCell(char const* pName) :
    mName(pName),
        mInstanceCount(0)
    {
    }

    KString mName;
    int mInstanceCount;		
    };    

    char                     mNamespaceSymbol;
    KCharPtrSet					mStringNameArray;

};

#include <fbxfilesdk/fbxfilesdk_nsend.h>

#endif
