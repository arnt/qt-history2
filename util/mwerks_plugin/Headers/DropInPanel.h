/*
 *  DropInPanel.h - Drop-In Preferences Panel Interface for Metrowerks CodeWarrior
 *
 *  Copyright (c) 1995 Metrowerks, Inc.  All rights reserved.
 *
 */

#ifndef __DROPINPANEL_H__
#define __DROPINPANEL_H__

#ifdef __MWERKS__
#	pragma once
#endif

#ifndef CW_STRICT_DIALOGS
#	define CW_STRICT_DIALOGS	0
#endif

#if macintosh
#ifndef F_PASCAL
#	define F_PASCAL(x) pascal x
#endif
#elif __sun__ || __linux__
#ifndef F_PASCAL
#	define F_PASCAL(x) x
#endif
#endif

/* system headers */
#ifndef __APPLEEVENTS__
#	include <AppleEvents.h>
#endif
#ifndef __CONTROLS__
#	include <Controls.h>
#endif
#ifndef __DIALOGS__
#	include <Dialogs.h>
#endif
#ifndef __DRAG__
#	include <Drag.h>
#endif

#ifndef __CWPlugins_H__
#	include "CWPlugins.h"
#endif

#ifdef	__MWERKS__
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
#define DROPINPANELAPIVERSION_1	1							/* CW7 API version		*/
#define DROPINPANELAPIVERSION_2	2							/* intermediate version	*/
#define DROPINPANELAPIVERSION_3	3							/* CW8 API version		*/
#define DROPINPANELAPIVERSION_4	4							/* CW9 API version		*/
#define DROPINPANELAPIVERSION_5 5							/* CW10-CW Pro 1 version */
#define DROPINPANELAPIVERSION_6 6							/* CW Pro 2 version		*/
#define DROPINPANELAPIVERSION_7 7							/* CW Pro 3 version		*/
#define DROPINPANELAPIVERSION_8 8							/* CW Pro 4 version		*/
#define DROPINPANELAPIVERSION_9 9							/* CW Pro 5 version		*/
#define DROPINPANELAPIVERSION_10 10							/* CW Pro 6 version		*/
#define DROPINPANELAPIVERSION_11 11							/* CW Pro 7 version (listView Support) */
#define DROPINPANELAPIVERSION	DROPINPANELAPIVERSION_11	/* current API version	*/

/* error codes */
#define kBadPrefVersion			1000
#define kMissingPrefErr			1001
#define kSettingNotFoundErr		1002
#define kSettingTypeMismatchErr	1003
#define kInvalidCallbackErr		1004
#define kSettingOutOfRangeErr	1005

/* requests codes */
enum {
	reqInitPanel = -2,		/* (called when panel is loaded)						*/
	reqTermPanel = -1,		/* (called when panel is unloaded)						*/
	reqInitDialog = 0,		/* initialize panel's dialog state						*/
	reqTermDialog,			/* clean up panel's dialog state						*/
	reqPutData,				/* copy options data to dialog items					*/
	reqGetData,				/* copy dialog items to options data					*/
	reqFilter,				/* filter a dialog event for the panel					*/
	reqItemHit,				/* process an itemHit in the panel						*/
	reqAEGetPref,			/* get preference setting for AppleEvent request		*/
	reqAESetPref,			/* set preference setting from AppleEvent request		*/
	reqValidate,			/* tell if current settings force recompile or relink	*/
	reqGetFactory,			/* retrieve factory settings							*/
	reqUpdatePref,			/* verify and/or modify prefs to fit current version	*/
	reqUpdateProject,		/* (only sent to built-in panels)						*/
	reqSetupDebug,			/* change settings to reflect debugging status			*/
	reqRenameProject,		/* change settings that depend on project name			*/
	reqPrefsLoaded,			/* (only sent to built-in panels)						*/
	reqDrawCustomItem,		/* draw a custom item									*/
	reqActivateItem,		/* activate a custom item								*/
	reqDeactivateItem,		/* deactivate a custom item								*/
	reqHandleClick,			/* handle mouse down in an active custom item			*/
	reqHandleKey,			/* handle key down in an active custom item				*/
	reqFindStatus,			/* enable/disable menu items for a custom item			*/
	reqObeyCommand,			/* execute a menu command for a custom item				*/
	reqDragEnter,			/* the user is dragging into the given item				*/
	reqDragWithin,			/* the user is dragging within the given item			*/
	reqDragExit,			/* the user is dragging out of the given item			*/
	reqDragDrop,			/* the user dropped onto the given item					*/
	reqByteSwapData,		/* byte swap the fields in the prefs data				*/
	reqFirstLoad,			/* panel has been loaded for the first time				*/
	reqReadSettings,		/* read settings from IDE to construct pref data handle	*/
	reqWriteSettings,		/* write the individual settings from pref data handle	*/
	reqItemDoubleClick,		/* process an double clicked in the panel, currently implemented for listviews  */
	reqItemIsCellEditable	/* currently implemented for listviews, tells whether or not the listview is editable  */
};

