/****************************************************************************
**
** Implementation of QFileDialog classes for mac.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

/*****************************************************************************
  QFileDialog debug facilities
 *****************************************************************************/
//#define DEBUG_FILEDIALOG_FILTERS

#include "qapplication.h"
#include <private/qapplication_p.h>
#include "qt_mac.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qstringlist.h"
#include "qtextcodec.h"
#include "qdesktopwidget.h"
#include <stdlib.h>

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
struct qt_mac_filter_name {
    QString description, regxp, filter;
};
static qt_mac_filter_name *extractFilter( const QString& rawFilter)
{
    qt_mac_filter_name *ret = new qt_mac_filter_name;
    ret->filter = rawFilter;
    QString result = rawFilter;
    QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    int index = r.search(result);
    if( index >= 0) {
	ret->description = r.cap(1).stripWhiteSpace();
	result = r.cap(2);
    }
    if(ret->description.isEmpty()) 
	ret->description = result;
    ret->regxp = result.replace(QChar(' '), QChar(';'));
    return ret;
}

// Makes a list of filters from ;;-separated text.
static QPtrList<qt_mac_filter_name> makeFiltersList(const QString &filter)
{
#ifdef DEBUG_FILEDIALOG_FILTERS
    qDebug("QFileDialog:%d - Got filter (%s)", __LINE__, filter.latin1());
#endif
    QString f(filter);
    if(f.isEmpty())
	f = QFileDialog::tr("All Files (*)");
    if(f.isEmpty())
	return QPtrList<qt_mac_filter_name>();
    QString sep(";;");
    int i = f.find(sep, 0);
    if(i == -1) {
	sep = "\n";
	if(f.find(sep, 0) != -1)
	    i = f.find(sep, 0);
    }

    QPtrList<qt_mac_filter_name> ret;
    QStringList filts = QStringList::split(sep, f);
    for (QStringList::Iterator it = filts.begin(); it != filts.end(); ++it) {
	qt_mac_filter_name *filter = extractFilter((*it));
#ifdef DEBUG_FILEDIALOG_FILTERS
	qDebug("QFileDialog:%d Split out filter (%d) '%s' '%s'", __LINE__, ret.count(),
	       filter->regxp.latin1(), filter->description.latin1());
#endif
	ret.append(filter);
    }
    return ret;
}

struct qt_mac_nav_filter_type {
    unsigned int index;
    QPtrList<qt_mac_filter_name> *filts;
};

static QMAC_PASCAL Boolean qt_mac_nav_filter(AEDesc *theItem, void *info,
					     void *myd, NavFilterModes)
{
    qt_mac_nav_filter_type *t = (qt_mac_nav_filter_type *)myd;
    if(!t || !t->filts || t->index >= t->filts->count())
	return true;

    NavFileOrFolderInfo *theInfo = (NavFileOrFolderInfo *)info;
    QString file;
    qt_mac_filter_name *fn = t->filts->at(t->index);
    if(!fn)
	return true;
    if(theItem->descriptorType == typeFSS) {
	AliasHandle alias;
	Str63 str;
	FSSpec      FSSpec;
	AliasInfoType x = 0;
	AEGetDescData(theItem, &FSSpec, sizeof(FSSpec));
	if(NewAlias(NULL, &FSSpec, &alias) != noErr)
	    return true;
	GetAliasInfo(alias, (AliasInfoType)x++, str);
	if(str[0]) {
	    char tmp[sizeof(Str63)+2];
	    strncpy((char *)tmp, (const char *)str+1, str[0]);
	    tmp[str[0]] = '\0';
	    file = tmp;
	}
    } else if(theItem->descriptorType == typeFSRef) {
	FSRef ref;
	AEGetDescData(theItem, &ref, sizeof(ref));
	if(!str_buffer) {
	    qAddPostRoutine(cleanup_str_buffer);
	    str_buffer = (UInt8 *)malloc(1024);
	}
	FSRefMakePath(&ref, str_buffer, 1024);
	file = QString::fromUtf8((const char *)str_buffer);
	int slsh = file.findRev('/');
	if(slsh != -1) 
	    file = file.right(file.length() - slsh - 1);
    }
    QStringList reg = QStringList::split(";", fn->regxp);
    for(QStringList::Iterator it = reg.begin(); it != reg.end(); ++it) {
	QRegExp rg((*it), TRUE, TRUE);
#ifdef DEBUG_FILEDIALOG_FILTERS
	qDebug("QFileDialog:%d, asked to filter.. %s (%s)", __LINE__, 
	       file.latin1(), (*it).latin1());
#endif
	if(rg.exactMatch(file)) 
	    return true;
    }
    return (theInfo->isFolder && !file.endsWith(".app"));
}

