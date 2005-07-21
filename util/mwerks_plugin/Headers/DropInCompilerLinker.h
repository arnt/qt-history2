/*
 *  DropInCompilerLinker.h - Drop-In Interface for Metrowerks CodeWarrior Compilers and Linkers
 *
 *  Copyright (c) 1996 Metrowerks, Inc.  All rights reserved.
 *
 */

#ifndef __DROPINCOMPILERLINKER_H__
#define __DROPINCOMPILERLINKER_H__

#ifdef __MWERKS__
#	pragma once
#endif

#ifndef __CWPlugins_H__
#include "CWPlugins.h"
#endif

#ifndef __COMPILERMAPPING_H__
#include "CompilerMapping.h"
#endif

#ifdef __MWERKS__
#pragma options align=mac68k
#endif

#ifdef	_MSC_VER
#pragma pack(push,2)
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#if CW_USE_PRAGMA_IMPORT
#pragma import on
#endif


/* this is the current version number of the API documented herein */
#define DROPINCOMPILERLINKERAPIVERSION_4		4
#define DROPINCOMPILERLINKERAPIVERSION_5		5
#define DROPINCOMPILERLINKERAPIVERSION_6		6
#define DROPINCOMPILERLINKERAPIVERSION_7		7
#define DROPINCOMPILERLINKERAPIVERSION_8		8
#define DROPINCOMPILERLINKERAPIVERSION_9		9
#define DROPINCOMPILERLINKERAPIVERSION_10		10
#define DROPINCOMPILERLINKERAPIVERSION_11		11
#define DROPINCOMPILERLINKERAPIVERSION_12		12
#define DROPINCOMPILERLINKERAPIVERSION_13		13
#define DROPINCOMPILERLINKERAPIVERSION			DROPINCOMPILERLINKERAPIVERSION_13


/* deprecated request codes */
enum {
	reqInitCompiler = reqInitialize,	/* (called when compiler is loaded)					*/
	reqTermCompiler = reqTerminate,		/* (called when compiler is unloaded)				*/
	reqInitLinker = reqInitialize,		/* (called when linker is loaded)					*/
	reqTermLinker = reqTerminate		/* (called when linker is unloaded)					*/
};

/* request codes (common) */
enum {
	reqTargetCompileEnded      = -17,      /* Sent when target's compile phase ends */  
	reqTargetCompileStarted    = -16,      /* Sent when target's compile phase starts */
    reqTargetLinkEnded         = -15,      /* Sent when the link has ended           */
	reqTargetLinkStarted       = -14,      /* Sent when the link starts */
	reqFileListBuildEnded      = -13,      /* Sent when the group file build ends */
	reqFileListBuildStarted    = -12,      /* Sent when the group file build starts */
	reqSubProjectBuildEnded    = -11,      /* Sent when subproject's build ends */
	reqSubProjectBuildStarted  = -10,      /* Sent when subproject's build starts */
	reqTargetBuildEnded        = -9,       /* Sent when target's build ends */
	reqTargetBuildStarted      = -8,       /* Sent when target's build starts */
	reqProjectBuildEnded       = -7,       /* Sent when project build ends         */
	reqProjectBuildStarted     = -6,       /* Sent when project build starts       */
	reqTargetLoaded   		   = -5,	   /* called when the "keeps target info" flag is set	*/
	reqTargetPrefsChanged	   = -4,	   /* called when the "keeps target info" flag is set	*/
	reqTargetUnloaded		   = -3		   /* called when the "keeps target info" flag is set	*/
};

/* requests codes (compilers) */
enum {
	reqCompile = 0,						/* compile/precompile/preprocess source file		*/
										/*   and return results								*/
	reqMakeParse,						/* used internally by Metrowerks					*/
	reqCompDisassemble,					/* disassemble a specific file in the project		*/
	reqCheckSyntax,						/* check syntax, but don't generate code			*/
	reqPreprocessForDebugger			/* when preprocess, don't open window w/results		*/
};

/* requests codes (linkers) */
enum {
	reqLink = 0,						/* link project and generate executable or library	*/
	reqDisassemble,						/* disassemble a specific file in the project		*/
	reqTargetInfo,						/* get info about the generated executable			*/
	reqPreRun							/* give linker a last chance to modify target info	*/
};

/* Returned as part of CWFileInfo to indicate the */
/* type of data pointed to by the filedata pointer */
enum {
	filetypeText,						/* data is text										*/
	filetypeUnknown						/* unknown type, could be precompiled header		*/
};

