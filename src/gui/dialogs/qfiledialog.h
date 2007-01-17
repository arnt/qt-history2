/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFILEDIALOG_H
#define QFILEDIALOG_H

#include <QtCore/qdir.h>
#include <QtCore/qstring.h>
#include <QtGui/qdialog.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_FILEDIALOG

class QModelIndex;
class QItemSelection;
struct QFileDialogArgs;
class QFileIconProvider;
class QFileDialogPrivate;
class QAbstractItemDelegate;
class QUrl;

class Q_GUI_EXPORT QFileDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(FileMode fileMode READ fileMode WRITE setFileMode)
    Q_PROPERTY(AcceptMode acceptMode READ acceptMode WRITE setAcceptMode)
    Q_PROPERTY(bool detailsExpanded READ isDetailsExpanded WRITE setDetailsExpanded)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(bool resolveSymlinks READ resolveSymlinks WRITE setResolveSymlinks)
    Q_PROPERTY(bool confirmOverwrite READ confirmOverwrite WRITE setConfirmOverwrite)
    Q_PROPERTY(QString defaultSuffix READ defaultSuffix WRITE setDefaultSuffix)
    Q_ENUMS(ViewMode FileMode AcceptMode)

public:
    enum ViewMode { Detail, List };
    enum FileMode { AnyFile, ExistingFile, Directory, ExistingFiles, DirectoryOnly };
    enum AcceptMode { AcceptOpen, AcceptSave };
    enum DialogLabel { LookIn, FileName, FileType, Accept, Reject };

    enum Option {
        ShowDirsOnly = 0x01,
        DontResolveSymlinks = 0x02,
        DontConfirmOverwrite = 0x04,
        DontUseSheet = 0x08,
        DontUseNativeDialog = 0x10,
        StoreState = 0x12
    };
    Q_DECLARE_FLAGS(Options, Option)

    QFileDialog(QWidget *parent, Qt::WindowFlags f);
    explicit QFileDialog(QWidget *parent = 0,
                         const QString &caption = QString(),
                         const QString &directory = QString(),
                         const QString &filter = QString());
    ~QFileDialog();

    void setDirectory(const QString &directory);
    inline void setDirectory(const QDir &directory);
    QDir directory() const;

    void selectFile(const QString &filename);
    QStringList selectedFiles() const;

    void setFilter(const QString &filter);
    void setFilters(const QStringList &filters);
    QStringList filters() const;

    void selectFilter(const QString &filter);
    QString selectedFilter() const;

    void setViewMode(ViewMode mode);
    ViewMode viewMode() const;

    void setFileMode(FileMode mode);
    FileMode fileMode() const;

    void setAcceptMode(AcceptMode mode);
    AcceptMode acceptMode() const;

    void setDetailsExpanded(bool enabled);
    bool isDetailsExpanded() const;

    void setReadOnly(bool enabled);
    bool isReadOnly() const;

    void setResolveSymlinks(bool enabled);
    bool resolveSymlinks() const;

    void setSidebarUrls(const QList<QUrl> &urls);
    QList<QUrl> sidebarUrls() const;

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

    void setConfirmOverwrite(bool enabled);
    bool confirmOverwrite() const;

    void setDefaultSuffix(const QString &suffix);
    QString defaultSuffix() const;

    void setHistory(const QStringList &paths);
    QStringList history() const;

    void setItemDelegate(QAbstractItemDelegate *delegate);
    QAbstractItemDelegate *itemDelegate() const;

    void setIconProvider(QFileIconProvider *provider);
    QFileIconProvider *iconProvider() const;

    void setLabelText(DialogLabel label, const QString &text);
    QString labelText(DialogLabel label) const;

Q_SIGNALS:
    void filesSelected(const QStringList &files);
    void currentChanged(const QString &path);
    void dirEntered(const QString &);
    void filterSelected(const QString &);

public:
#ifdef QT3_SUPPORT
    typedef FileMode Mode;
    inline QT3_SUPPORT void setMode(FileMode m) { setFileMode(m); }
    inline QT3_SUPPORT FileMode mode() const { return fileMode(); }
    inline QT3_SUPPORT void setDir(const QString &directory) { setDirectory(directory); }
    inline QT3_SUPPORT void setDir( const QDir &directory ) { setDirectory(directory); }
    QT3_SUPPORT QString selectedFile() const;
#endif

    static QString getOpenFileName(QWidget *parent = 0,
                                   const QString &caption = QString(),
                                   const QString &dir = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   Options options = 0);

    static QString getSaveFileName(QWidget *parent = 0,
                                   const QString &caption = QString(),
                                   const QString &dir = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   Options options = 0);

    static QString getExistingDirectory(QWidget *parent = 0,
                                        const QString &caption = QString(),
                                        const QString &dir = QString(),
                                        Options options = ShowDirsOnly);

    static QStringList getOpenFileNames(QWidget *parent = 0,
                                        const QString &caption = QString(),
                                        const QString &dir = QString(),
                                        const QString &filter = QString(),
                                        QString *selectedFilter = 0,
                                        Options options = 0);

