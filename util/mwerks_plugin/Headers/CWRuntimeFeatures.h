/*****************************************************************************/
/*  Name......:   CWRuntimeFeatures.h                                        */
/*  Purpose...:   Macros to parameterize your runtime environment based on   */
/*                    platform-specific compiler macros. Target/host         */
/*                    configurations to set host/target values as well as    */
/*                    CW_HOST, CW_HOSTOS, CW_HOST_ENDIAN, CW_LINE_ENDING,    */
/*                    and CW_UNIX_TYPE, which is used to discriminate among  */
/*                    the various UNIX versions.                             */
/*  Info......:   These settings are common enough to factor into a location */
/*                    that can be shared by all components, including those  */
/*                    used internally and those shared externally.           */
/*  Copyright.:   (c)Copyright 2000 by metrowerks inc. All rights reserved.  */
/*****************************************************************************/

#ifndef CW_RuntimeFeatures_H
#define CW_RuntimeFeatures_H


/*************************
 *  Runtime Definitions  *
 *************************/

  /* host / target processors values used to set HOST macro.  */

#define CW_MC68K            1
#define CW_POWERPC          2
#define CW_INTEL            3
#define CW_MIPS             4
#define CW_SPARC            5
#define CW_PA_RISC          6

  /* host / target operating systems values used to set CW_HOSTOS macro. */

#define	CW_MACOS            1
#define CW_BEWORKS          2
#define CW_UNIX             3
#define CW_MSWIN            4
#define CW_MAGIC            5
#define CW_QNX             11

  /* UNIX specialization values used to set the CW_UNIX_TYPE macro. */

#define CW_SOLARIS         32  /* Base value to support bit manipulation */
#define CW_LINUX           33
#define CW_RHAPSODY        34
#define CW_HPUX            35
#define CW_IRIX            36
#define CW_AIX             37
#define CW_FREEBSD         38

  /* big/little endian values used to set CW_HOST_ENDIAN macro. */

#define CW_ENDIAN_NEUTRAL	0x7fffffff
#define	CW_BIG_ENDIAN		1
#define CW_LITTLE_ENDIAN	2
#define CW_EITHER_ENDIAN	3

/*
** Detemine run-time environment settings based on clues from the
** compile time settings given by specific compilers.
*/
#if defined(__MWERKS__)
  /* Metrowerks Compiler */
  
  #if macintosh && __MC68K__
  
  		/* 68K MacOS */
  	#define	CW_HOST    			CW_MC68K
  	#define	CW_HOSTOS			CW_MACOS
  	#define CW_HOST_ENDIAN		CW_BIG_ENDIAN
  	#define CW_LINE_ENDING		"\r"

  #elif macintosh && __POWERPC__
	
		/* PPC MacOS */
  	#define CW_HOST				CW_POWERPC
  	#define	CW_HOSTOS			CW_MACOS
  	#define CW_HOST_ENDIAN		CW_BIG_ENDIAN
  	#define CW_LINE_ENDING		"\r"

  #elif __INTEL__ && defined(__BEOS__)
  
		/* x86 BeOS */
  	#define	CW_HOST    			CW_INTEL
  	#define	CW_HOSTOS			CW_BEWORKS
  	#define CW_HOST_ENDIAN		CW_LITTLE_ENDIAN
  	#define CW_LINE_ENDING		"\n"

  #elif __INTEL__ && defined(__QNX__)
  
		/* x86 QNX */
  	#define	CW_HOST    			CW_INTEL
  	#define	CW_HOSTOS			CW_QNX
  	#define CW_HOST_ENDIAN		CW_LITTLE_ENDIAN
  	#define CW_LINE_ENDING		"\n"
  	
  #elif __INTEL__ && __linux__
  
		/* x86 Linux */
  	#define CW_HOST				CW_INTEL
  	#define CW_HOSTOS			CW_UNIX
  	#define CW_UNIX_TYPE			CW_LINUX
  	#define CW_HOST_ENDIAN		CW_LITTLE_ENDIAN
  	#define CW_LINE_ENDING		"\n"  

  #elif __INTEL__  /* default to Win32 */
  
		/* x86 Assuming Windows */
  	#define	CW_HOST    			CW_INTEL
  	#define	CW_HOSTOS			CW_MSWIN
  	#define CW_HOST_ENDIAN		CW_LITTLE_ENDIAN
  	#define CW_LINE_ENDING		"\r\n"
  	
  #else
  
  	#error "UNKNOWN CW COMPILER USE"
  	
  #endif

#elif defined(__GNUC__)
  /* GCC/EGCS Compiler */
  
  #if __i386__ && __linux__
  
		/* x86 Linux */
	#define CW_HOST				CW_INTEL
  	#define CW_HOSTOS			CW_UNIX
  	#define CW_UNIX_TYPE			CW_LINUX
	#define CW_HOST_ENDIAN		CW_LITTLE_ENDIAN
	#define CW_LINE_ENDING		"\n"
    
  #elif __i386__ && __sun__
  
		/* x86 Solaris */
  	#define CW_HOST				CW_INTEL
  	#define CW_HOSTOS			CW_UNIX
  	#define CW_UNIX_TYPE			CW_SOLARIS
  	#define CW_HOST_ENDIAN		CW_LITTLE_ENDIAN
  	#define CW_LINE_ENDING		"\n"
  	
  #elif __sparc__ && __linux__
  
		/* SPARC Linux */
  	#define CW_HOST				CW_SPARC
  	#define CW_HOSTOS			CW_UNIX
  	#define CW_UNIX_TYPE			CW_LINUX
  	#define CW_HOST_ENDIAN		CW_BIG_ENDIAN
  	#define CW_LINE_ENDING		"\n"
  	
  #elif __sparc__ && __sun__
  
		/* SPARC Solaris */
	#define CW_HOST				CW_SPARC
  	#define CW_HOSTOS			CW_UNIX
  	#define CW_UNIX_TYPE			CW_SOLARIS
	#define CW_HOST_ENDIAN		CW_BIG_ENDIAN
	#define CW_LINE_ENDING		"\n"
  	
  #elif __i386__ && __BEOS__
  
  		/* x86 BeOS */
  	#define CW_HOST				CW_INTEL
  	#define CW_HOSTOS			CW_BEWORKS
  	#define CW_HOST_ENDIAN		CW_LITTLE_ENDIAN
  	#define CW_LINE_ENDING		"\n"
  	
  #elif __powerpc__ && __linux__
  
  		/* PPC Linux */
  	#define CW_HOST				CW_POWERPC
  	#define CW_HOSTOS			CW_UNIX
  	#define CW_UNIX_TYPE			CW_LINUX
  	#define CW_HOST_ENDIAN		CW_BIG_ENDIAN
  	#define CW_LINE_ENDING		"\n"
  	
  #else
  
  	#error "UNKNOWN GNU COMPILER USE"
  	
  #endif
  
#elif defined(_MSC_VER)
  /* Microsoft VC Compiler */
  
  #if defined(_M_IX86)
  
		/* x86 Windows */
  	#define CW_HOST				CW_INTEL
  	#define CW_HOSTOS			CW_MSWIN
  	#define CW_HOST_ENDIAN		CW_LITTLE_ENDIAN
  	#define CW_LINE_ENDING		"\r\n"
  	
  #else
  
  	#error "UNKNOWN MSC COMPILER USE"
  	
  #endif

#else

	#error	"UNKNOWN COMPILER"

#endif


#endif /* CW_RuntimeFeatures_H */