/* menu command codes */
enum {
	menu_Cut = 1,
	menu_Copy,
	menu_Paste,
	menu_Clear,
	menu_SelectAll
};

/* Layout and bit flags for 'Flag' resource for panels 								*/
/*																					*/
/* For the version 3 of these resource, we renamed the 'apiversion' field to		*/
/* 'earliestCompatibleAPIVersion' and added the 'newestAPIVersion' field.			*/
/* This allows plugins to support more than one API version and therefore run		*/
/* under more than one version of the IDE.											*/

typedef struct PanelFlags {
	unsigned short	rsrcversion;		/*	version number of resource				*/
	CWDataType		dropintype;			/*	dropin type ('Comp', 'Link', 'Panl')	*/
										/*  earliest API support by this plugin		*/
	unsigned short	earliestCompatibleAPIVersion;
	unsigned long	dropinflags;		/*	capability flags (see enum below)		*/
	CWDataType		panelfamily;		/*	family of panel (linker, codegen, etc)	*/
	unsigned short	newestAPIVersion;	/*	newest API version supported			*/
	unsigned short	dataversion;		/*	version number of prefs data			*/
	unsigned short	panelscope;			/*	scope of panel (see enum below)			*/
} PanelFlags;

/* capability flags, as used in member dropinflags of PanelFlags struct				*/
enum {
	usesStrictAPI			= 1 << 31,			/* this panel is built with the	strict API	*/
	supportsByteSwapping	= 1 << 30,			/* this panel support the byte-swapping request */
	supportsTextSettings	= 1 << 29,			/* this panel supports the read & write settings requests */
	usesCrossPlatformAPI	= 1 << 28			/* uses the cross-platform API rather than Mac API */
	/* remaining flags are reserved for future use and should be zero-initialized	*/
};

/* panel scopes, as used in member panelscope of PanelFlags struct					*/
/*																					*/
/* The scope of a panel tells the IDE which settings window to display the panel.	*/
/* Currently, only panels for debugger plug-ins use panelScopeGlobal and only		*/
/* panels for VCS plug-ins use panelScopeProject. A panel for a compiler or linker	*/
/* must use panelScopeTarget.														*/

enum {
	panelScopeGlobal,	/*	this panel is scoped to the global preferences window	*/
	panelScopeProject,	/*	this panel is scoped to the VCS settings window			*/
	panelScopeTarget,	/* 	this panel is scoped to the target settings window		*/
	panelScopeRConn		/*	this panel is scoped to configure remote debugging connections */		
};

/* pre-defined panel families, used in panelfamily field of PanelFlags struct		*/

enum {
	panelFamilyProject		= CWFOURCHAR('p', 'r', 'o', 'j'),
	panelFamilyFrontEnd		= CWFOURCHAR('f', 'e', 'n', 'd'),
	panelFamilyBackEnd		= CWFOURCHAR('b', 'e', 'n', 'd'),
	panelFamilyBrowser		= CWFOURCHAR('b', 'r', 'o', 'w'),
	panelFamilyEditor		= CWFOURCHAR('e', 'd', 'i', 't'),
	panelFamilyDebugger		= CWFOURCHAR('d', 'b', 'u', 'g'),
	panelFamilyLinker		= CWFOURCHAR('l', 'i', 'n', 'k'),
	panelFamilyMisc			= CWFOURCHAR('*', '*', '*', '*')
};

