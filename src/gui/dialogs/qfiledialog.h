/****************************************************************************
**
** Definition of QFileDialog class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFILEDIALOG_H
#define QFILEDIALOG_H

class QPushButton;
class QButton;
class QLabel;
class QWidget;
class QFileDialog;
class QTimer;
class QNetworkOperation;
class QLineEdit;
class QListViewItem;
class QListBoxItem;
class QFileDialogPrivate;
class QFileDialogQFileListView;

#ifndef QT_H
#include "qdir.h"
#include "qdialog.h"
#include "qurloperator.h"
#include "qurlinfo.h"
#endif // QT_H

#ifndef QT_NO_FILEDIALOG

class Q_COMPAT_EXPORT QFileIconProvider : public QObject
{
    Q_OBJECT
public:
    QFileIconProvider(QObject * parent = 0, const char* name = 0);
    virtual const QPixmap * pixmap(const QFileInfo &);

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFileIconProvider(const QFileIconProvider &);
    QFileIconProvider& operator=(const QFileIconProvider &);
#endif
};

class Q_COMPAT_EXPORT QFilePreview
{
public:
    QFilePreview();
    virtual void previewUrl(const QUrl &url) = 0;

};

class Q_COMPAT_EXPORT QFileDialog : public QDialog
{
    Q_OBJECT
    Q_ENUMS(Mode ViewMode PreviewMode)
    // ##### Why are this read-only properties ?
    Q_PROPERTY(QString selectedFile READ selectedFile)
    Q_PROPERTY(QString selectedFilter READ selectedFilter)
    Q_PROPERTY(QStringList selectedFiles READ selectedFiles)
    // #### Should not we be able to set the path ?
    Q_PROPERTY(QString dirPath READ dirPath)
    Q_PROPERTY(bool showHiddenFiles READ showHiddenFiles WRITE setShowHiddenFiles)
    Q_PROPERTY(Mode mode READ mode WRITE setMode)
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(PreviewMode previewMode READ previewMode WRITE setPreviewMode)
    Q_PROPERTY(bool infoPreview READ isInfoPreviewEnabled WRITE setInfoPreviewEnabled)
    Q_PROPERTY(bool contentsPreview READ isContentsPreviewEnabled WRITE setContentsPreviewEnabled)

public:
    QFileDialog(const QString& dirName, const QString& filter = QString::null,
                 QWidget* parent=0, const char* name=0, bool modal = false);
    QFileDialog(QWidget* parent=0, const char* name=0, bool modal = false);
    ~QFileDialog();

    // recommended static functions

    static QString getOpenFileName(const QString &initially = QString::null,
                                    const QString &filter = QString::null,
                                    QWidget *parent = 0, const char* name = 0,
                                    const QString &caption = QString::null,
                                    QString *selectedFilter = 0,
                                    bool resolveSymlinks = true);
    static QString getSaveFileName(const QString &initially = QString::null,
                                    const QString &filter = QString::null,
                                    QWidget *parent = 0, const char* name = 0,
                                    const QString &caption = QString::null,
                                    QString *selectedFilter = 0,
                                    bool resolveSymlinks = true);
    static QString getExistingDirectory(const QString &dir = QString::null,
                                         QWidget *parent = 0,
                                         const char* name = 0,
                                         const QString &caption = QString::null,
                                         bool dirOnly = true,
                                         bool resolveSymlinks = true);
    static QStringList getOpenFileNames(const QString &filter= QString::null,
                                         const QString &dir = QString::null,
                                         QWidget *parent = 0,
                                         const char* name = 0,
                                         const QString &caption = QString::null,
                                         QString *selectedFilter = 0,
                                         bool resolveSymlinks = true);

    // other static functions

    static void setIconProvider(QFileIconProvider *);
    static QFileIconProvider * iconProvider();

    // non-static function for special needs

    QString selectedFile() const;
    QString selectedFilter() const;
    virtual void setSelectedFilter(const QString&);
    virtual void setSelectedFilter(int);

    void setSelection(const QString &);

    void selectAll(bool b);

    QStringList selectedFiles() const;

    QString dirPath() const;

    void setDir(const QDir &);
    const QDir *dir() const;

    void setShowHiddenFiles(bool s);
    bool showHiddenFiles() const;

    void rereadDir();
    void resortDir();

    enum Mode { AnyFile, ExistingFile, Directory, ExistingFiles, DirectoryOnly };
    void setMode(Mode);
    Mode mode() const;

    enum ViewMode { Detail, List };
    enum PreviewMode { NoPreview, Contents, Info };
    void setViewMode(ViewMode m);
    ViewMode viewMode() const;
    void setPreviewMode(PreviewMode m);
    PreviewMode previewMode() const;

    bool eventFilter(QObject *, QEvent *);

    bool isInfoPreviewEnabled() const;
    bool isContentsPreviewEnabled() const;
    void setInfoPreviewEnabled(bool);
    void setContentsPreviewEnabled(bool);

    void setInfoPreview(QWidget *w, QFilePreview *preview);
    void setContentsPreview(QWidget *w, QFilePreview *preview);

    QUrl url() const;

    void addFilter(const QString &filter);

public slots:
    void done(int);
    void setDir(const QString&);
    void setUrl(const QUrlOperator &url);
    void setFilter(const QString&);
    void setFilters(const QString&);
    void setFilters(const char **);
    void setFilters(const QStringList&);

protected:
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);

    void addWidgets(QLabel *, QWidget *, QPushButton *);
    void addToolButton(QButton *b, bool separator = false);
    void addLeftWidget(QWidget *w);
    void addRightWidget(QWidget *w);

signals:
    void fileHighlighted(const QString&);
    void fileSelected(const QString&);
    void filesSelected(const QStringList&);
    void dirEntered(const QString&);
    void filterSelected(const QString&);

private slots:
    void detailViewSelectionChanged();
    void listBoxSelectionChanged();
    void changeMode(int);
    void fileNameEditReturnPressed();
    void stopCopy();
    void removeProgressDia();

    void fileSelected(int);
    void fileHighlighted(int);
    void dirSelected(int);
    void pathSelected(int);

    void updateFileNameEdit(QListViewItem *);
    void selectDirectoryOrFile(QListViewItem *);
    void popupContextMenu(QListViewItem *, const QPoint &, int);
    void popupContextMenu(QListBoxItem *, const QPoint &);
    void updateFileNameEdit(QListBoxItem *);
    void selectDirectoryOrFile(QListBoxItem *);
    void fileNameEditDone();

    void okClicked();
    void filterClicked(); // not used
    void cancelClicked();

    void cdUpClicked();
    void newFolderClicked();

    void fixupNameEdit();

    void doMimeTypeLookup();

    void updateGeometries();
    void modeButtonsDestroyed();
    void urlStart(QNetworkOperation *op);
    void urlFinished(QNetworkOperation *op);
    void dataTransferProgress(int bytesDone, int bytesTotal, QNetworkOperation *);
    void insertEntry(const QList<QUrlInfo> &fi, QNetworkOperation *op);
    void removeEntry(QNetworkOperation *);
    void createdDirectory(const QUrlInfo &info, QNetworkOperation *);
    void itemChanged(QNetworkOperation *);
    void goBack();

private:
    enum PopupAction {
        PA_Open = 0,
        PA_Delete,
        PA_Rename,
        PA_SortName,
        PA_SortSize,
        PA_SortType,
        PA_SortDate,
        PA_SortUnsorted,
        PA_Cancel,
        PA_Reload,
        PA_Hidden
    };

    void init();
    bool trySetSelection(bool isDir, const QUrlOperator &, bool);
    void deleteFile(const QString &filename);
    void popupContextMenu(const QString &filename, bool withSort,
                           PopupAction &action, const QPoint &p);
    void updatePreviews(const QUrl &u);

    QDir reserved; // was cwd
    QString fileName;

    friend class QFileDialogQFileListView;
    friend class QFileListBox;

    QFileDialogPrivate *d;
    QFileDialogQFileListView  *files;

    QLineEdit  *nameEdit; // also filter
    QPushButton *okB;
    QPushButton *cancelB;

#if defined(Q_WS_WIN)
    static QString winGetOpenFileName(const QString &initialSelection,
                                       const QString &filter,
                                       QString* workingDirectory,
                                       QWidget *parent = 0,
                                       const char* name = 0,
                                       const QString& caption = QString::null,
                                       QString* selectedFilter = 0);
    static QString winGetSaveFileName(const QString &initialSelection,
                                       const QString &filter,
                                       QString* workingDirectory,
                                       QWidget *parent = 0,
                                       const char* name = 0,
                                       const QString& caption = QString::null,
                                       QString* selectedFilter = 0);
    static QStringList winGetOpenFileNames(const QString &filter,
                                            QString* workingDirectory,
                                            QWidget *parent = 0,
                                            const char* name = 0,
                                            const QString& caption = QString::null,
                                            QString* selectedFilter = 0);
    static QString winGetExistingDirectory(const QString &initialDirectory,
                                            QWidget* parent = 0,
                                            const char* name = 0,
                                            const QString& caption = QString::null);
    static QString resolveLinkFile(const QString& linkfile);
    int old_qt_ntfs_permission_lookup;
#endif
#if defined(Q_WS_MAC)
    static QString macGetSaveFileName(const QString &, const QString &,
                                       QString *, QWidget *, const char*,
                                       const QString&, QString *);
    static QStringList macGetOpenFileNames(const QString &, QString*,
                                            QWidget *, const char *,
                                            const QString&, QString *,
                                            bool = true, bool = false);
#endif


private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFileDialog(const QFileDialog &);
    QFileDialog &operator=(const QFileDialog &);
#endif
};

#endif

#endif // QFILEDIALOG_H
