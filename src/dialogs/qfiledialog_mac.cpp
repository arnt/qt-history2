/****************************************************************************
** $Id$
**
** Implementation of QFileDialog classes for mac
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

#include "qapplication.h"
#include <private/qapplication_p.h>
#include "qt_mac.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qstringlist.h"
#include "qtextcodec.h"

static UInt8 *str_buffer = NULL;
static void cleanup_str_buffer()
{
    if(str_buffer) {
	free(str_buffer);
	str_buffer = NULL;
    }
}

extern const char qt_file_dialog_filter_reg_exp[]; // defined in qfiledialog.cpp

// Returns the wildcard part of a filter.
static QString extractFilter( const QString& rawFilter )
{
    QString result = rawFilter;
    QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
    int index = r.search( result );
    if ( index >= 0 )
	result = r.cap( 1 );
    return result.replace( QRegExp(QString::fromLatin1(" ")), QChar(';') );
}

// Makes a list of filters from ;;-separated text.
static QPtrList<QRegExp> makeFiltersList( const QString &filter )
{
    QString f( filter );

    if ( f.isEmpty( ) )
	f = QFileDialog::tr( "All Files (*.*)" );

    if ( f.isEmpty() )
	return QPtrList<QRegExp>();

    int i = f.find( ";;", 0 );
    QString sep( ";;" );
    if ( i == -1 ) {
	if ( f.find( "\n", 0 ) != -1 ) {
	    sep = "\n";
	    i = f.find( sep, 0 );
	}
    }

    QPtrList<QRegExp> ret;
    QStringList filts = QStringList::split( sep, f);
    for (QStringList::Iterator it = filts.begin(); it != filts.end(); ++it )
	ret.append(new QRegExp(extractFilter((*it)), TRUE, TRUE));
    return ret;
}


QMAC_PASCAL static Boolean qt_mac_nav_filter(AEDesc *theItem, void *info,
				 void *myd, NavFilterModes)
{
    QPtrList<QRegExp> *filt = (QPtrList<QRegExp> *)myd;
    if(!filt)
	return true;

    NavFileOrFolderInfo *theInfo = (NavFileOrFolderInfo *)info;
    if(theItem->descriptorType == typeFSS ) {
	if( !theInfo->isFolder ) {
	    AliasHandle alias;
	    Str63 str;
	    char tmp[sizeof(Str63)+2];
	    FSSpec      FSSpec;
	    AliasInfoType x = 0;
	    AEGetDescData( theItem, &FSSpec, sizeof(FSSpec));
	    if(NewAlias( NULL, &FSSpec, &alias ) != noErr)
		return true;
	    GetAliasInfo(alias, (AliasInfoType)x++, str);
	    if(str[0]) {
		strncpy((char *)tmp, (const char *)str+1, str[0]);
		tmp[str[0]] = '\0';
		for (QPtrListIterator<QRegExp> it(*filt); it.current(); ++it ) {
		    if(it.current()->exactMatch( tmp ))
			return true;
		}
	    }
	    return false;
	}
    }
    return true;
}
static NavObjectFilterUPP mac_navUPP = NULL;
static void cleanup_navUPP()
{
    DisposeNavObjectFilterUPP(mac_navUPP);
    mac_navUPP = NULL;
}
static const NavObjectFilterUPP make_navUPP()
{
    if(mac_navUPP)
	return mac_navUPP;
    qAddPostRoutine( cleanup_navUPP );
    return mac_navUPP = NewNavObjectFilterUPP(qt_mac_nav_filter);
}

const unsigned char * p_str(const char *, int len=-1);
QMAC_PASCAL OSErr FSpLocationFromFullPath( short fullPathLength,
				      const void *fullPath,
				      FSSpec *spec);

QStringList QFileDialog::macGetOpenFileNames( const QString &filter, QString *,
					      QWidget *parent, const char* /*name*/,
					      const QString& caption, bool multi,
					      bool directory )
{
    OSErr err;
    QString tmpstr;
    QStringList retstrl;
    static const int w = 450, h = 350;
    NavDialogCreationOptions options;
    NavGetDefaultDialogCreationOptions( &options );
    if(multi) 
	options.optionFlags |= 	kNavAllowMultipleFiles;
    options.location.h = options.location.v = -1;
    if(!caption.isEmpty()) 
	options.windowTitle = CFStringCreateWithCharacters(NULL, (UniChar *)caption.unicode(), 
							   caption.length());
    if(parent) {
	parent = parent->topLevelWidget();
	QString s = parent->caption();
	options.clientName = CFStringCreateWithCharacters(NULL, (UniChar *)s.unicode(), s.length());
	options.location.h = (parent->x() + (parent->width() / 2)) - (w / 2);
	options.location.v = (parent->y() + (parent->height() / 2)) - (h / 2);
    } else if(QWidget *p = qApp->mainWidget()) {
	static int last_screen = -1;
	int scr = QApplication::desktop()->screenNumber(p);
	if(last_screen != scr) {
	    QRect r = QApplication::desktop()->screenGeometry(scr);
	    options.location.h = (r.x() + (r.width() / 2)) - (w / 2);
	    options.location.v = (r.y() + (r.height() / 2)) - (h / 2);
	}
    }

    QPtrList<QRegExp> filts = makeFiltersList(filter);
    NavDialogRef dlg;
    if (directory) {
	if(NavCreateChooseFolderDialog(&options, NULL, NULL, NULL, &dlg)) {
	    qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	    return retstrl;
	}
    } else {
	if(NavCreateGetFileDialog(&options, NULL, NULL, NULL, make_navUPP(), 
				  (void *) (filts.isEmpty() ? NULL : &filts), &dlg)) {
	    qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	    return retstrl;
	}
    }
    NavDialogRun(dlg);
    if (!(NavDialogGetUserAction(dlg) & 
	  (kNavUserActionOpen | kNavUserActionChoose | kNavUserActionNewFolder))) {
	NavDialogDispose(dlg);
	return retstrl;
    }
    NavReplyRecord ret;
    NavDialogGetReply(dlg, &ret);
    NavDialogDispose(dlg);

    long count;
    err = AECountItems(&(ret.selection), &count);
    if(!ret.validRecord || err != noErr || !count) {
	NavDisposeReply(&ret);
	return retstrl;
    }

    AEKeyword	keyword;
    DescType    type;
    Size        size;
    FSRef ref;
#ifdef Q_WS_MAC9
    FSSpec      spec;
#endif

    for(long index = 1; index <= count; index++) {
#ifdef Q_WS_MAC9
	err = AEGetNthPtr(&(ret.selection), index, typeFSS, &keyword,
			  &type,&spec, sizeof(spec), &size);

#else
	err = AEGetNthPtr(&(ret.selection), index, typeFSRef, &keyword,
			  &type,&ref, sizeof(ref), &size);
#endif
	if(err != noErr)
	    break;

#ifdef Q_WS_MAC9
	//we must *try* to create a file, and remove it if successfull
	//to actually get a path, bogus? I think so.
	bool delete_file = (FSpCreate(&spec, 'CUTE', 'TEXT', smSystemScript) == noErr);
	FSpMakeFSRef(&spec, &ref);
#endif
	if(!str_buffer) {
	    qAddPostRoutine( cleanup_str_buffer );
	    str_buffer = (UInt8 *)malloc(1024);
	}
	FSRefMakePath(&ref, str_buffer, 1024);
#ifdef Q_WS_MAC9
	if(delete_file) 
	    FSpDelete(&spec);
#endif
	tmpstr = QString::fromUtf8((const char *)str_buffer);
	retstrl.append(tmpstr);
    }
    NavDisposeReply(&ret);
    return retstrl;
}