//filter UPP stuff
static NavObjectFilterUPP mac_navFilterUPP = NULL;
static void cleanup_navFilterUPP()
{
    DisposeNavObjectFilterUPP(mac_navFilterUPP);
    mac_navFilterUPP = NULL;
}
static const NavObjectFilterUPP make_navFilterUPP()
{
    if(mac_navFilterUPP)
	return mac_navFilterUPP;
    qAddPostRoutine(cleanup_navFilterUPP);
    return mac_navFilterUPP = NewNavObjectFilterUPP(qt_mac_nav_filter);
}
//event UPP stuff
static NavEventUPP mac_navProcUPP = NULL;
static void cleanup_navProcUPP()
{
    DisposeNavEventUPP(mac_navProcUPP);
    mac_navProcUPP = NULL;
}
static bool g_nav_blocking=TRUE;
static QMAC_PASCAL void qt_mac_filedialog_event_proc(const NavEventCallbackMessage msg, 
						     NavCBRecPtr p, NavCallBackUserData myd)
{ 
    switch(msg) {
    case kNavCBPopupMenuSelect: {
	qt_mac_nav_filter_type *t = (qt_mac_nav_filter_type *)myd;
	NavMenuItemSpec *s = (NavMenuItemSpec*)p->eventData.eventDataParms.param;
	t->index = s->menuType;
#ifdef DEBUG_FILEDIALOG_FILTERS
	qDebug("QFileDialog:%d - Selected a filter: %ld", __LINE__, s->menuType);
#endif
	break; }
    case kNavCBStart:
	g_nav_blocking=TRUE;
	break;
    case kNavCBUserAction:
	g_nav_blocking=FALSE;
	break;
    }
}
static const NavEventUPP make_navProcUPP()
{
    if(mac_navProcUPP)
	return mac_navProcUPP;
    qAddPostRoutine(cleanup_navProcUPP);
    return mac_navProcUPP = NewNavEventUPP(qt_mac_filedialog_event_proc);
}


const unsigned char * p_str(const char *, int len=-1);
QMAC_PASCAL OSErr FSpLocationFromFullPath(short fullPathLength,
				      const void *fullPath,
				      FSSpec *spec);




