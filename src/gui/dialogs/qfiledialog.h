/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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

public:
    QFileDialog(QWidget *parent);
    ~QFileDialog();

    void setDirectory(const QDir &directory);
    QDir directory() const;

    void selectFile(const QString &filename);
    QStringList selectedFiles() const;

    void addFilter(const QString &filter);
    QStringList filters() const;

    void selectFilter(const QString &filter);
    QString selectedFilter() const;

    enum ViewMode { Detail, List };
    void setViewMode(ViewMode mode);
    ViewMode viewMode() const;

    enum FileMode { AnyFile, ExistingFile, Directory, ExistingFiles, DirectoryOnly };
    void setFileMode(FileMode mode);
    FileMode fileMode() const;

    static QString getOpenFileName(const QString &initially = QString::null,
                                   const QString &filter = QString::null,
                                   QWidget *parent = 0,
                                   const QString &caption = QString::null,
                                   QString *selectedFilter = 0,
                                   bool resolveSymlinks = true);

    static QString getSaveFileName(const QString &initially = QString::null,
                                   const QString &filter = QString::null,
                                   QWidget *parent = 0,
                                   const QString &caption = QString::null,
                                   QString *selectedFilter = 0,
                                   bool resolveSymlinks = true);

    static QString getExistingDirectory(const QString &dir = QString::null,
                                        QWidget *parent = 0,
                                        const QString &caption = QString::null,
                                        bool dirOnly = true,
                                        bool resolveSymlinks = true);

    static QStringList getOpenFileNames(const QString &filter= QString::null,
                                        const QString &dir = QString::null,
                                        QWidget *parent = 0,
                                        const QString &caption = QString::null,
                                        QString *selectedFilter = 0,
                                        bool resolveSymlinks = true);

protected:
    void done(int result);
    void accept();
    void reject();

protected slots:
    void back();
    void up();
    void mkdir();
    void showList();
    void showDetail();
    void doubleClicked(const QModelIndex &index);
    void deletePressed(const QModelIndex &index);
    void currentChanged(const QModelIndex &old, const QModelIndex &current);
    void textChanged(const QString &text);
    void setFilter(const QString &filter);
    void setCurrentDir(const QString &path);
    void populateContextMenu(QMenu *menu, const QModelIndex &index) const;
    void headerClicked(int section);
    void renameCurrent();
    void deleteCurrent();
    void reload();
    void sortByName();
    void sortBySize();
    void sortByDate();
    void setUnsorted();
    void showHidden();
};

#endif // QFILEDIALOG_H