typedef struct MWSetting* CWSettingID;
#define kNoSettingID 0

/* alias types */
#if CW_STRICT_DIALOGS
	typedef struct DummyDialog*	CWDialog;
#else
	typedef DialogPtr			CWDialog;
#endif

/* forward declarations */
typedef struct CWPanelCallbacks	CWPanelCallbacks;

/* parameter block -- this is passed to the dropin at each request */
typedef struct PanelParameterBlock {
	/* common to all dropins */
	long		request;			/* [->]  requested action (see below)		*/
	long		version;			/* [->]  version # of shell's API			*/
	void		*context;			/* [->]  reserved for use by shell			*/
	void		*storage;			/* [<->] reserved for use by the dropin		*/
	FSSpec		targetfile;			/* [->]  FSSpec of current project			*/
	
	/* specific to panels */
	CWDialog	dialog;				/* [->]  pointer to Preferences dialog		*/
	Handle		originalPrefs;		/* [->]  panel's original options data		*/
	Handle		currentPrefs;		/* [<->] panel's current options data		*/
	Handle		factoryPrefs;		/* [<->] panel's "factory" options data		*/
	EventRecord	*event;				/* [->]  dialog event (for reqFilterEvent)	*/
	short		baseItems;			/* [->]  # of items in dialog shell			*/
	short		itemHit;			/* [<->] for reqFilterEvent and reqItemHit	*/
	Boolean		canRevert;			/* [<-]  enable Revert button				*/
	Boolean		canFactory;			/* [<-]  enable Factory button				*/
	Boolean		reset;				/* [<-]  access paths must be reset			*/
	Boolean		recompile;			/* [<-]  files must be recompiled			*/
	Boolean		relink;				/* [<-]  project must be relinked			*/
	AEKeyword	prefsKeyword;		/* [->]  for reqAEGetPref and reqAESetPref	*/
	AEDesc		prefsDesc;			/* [->]  for reqAESetPref					*/
	Boolean		debugOn;			/* [->]  turning on debugging?				*/
	FSSpec		oldtargfile;		/* [->]  previous project file FSSpec		*/
	
	/* version 2 API */
	CWPanelCallbacks*	callbacks;
	
	/* version 3 API */
	Boolean		reparse;			/* [<-]  project must be reparsed			*/
	
	/* version 4 API */
	DragReference	dragref;		/* [->]  for drag-related requests			*/
	Rect			dragrect;		/* [<-]  rect to track mouse in				*/
	Point			dragmouse;		/* [->]  mouse location during drag			*/
	
	/* version 5 API */
	unsigned char	toEndian;		/* [->]  for reqByteSwapData, the endian we are swapping to */
	
	/* CWPro 3 temporary placeholders for opaque references to prefs data. 
		These will be removed in Pro 4.
	*/
	CWMemHandle		originalPrefsMemHandle;
	CWMemHandle		currentPrefsMemHandle;
	CWMemHandle		factoryPrefsMemHandle;
	CWMemHandle		panelPrefsMemHandle;

	/* version 11 api */
	long 			listViewCellRow;			/* [->]  the cell row of the listView */
	long 			listViewCellCol;			/* [->]  the cell row of the listView */
	
} PanelParameterBlock, *PanelParameterBlockPtr;

typedef PanelParameterBlock		PanelParamBlk;
typedef PanelParameterBlockPtr	PanelParamBlkPtr;


