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

#include <private/qfiledialog_p.h>
#include <qapplication.h>
#include <private/qapplication_p.h>
#include <qt_windows.h>
#include <qglobal.h>
#include <qregexp.h>
#include <qbuffer.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qlibrary.h>

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

#include "shlobj.h"

#ifdef Q_OS_TEMP
#include "commdlg.h"
#endif


// Don't remove the lines below!
//
// resolving the W methods manually is needed, because Windows 95 doesn't include
// these methods in Shell32.lib (not even stubs!), so you'd get an unresolved symbol
// when Qt calls getEsistingDirectory(), etc.
typedef LPITEMIDLIST (WINAPI *PtrSHBrowseForFolder)(BROWSEINFO*);
static PtrSHBrowseForFolder ptrSHBrowseForFolder = 0;
typedef BOOL (WINAPI *PtrSHGetPathFromIDList)(LPITEMIDLIST,LPWSTR);
static PtrSHGetPathFromIDList ptrSHGetPathFromIDList = 0;

static void qt_win_resolve_libs()
{
#ifndef Q_OS_TEMP
    static bool triedResolve = false;

    if (!triedResolve) {
#ifdef QT_THREAD_SUPPORT
        // protect initialization
        QMutexLocker locker(qt_global_mutexpool ?
                             qt_global_mutexpool->get(&triedResolve) : 0);
        // check triedResolve again, since another thread may have already
        // done the initialization
        if (triedResolve) {
            // another thread did initialize the security function pointers,
            // so we shouldn't do it again.
            return;
        }
#endif

        triedResolve = true;
        if (!(QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)) {
            QLibrary lib("shell32");
            ptrSHBrowseForFolder = (PtrSHBrowseForFolder) lib.resolve("SHBrowseForFolderW");
            ptrSHGetPathFromIDList = (PtrSHGetPathFromIDList) lib.resolve("SHGetPathFromIDListW");
        }
    }
#endif
}
#ifdef Q_OS_TEMP
#define PtrSHBrowseForFolder SHBrowseForFolder ;
#define PtrSHGetPathFromIDList SHGetPathFromIDList;
#endif


extern const char* qt_file_dialog_filter_reg_exp; // defined in qfiledialog.cpp
extern QStringList qt_make_filter_list(const QString &filter);

const int maxNameLen = 1023;
const int maxMultiLen = 65535;

// Returns the wildcard part of a filter.
static QString qt_win_extract_filter(const QString &rawFilter)
{
    QString result = rawFilter;
    QRegExp r(QString::fromLatin1(qt_file_dialog_filter_reg_exp));
    int index = r.indexIn(result);
    if (index >= 0)
        result = r.cap(2);
    return result.replace(QChar(' '), QChar(';'));
}

static QStringList qt_win_make_filters_list(const QString &filter)
{
    QString f(filter);

    if (f.isEmpty())
        f = QObject::tr("All Files (*.*)");

    return qt_make_filter_list(f);
}

// Makes a NUL-oriented Windows filter from a Qt filter.
static QString qt_win_filter(const QString &filter)
{
    QStringList filterLst = qt_win_make_filters_list(filter);
    QStringList::Iterator it = filterLst.begin();
    QString winfilters;
    for (; it != filterLst.end(); ++it) {
        winfilters += *it;
        winfilters += QChar();
        winfilters += qt_win_extract_filter(*it);
        winfilters += QChar();
    }
    winfilters += QChar();
    return winfilters;
}

static QString qt_win_selected_filter(const QString &filter, DWORD idx)
{
    return qt_win_make_filters_list(filter).at((int)idx - 1);
}

#ifndef Q_OS_TEMP
// Static vars for OFNA funcs:
static QByteArray aInitDir;
static QByteArray aInitSel;
static QByteArray aTitle;
static QByteArray aFilter;
// Use ANSI strings and API