/* executable linkage types, used in CWTargetInfo.linkType									*/
enum {
	exelinkageFlat,						/* flat executable									*/
	exelinkageSegmented,				/* uses 68K code segments							*/
	exelinkageOverlay1					/* uses overlay groups and overlays					*/
};

/* output file type, used in CWTargetInfo.outputType										*/
enum {
	linkOutputNone,						/* linker produces no output						*/
	linkOutputFile,						/* linker produces a file							*/
	linkOutputDirectory					/* linker produces a directory						*/
};

/* Constants for the targetCPU field of the CWTargetInfo struct								*/
enum {
	targetCPU68K			=	CWFOURCHAR('6','8','k',' '),
	targetCPUPowerPC		=	CWFOURCHAR('p','p','c',' '),
	targetCPUi80x86			=	CWFOURCHAR('8','0','8','6'),
	targetCPUMips			=	CWFOURCHAR('m','i','p','s'),
	targetCPUNECv800		=	CWFOURCHAR('v','8','0','0'),
	targetCPUEmbeddedPowerPC =	CWFOURCHAR('e','P','P','C'),
	targetCPUARM			=	CWFOURCHAR('a','r','m',' '),
	targetCPUSparc			=	CWFOURCHAR('s','p','r','c'),
	targetCPUIA64			=	CWFOURCHAR('I','A','6','4'),
	targetCPUAny			=	CWFOURCHAR('*','*','*','*'),
	targetCPUMCORE			=	CWFOURCHAR('m','c','o','r'),
	targetCPU_Intent        =   CWFOURCHAR('n','t','n','t')
};

/* Constants for the targetOS field of the CWTargetInfo struct								*/
enum {
	targetOSMacintosh		=	CWFOURCHAR('m','a','c',' '),
	targetOSWindows			=	CWFOURCHAR('w','i','n','t'),
	targetOSNetware			=	CWFOURCHAR('n','l','m',' '),
	targetOSMagicCap		=	CWFOURCHAR('m','c','a','p'),
	targetOSOS9				=	CWFOURCHAR('o','s','9',' '),
	targetOSEmbeddedABI		=	CWFOURCHAR('E','A','B','I'),
	targetOSJava			= 	CWFOURCHAR('j','a','v','a'),	/* java (no VM specification)	*/
	targetOSJavaMS			=	CWFOURCHAR('j','v','m','s'),	/* Microsoft VM					*/
	targetOSJavaSun			=	CWFOURCHAR('j','v','s','n'),	/* Sun VM						*/
	targetOSJavaMRJ			=	CWFOURCHAR('j','v','m','r'),	/* MRJ VM						*/
	targetOSJavaMW			=	CWFOURCHAR('j','v','m','w'),	/* Metrowerks VM				*/
	targetOSPalm			=	CWFOURCHAR('p','a','l','m'),
	targetOSGTD5			=   CWFOURCHAR('g','t','d','5'),
	targetOSSolaris			=	CWFOURCHAR('s','l','r','s'),
	targetOSLinux			=	CWFOURCHAR('l','n','u','x'),
	targetOSAny				=	CWFOURCHAR('*','*','*','*'),
	targetOS_Intent         =   CWFOURCHAR('n','t','n','t')
};

/* linker flags, as used in member dropinflags of DropInFlags struct returned by linkers		*/
enum {
	cantDisassemble  						= 1L << 31,		/* this linker doesn't support 'Disassemble'		*/
	isPostLinker	 						= 1L << 30,		/* this is a post linker							*/
	linkAllowDupFileNames					= 1L << 29,		/* allow multiple project files with the same name  */
	linkMultiTargAware						= 1L << 28,		/* the linker can be used with multiple targets		*/
	isPreLinker								= 1L << 27,		/* this is a pre linker								*/
	linkerUsesTargetStorage					= 1L << 26,		/* the linker keeps storage per target				*/
	linkerUnmangles							= 1L << 25,		/* The linker supports unmangling.					*/
	magicCapLinker							= 1L << 24,		/* Pre- or post-linker is used for MagicCap			*/
	linkAlwaysReload						= 1L << 23,		/* Always reload the linker before request			*/
	linkRequiresProjectBuildStartedMsg		= 1L << 22,		/* Linker listens for a Project Build Started/Ended message     */
	linkRequiresTargetBuildStartedMsg		= 1L << 21,		/* Linker listens for a Target Build Started/Ended message      */
	linkRequiresSubProjectBuildStartedMsg	= 1L << 20,		/* Linker listens for a Sub Project Build Started/Ended message */
	linkRequiresFileListBuildStartedMsg		= 1L << 19,		/* Linker listens for a File List Build Started/Ended message   */
	linkRequiresTargetLinkStartedMsg		= 1L << 18,		/* Linker listens for a Target Link Started/Ended message       */
	linkerWantsPreRunRequest				= 1L << 17,		/* Linker wants to be sent the pre-run request					*/
	linkerGetTargetInfoThreadSafe			= 1L << 16,		/* GetTargetInfo call doesn't use any globals, etc.				*/
	linkerUsesCaseInsensitiveSymbols		= 1L << 15,		/* All languages used by linker contain case insensitive browser symbols	*/
	linkerDisasmRequiresPreprocess			= 1L << 14,		/* (obsolete) file must be preprocesed before being passed to linker for disasm		*/
	linkerUsesFrameworks					= 1L << 13,		/* target uses frameworks; enables framework-style file searching			*/
	linkerInitializeOnMainThread			= 1L << 12		/* The Linker needs to be intialized on the main thread. */
	/* remaining flags are reserved for future use and should be zero-initialized	*/
};

