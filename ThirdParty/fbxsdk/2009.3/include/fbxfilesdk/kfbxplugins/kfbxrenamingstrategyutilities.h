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
#ifndef _KFBX_RENAMING_STRATEGY_UTILITIES_H_
#define _KFBX_RENAMING_STRATEGY_UTILITIES_H_

#include <fbxfilesdk/components/kbaselib/klib/kstring.h>
#include <fbxfilesdk/kfbxplugins/kfbxscene.h>
#include <fbxfilesdk/fbxfilesdk_nsbegin.h>
#define NAMECLASH1_KEY      "_ncl1_" // name (x)
#define NAMECLASH2_KEY		"_ncl2_" // Upper/lower cases clash

#define UPPERTOLOWER_KEY	"ul"
#define LOWERTOUPPER_KEY	"lu"


class KFBX_DLL KFbxRenamingStrategyUtilities
{
public:

    //Check if the string has non alphanumeric values
    //if so, change that character by: FBXASC### where ### is the ascii value
    //There's a boolean that says if the first char must be alpha only.
    static bool EncodeNonAlpha(KString &pString, bool pFirstCharMustBeAlphaOnly=false, KString pPermittedChars="");

    //Take a string that has been encoded by EncodeNonAlpha and reextract the non
    //alphanumeric values.
    static bool DecodeNonAlpha(KString &pString);

    //ALWAYS call Encode Duplicate BEFORE Encode Case Insensitive.


    //This will will add the _ncl1_ with the provided pInstanceNumber to the string
    //Will always return true
    static bool EncodeDuplicate(KString &pString, int pInstanceNumber=0);

    //This will remove the _ncl1_xxx to the string
    //Will return true if the pstring has been modified
    static bool DecodeDuplicate(KString &pString);

    //This will compare string1 and string2 must be identical except for casing.
    //This will compare string 1 and string 2, set string 1 to string2 and append the ncl2 suffix to it
    //will return true if pString has been modified
    static bool EncodeCaseInsensitive(KString &pString, const KString pString2);

    //This will decode a string that has a ncl2 to it
    //Will return true if pString has been modified
    static bool DecodeCaseInsensitive(KString &pString);

};


#include <fbxfilesdk/fbxfilesdk_nsend.h>
#endif
