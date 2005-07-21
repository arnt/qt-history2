/*
 *  CompilerMapping.h - File Type & Extension => Compiler Mapping for Metrowerks CodeWarrior
 *
 *  Copyright (c) 1995 Metrowerks, Inc.  All rights reserved.
 *	
 */

#ifndef __COMPILERMAPPING_H__
#define __COMPILERMAPPING_H__

#ifdef __MWERKS__
#	pragma once
#endif

#ifndef __CWPLUGINS_H__
#include "CWPlugins.h"
#endif

#ifdef __MWERKS__
#pragma options align=mac68k
#endif

#ifdef	_MSC_VER
#pragma	pack(push,2)
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __cplusplus
	const CWDataType Lang_C_CPP		= CWFOURCHAR('c','+','+',' ');
	const CWDataType Lang_Pascal	= CWFOURCHAR('p','a','s','c');
	const CWDataType Lang_Rez		= CWFOURCHAR('r','e','z',' ');
	const CWDataType Lang_Java		= CWFOURCHAR('j','a','v','a');
	const CWDataType Lang_MISC		= CWFOURCHAR('\?','\?','\?','\?');
#else
	#define Lang_C_CPP			CWFOURCHAR('c','+','+',' ')
	#define Lang_Pascal			CWFOURCHAR('p','a','s','c')
	#define Lang_Rez			CWFOURCHAR('r','e','z',' ')
	#define Lang_Java			CWFOURCHAR('j','a','v','a')
	#define Lang_MISC			CWFOURCHAR('\?','\?','\?','\?')
#endif

/* Compiler flags, as used in member dropinflags of struct DropInFlags returned by compilers		*/
enum
{
	kGeneratescode   =  1L << 31,		/* this compiler generates code							*/
	kGeneratesrsrcs  =  1L << 30,		/* this compiler generates resources					*/
	kCanpreprocess   =  1L << 29,		/* this compiler can accept a Preprocess request		*/
	kCanprecompile   =  1L << 28,		/* this compiler can accept a Precompile request		*/
	kIspascal        =  1L << 27,		/* this is the pascal compiler							*/
	kCanimport       =  1L << 26,		/* this compiler needs the "Import Weak" popup			*/
	kCandisassemble  =  1L << 25,		/* this compiler can disassemble 						*/
	kPersistent      =  1L << 24,		/* keep the compiler resident except on context switches*/
    kCompAllowDupFileNames = 1L << 23,	/* allow multiple project files with the same name  	*/
    kCompMultiTargAware  =  1L << 22,	/* the compiler can be used with multiple targets		*/
	kIsMPAware		=  1L << 21,		/* the compiler can be run in an MP thread				*/
	kCompUsesTargetStorage = 1L << 20,	/* the compiler keeps storage per target				*/
	kCompEmitsOwnBrSymbols = 1L << 19,	/* browser info includes compiler-specific symbols		*/
	kCompAlwaysReload = 1L << 18,		/* always reload the compiler before request			*/
	kCompRequiresProjectBuildStartedMsg   = 1L << 17,  /*  Compiler listens for  project build started messages     */
	kCompRequiresTargetBuildStartedMsg = 1L << 16,     /*  Compiler listens for  target build started messages      */
	kCompRequiresSubProjectBuildStartedMsg = 1L << 15, /*  Compiler listens for  Sub project build started messages */
	kCompRequiresFileListBuildStartedMsg = 1L << 14,    /*  Compiler listens for  filelist build started messages    */  
	kCompReentrant = 1L << 13,			/* Compiler can use re-entrant DropIn and is re-entry safe */
	kCompSavesDbgPreprocess = 1 << 12,	/* Compiler will save preprocessed files for debugging needs */
	kCompRequiresTargetCompileStartedMsg = 1 << 11	/* Compiler listens for target compile started/ended messages */
	/* remaining flags are reserved for future use and should be zero-initialized				                    */
};

/* Compiler mapping flags, used in CompilerMapping.flags & CWExtensionMapping.flags		*/
typedef unsigned long CompilerMappingFlags;
enum
{
	kPrecompile = 1L << 31,		/* should this file type be Precompiled?				*/
	kLaunchable = 1L << 30,		/* can this file type be double-clicked on?				*/
	kRsrcfile   = 1L << 29,		/* does this file type contain resources for linking?	*/
	kIgnored    = 1L << 28		/* should files of this type be ignored during Make?	*/
	/* remaining flags are reserved for future use and should be zero-initialized		*/
};

/* Format of data in 'EMap' resource, or as returned by  a compiler's 							*/
/* GetExtensionMapping entry point																*/

typedef struct CWExtensionMapping {
	CWDataType		type;				/* MacOS file type, e.g. 'TEXT' or 0					*/
	char			extension[32];		/* file extension, e.g. .c/.cp/.pch or ""				*/
	CompilerMappingFlags flags;			/* see above											*/
	char			editlanguage[32];	/* edit language or "" to use default language for plugin */
} CWExtensionMapping;

#define kCurrentCWExtMapListVersion	2
#define kCurrentCWExtMapListResourceVersion 2

typedef struct CWExtMapList {
	short					version;
	short					nMappings;
	CWExtensionMapping*		mappings;
} CWExtMapList;

/* Format of data returned by GetTargetList entry point									*/

#define kCurrentCWTargetListVersion	1
#define kCurrentCWTargetListResourceVersion 1

typedef struct CWTypeList {
	short		count;
	CWDataType	items[1];
} CW_CPUList, CW_OSList;

typedef struct CWTargetList {
	short		version;
	short		cpuCount;
	CWDataType*	cpus;
	short		osCount;
	CWDataType*	oss;
} CWTargetList;

typedef struct CWTargetListResource {
	short		version;
	CW_CPUList	cpus;
	CW_OSList	oss;
} CWTargetListResource;


#ifdef __cplusplus
	}
#endif

#ifdef	_MSC_VER
#pragma	pack(pop,2)
#endif

#ifdef	__MWERKS__
#pragma options align=reset
#endif

#endif	/* __COMPILERMAPPING_H__ */
