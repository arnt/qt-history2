#ifndef Q4FILEDIALOG_H
#define Q4FILEDIALOG_H

#include <qdir.h>
#include <qstring.h>
#include <qdialog.h>

class QModelIndex;
class QFileDialogPrivate;

class QFileDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFileDialog);

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

    enum ViewMode { Detail, List, Large };
    void setViewMode(ViewMode mode);
    ViewMode viewMode() const;

//    enum FileMode { AnyFile, ExistingFile, Directory }
    enum FileMode { AnyFile, ExistingFile, Directory, ExistingFiles, DirectoryOnly };

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

};

#endif // Q4FILEDIALOG_H