/* ways to store dependency returned as CWFileSpec in StoreObjectData */
enum {
	cwAccessAbsolute,
	cwAccessPathRelative,
	cwAccessFileName,
	cwAccessFileRelative
};

/* specifies what browser information compilers should generate  */
/* they should always generate records for globals and functions */
typedef struct CWBrowseOptions {
	Boolean recordClasses;				/* [<-] do we record info for classes				*/
	Boolean recordEnums;				/* [<-] do we record info for enums					*/
	Boolean recordMacros;				/* [<-] do we record info for macros				*/
	Boolean recordTypedefs;				/* [<-] do we record info for typedefs				*/
	Boolean recordConstants;			/* [<-] do we record info for constants				*/
	Boolean recordTemplates;			/* [<-] do we record info for templates				*/
	Boolean recordUndefinedFunctions;	/* [<-] do we record info for undefined functions	*/
	long	reserved1;					/* reserved space									*/
	long	reserved2;					/* reserved space									*/
} CWBrowseOptions;

/* A dependency tag is associated with unit data in the StoreUnitData callback. It should	*/
/* change whenever unit data has changed in a way that forces its dependents to be rebuilt.	*/
/* Typically, it is just a checksum on the unit data.										*/
typedef unsigned long CWDependencyTag;

/* dependency information passed back in StoreObjectData */
typedef struct CWDependencyInfo {
	long		fileIndex;					/* Another project entry. -1 => use fileSpec	*/
	CWFileSpec	fileSpec;					/* location of file. Only used if fileIndex < 0	*/
	short		fileSpecAccessType;			/* One of cwAccessAbsolute, etc. above. Only	*/
											/* used is specifying via fileSpec.				*/
	short		dependencyType;				/* cwNormalDependency or cwInterfaceDependency.	*/
											/* Values defined in CWPlugins.h				*/
} CWDependencyInfo;

/* information used when calling StoreObjectData */
typedef struct CWObjectData {
	CWMemHandle			objectdata;			/* Handle to generated object code, 		*/
											/*	resource data, or preprocessed text		*/
	CWMemHandle			browsedata;			/* Handle to generated browse data	 		*/
	long				reserved1;			/* reserved for future use, set to zero		*/
	long				codesize;			/* size of generated code					*/
	long				udatasize;			/* size of uninitialized data				*/
	long				idatasize;			/* size of initialized data					*/
	long				compiledlines;		/* number of lines of source compiled		*/
	Boolean				interfaceChanged;	/* recompile interface dependents?			*/
	long				reserved2;	        /* reserved for future use, set to zero		*/
	void*				compilecontext;		/* used internally by Metrowerks			*/
	CWDependencyInfo*	dependencies;		/* optional array of dependencies			*/
	short				dependencyCount;	/* overrides those collected by IDE			*/
	CWFileSpec*			objectfile;			/* external object code file (i.e. .o file)	*/
} CWObjectData;


