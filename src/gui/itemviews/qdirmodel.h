#ifndef QDIRMODEL_H
#define QDIRMODEL_H

#include <qabstractitemmodel.h>
#include <qfileinfo.h>
#include <qdir.h>

class QDirModelPrivate;

class Q4FileIconProvider
{
public:
    Q4FileIconProvider();
    virtual ~Q4FileIconProvider();
    virtual QIconSet icons(const QFileInfo &info) const;
    virtual QString type(const QFileInfo &info) const;
protected:
    QIconSet file;
    QIconSet dir;
    QIconSet linkFile;
    QIconSet linkDir;
};

class QDirModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDirModel)

public:
    QDirModel(const QDir &directory, QObject *parent = 0);
    ~QDirModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = 0,
                      QModelIndex::Type type = QModelIndex::View) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    void setData(const QModelIndex &index, int role, const QVariant &value);

    bool hasChildren(const QModelIndex &index) const;
    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;
    bool isDragEnabled(const QModelIndex &index) const;

    void setIconProvider(Q4FileIconProvider *provider);
    Q4FileIconProvider *iconProvider() const;

    void setNameFilter(const QString &filter);
    QString nameFilter() const;
    void setFilter(QDir::FilterSpec spec);
    QDir::FilterSpec filter() const;
    void setSorting(QDir::SortSpec spec);
    QDir::SortSpec sorting() const;

    QFileInfo fileInfo(const QModelIndex &index) const;
    void mkdir(const QModelIndex &parent, const QString &name);
    void rmdir(const QModelIndex &index);

protected:
    QDirModel(QDirModelPrivate &, const QDir &directory, QObject *parent = 0);
    void init(const QDir &directory);
};

#endif //QDIRMODEL_H
