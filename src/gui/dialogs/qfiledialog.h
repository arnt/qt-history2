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

#include <qdir.h>
#include <qstring.h>
#include <qdialog.h>

class QMenu;
class QModelIndex;
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
    Q_ENUMS(ViewMode FileMode AcceptMode)

public:
    enum ViewMode { Detail, List };
    enum FileMode { AnyFile, ExistingFile, Directory, ExistingFiles, DirectoryOnly };
    enum AcceptMode { AcceptOpen, AcceptSave };

    enum Option { DontResolveSymlinks = 0x01, ShowDirsOnly = 0x02 };
    Q_DECLARE_FLAGS(Options, Option)

    QFileDialog(QWidget *parent, Qt::WFlags f);
    QFileDialog(QWidget *parent = 0,
                const QString &caption = QString(),
                const QString &dir = QString(),
                const QString &filter = QString(),
                const QString &selectedFilter = QString(),
                const QString &selectedFile = QString(),
                FileMode fileMode = AnyFile);
    ~QFileDialog();

    void setDirectory(const QString &directory);
    inline void setDirectory(const QDir &directory) { setDirectory(directory.absolutePath()); }
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

    void setItemDelegate(QAbstractItemDelegate *delegate);
    QAbstractItemDelegate *itemDelegate() const;

    void setIconProvider(QFileIconProvider *provider);
    QFileIconProvider *iconProvider() const;

signals:
    void filesSelected(const QStringList &);

public:
#ifdef QT_COMPAT
    typedef FileMode Mode;
    inline QT_COMPAT void setMode(FileMode m) { setFileMode(m); }
    inline QT_COMPAT FileMode mode() const { return fileMode(); }
    inline QT_COMPAT void setDir(const QString &directory) { setDirectory(directory); }
    inline QT_COMPAT void setDir( const QDir &directory ) { setDirectory(directory); }
    QT_COMPAT QString selectedFile() const;
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

#ifdef QT_COMPAT
    inline static QString QT_COMPAT getOpenFileName(const QString &dir,
                                                    const QString &filter = QString(),
                                                    QWidget *parent = 0, const char* name = 0,
                                                    const QString &caption = QString(),
                                                    QString *selectedFilter = 0,
                                                    bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getOpenFileName(parent, caption, dir, filter, selectedFilter,
                                 resolveSymlinks ? Option(0) : DontResolveSymlinks); }

    inline static QString QT_COMPAT getSaveFileName(const QString &dir,
                                                    const QString &filter = QString(),
                                                    QWidget *parent = 0, const char* name = 0,
                                                    const QString &caption = QString(),
                                                    QString *selectedFilter = 0,
                                                    bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getSaveFileName(parent, caption, dir, filter, selectedFilter,
                                 resolveSymlinks ? Option(0) : DontResolveSymlinks); }

    inline static QString QT_COMPAT getExistingDirectory(const QString &dir,
                                                         QWidget *parent = 0,
                                                         const char* name = 0,
                                                         const QString &caption = QString(),
                                                         bool dirOnly = true,
                                                         bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getExistingDirectory(parent, caption, dir,
                                      Options((resolveSymlinks ? Option(0) : DontResolveSymlinks)
                                      | (dirOnly ? ShowDirsOnly : Option(0)))); }

    inline static QStringList QT_COMPAT getOpenFileNames(const QString &filter,
                                                         const QString &dir = QString(),
                                                         QWidget *parent = 0,
                                                         const char* name = 0,
                                                         const QString &caption = QString(),
                                                         QString *selectedFilter = 0,
                                                         bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getOpenFileNames(parent, caption, dir, filter, selectedFilter,
                                  resolveSymlinks ? Option(0) : DontResolveSymlinks); }
#endif // QT_COMPAT

protected:
    void done(int result);
    void accept();

private:
    Q_DECLARE_PRIVATE(QFileDialog)
    Q_DISABLE_COPY(QFileDialog)
    Q_PRIVATE_SLOT(d, void backClicked())
    Q_PRIVATE_SLOT(d, void upClicked())
    Q_PRIVATE_SLOT(d, void mkdirClicked())
    Q_PRIVATE_SLOT(d, void showListClicked())
    Q_PRIVATE_SLOT(d, void showDetailClicked())
    Q_PRIVATE_SLOT(d, void enterSubdir(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void keyPressed(const QModelIndex &index, Qt::Key key, Qt::KeyboardModifiers modifiers))
    Q_PRIVATE_SLOT(d, void deletePressed(const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void selectionChanged(const QItemSelection &selection))
    Q_PRIVATE_SLOT(d, void fileNameChanged(const QString &text))
    Q_PRIVATE_SLOT(d, void lookInChanged(const QString &text))
    Q_PRIVATE_SLOT(d, void useFilter(const QString &filter))
    Q_PRIVATE_SLOT(d, void setCurrentDir(const QString &path))
    Q_PRIVATE_SLOT(d, void populateContextMenu(QMenu *menu, const QModelIndex &index))
    Q_PRIVATE_SLOT(d, void headerPressed(int section))
    Q_PRIVATE_SLOT(d, void renameCurrent())
    Q_PRIVATE_SLOT(d, void deleteCurrent())
    Q_PRIVATE_SLOT(d, void reload())
    Q_PRIVATE_SLOT(d, void lookInReturnPressed())
    Q_PRIVATE_SLOT(d, void sortByName())
    Q_PRIVATE_SLOT(d, void sortBySize())
    Q_PRIVATE_SLOT(d, void sortByDate())
    Q_PRIVATE_SLOT(d, void setUnsorted())
    Q_PRIVATE_SLOT(d, void showHidden())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFileDialog::Options);

#endif // QFILEDIALOG_H