/* characteristics of a link target */
typedef struct CWTargetInfo {
	short			outputType;			/* outputs file, directory, or nothing					*/
	CWFileSpec		outfile;			/* generated executable file/folder spec				*/
	CWFileSpec		symfile;			/* generated debug file spec							*/
	CWFileSpec		runfile;			/* file to run, can be same or different from outfile	*/
	short			linkType;			/* flat, segmented, or overlays							*/
	Boolean			canRun;				/* "Run" can be performed on this target				*/
	Boolean			canDebug;			/* "Run with Debugger" can be performed on this target	*/
	CWDataType		targetCPU;			/* target CPU architecture (e.g. 68K, PowerPC, x86, MIPS)*/	
	CWDataType		targetOS;			/* target OS (e.g. MacOS, Windows, Unix)				 */

#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS
	OSType			outfileCreator;		/* file creator, if outputType == linkOutputFile		*/
	OSType			outfileType;		/* file type, if outputType == linkOutputFile			*/
	OSType			debuggerCreator;	/* file creator of debugger for this target				*/
	OSType			runHelperCreator;	/* creator of helper app to use when running this file.	*/
#endif
#if CWPLUGIN_HOST == CWPLUGIN_HOST_WIN32
	Boolean			runHelperIsRegKey;	/* true if runHelperName is a registry key				*/
	Boolean			debugHelperIsRegKey;/* true if debugHelperName is a registry key			*/
	char			args[512];			/* command line arguments								*/
	char			runHelperName[512];	/* full path to the run helper executable or a reg key	*/
 	Boolean			runHelperRequiresURL;/* Indicates whether the outfile must be converted to	*/
										/* a file scheme URL before being passed to the run		*/
										/* helper app											*/
	char			reserved2;
	char			debugHelperName[512];/* full path to the debug helper executable or a reg key*/
#endif

// We need the args for Solaris
#if (CWPLUGIN_HOST == CWPLUGIN_HOST_SOLARIS || CWPLUGIN_HOST == CWPLUGIN_HOST_LINUX)
	char			args[512];				/* command line arguments										*/
	char			runHelperName[512];		/* Relative path name from (Helper Apps) to run helper program	*/
	Boolean			runHelperRequiresURL;	/* Indicates whether the outfile must be converted to			*/
											/* a file scheme URL before being passed to the run				*/
											/* helper app													*/
	char			reserved2[3];
	char			debugHelperName[512];	/* Relative path name from (Helper Apps) to debug helper		*/
		
	
#endif	

	CWFileSpec	linkAgainstFile;	/* file parent targets link against (e.g. lib file)	*/
} CWTargetInfo;

typedef struct CWUnmangleInfo {
	void*			targetStorage;
	const char*		mangledName;
	char*			unmangleBuff;
	long			unmangleBuffSize;
	unsigned short	browserClassID;
	unsigned char	browserLang;
	unsigned char	filler1;
} CWUnmangleInfo;


/* Types used to get compiler-specific browser symbol information. */
typedef struct CWCompilerBrSymbol {
	char	symName[32];
	char	symUIName[32];
} CWCompilerBrSymbol;


typedef struct CWCompilerBrSymbolList {
	short				count;
	CWCompilerBrSymbol	items[1];
} CWCompilerBrSymbolList;

typedef struct CWCompilerBrSymbolInfo {
	void*					targetStorage;			
	CWCompilerBrSymbolList*	symList;			/* [<-] Compiler should put a pointer to the read-only list here.*/
} CWCompilerBrSymbolInfo;

typedef struct CWFrameworkInfo {
	CWFileSpec	fileSpec;						/* location of ".framework" directory								*/
	char		version[256];					/* which version directory to use; if empty use "Current" sym link	*/
} CWFrameworkInfo;

/*** Declaration of plugin entry points that must be implemented by non-MacOS plugins ***/
/*** It can also be implemented by a MacOS plugin to override the 'Targ' resource	  ***/

CWPLUGIN_ENTRY (CWPlugin_GetTargetList)(const struct CWTargetList**);

/*** Optional entry points															  ***/

CWPLUGIN_ENTRY (CWPlugin_GetDefaultMappingList)(const struct CWExtMapList**);
CWPLUGIN_ENTRY (Helper_Unmangle)(CWUnmangleInfo*);
CWPLUGIN_ENTRY (Helper_GetCompilerBrSymbols)(CWCompilerBrSymbolInfo*);

/*** callbacks to the IDE, in addition to those in CWPlugins.h 						  ***/

CW_CALLBACK CWIsPrecompiling(CWPluginContext context, Boolean* isPrecompiling);
CW_CALLBACK CWIsAutoPrecompiling(CWPluginContext context, Boolean* isAutoPrecompiling);
CW_CALLBACK CWIsPreprocessing(CWPluginContext context, Boolean* isPreprocessing);
CW_CALLBACK CWIsGeneratingDebugInfo(CWPluginContext context, Boolean* isGenerating);
CW_CALLBACK CWIsCachingPrecompiledHeaders(CWPluginContext context, Boolean* isCaching);
CW_CALLBACK CWGetBrowseOptions(CWPluginContext context, CWBrowseOptions* browseOptions);
CW_CALLBACK CWGetBuildSequenceNumber(CWPluginContext context, long* sequenceNumber);
CW_CALLBACK CWGetTargetInfo(CWPluginContext context, CWTargetInfo* targetInfo);
CW_CALLBACK CWSetTargetInfo(CWPluginContext context, CWTargetInfo* targetInfo);

