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

#ifndef QT_H
#include <qdir.h>
#include <qstring.h>
#include <qdialog.h>
#endif

class QMenu;
class QModelIndex;
class QFileDialogPrivate;

class Q_GUI_EXPORT QFileDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFileDialog)

    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(FileMode fileMode READ fileMode WRITE setFileMode)
    Q_PROPERTY(AcceptMode acceptMode READ acceptMode WRITE setAcceptMode)
    Q_ENUMS(ViewMode FileMode AcceptMode)

public:
    enum ViewMode { Detail, List };
    enum FileMode { AnyFile, ExistingFile, Directory, ExistingFiles, DirectoryOnly };
    enum AcceptMode { Open, Save };

    enum Option { DontResolveSymlinks = 0x01, ShowDirsOnly = 0x02 };
    Q_DECLARE_FLAGS(Options, Option);

    QFileDialog(QWidget *parent, Qt::WFlags f);
    QFileDialog(QWidget *parent = 0,
                const QString &caption = QString::null,
                const QString &dir = QString::null,
                const QString &filter = QString::null,
                const QString &selectedFilter = QString::null,
                const QString &selectedFile = QString::null,
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
                                                    const QString &filter = QString::null,
                                                    QWidget *parent = 0, const char* name = 0,
                                                    const QString &caption = QString::null,
                                                    QString *selectedFilter = 0,
                                                    bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getOpenFileName(parent, caption, dir, filter, selectedFilter,
                                 resolveSymlinks ? Option(0) : DontResolveSymlinks); }

    inline static QString QT_COMPAT getSaveFileName(const QString &dir,
                                                    const QString &filter = QString::null,
                                                    QWidget *parent = 0, const char* name = 0,
                                                    const QString &caption = QString::null,
                                                    QString *selectedFilter = 0,
                                                    bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getSaveFileName(parent, caption, dir, filter, selectedFilter,
                                 resolveSymlinks ? Option(0) : DontResolveSymlinks); }

    inline static QString QT_COMPAT getExistingDirectory(const QString &dir,
                                                         QWidget *parent = 0,
                                                         const char* name = 0,
                                                         const QString &caption = QString::null,
                                                         bool dirOnly = true,
                                                         bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getExistingDirectory(parent, caption, dir,
                                      Options((resolveSymlinks ? Option(0) : DontResolveSymlinks)
                                      | (dirOnly ? ShowDirsOnly : Option(0)))); }

    inline static QStringList QT_COMPAT getOpenFileNames(const QString &filter,
                                                         const QString &dir = QString::null,
                                                         QWidget *parent = 0,
                                                         const char* name = 0,
                                                         const QString &caption = QString::null,
                                                         QString *selectedFilter = 0,
                                                         bool resolveSymlinks = true)
        { Q_UNUSED(name);
          return getOpenFileNames(parent, caption, dir, filter, selectedFilter,
                                  resolveSymlinks ? Option(0) : DontResolveSymlinks); }
#endif // QT_COMPAT

protected:
    void done(int result);
    void accept();

protected slots:
    void back();
    void up();
    void mkdir();
    void showList();
    void showDetail();
    void doubleClicked(const QModelIndex &index);
    void deletePressed(const QModelIndex &index);
    void currentChanged(const QModelIndex &old, const QModelIndex &current);
    void fileNameChanged(const QString &text);
    void lookInChanged(const QString &text);
    void useFilter(const QString &filter);
    void setCurrentDir(const QString &path);
    void populateContextMenu(QMenu *menu, const QModelIndex &index) const;
    void headerClicked(int section);
    void renameCurrent();
    void deleteCurrent();
    void reload();
    void lookIn();
    void sortByName();
    void sortBySize();
    void sortByDate();
    void setUnsorted();
    void showHidden();
};

#endif // QFILEDIALOG_H