QStringList QFileDialog::macGetOpenFileNames(const QString &filter, QString *,
					     QWidget *parent, const char* /*name*/,
					     const QString& caption, QString *selectedFilter,
					     bool multi, bool directory)
{
    OSErr err;
    QStringList retstrl;

    NavDialogCreationOptions options;
    NavGetDefaultDialogCreationOptions(&options);
    options.modality = kWindowModalityAppModal;
    options.optionFlags |= kNavDontConfirmReplacement | kNavSupportPackages;
    if(multi) 
	options.optionFlags |= 	kNavAllowMultipleFiles;
    if(!caption.isEmpty()) 
	options.windowTitle = CFStringCreateWithCharacters(NULL, (UniChar *)caption.unicode(), 
							   caption.length());

    static const int w = 450, h = 350;
    options.location.h = options.location.v = -1;
    if(parent && parent->isVisible()) {
	if(!parent->topLevelWidget()->isDesktop()) {
	    options.modality = kWindowModalityWindowModal;
	    options.parentWindow = (WindowRef)parent->handle();
	} else {
	    parent = parent->topLevelWidget();
	    QString s = parent->caption();
	    options.clientName = CFStringCreateWithCharacters(NULL, (UniChar *)s.unicode(), s.length());
	    options.location.h = (parent->x() + (parent->width() / 2)) - (w / 2);
	    options.location.v = (parent->y() + (parent->height() / 2)) - (h / 2);

	    QRect r = QApplication::desktop()->screenGeometry(
		QApplication::desktop()->screenNumber(parent));
	    if(options.location.h + w > r.right())
		options.location.h -= (options.location.h + w) - r.right() + 10;
	    if(options.location.v + h > r.bottom())
		options.location.v -= (options.location.v + h) - r.bottom() + 10;
	}
    } else if(QWidget *p = qApp->mainWidget()) {
	static int last_screen = -1;
	int scr = QApplication::desktop()->screenNumber(p);
	if(last_screen != scr) {
	    QRect r = QApplication::desktop()->screenGeometry(scr);
	    options.location.h = (r.x() + (r.width() / 2)) - (w / 2);
	    options.location.v = (r.y() + (r.height() / 2)) - (h / 2);
	}
    }

    QPtrList<qt_mac_filter_name> filts = makeFiltersList(filter);
    qt_mac_nav_filter_type t;
    t.index = 0;
    t.filts = &filts;
    filts.setAutoDelete(TRUE);
    if(filts.count() > 1) {
	int i = 0;
	CFStringRef *arr = (CFStringRef *)malloc(sizeof(CFStringRef) * filts.count());
	for (QPtrListIterator<qt_mac_filter_name> it(filts); it.current(); ++it) {
	    QString rg = (*it)->description;
	    arr[i++] = CFStringCreateWithCharacters(NULL, (UniChar *)rg.unicode(), rg.length());
	}
	options.popupExtension = CFArrayCreate(NULL, (const void **)arr, filts.count(), NULL);
    }

    NavDialogRef dlg;
    if(directory) {
	if(NavCreateChooseFolderDialog(&options, make_navProcUPP(), NULL, NULL, &dlg)) {
	    qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	    return retstrl;
	}
    } else {
	if(NavCreateGetFileDialog(&options, NULL, make_navProcUPP(), NULL, 
				  make_navFilterUPP(), (void *) (filts.isEmpty() ? NULL : &t), 
				  &dlg)) {
	    qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	    return retstrl;
	}
    }
    NavDialogRun(dlg);
    if(options.modality == kWindowModalityWindowModal) { //simulate modality
	QWidget modal_widg(parent, __FILE__ "__modal_dlg", 
			   WType_TopLevel | WStyle_Customize | WStyle_DialogBorder);
	qt_enter_modal(&modal_widg);
	while(g_nav_blocking) 
	    qApp->processEvents();
	qt_leave_modal(&modal_widg);
    }

    if(!(NavDialogGetUserAction(dlg) & 
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
	    qAddPostRoutine(cleanup_str_buffer);
	    str_buffer = (UInt8 *)malloc(1024);
	}
	FSRefMakePath(&ref, str_buffer, 1024);
#ifdef Q_WS_MAC9
	if(delete_file) 
	    FSpDelete(&spec);
#endif
	retstrl.append(QString::fromUtf8((const char *)str_buffer));
    }
    NavDisposeReply(&ret);
    if(selectedFilter) 
	*selectedFilter = filts.at(t.index)->filter;
    return retstrl;
}