QString QFileDialog::macGetSaveFileName( const QString &, const QString &, 
					 QString *,
					 QWidget *parent, const char* /*name*/,
					 const QString& caption )
{
    OSErr err;
    QString retstr;
    NavDialogCreationOptions options;
    NavGetDefaultDialogCreationOptions( &options );
    static const int w = 450, h = 350;
    options.optionFlags &= ~kNavDontConfirmReplacement;
    options.modality = kWindowModalityAppModal;
    options.location.h = options.location.v = -1;
    if(!caption.isEmpty())
	options.windowTitle = CFStringCreateWithCharacters(NULL, (UniChar *)caption.unicode(), 
							   caption.length());
    if(parent) {
	options.parentWindow = (WindowRef)parent->handle();
	parent = parent->topLevelWidget();
	QString s = parent->caption();
	options.clientName = CFStringCreateWithCharacters(NULL, (UniChar *)s.unicode(), s.length());
	options.location.h = (parent->x() + (parent->width() / 2)) - (w / 2);
	options.location.v = (parent->y() + (parent->height() / 2)) - (h / 2);
    } else if(QWidget *p = qApp->mainWidget()) {
	static int last_screen = -1;
	int scr = QApplication::desktop()->screenNumber(p);
	if(last_screen != scr) {
	    QRect r = QApplication::desktop()->screenGeometry(scr);
	    options.location.h = (r.x() + (r.width() / 2)) - (w / 2);
	    options.location.v = (r.y() + (r.height() / 2)) - (h / 2);
	}
    }

    NavDialogRef dlg;
    if(NavCreatePutFileDialog(&options, 'cute', kNavGenericSignature, NULL, NULL, &dlg)) {
	qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	return retstr;
    }
    NavDialogRun(dlg);
    if(NavDialogGetUserAction(dlg) != kNavUserActionSaveAs) {
	NavDialogDispose(dlg);
	return retstr;
    }
    NavReplyRecord ret;
    NavDialogGetReply(dlg, &ret);
    NavDialogDispose(dlg);

    long count;
    err = AECountItems(&(ret.selection), &count);
    if(!ret.validRecord || err != noErr || !count) {
	NavDisposeReply(&ret);
	return retstr;
    }

    AEKeyword	keyword;
    DescType    type;
    Size        size;
    FSRef ref;
#ifdef Q_WS_MAC9
    FSSpec      spec;
    err = AEGetNthPtr(&(ret.selection), 1, typeFSS, &keyword,
		      &type, &spec, sizeof(spec), &size);
#else
    err = AEGetNthPtr(&(ret.selection), 1, typeFSRef, &keyword,
		      &type, &ref, sizeof(ref), &size);
    qDebug("%d", err);
#endif    
    if(err == noErr) {
#ifdef Q_WS_MAC9
	//we must *try* to create a file, and remove it if successfull
	//to actually get a path, bogus? I think so.
	bool delete_file = (FSpCreate(&spec, 'CUTE', 'TEXT', smSystemScript) == noErr);
	FSpMakeFSRef(&spec, &ref);
#endif
	if(!str_buffer) {
	    qAddPostRoutine( cleanup_str_buffer );
	    str_buffer = (UInt8 *)malloc(1024);
	}
	FSRefMakePath(&ref, str_buffer, 1024);
#ifdef Q_WS_MAC9
	if(delete_file) 
	    FSpDelete(&spec);
#endif
	retstr = QString::fromUtf8((const char *)str_buffer);
	//now filename
	CFStringGetCString(ret.saveFileName, (char *)str_buffer, 1024, kCFStringEncodingUTF8);
	retstr += "/" + QString::fromUtf8((const char *)str_buffer);
    }
    NavDisposeReply(&ret);
    return retstr;
}

#endif