/* callbacks to the IDE */
extern F_PASCAL(OSErr)	CWPanlAppendItems(PanelParamBlkPtr ppb, short ditlID);
extern F_PASCAL(OSErr)	CWPanlDrawPanelBox(PanelParamBlkPtr ppb, long whichItem, ConstStr255Param title);
extern F_PASCAL(OSErr)	CWPanlShowItem(PanelParamBlkPtr ppb, long whichItem, Boolean showIt);
extern F_PASCAL(OSErr)	CWPanlEnableItem(PanelParamBlkPtr ppb, long whichItem, Boolean enableIt);
extern F_PASCAL(OSErr)	CWPanlActivateItem(PanelParamBlkPtr ppb, long whichItem);
extern F_PASCAL(OSErr)	CWPanlGetItemValue(PanelParamBlkPtr ppb, long whichItem, long* value);
extern F_PASCAL(OSErr)	CWPanlSetItemValue(PanelParamBlkPtr ppb, long whichItem, long value);
extern F_PASCAL(OSErr)	CWPanlGetItemText(PanelParamBlkPtr ppb, long whichItem, StringPtr str, short maxLen);
extern F_PASCAL(OSErr)	CWPanlSetItemText(PanelParamBlkPtr ppb, long whichItem, ConstStr255Param str);
extern F_PASCAL(OSErr)	CWPanlGetPanelPrefs(PanelParamBlkPtr ppb, StringPtr inPanelName, Handle *prefs, Boolean* requiresByteSwap);
extern F_PASCAL(OSErr)	CWPanlGetItemRect(PanelParamBlkPtr ppb, long whichItem, Rect* rect);
extern F_PASCAL(OSErr)	CWPanlGetItemControl(PanelParamBlkPtr ppb, long whichItem, ControlRef* control);
extern F_PASCAL(OSErr)	CWPanlInvalItem(PanelParamBlkPtr ppb, long whichItem);
extern F_PASCAL(OSErr)	CWPanlValidItem(PanelParamBlkPtr ppb, long whichItem);
extern F_PASCAL(OSErr)	CWPanlGetMacPort(PanelParamBlkPtr ppb, GrafPtr* port);
extern F_PASCAL(OSErr)	CWPanlGetItemTextHandle(PanelParamBlkPtr ppb, long whichItem, Handle *text);
extern F_PASCAL(OSErr)	CWPanlSetItemTextHandle(PanelParamBlkPtr ppb, long whichItem, Handle text);
extern F_PASCAL(OSErr)	CWPanlGetItemData(PanelParamBlkPtr ppb, long whichItem, void *outData, long *outDataLength);
extern F_PASCAL(OSErr)	CWPanlSetItemData(PanelParamBlkPtr ppb, long whichItem, void *inData, long inDataLength);
extern F_PASCAL(OSErr)	CWPanlGetItemMaxLength(PanelParamBlkPtr ppb, long whichItem, short *outLength);
extern F_PASCAL(OSErr)	CWPanlSetItemMaxLength(PanelParamBlkPtr ppb, long whichItem, short inLength);
extern F_PASCAL(OSErr)	CWPanlChooseRelativePath(PanelParamBlkPtr ppb, CWRelativePath* path, Boolean isFolder, short filterCount, void* filterList, char* prompt);
extern F_PASCAL(OSErr)	CWPanlGetRelativePathString(PanelParamBlkPtr ppb, CWRelativePath* path, char* pathString, long* maxLength);

#if macintosh
extern F_PASCAL(OSErr)	CWPanlReadRelativePathAEDesc(PanelParamBlkPtr ppb, CWRelativePath* path, const AEDesc* desc);
extern F_PASCAL(OSErr)	CWPanlWriteRelativePathAEDesc(PanelParamBlkPtr ppb, const CWRelativePath* path, AEDesc* desc);
#endif

/* utility routines */
extern F_PASCAL(OSErr)	CWPanlDrawUserItemBox(DialogPtr dialog, short whichItem, ConstStr255Param title);
extern F_PASCAL(OSErr)	CWPanlInstallUserItem(PanelParamBlkPtr ppb, short whichItem, UserItemProcPtr proc);
extern F_PASCAL(OSErr)	CWPanlRemoveUserItem(PanelParamBlkPtr ppb, short whichItem);