// If you change this, then make sure you change qt_win_make_OFN (below) too
static OPENFILENAMEA *qt_win_make_OFNA(QWidget *parent,
				       const QString &initialSelection,
				       const QString &initialDirectory,
				       const QString &title,
				       const QString &filters,
				       QFileDialog::FileMode mode,
				       QFileDialog::Options options)
{
    if (parent)
        parent = parent->window();
    else
        parent = qApp->activeWindow();

    aTitle = title.toLocal8Bit();
    aInitDir = QDir::convertSeparators(initialDirectory).toLocal8Bit();
    if (initialSelection.isEmpty()) {
        aInitSel = "";
    } else {
        aInitSel = QDir::convertSeparators(initialSelection).toLocal8Bit();
	aInitSel.replace("<", "");
	aInitSel.replace(">", "");
	aInitSel.replace("\"", "");
	aInitSel.replace("|", "");
    }
    int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    aInitSel.resize(maxLen + 1);                // make room for return value
    aFilter = filters.toLocal8Bit();

    OPENFILENAMEA* ofn = new OPENFILENAMEA;
    memset(ofn, 0, sizeof(OPENFILENAMEA));

#if defined(Q_CC_BOR) && (WINVER >= 0x0500) && (_WIN32_WINNT >= 0x0500)
    // according to the MSDN, this should also be necessary for MSVC, but
    // OPENFILENAME_SIZE_VERSION_400A is in not Microsoft header, as it seems
    if (QApplication::winVersion()==Qt::WV_NT || QApplication::winVersion()&Qt::WV_DOS_based) {
        ofn->lStructSize = OPENFILENAME_SIZE_VERSION_400A;
    } else {
        ofn->lStructSize = sizeof(OPENFILENAMEA);
    }
#else
    ofn->lStructSize = sizeof(OPENFILENAMEA);
#endif
    ofn->hwndOwner = parent ? parent->winId() : 0;
    ofn->lpstrFilter = aFilter;
    ofn->lpstrFile = aInitSel.data();
    ofn->nMaxFile = maxLen;
    ofn->lpstrInitialDir = aInitDir.data();
    ofn->lpstrTitle = aTitle.data();
    ofn->Flags = (OFN_NOCHANGEDIR | OFN_HIDEREADONLY);

    if (mode == QFileDialog::ExistingFile ||
         mode == QFileDialog::ExistingFiles)
        ofn->Flags |= (OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
    if (mode == QFileDialog::ExistingFiles)
        ofn->Flags |= (OFN_ALLOWMULTISELECT | OFN_EXPLORER);
    if (!(options & QFileDialog::DontConfirmOverwrite))
        ofn->Flags |= OFN_OVERWRITEPROMPT;

    return ofn;
}

static void qt_win_clean_up_OFNA(OPENFILENAMEA **ofn)
{
    delete *ofn;
    *ofn = 0;
}
#endif

static QString tFilters, tTitle, tInitDir;

#ifdef UNICODE
// If you change this, then make sure you change qt_win_make_OFNA (above) too
static OPENFILENAME* qt_win_make_OFN(QWidget *parent,
                                     const QString& initialSelection,
                                     const QString& initialDirectory,
                                     const QString& title,
                                     const QString& filters,
                                     QFileDialog::FileMode mode,
				     QFileDialog::Options options)
{
    if (parent)
        parent = parent->window();
    else
        parent = qApp->activeWindow();

    tInitDir = QDir::convertSeparators(initialDirectory);
    tFilters = filters;
    tTitle = title;
    QString initSel = QDir::convertSeparators(initialSelection);
    if (!initSel.isEmpty()) {
	initSel.replace("<", "");
	initSel.replace(">", "");
	initSel.replace("\"", "");
	initSel.replace("|", "");
    }

    int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    TCHAR *tInitSel = new TCHAR[maxLen+1];
    if (initSel.length() > 0 && initSel.length() <= maxLen)
        memcpy(tInitSel, initSel.utf16(), (initSel.length()+1)*sizeof(QChar));
    else
        tInitSel[0] = 0;

    OPENFILENAME* ofn = new OPENFILENAME;
    memset(ofn, 0, sizeof(OPENFILENAME));

#if defined(Q_CC_BOR) && (WINVER >= 0x0500) && (_WIN32_WINNT >= 0x0500)
    // according to the MSDN, this should also be necessary for MSVC, but
    // OPENFILENAME_SIZE_VERSION_400 is in not Microsoft header, as it seems
    if (QApplication::winVersion()==Qt::WV_NT || QApplication::winVersion()&Qt::WV_DOS_based) {
        ofn->lStructSize= OPENFILENAME_SIZE_VERSION_400;
    } else {
        ofn->lStructSize = sizeof(OPENFILENAME);
    }
#else
    ofn->lStructSize = sizeof(OPENFILENAME);
#endif
    ofn->hwndOwner = parent ? parent->winId() : 0;
    ofn->lpstrFilter = (TCHAR *)tFilters.utf16();
    ofn->lpstrFile = tInitSel;
    ofn->nMaxFile = maxLen;
    ofn->lpstrInitialDir = (TCHAR *)tInitDir.utf16();
    ofn->lpstrTitle = (TCHAR *)tTitle.utf16();
    ofn->Flags = (OFN_NOCHANGEDIR | OFN_HIDEREADONLY);

    if (mode == QFileDialog::ExistingFile ||
         mode == QFileDialog::ExistingFiles)
        ofn->Flags |= (OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
    if (mode == QFileDialog::ExistingFiles)
        ofn->Flags |= (OFN_ALLOWMULTISELECT | OFN_EXPLORER);
    if (!(options & QFileDialog::DontConfirmOverwrite))
        ofn->Flags |= OFN_OVERWRITEPROMPT;

    return ofn;
}


static void qt_win_clean_up_OFN(OPENFILENAME **ofn)
{
    delete (*ofn)->lpstrFile;
    delete *ofn;
    *ofn = 0;
}

#endif // UNICODE

extern void qt_win_eatMouseMove();

QString qt_win_get_open_file_name(const QFileDialogArgs &args,
                                  QString *initialDirectory,
                                  QString *selectedFilter)
{
    QString result;

    QString isel = args.selection;

    if (initialDirectory && initialDirectory->left(5) == "file:")
        initialDirectory->remove(0, 5);
    QFileInfo fi(*initialDirectory);

    if (initialDirectory && !fi.isDir()) {
        *initialDirectory = fi.absolutePath();
        if (isel.isEmpty())
            isel = fi.fileName();
    }

    if (!fi.exists())
        *initialDirectory = QDir::homePath();

    QString title = args.caption;
    if (title.isNull())
        title = QObject::tr("Open");

    DWORD selFilIdx;

    int idx = 0;
    if (selectedFilter) {
        QStringList filterLst = qt_win_make_filters_list(args.filter);
        idx = filterLst.indexOf(*selectedFilter);
    }

    if (args.parent) {
        QApplicationPrivate::enterModal(args.parent);
    }
    QT_WA({
        // Use Unicode strings and API
        OPENFILENAME* ofn = qt_win_make_OFN(args.parent, args.selection,
                                            args.directory, args.caption,
                                            qt_win_filter(args.filter),
					    QFileDialog::ExistingFile,
					    args.options);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetOpenFileName(ofn)) {
            result = QString::fromUtf16((ushort*)ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
        }
        qt_win_clean_up_OFN(&ofn);
    } , {
        // Use ANSI strings and API
        OPENFILENAMEA* ofn = qt_win_make_OFNA(args.parent, args.selection,
                                              args.directory, args.caption,
                                              qt_win_filter(args.filter),
					      QFileDialog::ExistingFile,
					      args.options);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetOpenFileNameA(ofn)) {
            result = QString::fromLocal8Bit(ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
        }
        qt_win_clean_up_OFNA(&ofn);
    });
    if (args.parent) {
        QApplicationPrivate::leaveModal(args.parent);
    }

    qt_win_eatMouseMove();

    if (result.isEmpty())
        return result;

    fi = result;
    *initialDirectory = fi.path();
    if (selectedFilter)
        *selectedFilter = qt_win_selected_filter(args.filter, selFilIdx);
    return fi.absoluteFilePath();
}

QString qt_win_get_save_file_name(const QFileDialogArgs &args,
                                  QString *initialDirectory,
				  QString *selectedFilter)
{
    QString result;

    QString isel = args.selection;
    if (initialDirectory && initialDirectory->left(5) == "file:")
        initialDirectory->remove(0, 5);
    QFileInfo fi(*initialDirectory);

    if (initialDirectory && !fi.isDir()) {
        *initialDirectory = fi.absolutePath();
        if (isel.isEmpty())
            isel = fi.fileName();
    }

    if (!fi.exists())
        *initialDirectory = QDir::homePath();

    QString title = args.caption;
    if (title.isNull())
        title = QObject::tr("Save As");

    DWORD selFilIdx;

    int idx = 0;
    if (selectedFilter) {
        QStringList filterLst = qt_win_make_filters_list(args.filter);
        idx = filterLst.indexOf(*selectedFilter);
    }

    if (args.parent) {
        QApplicationPrivate::enterModal(args.parent);
    }
    QT_WA({
        // Use Unicode strings and API
        OPENFILENAME *ofn = qt_win_make_OFN(args.parent, args.selection,
                                            args.directory, args.caption,
                                            qt_win_filter(args.filter),
					    QFileDialog::AnyFile,
					    args.options);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetSaveFileName(ofn)) {
            result = QString::fromUtf16((ushort*)ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
        }
        qt_win_clean_up_OFN(&ofn);
    } , {
        // Use ANSI strings and API
        OPENFILENAMEA *ofn = qt_win_make_OFNA(args.parent, args.selection,
                                              args.directory, args.caption,
                                              qt_win_filter(args.filter),
					      QFileDialog::AnyFile,
					      args.options);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetSaveFileNameA(ofn)) {
            result = QString::fromLocal8Bit(ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
        }
        qt_win_clean_up_OFNA(&ofn);
    });
    if (args.parent) {
        QApplicationPrivate::leaveModal(args.parent);
    }

    qt_win_eatMouseMove();

    if (result.isEmpty())
        return result;

    fi = result;
    *initialDirectory = fi.path();
    if (selectedFilter)
        *selectedFilter = qt_win_selected_filter(args.filter, selFilIdx);
    return fi.absoluteFilePath();
}

QStringList qt_win_get_open_file_names(const QFileDialogArgs &args,
                                       QString *initialDirectory,
                                       QString *selectedFilter)
{
    QStringList result;
    QFileInfo fi;
    QDir dir;
    QString isel;

    if (initialDirectory && initialDirectory->left(5) == "file:")
        initialDirectory->remove(0, 5);
    fi = QFileInfo(*initialDirectory);

    if (initialDirectory && !fi.isDir()) {
        *initialDirectory = fi.absolutePath();
        isel = fi.fileName();
    }

    if (!fi.exists())
        *initialDirectory = QDir::homePath();

    QString title = args.caption;
    if (title.isNull())
        title = QObject::tr("Open ");

    DWORD selFilIdx;

    int idx = 0;
    if (selectedFilter) {
        QStringList filterLst = qt_win_make_filters_list(args.filter);
        idx = filterLst.indexOf(*selectedFilter);
    }

    if (args.parent) {
        QApplicationPrivate::enterModal(args.parent);
    }
    QT_WA({
        OPENFILENAME* ofn = qt_win_make_OFN(args.parent, args.selection,
                                            args.directory, title,
                                            qt_win_filter(args.filter),
					    QFileDialog::ExistingFiles,
					    args.options);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetOpenFileName(ofn)) {
            QString fileOrDir = QString::fromUtf16((ushort*)ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
            int offset = fileOrDir.length() + 1;
            if (ofn->lpstrFile[offset] == 0) {
                // Only one file selected; has full path
                fi.setFile(fileOrDir);
                QString res = fi.absoluteFilePath();
                if (!res.isEmpty())
                    result.append(res);
            }
            else {
                // Several files selected; first string is path
                dir.setPath(fileOrDir);
                QString f;
                while(!(f = QString::fromUtf16((ushort*)ofn->lpstrFile+offset)).isEmpty()) {
                    fi.setFile(dir, f);
                    QString res = fi.absoluteFilePath();
                    if (!res.isEmpty())
                        result.append(res);
                    offset += f.length() + 1;
                }
            }
        }
        qt_win_clean_up_OFN(&ofn);
    } , {
        OPENFILENAMEA* ofn = qt_win_make_OFNA(args.parent, args.selection,
                                              args.directory, args.caption,
                                              qt_win_filter(args.filter),
					      QFileDialog::ExistingFiles,
					      args.options);
        if (idx)
            ofn->nFilterIndex = idx + 1;
        if (GetOpenFileNameA(ofn)) {
            QByteArray fileOrDir(ofn->lpstrFile);
            selFilIdx = ofn->nFilterIndex;
            int offset = fileOrDir.length() + 1;
            if (ofn->lpstrFile[offset] == '\0') {
                // Only one file selected; has full path
                fi.setFile(QString::fromLocal8Bit(fileOrDir));
                QString res = fi.absoluteFilePath();
                if (!res.isEmpty())
                    result.append(res);
            }
            else {
                // Several files selected; first string is path
                dir.setPath(QString::fromLocal8Bit(fileOrDir));
                QByteArray f;
                while (!(f = QByteArray(ofn->lpstrFile + offset)).isEmpty()) {
                    fi.setFile(dir, QString::fromLocal8Bit(f));
                    QString res = fi.absoluteFilePath();
                    if (!res.isEmpty())
                        result.append(res);
                    offset += f.length() + 1;
                }
            }
            qt_win_clean_up_OFNA(&ofn);
        }
    });
    if (args.parent) {
        QApplicationPrivate::leaveModal(args.parent);
    }

    qt_win_eatMouseMove();

    if (!result.isEmpty()) {
        *initialDirectory = fi.path();    // only save the path if there is a result
        if (selectedFilter)
            *selectedFilter = qt_win_selected_filter(args.filter, selFilIdx);
    }
    return result;
}

// MFC Directory Dialog. Contrib: Steve Williams (minor parts from Scott Powers)

static int __stdcall winGetExistDirCallbackProc(HWND hwnd,
                                                UINT uMsg,
                                                LPARAM lParam,
                                                LPARAM lpData)
{
#ifndef Q_OS_TEMP
    if (uMsg == BFFM_INITIALIZED && lpData != 0) {
        QString *initDir = (QString *)(lpData);
        if (!initDir->isEmpty()) {
            // ### Lars asks: is this correct for the A version????
            QT_WA({
                SendMessage(hwnd, BFFM_SETSELECTION, true, ulong(initDir->utf16()));
            } , {
                SendMessageA(hwnd, BFFM_SETSELECTION, true, ulong(initDir->utf16()));
            });
        }
    } else if (uMsg == BFFM_SELCHANGED) {
        QT_WA({
            qt_win_resolve_libs();
            TCHAR path[MAX_PATH];
            ptrSHGetPathFromIDList(LPITEMIDLIST(lParam), path);
            QString tmpStr = QString::fromUtf16((ushort*)path);
            if (!tmpStr.isEmpty())
                SendMessage(hwnd, BFFM_ENABLEOK, 1, 1);
            else
                SendMessage(hwnd, BFFM_ENABLEOK, 0, 0);
            SendMessage(hwnd, BFFM_SETSTATUSTEXT, 1, ulong(path));
        } , {
            char path[MAX_PATH];
            SHGetPathFromIDListA(LPITEMIDLIST(lParam), path);
            QString tmpStr = QString::fromLocal8Bit(path);
            if (!tmpStr.isEmpty())
                SendMessageA(hwnd, BFFM_ENABLEOK, 1, 1);
            else
                SendMessageA(hwnd, BFFM_ENABLEOK, 0, 0);
            SendMessageA(hwnd, BFFM_SETSTATUSTEXT, 1, ulong(path));
        });
    }
#endif
    return 0;
}

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE     0x0040   // Use the new dialog layout with the ability to resize
#endif


QString qt_win_get_existing_directory(const QFileDialogArgs &args)
{
#ifndef Q_OS_TEMP
    QString currentDir = QDir::currentPath();
    QString result;
    QWidget *parent = args.parent;
    if (parent)
        parent = parent->window();
    else
        parent = qApp->activeWindow();
    QString title = args.caption;
    if (title.isNull())
        title = QObject::tr("Select a Directory");

    if (parent) {
        QApplicationPrivate::enterModal(parent);
    }
    QT_WA({
        qt_win_resolve_libs();
        QString initDir = QDir::convertSeparators(args.directory);
        TCHAR path[MAX_PATH];
        TCHAR initPath[MAX_PATH];
        initPath[0] = 0;
        path[0] = 0;
        tTitle = title;
        BROWSEINFO bi;
        bi.hwndOwner = (parent ? parent->winId() : 0);
        bi.pidlRoot = NULL;
        bi.lpszTitle = (TCHAR*)tTitle.utf16();
        bi.pszDisplayName = initPath;
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_NEWDIALOGSTYLE;
        bi.lpfn = winGetExistDirCallbackProc;
        bi.lParam = ulong(&initDir);
        LPITEMIDLIST pItemIDList = ptrSHBrowseForFolder(&bi);
        if (pItemIDList) {
            ptrSHGetPathFromIDList(pItemIDList, path);
            IMalloc *pMalloc;
            if (SHGetMalloc(&pMalloc) != NOERROR)
                result = QString();
            else {
                pMalloc->Free(pItemIDList);
                pMalloc->Release();
                result = QString::fromUtf16((ushort*)path);
            }
        } else
            result = QString();
        tTitle = QString();
    } , {
        QString initDir = QDir::convertSeparators(args.directory);
        char path[MAX_PATH];
        char initPath[MAX_PATH];
        QByteArray ctitle = title.toLocal8Bit();
        initPath[0]=0;
        path[0]=0;
        BROWSEINFOA bi;
        bi.hwndOwner = (parent ? parent->winId() : 0);
        bi.pidlRoot = NULL;
        bi.lpszTitle = ctitle;
        bi.pszDisplayName = initPath;
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_NEWDIALOGSTYLE;
        bi.lpfn = winGetExistDirCallbackProc;
        bi.lParam = ulong(&initDir);
        LPITEMIDLIST pItemIDList = SHBrowseForFolderA(&bi);
        if (pItemIDList) {
            SHGetPathFromIDListA(pItemIDList, path);
            IMalloc *pMalloc;
            if (SHGetMalloc(&pMalloc) != NOERROR)
                result = QString();
            else {
                pMalloc->Free(pItemIDList);
                pMalloc->Release();
                result = QString::fromLocal8Bit(path);
            }
        } else
            result = QString();
    });
    if (parent) {
        QApplicationPrivate::leaveModal(parent);
    }

    qt_win_eatMouseMove();

    // Due to a bug on Windows Me, we need to reset the current
    // directory
    if ((QSysInfo::WindowsVersion == QSysInfo::WV_98 || QSysInfo::WindowsVersion == QSysInfo::WV_Me)
	&& QDir::currentPath() != currentDir)
        QDir::setCurrent(currentDir);

    if (!result.isEmpty())
        result.replace("\\", "/");
    return result;
#else
    return QString();
#endif
}


#endif
