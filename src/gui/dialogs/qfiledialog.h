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

#ifndef QFILEDIALOG_H
#define QFILEDIALOG_H

#include <QtCore/qdir.h>
#include <QtCore/qstring.h>
#include <QtGui/qdialog.h>

class QModelIndex;
class QItemSelection;
struct QFileDialogArgs;
class QFileIconProvider;
class QFileDialogPrivate;
class QAbstractItemDelegate;

class Q_GUI_EXPORT QFileDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(FileMode fileMode READ fileMode WRITE setFileMode)
    Q_PROPERTY(AcceptMode acceptMode READ acceptMode WRITE setAcceptMode)
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
        DontUseNativeDialog = 0x10
    };
    Q_DECLARE_FLAGS(Options, Option)

    QFileDialog(QWidget *parent, Qt::WFlags f);
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

    void setReadOnly(bool enabled);
    bool isReadOnly() const;

    void setResolveSymlinks(bool enabled);
    bool resolveSymlinks() const;

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

signals:
    void filesSelected(const QStringList &files);

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
    Q_PRIVATE_SLOT(d_func(), void reload())
    Q_PRIVATE_SLOT(d_func(), void navigateToPrevious())
    Q_PRIVATE_SLOT(d_func(), void navigateToParent())
    Q_PRIVATE_SLOT(d_func(), void enterDirectory(const QModelIndex &index))
    Q_PRIVATE_SLOT(d_func(), void enterDirectory(const QString &path))
    Q_PRIVATE_SLOT(d_func(), void enterDirectory())
    Q_PRIVATE_SLOT(d_func(), void showList())
    Q_PRIVATE_SLOT(d_func(), void showDetails())
    Q_PRIVATE_SLOT(d_func(), void showHidden())
    Q_PRIVATE_SLOT(d_func(), void useFilter(const QString &filter))
    Q_PRIVATE_SLOT(d_func(), void updateFileName(const QItemSelection &selection))
    Q_PRIVATE_SLOT(d_func(), void autoCompleteFileName(const QString &text))
    Q_PRIVATE_SLOT(d_func(), void autoCompleteDirectory(const QString &text))
    Q_PRIVATE_SLOT(d_func(), void showContextMenu(const QPoint &pos))
    Q_PRIVATE_SLOT(d_func(), void createDirectory())
    Q_PRIVATE_SLOT(d_func(), void renameCurrent())
    Q_PRIVATE_SLOT(d_func(), void deleteCurrent())
    Q_PRIVATE_SLOT(d_func(), void sortByName())
    Q_PRIVATE_SLOT(d_func(), void sortBySize())
    Q_PRIVATE_SLOT(d_func(), void sortByDate())
    Q_PRIVATE_SLOT(d_func(), void setUnsorted())
};

inline void QFileDialog::setDirectory(const QDir &adirectory)
{ setDirectory(adirectory.absolutePath()); }

Q_DECLARE_OPERATORS_FOR_FLAGS(QFileDialog::Options)

#endif // QFILEDIALOG_H