/* reading and writing scalar settings */
extern F_PASCAL(OSErr)	CWPanlReadBooleanSetting(PanelParamBlkPtr ppb, const char* name, Boolean* value);
extern F_PASCAL(OSErr)	CWPanlReadIntegerSetting(PanelParamBlkPtr ppb, const char* name, long* value);
extern F_PASCAL(OSErr)	CWPanlReadFloatingPointSetting(PanelParamBlkPtr ppb, const char* name, double* value);
extern F_PASCAL(OSErr)	CWPanlReadStringSetting(PanelParamBlkPtr ppb, const char* name, const char** value);
extern F_PASCAL(OSErr)	CWPanlReadRelativePathSetting(PanelParamBlkPtr ppb, const char* name, CWRelativePath* value);

extern F_PASCAL(OSErr)	CWPanlWriteBooleanSetting(PanelParamBlkPtr ppb, const char* name, Boolean value);
extern F_PASCAL(OSErr)	CWPanlWriteIntegerSetting(PanelParamBlkPtr ppb, const char* name, long value);
extern F_PASCAL(OSErr)	CWPanlWriteFloatingPointSetting(PanelParamBlkPtr ppb, const char* name, double value);
extern F_PASCAL(OSErr)	CWPanlWriteStringSetting(PanelParamBlkPtr ppb, const char* name, const char* value);
extern F_PASCAL(OSErr)	CWPanlWriteRelativePathSetting(PanelParamBlkPtr ppb, const char* name, const CWRelativePath* value);

/* reading and writing array and structure settings */
extern F_PASCAL(OSErr)	CWPanlGetNamedSetting(PanelParamBlkPtr ppb, const char* name, CWSettingID* settingID);
extern F_PASCAL(OSErr)	CWPanlGetStructureSettingField(PanelParamBlkPtr ppb, CWSettingID settingID, const char* name, CWSettingID* fieldSettingID);
extern F_PASCAL(OSErr)	CWPanlGetArraySettingSize(PanelParamBlkPtr ppb, CWSettingID settingID, long* size);
extern F_PASCAL(OSErr)	CWPanlGetArraySettingElement(PanelParamBlkPtr ppb, CWSettingID settingID, long index, CWSettingID* elementSettingID);

extern F_PASCAL(OSErr)	CWPanlGetBooleanValue(PanelParamBlkPtr ppb, CWSettingID settingID, Boolean* value);
extern F_PASCAL(OSErr)	CWPanlGetIntegerValue(PanelParamBlkPtr ppb, CWSettingID settingID, long* value);
extern F_PASCAL(OSErr)	CWPanlGetFloatingPointValue(PanelParamBlkPtr ppb, CWSettingID settingID, double* value);
extern F_PASCAL(OSErr)	CWPanlGetStringValue(PanelParamBlkPtr ppb, CWSettingID settingID, const char** value);
extern F_PASCAL(OSErr)	CWPanlGetRelativePathValue(PanelParamBlkPtr ppb, CWSettingID settingID, CWRelativePath* value);

extern F_PASCAL(OSErr)	CWPanlSetBooleanValue(PanelParamBlkPtr ppb, CWSettingID settingID, Boolean value);
extern F_PASCAL(OSErr)	CWPanlSetIntegerValue(PanelParamBlkPtr ppb, CWSettingID settingID, long value);
extern F_PASCAL(OSErr)	CWPanlSetFloatingPointValue(PanelParamBlkPtr ppb, CWSettingID settingID, double value);
extern F_PASCAL(OSErr)	CWPanlSetStringValue(PanelParamBlkPtr ppb, CWSettingID settingID, const char* value);
extern F_PASCAL(OSErr)	CWPanlSetRelativePathValue(PanelParamBlkPtr ppb, CWSettingID settingID, const CWRelativePath* value);

#if CW_USE_PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
	}
#endif

#ifdef	__MWERKS__
#pragma options align=reset
#endif

#ifdef	_MSC_VER
#pragma pack(pop)
#endif

#endif	/* __DROPINPANEL_H__ */