#ifdef QT3_SUPPORT
    inline static QString QT3_SUPPORT getOpenFileName(const QString &dir,
                                                    const QString &filter = QString(),
                                                    QWidget *parent = 0, const char* name = 0,
                                                    const QString &caption = QString(),
                                                    QString *selectedFilter = 0,
                                                    bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getOpenFileName(parent, caption, dir, filter, selectedFilter,
                                 resolveSymlinks ? Option(0) : DontResolveSymlinks); }

    inline static QString QT3_SUPPORT getSaveFileName(const QString &dir,
                                                    const QString &filter = QString(),
                                                    QWidget *parent = 0, const char* name = 0,
                                                    const QString &caption = QString(),
                                                    QString *selectedFilter = 0,
                                                    bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getSaveFileName(parent, caption, dir, filter, selectedFilter,
                                 resolveSymlinks ? Option(0) : DontResolveSymlinks); }

    inline static QString QT3_SUPPORT getExistingDirectory(const QString &dir,
                                                         QWidget *parent = 0,
                                                         const char* name = 0,
                                                         const QString &caption = QString(),
                                                         bool dirOnly = true,
                                                         bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getExistingDirectory(parent, caption, dir,
                                      Options((resolveSymlinks ? Option(0) : DontResolveSymlinks)
                                      | (dirOnly ? ShowDirsOnly : Option(0)))); }

    inline static QStringList QT3_SUPPORT getOpenFileNames(const QString &filter,
                                                         const QString &dir = QString(),
                                                         QWidget *parent = 0,
                                                         const char* name = 0,
                                                         const QString &caption = QString(),
                                                         QString *selectedFilter = 0,
                                                         bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getOpenFileNames(parent, caption, dir, filter, selectedFilter,
                                  resolveSymlinks ? Option(0) : DontResolveSymlinks); }
#endif // QT3_SUPPORT

protected:
    QFileDialog(const QFileDialogArgs &args);
    void done(int result);
    void accept();

private:
    Q_DECLARE_PRIVATE(QFileDialog)
    Q_DISABLE_COPY(QFileDialog)

    Q_PRIVATE_SLOT(d_func(), void _q_pathChanged(const QString &))

    Q_PRIVATE_SLOT(d_func(), void _q_navigateBackward())
    Q_PRIVATE_SLOT(d_func(), void _q_navigateForward())
    Q_PRIVATE_SLOT(d_func(), void _q_navigateToParent())
    Q_PRIVATE_SLOT(d_func(), void _q_createDirectory())
    Q_PRIVATE_SLOT(d_func(), void _q_showListView())
    Q_PRIVATE_SLOT(d_func(), void _q_showDetailsView())
    Q_PRIVATE_SLOT(d_func(), void _q_showContextMenu(const QPoint &))
    Q_PRIVATE_SLOT(d_func(), void _q_renameCurrent())
    Q_PRIVATE_SLOT(d_func(), void _q_deleteCurrent())
    Q_PRIVATE_SLOT(d_func(), void _q_showHidden())
    Q_PRIVATE_SLOT(d_func(), void _q_updateOkButton())
    Q_PRIVATE_SLOT(d_func(), void _q_currentChanged(const QModelIndex &index))
    Q_PRIVATE_SLOT(d_func(), void _q_enterDirectory(const QModelIndex &index))
    Q_PRIVATE_SLOT(d_func(), void _q_goToDirectory(const QString &path))
    Q_PRIVATE_SLOT(d_func(), void _q_useNameFilter(const QString &nameFilter))
    Q_PRIVATE_SLOT(d_func(), void _q_selectionChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_goToUrl(const QUrl &url))
    Q_PRIVATE_SLOT(d_func(), void _q_chooseLocation())
    Q_PRIVATE_SLOT(d_func(), void _q_goHome())
    Q_PRIVATE_SLOT(d_func(), void _q_showHeader(QAction *))
    Q_PRIVATE_SLOT(d_func(), void _q_animateDialog())
    Q_PRIVATE_SLOT(d_func(), void _q_animateDialogV(int))
    Q_PRIVATE_SLOT(d_func(), void _q_animateDialogH(int))
    Q_PRIVATE_SLOT(d_func(), void _q_autoCompleteFileName(const QString &text))
    Q_PRIVATE_SLOT(d_func(), void _q_rowsInserted(const QModelIndex & parent))
};

inline void QFileDialog::setDirectory(const QDir &adirectory)
{ setDirectory(adirectory.absolutePath()); }

Q_DECLARE_OPERATORS_FOR_FLAGS(QFileDialog::Options)

#endif // QT_NO_FILEDIALOG

QT_END_HEADER

#endif // QFILEDIALOG_H
