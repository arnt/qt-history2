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

#ifndef Q3FILEDIALOG_H
#define Q3FILEDIALOG_H

class QAbstractButton;
class QPushButton;
class QLabel;
class QWidget;
class QFileDialog;
class QTimer;
class QNetworkOperation;
class QLineEdit;
class Q3ListViewItem;
class QListBoxItem;
class Q3FileDialogPrivate;
class Q3FileDialogQFileListView;

#include "qdir.h"
#include "qdialog.h"
#include "qurloperator.h"
#include "qurlinfo.h"

#ifndef QT_NO_FILEDIALOG

class Q_COMPAT_EXPORT Q3FileIconProvider : public QObject
{
    Q_OBJECT
public:
    Q3FileIconProvider(QObject * parent = 0, const char* name = 0);
    virtual const QPixmap * pixmap(const QFileInfo &);

private:
    Q_DISABLE_COPY(Q3FileIconProvider)
};

class Q_COMPAT_EXPORT Q3FilePreview
{
public:
    Q3FilePreview();
    virtual void previewUrl(const Q3Url &url) = 0;

};

class Q_COMPAT_EXPORT Q3FileDialog : public QDialog
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
    Q3FileDialog(const QString& dirName, const QString& filter = QString::null,
                 QWidget* parent=0, const char* name=0, bool modal = false);
    Q3FileDialog(QWidget* parent=0, const char* name=0, bool modal = false);
    ~Q3FileDialog();

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

    static void setIconProvider(Q3FileIconProvider *);
    static Q3FileIconProvider * iconProvider();

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

    void setInfoPreview(QWidget *w, Q3FilePreview *preview);
    void setContentsPreview(QWidget *w, Q3FilePreview *preview);

    Q3Url url() const;

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
    void addToolButton(QAbstractButton *b, bool separator = false);
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

    void updateFileNameEdit(Q3ListViewItem *);
    void selectDirectoryOrFile(Q3ListViewItem *);
    void popupContextMenu(Q3ListViewItem *, const QPoint &, int);
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
    Q_DISABLE_COPY(Q3FileDialog)

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
    void updatePreviews(const Q3Url &u);

    QDir reserved; // was cwd
    QString fileName;

    friend class Q3FileDialogQFileListView;
    friend class QFileListBox;

    Q3FileDialogPrivate *d;
    Q3FileDialogQFileListView  *files;

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
};

#endif
#endif // Q3FILEDIALOG_H
