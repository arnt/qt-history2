/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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

#include <qapplication.h>
#include <private/qapplication_p.h>
#include <private/qt_mac_p.h>
#include <qregexp.h>
#include <qbuffer.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qdesktopwidget.h>
#include <stdlib.h>

/*****************************************************************************
  Externals
 *****************************************************************************/
extern WindowPtr qt_mac_window_for(const QWidget*); //qwidget_mac.cpp
extern QStringList qt_make_filter_list(const QString &filter); //qfiledialog.cpp
extern const char *qt_file_dialog_filter_reg_exp; //qfiledialog.cpp

/*****************************************************************************
  QFileDialog utility functions
 *****************************************************************************/
static UInt8 *str_buffer = NULL;
static void cleanup_str_buffer()
{
    if(str_buffer) {
        free(str_buffer);
        str_buffer = NULL;
    }
}

// Returns the wildcard part of a filter.
struct qt_mac_filter_name {
    QString description, regxp, filter;
};

static qt_mac_filter_name *qt_mac_extract_filter(const QString &rawFilter)
{
    qt_mac_filter_name *ret = new qt_mac_filter_name;
    ret->filter = rawFilter;
    QString result = rawFilter;
    QRegExp r(QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    int index = r.indexIn(result);
    if(index >= 0) {
        ret->description = r.cap(1).trimmed();
        result = r.cap(2);
    }
    if(ret->description.isEmpty())
        ret->description = result;
    ret->regxp = result.replace(QChar(' '), QChar(';'));
    return ret;
}

// Makes a list of filters from ;;-separated text.
static QList<qt_mac_filter_name*> qt_mac_make_filters_list(const QString &filter)
{
#ifdef DEBUG_FILEDIALOG_FILTERS
    qDebug("QFileDialog:%d - Got filter (%s)", __LINE__, filter.latin1());
#endif
    QString f(filter);
    if(f.isEmpty())
        f = QObject::tr("All Files (*)");
    if(f.isEmpty())
        return QList<qt_mac_filter_name*>();
/*
    QString sep(";;");
    int i = f.indexOf(sep, 0);
    if(i == -1) {
        sep = "\n";
        if(f.indexOf(sep, 0) != -1)
            i = f.indexOf(sep, 0);
    }
    QStringList filts = f.split(sep);
*/
    QStringList filts = qt_make_filter_list(f);
    QList<qt_mac_filter_name*> ret;
    for (QStringList::Iterator it = filts.begin(); it != filts.end(); ++it) {
        qt_mac_filter_name *filter = qt_mac_extract_filter((*it));
#ifdef DEBUG_FILEDIALOG_FILTERS
        qDebug("QFileDialog:%d Split out filter (%d) '%s' '%s'", __LINE__, ret.count(),
               filter->regxp.latin1(), filter->description.latin1());
#endif
        ret.append(filter);
    }
    return ret;
}

struct qt_mac_nav_filter_type {
    int index;
    QList<qt_mac_filter_name*> *filts;
};

static Boolean qt_mac_nav_filter(AEDesc *theItem, void *info,
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
        int slsh = file.lastIndexOf('/');
        if(slsh != -1)
            file = file.right(file.length() - slsh - 1);
    }
    QStringList reg = fn->regxp.split(";");
    for(QStringList::Iterator it = reg.begin(); it != reg.end(); ++it) {
        QRegExp rg(*it, Qt::CaseInsensitive, QRegExp::Wildcard);
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
static bool g_nav_blocking=true;
static void qt_mac_filedialog_event_proc(const NavEventCallbackMessage msg,
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
        g_nav_blocking=true;
        break;
    case kNavCBUserAction:
        g_nav_blocking=false;
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

OSErr qt_mac_create_fsspec(const QString &path, FSSpec *spec); //qglobal.cpp

QStringList qt_mac_get_open_file_names(const QString &filter, QString *pwd,
                                       QWidget *parent, const QString &caption,
                                       QString *selectedFilter,
                                       bool multi, bool directory)
{
    OSErr err;
    QStringList retstrl;

    NavDialogCreationOptions options;
    NavGetDefaultDialogCreationOptions(&options);
    options.modality = kWindowModalityAppModal;
    options.optionFlags |= kNavDontConfirmReplacement | kNavSupportPackages;
    if(multi)
        options.optionFlags |=         kNavAllowMultipleFiles;
    if(!caption.isEmpty())
        options.windowTitle = CFStringCreateWithCharacters(NULL, (UniChar *)caption.unicode(),
                                                           caption.length());

    static const int w = 450, h = 350;
    options.location.h = options.location.v = -1;
    if(parent && parent->isVisible()) {
        if(!parent->topLevelWidget()->isDesktop()) {
            options.modality = kWindowModalityWindowModal;
            options.parentWindow = qt_mac_window_for(parent);
        } else {
            parent = parent->topLevelWidget();
            QString s = parent->windowTitle();
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

    QList<qt_mac_filter_name*> filts = qt_mac_make_filters_list(filter);
    qt_mac_nav_filter_type t;
    t.index = 0;
    t.filts = &filts;
    if(filts.count() > 1) {
        int i = 0;
        CFStringRef *arr = (CFStringRef *)malloc(sizeof(CFStringRef) * filts.count());
        for (QList<qt_mac_filter_name*>::Iterator it = filts.begin(); it != filts.end(); ++it) {
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
    if(pwd && !pwd->isEmpty()) {
        FSSpec spec;
        if(qt_mac_create_fsspec(*pwd, &spec) == noErr) {
            AEDesc desc;
            if(AECreateDesc(typeFSS, &spec, sizeof(FSSpec), &desc) == noErr)
                NavCustomControl(dlg, kNavCtlSetLocation, (void*)&desc);
        }
    }

    NavDialogRun(dlg);
    if(options.modality == kWindowModalityWindowModal) { //simulate modality
        QWidget modal_widg(parent, Qt::WType_TopLevel | Qt::WStyle_Customize | 
                           Qt::WStyle_DialogBorder | Qt::WMacSheet);
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

    for(long index = 1; index <= count; index++) {
        FSRef ref;
        err = AEGetNthPtr(&(ret.selection), index, typeFSRef, 0, 0, &ref, sizeof(ref), 0);
        if(err != noErr)
            break;

        if(!str_buffer) {
            qAddPostRoutine(cleanup_str_buffer);
            str_buffer = (UInt8 *)malloc(1024);
        }
        FSRefMakePath(&ref, str_buffer, 1024);
        retstrl.append(QString::fromUtf8((const char *)str_buffer));
    }
    NavDisposeReply(&ret);
    if(selectedFilter)
        *selectedFilter = filts.at(t.index)->filter;
    while (!filts.isEmpty())
        delete filts.takeFirst();
    return retstrl;
}

QString qt_mac_get_save_file_name(const QString &start, const QString &filter,
                                  QString *pwd, QWidget *parent, const QString &caption,
                                  QString *selectedFilter)
{
    OSErr err;
    QString retstr;
    NavDialogCreationOptions options;
    NavGetDefaultDialogCreationOptions(&options);
    static const int w = 450, h = 350;
    options.optionFlags |= kNavDontConfirmReplacement;
    options.modality = kWindowModalityAppModal;
    options.location.h = options.location.v = -1;
    if (!start.isEmpty())
        options.saveFileName = CFStringCreateWithCharacters(0,
                                                            (UniChar *)start.unicode(),
                                                            start.length());
    if(!caption.isEmpty())
        options.windowTitle = CFStringCreateWithCharacters(NULL, (UniChar *)caption.unicode(),
                                                           caption.length());
    if(parent && parent->isVisible()) {
        if(!parent->topLevelWidget()->isDesktop()) {
            options.modality = kWindowModalityWindowModal;
            options.parentWindow = qt_mac_window_for(parent);
        } else {
            parent = parent->topLevelWidget();
            QString s = parent->windowTitle();
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

    QList<qt_mac_filter_name*> filts = qt_mac_make_filters_list(filter);
    qt_mac_nav_filter_type t;
    t.index = 0;
    t.filts = &filts;
    if(filts.count() > 1) {
        int i = 0;
        CFStringRef *arr = (CFStringRef *)malloc(sizeof(CFStringRef) * filts.count());
        for (QList<qt_mac_filter_name*>::Iterator it = filts.begin(); it != filts.end(); ++it) {
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
    if (pwd && !pwd->isEmpty()) {
        FSSpec spec;
        if (qt_mac_create_fsspec(*pwd, &spec) == noErr) {
            AEDesc desc;
            if (AECreateDesc(typeFSS, &spec, sizeof(FSSpec), &desc) == noErr)
                NavCustomControl(dlg, kNavCtlSetLocation, (void*)&desc);
        }
    }
    NavDialogRun(dlg);
    if(options.modality == kWindowModalityWindowModal) { //simulate modality
        QWidget modal_widg(parent, Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_DialogBorder);
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

    AEKeyword        keyword;
    DescType    type;
    Size        size;
    FSRef ref;
    err = AEGetNthPtr(&(ret.selection), 1, typeFSRef, &keyword,
                      &type, &ref, sizeof(ref), &size);
    if(err == noErr) {
        if(!str_buffer) {
            qAddPostRoutine(cleanup_str_buffer);
            str_buffer = (UInt8 *)malloc(1024);
        }
        FSRefMakePath(&ref, str_buffer, 1024);
        retstr = QString::fromUtf8((const char *)str_buffer);
        //now filename
        CFStringGetCString(ret.saveFileName, (char *)str_buffer, 1024, kCFStringEncodingUTF8);
        retstr += "/" + QString::fromUtf8((const char *)str_buffer);
    }
    NavDisposeReply(&ret);
    if(selectedFilter)
        *selectedFilter = filts.at(t.index)->filter;
    while (!filts.isEmpty())
        delete filts.takeFirst();
    return retstr;
}

#endif