QString QFileDialog::macGetSaveFileName(const QString &start, const QString &filter, 
					 QString *, QWidget *parent, const char* /*name*/,
					 const QString& caption, QString *selectedFilter)
{
    OSErr err;
    QString retstr;
    NavDialogCreationOptions options;
    NavGetDefaultDialogCreationOptions(&options);
    static const int w = 450, h = 350;
    options.optionFlags |= kNavDontConfirmReplacement;
    options.modality = kWindowModalityAppModal;
    options.location.h = options.location.v = -1;
    if(!start.isEmpty()) {
	QString startf = start.section(QDir::separator(), -1);
	options.saveFileName = CFStringCreateWithCharacters(NULL, (UniChar *)startf.unicode(), 
							   startf.length());
    }
    if(!caption.isEmpty())
	options.windowTitle = CFStringCreateWithCharacters(NULL, (UniChar *)caption.unicode(), 
							   caption.length());
    if(parent && parent->isVisible()) {
	if(!parent->topLevelWidget()->isDesktop()) {
	    options.modality = kWindowModalityWindowModal;
	    options.parentWindow = (WindowRef)parent->handle();
	} else {
	    parent = parent->topLevelWidget();
	    QString s = parent->caption();
	    options.clientName = CFStringCreateWithCharacters(NULL, (UniChar *)s.unicode(), s.length());
	    options.location.h = (parent->x() + (parent->width() / 2)) - (w / 2);
	    options.location.v = (parent->y() + (parent->height() / 2)) - (h / 2);

	    QRect r = QApplication::desktop()->screenGeometry(
		QApplication::desktop()->screenNumber(parent));
	    if(options.location.h + w > r.right())
		options.location.h -= (options.location.h + w) - r.right() + 10;
	    if(options.location.v + h > r.bottom())
		options.location.v -= (options.location.v + h) - r.bottom() + 10;
	}
    } else if(QWidget *p = qApp->mainWidget()) {
	static int last_screen = -1;
	int scr = QApplication::desktop()->screenNumber(p);
	if(last_screen != scr) {
	    QRect r = QApplication::desktop()->screenGeometry(scr);
	    options.location.h = (r.x() + (r.width() / 2)) - (w / 2);
	    options.location.v = (r.y() + (r.height() / 2)) - (h / 2);
	}
    }

    QPtrList<qt_mac_filter_name> filts = makeFiltersList(filter);
    qt_mac_nav_filter_type t;
    t.index = 0;
    t.filts = &filts;
    filts.setAutoDelete(TRUE);
    if(filts.count() > 1) {
	int i = 0;
	CFStringRef *arr = (CFStringRef *)malloc(sizeof(CFStringRef) * filts.count());
	for (QPtrListIterator<qt_mac_filter_name> it(filts); it.current(); ++it) {
	    QString rg = (*it)->description;
	    arr[i++] = CFStringCreateWithCharacters(NULL, (UniChar *)rg.unicode(), rg.length());
	}
	options.popupExtension = CFArrayCreate(NULL, (const void **)arr, filts.count(), NULL);
    }

    NavDialogRef dlg;
    if(NavCreatePutFileDialog(&options, 'cute', kNavGenericSignature, make_navProcUPP(), 
			      (void *) (filts.isEmpty() ? NULL : &t), &dlg)) {
	qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	return retstr;
    }
    NavDialogRun(dlg);
    if(options.modality == kWindowModalityWindowModal) { //simulate modality
	QWidget modal_widg(parent, __FILE__ "__modal_dlg", 
			   WType_TopLevel | WStyle_Customize | WStyle_DialogBorder);
	qt_enter_modal(&modal_widg);
	while(g_nav_blocking) 
	    qApp->processEvents();
	qt_leave_modal(&modal_widg);
    }

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
#endif    
    if(err == noErr) {
#ifdef Q_WS_MAC9
	//we must *try* to create a file, and remove it if successfull
	//to actually get a path, bogus? I think so.
	bool delete_file = (FSpCreate(&spec, 'CUTE', 'TEXT', smSystemScript) == noErr);
	FSpMakeFSRef(&spec, &ref);
#endif
	if(!str_buffer) {
	    qAddPostRoutine(cleanup_str_buffer);
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
    if(selectedFilter) 
	*selectedFilter = filts.at(t.index)->filter;
    return retstr;
}

#endif
