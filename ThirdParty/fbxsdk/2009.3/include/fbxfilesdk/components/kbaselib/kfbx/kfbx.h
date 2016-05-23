#ifndef FBXFILESDK_COMPONENTS_KBASELIB_KFBX_KFBX_H
#define FBXFILESDK_COMPONENTS_KBASELIB_KFBX_KFBX_H

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

#include <fbxfilesdk/components/kbaselib/kbaselib_h.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <fbxfilesdk/components/kbaselib/klib/kstring.h>
#include <fbxfilesdk/components/kbaselib/klib/ktime.h>
#include <fbxfilesdk/components/kbaselib/klib/kerror.h>
#include <fbxfilesdk/components/kbaselib/klib/karrayul.h>
#include <fbxfilesdk/components/kbaselib/klib/ksystemtime.h>
#include <fbxfilesdk/components/kbaselib/klib/kcharptrset.h>

#include <fbxfilesdk/fbxfilesdk_nsbegin.h>

class KFBX_DLL KFbxXRefManager;
class KFile;
class KFbxReader;
class KFbxWriter;

#include <fbxfilesdk/fbxfilesdk_nsend.h>

#include <fbxfilesdk/components/kbaselib/kbaselib_forward.h>
#include <fbxfilesdk/fbxfilesdk_nsbegin.h>

    // Defines the current version number.
    // Please keep this version journal updated.


    #define FBX_DEFAULT_VERSION_NUMBER 6100

    #define FBX7_DEFAULT_VERSION_NUMBER 7000


    // Defines the current version number.
    // Please keep this version journal updated.

    // Version 2000
    // New Bob KFCurve and embedded FBX.; no FCurve/FCurve node storing
    // No more .takf must has been created in an earlier version, no history
    // Ben's data is saved and retrieve

    // Version 2001
    // Version incremented to support KTime save in native (integer, not double!) format.

    // Version 3000
    // FiLMBOX 3.0 version, nothing changed in current class since version 2001.
    // FBX SDK 3.0 and 3.6
    // Ben's data is retrieve only to keep compatibility

    // Version 3001
    // FiLMBOX 3.0 encrypted version, only a trial.
    // Encrypted files could only be written in debug versions.

    // Gross big fat mistake! Cannot switch to a higher version number now because any
    // file with a version number >= 3001 is read as encrypted.
    // Hence, this value now only gives file type. (3000 or less -> binary, 3001 or more -> encrypted)

    // FiLMBOX 3.2, FiLMBOX 3.5 and Online 3.51 have been released with version 3000.

    // Version 4000
    // MotionBuilder 4.0, new type in KFCurve tangents, supported in FiLMBOX 3.5 but not by earlier versions.
    // Version number is now stored in section footer.
    // Before September 3rd 2002, the version number was always 3000 in main section footer.
    // Now the main section footer has version number 4000.
    // The minimum version number in footer of an extension section is 4000.

    // Version 4001
    // ASCII Header is 4.1 !!!!
    // MotionBuilder 4.01, to fix FCurveNode problem with layertypes in version 4000
    // Now the main section footer has version number 4001.
    // Now the footer for extension sections has version number 4001.

    // Version 4050
    // ASCII Header is 4.5 !!!!
    // MotionBuilder 4.1 or 4.5 (we dont know yet) before January 22nd 2003
    // This is because EvaluationProperties now have color.
    // Now the main section footer has version number 4050.
    // Now the footer for extension sections has version number 4050.

    // Version 5000
    // ASCII Header is not compatible anymore with MotionBuilder 4.0, 4.01 and 4.02 and FBX SDK 3.6 and 3.7
    // MotionBuilder 4.03 and 4.1 or 4.5 (we dont know yet) from January 22nd 2003
    // FBX SDK 3.6.1
    // New extended header to improve FBX file version management.
    // Now the extended header and the main section footer have version number 5000.
    // Now the footer for extension sections has version number 5000.

    // ************************************************************************************************
    // ************************************************************************************************
    // ***********************                                                    *********************
    // Version 5800
    // THIS IS A TEMPORARY VERSION WHILE WAITING FOR THE VERSION 6000 which will render the previous
    // versions incompatible with MotionBuilder 6.0. For now, however, this format is needed to allow
    // some tools/plugins (ex:maya) to correctly detect that the file has some features that are not
    // totally backward compatible (ex: pivots defined with _pre/_post nodes which require a special
    // processing). By incrementing only the minor version we do not compromise the integrity of the
    // files.
    // ***********************                                                    *********************
    // ************************************************************************************************
    // ************************************************************************************************

    // Version 6000
    // Header version is now 6.0
    // Extended header now contain a creation time stamp that can be retrieve without parsing the
    // main section of the file.
    // A creator tag (string) is now stored in the Extended header. This contain the originator (MB/FBXSDK)
    // of the file and the build number of the originator.
    // First release of the file format using the KProperties to store/retrieve information.


    // Version 6100
    // Add support for multiple attributes (mainly multiple geometry) at the node level.
    // The pointer to the node attribute have been replaced by a connection between the node and its attribute(s).

    // Version 7000
    // First version of the 7.0 series; most likely very short-lived, developped for Protein, before ADP.
    // Supports reference cloning, external documents, blobs, unique IDs (per file), property templates.
    // So many changes that it was decided to break from 6.0 to keep Motion Builder intact.

    enum
    {
        FBX_NO_SECTION = -1,
        FBX_MAIN_SECTION,
        FBX_EXTENSION_SECTION_0,
        // FBX_EXTENSION_SECTION_1,
        // FBX_EXTENSION_SECTION_2,
        // ...
    };

    class KFBX_DLL KDefaultRenderResolution
    {
    public:
        bool    mIsOK;
        KString mCameraName;
        KString mResolutionMode;
        double mResolutionW;
        double mResolutionH;

        KDefaultRenderResolution();

		void Reset();
    };

    class KFBX_DLL KFbxFileHeaderInfo
    {
    public:
        KFbxFileHeaderInfo();
        virtual ~KFbxFileHeaderInfo();

		virtual void				Reset();

        // Derived classes can get funky and read more info out of the header.
        // Return false in case of failure that should stop loading the file.
        virtual bool				ReadExtendedHeaderInformation(KFbx*);

        KDefaultRenderResolution    mDefaultRenderResolution;

		//Read only properties (not used for file write)
        int                         mFileVersion;
        bool                        mCreationTimeStampPresent;
        kLocalTime                  mCreationTimeStamp;
        KString                     mCreator;				
        bool                        mPLE;
		bool						mIOPlugin;
    };

    /***********************************************************************
        CLASS KFbx

        Description: FBX physical file I/O class.
    ************************************************************************/
    class KFBX_DLL KFbx
    {

    public:
        // Exception-safe way of setting/resetting the xref manager in a KFbx object.
        struct KFbxAutoResetXRefManager
        {
            KFbx*                   mFbx;
            const KFbxXRefManager*  mXRefManager;

            KFbxAutoResetXRefManager(KFbx* pFbx, KFbxXRefManager& pXRefManager)
            : mFbx(pFbx)
            , mXRefManager(NULL)
            {
                if( mFbx )
                {
                    mXRefManager = mFbx->ProjectGetXRefManager();
                    mFbx->ProjectSetXRefManager(&pXRefManager);
                }
            }

            ~KFbxAutoResetXRefManager()
            {
                if( mFbx )
                {
                    mFbx->ProjectSetXRefManager(mXRefManager);
                }
            }
        };


        static KFbx* KFbxObjectCreate(){ return new KFbx; }

        KFbx();
        virtual ~KFbx();

        /**
        * \name Project Global
        */
        //@{

        //! Open a project already in Memory
        bool ProjectOpen(void* pAddress, kULong pMaxLength, KFbxReader* pReader, bool pCheckCRC = false, bool pOpenMainSection = true, KFbxFileHeaderInfo* pFileHeaderInfo = NULL);

        //! Open a project.
        bool ProjectOpen(const char* pName, KFbxReader* pReader, bool pCheckCRC = false, bool pOpenMainSection = true, KFbxFileHeaderInfo* pFileHeaderInfo = NULL);

        //! Open a project on the specified KFile. The KFile object must exists until closed.
        bool ProjectOpen (KFile * pFile, KFbxReader* pReader, bool pCheckCRC = false, bool pOpenMainSection = true, KFbxFileHeaderInfo* pFileHeaderInfo = NULL);

        //! Open project file without necessarily an .fbx extension.
        bool ProjectOpenDirect(const char* pName, KFbxReader* pReader, bool pCheckCRC = false, bool pOpenMainSection = true, KFbxFileHeaderInfo* pFileHeaderInfo = NULL);

        //! Create a project in Memory
        bool ProjectCreate(void* pAddress, kUInt pSize, KFbxWriter* pWriter, bool pBinary, bool pEncrypted, KFbxFileHeaderInfo* pFileHeaderInfo = NULL);

        //! Create a project.
        bool ProjectCreate(const char* pName, KFbxWriter* pWriter, bool pBinary, bool pEncrypted, KFbxFileHeaderInfo* pFileHeaderInfo = NULL);

        //! Create a project without necessary an .fbx extension.
        bool ProjectCreateDirect(const char* pName, KFbxWriter* pWriter, bool pBinary, bool pEncrypted, KFbxFileHeaderInfo* pFileHeaderInfo = NULL);

        //! Create a project, without writing out the header (yet)
        bool ProjectCreateEmpty(const char* pName, KFbxWriter* pWriter, int pVersion, bool pBinary, bool pEncrypted);

        // Write FBX signature at the top of the file, prepare file for writing header information
        bool ProjectWrite_BeginFileHeader();

        // Open up the 'extended header'
        bool ProjectWrite_BeginExtendedHeader();

        // Write the contents of the extended header
        bool ProjectWrite_WriteExtendedHeader(const KFbxFileHeaderInfo* pExtendedHeader);

        // Close the extended header
        bool ProjectWrite_EndExtendedHeader();

        // Close up the header, prepare file for payload write.
        bool ProjectWrite_EndFileHeader();

        //! Close the project.
        bool ProjectClose(void** pData=0,kULong *pSize=0);

        /**
         * Provides the XRef Manager to use to create the .fbm folder. 
         * If NULL is used, the old behavior (using the .fbx's folder) 
         * is used instead. 
         * 
         */
        void ProjectSetXRefManager(const KFbxXRefManager*);

        /**
         * Returns NULL if no XRef manager has been set.
         */
        const KFbxXRefManager* ProjectGetXRefManager() const;

        /** Select (and create) a folder to store embedded files (the .fbm 
        *   file folder).  Takes into account the settings from the XRef Manager.
        *  
        *   If this already been called successfully, uses the path
        *   previously created.
        *  
        *   Client application is responsible for cleaning up this folder.
        *  
        *   This will be automatically called if ProjectSetXRefManager()
        *   has been called before the .fbm folder needs to be created. 
        */ 
        bool ProjectCreateEmbeddedFolder(const KFbxXRefManager& pXRefManager, KString& pCreatedFolder); 

        //! On store event, use this function to tell if we are embedding.
        void SetEmbedded(bool pValue);

        //! Return if file is embedded or not.
        bool IsEmbedded() const;

        //! Return if file is binary or ASCII
        bool IsBinary() const;

        //! Return if binary file is encrypted
        bool IsEncrypted () const;

        bool Fbx7Support() const;
        void Fbx7Support(bool pSupport);

        // This is only used for FBX7 files; should we compress arrays if possible?
        // The writer may decide not to compress all arrays, or it may even decide
        // not to compress anyway.  Flags are written in the file to help the reader
        // out, on a per-array basis.
        bool CompressArrays() const;
        void CompressArrays(bool pCompress);

        int  CompressMinimumSize() const;
        void CompressMinimumSize(int pSize);

        int  CompressLevel() const;
        void CompressLevel(int pLevel); // Range is: [0-9], from no compression to as-much-as-we-can

        /** Check CRC code. File must be open, binary and encrypted.
        *   \return \c true if CRC code is valid or file is not open, binary and encrypted.
        */
        bool CheckCRC();

        //! Return the file version number
        kUInt32 GetFileVersionNumber() const; 

        //@}

        /**
        * \name Project Section
        */
        //@{

        //! Open the main section of a project.
        bool ProjectOpenMainSection();

        //! Get the number of extension sections of a project.
        int ProjectGetExtensionSectionCount();

        //! Open an extension section of a project.
        bool ProjectOpenExtensionSection(int pExtensionSectionIndex);

        //! Create an extension section in a project, not allowed in ASCII and encrypted modes.
        bool ProjectCreateExtensionSection(bool pOverwriteLastExtensionSection = false);

        //! Close current section.
        void ProjectCloseSection();

        //! Remove last extension section.
        bool ProjectRemoveLastExtensionSection();

        //! Get current section.
        int ProjectGetCurrentSection();

        //! Get current section mode.
        int ProjectGetCurrentSectionMode();

        //! Get current section version.
        int ProjectGetCurrentSectionVersion();

        /** Get the version number of a section.
        * \remarks For main section it can be either 1000, 2000, 2001, 3000, 3001, 4000, 4001 or 4050.
        * \remarks For the extension section it can be either 4000, 4001 or 4050.
        * \remarks Returns 0 if section number does not exist.
        */
        int ProjectGetSectionVersion(int pSection);

        /** Split a version number into major, minor and revision numbers.
        * /param pVersion Version number to split.
        * /param pMajor Integer to receive major version number.
        * /param pMinor Integer to receive minor version number.
        * /param pMinor Integer to receive revision version number.
        */
        static void ProjectConvertVersionNumber(int pVersion, int& pMajor, int& pMinor, int& pRevision);

        /** Return the password protection flag.
        * \return \c true if the current section has a password, \c false otherwise.
        */
        bool IsPasswordProtected() const;

        /** Set password protection flag to \c false if the argument matches the password stored in the section.
        * \return \c true if the argument matches the password stored in the section, \c false otherwise.
        * \remarks This function only works in read mode.
        */
        bool CheckPassword(const char* pPassword);

        /** Encrypt and store password in a section.
        * \return \c true on success, \c false otherwise.
        * \remarks This function only works in write mode and out of any enclosing block.
        * \remarks This function must not be called more than once per section.
        */
        bool WritePassword(const char* pPassword);

        //@}

        /**
        * \name Directory
        */
        //@{

        //! Get project file name.
        const char* GetFilename() const;

        //! Get project data directory name.
        KString GetDataDirectory(bool pAutoCreate = true);

        /** Returns the current embedded folder used by this object. If
         *  ProjectCreateEmbeddedFolder has never been called this will
         *  return an emptry string, unless we're explicitely asked to
         *  create it.
         */
        KString GetMediaDirectory(bool pCreate = false);

        //! Get the path relative to project directory.
        char* GetRelativePath(const char* pPath);

        //! Get the file path relative to project directory.
        char* GetRelativeFilePath(const char* pFilePath);

        //! Get the full path of path relative to project directory.
        char* GetFullPath(const char* pRelativePath);

        //! Get the full file path of path relative to project directory.
        char* GetFullFilePath(const char* pRelativeFilePath);

        //! Get the temporary project name.
        char* GetTmpProjectName(const char* pName);

        //! Swap from temporary project.
        bool SwapFromTmpProject(const char* pName, char* pError=NULL);

        //@}

        /**
        * \name Read
        */
        //@{

        //! Reset the field read position.
        void FieldReadResetPosition();

        //! Return the number of fields.
        int FieldGetCount();

        //! Get the name of field indexed pFieldIndex
        const char* FieldGetName(int pFieldIndex);

        //! Get number of instance field pFieldName has.
        int FieldGetInstanceCount(const char* pFieldName);

        //! Start to read field instance refered by field indexed pFieldIndex, instance indexed pInstance.
        bool FieldReadBegin(int pFieldIndex, int pInstance);

        //! Start to read field pFieldName.
        bool FieldReadBegin(const char* pFieldName);

        //! Start to read field instance refered field pFieldName, instance indexed pInstance.
        bool FieldReadBegin(const char* pFieldName, int pInstance);

        //! Stop to read the current field.
        void FieldReadEnd();

        //! Return if current field is a block.
        bool FieldReadIsBlock();

        //! Start to read a field block.
        bool FieldReadBlockBegin();

        //! Stop to read a field block.
        void FieldReadBlockEnd();

        //! Return the number of read field.
        int FieldReadGetCount();

        //! Return the number of field remaining to be read.
        int FieldReadGetRemain();

        //! Return current field value type.
        char FieldReadGetType();

        //! Return current field value as a char.
        char FieldReadCH();

        //! Return field pFieldName's value as a char.
        char FieldReadCH(const char* pFieldName, char pDefault=0);

        //! Return current field value as a char pointer.
        const char* FieldReadC();

        //! Return field pFieldName's value as a char pointer.
        const char* FieldReadC(const char* pFieldName, const char* pDefault="");

        //! Return current field value as a string (a char pointer).
        const char* FieldReadS();

        //! Return field pFieldName's value as a char pointer.
        const char* FieldReadS(const char* pFieldName, const char* pDefault="");

        //! Return current field value as an bool.
        bool FieldReadB();

        //! Return field pFieldName's value as an integer.
        bool FieldReadB(const char* pFieldName, bool pDefault = false);

        //! Return current field value as an integer.
        int FieldReadI();

        //! Return field pFieldName's value as an integer.
        int FieldReadI(const char* pFieldName, int pDefault=0);

        //! Return current field value as an integer.
        kLongLong FieldReadLL();

        //! Return field pFieldName's value as an integer.
        kLongLong FieldReadLL(const char* pFieldName, kLongLong pDefault=0);

        //! Return current field value as a float.
        float FieldReadF();

        //! Return field pFieldName's value as a float.
        float FieldReadF(const char* pFieldName, float pDefault=0);

        //! Return current field value as a double.
        double FieldReadD();

        //! Return field pFieldName's value as a double.
        double FieldReadD(const char* pFieldName, double pDefault=0);

        //! Return field pFieldName's value as a time value.
        KTime FieldReadT(const char* pFieldName);

        //! Return field pFieldName's value as a time value.
        KTime FieldReadT();

        //! Return field pFieldName's value as a timespan value.
        KTimeSpan FieldReadTS(const char* pFieldName);

        //! Return field pFieldName's value as a timespan value.
        KTimeSpan FieldReadTS();

        //! Return current field value as a n floats array.
        void FieldReadFn(float* pValue, kUInt pn);

        //! Return current field value as a 3 floats array.
        void FieldRead3F(float* pValue);

        //! Return current field value as a 4 floats array.
        void FieldRead4F(float* pValue);

        //! Return field pFieldName's value as n floats array.
        void FieldReadFn(const char* pFieldName, float* pValue, const float *pDefault, kUInt pn);

        //! Return field pFieldName's value as 4 floats array.
        void FieldRead3F(const char* pFieldName, float* pValue, const float* pDefault=NULL);

        //! Return field pFieldName's value as 3 floats array.
        void FieldRead4F(const char* pFieldName, float* pValue, const float* pDefault=NULL);

        //! Return current field value as a n doubles array.
        void FieldReadDn(double* pValue, kUInt pn);

        //! Return current field value as a 3 doubles array.
        void FieldRead3D(double* pValue);

        //! Return current field value as a 4 doubles array.
        void FieldRead4D(double* pValue);

        //! Return field pFieldName's value as n doubles array.
        void FieldReadDn(const char* pFieldName, double* pValue, const double *pDefault, kUInt pn);

        //! Return field pFieldName's value as 4 doubles array.
        void FieldRead3D(const char* pFieldName, double* pValue, const double* pDefault=NULL);

        //! Return field pFieldName's value as 3 doubles array.
        void FieldRead4D(const char* pFieldName, double* pValue, const double* pDefault=NULL);

        //! Return current field value as raw data.
        void* FieldReadR(int* pByteSize);

        //! Return field pFieldName's value as raw data.
        void* FieldReadR(const char* pFieldName,int* pByteSize);

        /** Read field and copy it into a file.
        * \remarks Only works when file is binary. This function is not related to flag mEmbedded.
        * \return \c false if operation failed.
        */
        virtual bool FieldReadEmbeddedFile (KString& pFileName, KString& pRelativeFileName, const char* pEmbeddedMediaDirectory = "", bool *pIsFileCreated=NULL);

        const double*   FieldReadArrayD( int &pCount );
        const float*    FieldReadArrayF( int &pCount );
        const int*      FieldReadArrayI( int &pCount );
        const kLongLong*FieldReadArrayLL(int &pCount );
        const bool*     FieldReadArrayB( int &pCount );
        const kUByte*   FieldReadArrayBytes( int &pCount );

        const int*    FieldReadArray(int& pCount, const int*);
        const float*  FieldReadArray(int& pCount, const float*);
        const double* FieldReadArray(int& pCount, const double*);
        const kLongLong* FieldReadArray(int& pCount, const kLongLong*);
        const bool* FieldReadArray(int& pCount, const bool*);
        const kUByte* FieldReadArray(int& pCount, const kUByte*);

        //@}

        /**
        * \name Write
        */
        //@{

        //! Start to write a field called pFieldName.
        void FieldWriteBegin(const char* pFieldName);

        //! Stop to write the current field.
        void FieldWriteEnd();

        //! Start to write a field block.
        void FieldWriteBlockBegin();

        //! Start to write an object reference field.
        void FieldWriteObjectBegin(const char* pObjectType, const char* pName, const char* pSubType=NULL);

        //! Stop to write an object reference field.
        void FieldWriteObjectEnd();

        /** Start to write a field block in file pFileName.
        * This function is disabled but kept accessible for the FB SDK.
        */
        void FieldWriteBlockBegin(const char* pFileName);

        //! Stop to write a block of field.
        void FieldWriteBlockEnd ();

        //! Write field value as a char.
        void FieldWriteCH(char pValue);

        //! Write field pFieldName field with a char as a value.
        void FieldWriteCH(const char* pFieldName, char pValue);

        //! Write field value as char pointer pValue.
        void FieldWriteC(const char* pValue);

        //! Write field pFieldName with a char pointer as a value.
        void FieldWriteC(const char* pFieldName, const char* pValue);

        //! Write field value as KString pValue.
        void FieldWriteS(const char* pValue);
        void FieldWriteS(const KString& pValue);

        //! Write field pFieldName field with a KString as a value.
        void FieldWriteS(const char* pFieldName, const char* pValue);
        void FieldWriteS(const char* pFieldName, const KString& pValue);

        //! Write field value as bool.
        void FieldWriteB(bool pValue);

        //! Write field pFieldName field with a bool value.
        void FieldWriteB(const char* pFieldName, bool pValue);

        //! Write field value as integer.
        void FieldWriteI(int pValue);

        //! Write field pFieldName field with an int as a value.
        void FieldWriteI(const char* pFieldName, int pValue);

        //! Write field value as 64 bit integer.
        void FieldWriteLL(kLongLong pValue);

        //! Write field pFieldName field with an 64 bit int as a value.
        void FieldWriteLL(const char* pFieldName, kLongLong pValue);

        /** Write field value as float.
        * \remarks Only compatible with 1) MotionBuilder 4.0 and later 2) FBX SDK 3.6.1 and later.
        */
        void FieldWriteF(float pValue);

        /** Write field pFieldName field with a float as a value.
        * \remarks Only compatible with 1) MotionBuilder 4.0 and later 2) FBX SDK 3.6.1 and later.
        */
        void FieldWriteF(const char* pFieldName, float pValue);

        //! Write field value as double.
        void FieldWriteD(double  pValue);

        //! Write field pFieldName field with a double as a value.
        void FieldWriteD(const char* pFieldName, double pValue);

        //! Write field value as time value.
        void FieldWriteT(KTime pTime);

        //! Write field pFieldName field with a time as a value.
        void FieldWriteT(const char* pFieldName,KTime pValue);

        //! Write field value as timespan value.
        void FieldWriteTS(KTimeSpan pTimeSpan);

        //! Write field pFieldName field with a timespan as a value.
        void FieldWriteTS(const char* pFieldName,KTimeSpan pValue);

        //! Write field value as an array of n floats (nF vector).
        void FieldWriteFn(const float* pValue, kUInt pn);

        //! Write field pFieldName field with a array of n floats as a value.
        void FieldWriteFn(const char* pFieldName, const float* pValue, kUInt pn);

        //! Write field value as an array of 3 floats (3F vector).
        void FieldWrite3F(const float* pValue);

        //! Write field pFieldName field with a array of 3 floats as a value.
        void FieldWrite3F(const char* pFieldName, const float* pValue);

        //! Write field value as an array of 4 floats (4F vector).
        void FieldWrite4F(const float* pValue);

        //! Write field pFieldName field with a array of 4 floats as a value.
        void FieldWrite4F(const char* pFieldName, const float* pValue);

        //! Write field value as an array of n doubles (nD vector).
        void FieldWriteDn(const double* pValue, kUInt pn);

        //! Write field pFieldName field with a array of n doubles as a value.
        void FieldWriteDn(const char* pFieldName, const double* pValue, kUInt pn);

        //! Write field value as an array of 3 doubles (3D vector).
        void FieldWrite3D(const double* pValue);

        //! Write field pFieldName field with a array of 3 doubles as a value.
        void FieldWrite3D(const char* pFieldName, const double* pValue);

        //! Write field value as an array of 4 doubles (4D vector).
        void FieldWrite4D(const double* pValue);

        //! Write field pFieldName field with a array of 4 doubles as a value.
        void FieldWrite4D(const char* pFieldName, const double* pValue);

        // The maximum number of value entries is, in theory, 2**32.  In practice it should be a lot less than that.
        // pSize is the number of values to write from each pointer location, and stride is how much we 
        // advance to get to the next value; if the stride is zero, values are tighly packed together.
        // So in total we'll write n * pSize items.
        void FieldWriteArrayD( int n, const double*     pValue, int pSize = 1, int pStride = 0 );
        void FieldWriteArrayF( int n, const float*      pValue, int pSize = 1, int pStride = 0 );
        void FieldWriteArrayI( int n, const int*        pValue, int pSize = 1, int pStride = 0 );
        void FieldWriteArrayLL(int n, const kLongLong*  pValue, int pSize = 1, int pStride = 0 );
        void FieldWriteArrayB( int n, const bool*       pValue, int pSize = 1, int pStride = 0 );
        void FieldWriteArrayBytes( int n, const kUByte* pValue, int pSize = 1, int pStride = 0 );

        /** Write field value as a raw data.
        */
        void FieldWriteR(const void* pRawData, int pByteSize);

        /** Write field pFieldName field with raw data as a value.
        */
        void FieldWriteR(const char* pFieldName, const void* pRawData, int pByteSize);

        // ASCII files may limit how big you can write your raw data, forcing you to break it down into
        // chunks.
        int GetFieldRMaxChunkSize() const;

        //! Write object reference pName in the current field.
        void FieldWriteObjectReference(const char* pName);

        //! Write object reference pName in field pFieldName.
        void FieldWriteObjectReference(const char* pFieldName, const char* pName);

        /** Write field with file content as a value.
        * \remarks Only works when file is binary. This function is not related to flag mEmbedded.
        * \return \c false if operation failed.
        */
        bool FieldWriteEmbeddedFile (KString pFileName, KString pRelativeFileName);

        //! Write comments, only effective in ASCII mode.
        void WriteComments(const char* pFieldName);

        //@}

        /**
        * \name Error
        */
        //@{

        //! Error code
        enum EError
        {
            eFileCorrupted,
            eFileVersionNotSupportedYet,
            eFileVersionNotSupportedAnymore,
            eFileNotOpened,
            eFileNotCreated,
            eDirectoryNotCreated,
            eCrcCheckFailed,
            eNonExistingSection,
            eOperationCanceled,
            eCountError,
            eInvalidData,
            eWriteError,
            eReadError,
            eErrorCount
        };

        /** Retrieve error object.
        *   return error object.
        */
        KError& GetError();

        /** Get last error code.
        *   return last error code.
        */
        EError GetLastErrorID() const;

        /** Get last error code.
        *   return last error code.
        */
        const char* GetLastErrorString() const;

        //@}

    #ifdef _DEBUG
        // Dump function for debugging purpose only
        void StdoutDump();
    #endif

        bool GetHaveLoadedEmbededFile() const;

        void* GetMemoryFileInfo(kULong& pSize);

    public:
        bool    IsBeforeVersion6() const;
        void    SetIsBeforeVersion6(bool pIsBeforeVersion6);

    private:
        struct KFbxImpl;
        KFbxImpl* mImpl;

        //! Project Global

        void ProjectClear();
        void ProjectReset();

        bool ProjectReadHeader(bool pCheckASCIIHeader, bool pCheckCRC, bool pOpenMainSection, KFbxFileHeaderInfo* pFileHeaderInfo);
        bool ProjectReadExtendedHeader(int& pExtendedHeaderEnd, KFbxFileHeaderInfo* pFileHeaderInfo);
        bool BinaryReadHeader();
        bool BinaryReadSectionPosition();
        bool ASCIIReadHeader();
        bool ASCIIReadSectionPosition();

        bool ProjectWriteHeader(KFbxFileHeaderInfo* pFileHeaderInfo);
        bool ProjectWriteExtendedHeader(KFbxFileHeaderInfo* pFileHeaderInfo);
        void BinaryWriteHeader();
        void ASCIIWriteHeader();

        void ReadEncryptionKey(char* pEncryptionKey);
        void WriteEncryptionKey(char* pEncryptionKey);

        //! Project Section

        bool ProjectClearSection();
        bool ProjectOpenSection(int pSection);
        bool BinaryReadSectionHeader();
        int BinaryReadSectionFooter(char* pSourceCheck);
        bool BinaryReadExtensionCode(kUInt32 pFollowingSectionStart, kUInt32& pSectionStart, kUInt32& pSectionVersion);
        void BinaryReadSectionPassword();

        bool ProjectWriteSectionHeader();
        void BinaryWriteSectionFooter();
        bool BinaryWriteExtensionCode(kUInt32 pSectionStart, kUInt32 pSectionVersion);

        KString GetCreationTime();
        void SetCreationTime(KString pCreationTime);
        void CreateSourceCheck(char* lSourceCheck);
        bool TestSourceCheck(char* pSourceCheck, char* pSourceCompany);
        KString GetMangledCreationTime();
        void EncryptSourceCheck(char* pSourceCheck, char* pEncryptionData);
        void DecryptSourceCheck(char* pSourceCheck, const char* pEncryptionData);

        void EncryptPasswordV1(KString pOriginalPassword, KString &pEncryptedPassword);
        void DecryptPasswordV1(KString pEncryptedPassword, KString &pDecryptedPassword);

        //! Read

        void CheckValidityOfFieldName(const char* pFieldName);
        void GetUnusedEmbeddedName(const KString& pDirectory, const KString& pName, KString& pResult, bool pCreateSubdirectory);

        //! Get project media directory name
        KString GetDirectory(bool pAutoCreate, const char* pExtension);
    };

#include <fbxfilesdk/fbxfilesdk_nsend.h>

#endif // FBXFILESDK_COMPONENTS_KBASELIB_KFBX_KFBX_H