CW_CALLBACK	CWGetTargetStorage(CWPluginContext context, void** storage);
CW_CALLBACK	CWSetTargetStorage(CWPluginContext context, void* storage);

CW_CALLBACK CWGetMainFileNumber(CWPluginContext context, long* fileNumber);
CW_CALLBACK CWGetMainFileID(CWPluginContext context, short* fileID);
CW_CALLBACK CWGetMainFileSpec(CWPluginContext context, CWFileSpec* fileSpec);
CW_CALLBACK CWGetMainFileText(CWPluginContext context, const char** text, long* textLength);

CW_CALLBACK	CWCachePrecompiledHeader(CWPluginContext context, const CWFileSpec* filespec, CWMemHandle pchhandle);

CW_CALLBACK	CWLoadObjectData(CWPluginContext context, long whichfile, CWMemHandle* objectdata);
CW_CALLBACK	CWFreeObjectData(CWPluginContext context, long whichfile, CWMemHandle objectdata);
CW_CALLBACK	CWStoreObjectData(CWPluginContext context, long whichfile, CWObjectData* object);
CW_CALLBACK	CWGetSuggestedObjectFileSpec(CWPluginContext context, long whichfile, CWFileSpec* fileSpec);
CW_CALLBACK	CWGetStoredObjectFileSpec(CWPluginContext context, long whichfile, CWFileSpec* fileSpec);

CW_CALLBACK	CWDisplayLines(CWPluginContext context, long nlines);

CW_CALLBACK CWBeginSubCompile(CWPluginContext context, long whichfile, CWPluginContext* subContext);
CW_CALLBACK CWEndSubCompile(CWPluginContext subContext);

CW_CALLBACK	CWGetPrecompiledHeaderSpec(CWPluginContext context, CWFileSpec *pchspec, const char *target);
CW_CALLBACK	CWGetResourceFile(CWPluginContext context, CWFileSpec* filespec);
CW_CALLBACK	CWPutResourceFile(CWPluginContext context, const char* prompt, const char* name, CWFileSpec* filespec);

/* Metrowerks Pascal support */
CW_CALLBACK	CWLookUpUnit(CWPluginContext context, const char* name, Boolean isdependency, const void** unitdata, long* unitdatalength);
CW_CALLBACK	CWSBMfiles(CWPluginContext context, short libref);
CW_CALLBACK CWStoreUnit(CWPluginContext context, const char* unitname, CWMemHandle unitdata, CWDependencyTag dependencytag);
CW_CALLBACK CWReleaseUnit(CWPluginContext context, void* unitdata);
CW_CALLBACK	CWUnitNameToFileName(CWPluginContext context, const char* unitname, char* filename);

	/* obsolete, for MacOS backward compatibility only */
#if CWPLUGIN_API == CWPLUGIN_API_MACOS
CW_CALLBACK	CWOSAlert(CWPluginContext context, const char* message, OSErr errorcode);
CW_CALLBACK	CWOSErrorMessage(CWPluginContext context, const char *msg, OSErr errorcode);
#endif

CW_CALLBACK CWGetModifiedFiles(CWPluginContext context, long* modifiedFileCount, const long** modifiedFiles);

/* Get information from the "Runtime Settings" panel	*/
CW_CALLBACK CWGetCommandLineArgs(CWPluginContext context, const char** commandLineArgs);
CW_CALLBACK CWGetWorkingDirectory(CWPluginContext context, CWFileSpec* workingDirectorySpec);
CW_CALLBACK CWGetEnvironmentVariableCount(CWPluginContext context, long* count);
CW_CALLBACK CWGetEnvironmentVariable(CWPluginContext context, long index, const char** name, const char** value);

CW_CALLBACK CWGetFrameworkCount(CWPluginContext context, long* frameworkCount);
CW_CALLBACK CWGetFrameworkInfo(CWPluginContext context, long whichFramework, CWFrameworkInfo* frameworkInfo);
CW_CALLBACK CWGetFrameworkSharedLibrary(CWPluginContext context, long whichFramework, CWFileSpec* frameworkSharedLibrary);

#if CW_USE_PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
	}
#endif

#ifdef	_MSC_VER
#pragma	pack(pop,2)
#endif

#ifdef	__MWERKS__
#pragma options align=reset
#endif

#endif	/* __DROPINCOMPILERLINKER_H__ */
