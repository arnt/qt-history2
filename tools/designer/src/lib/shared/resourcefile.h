#ifndef RESOURCEFILE_H
#define RESOURCEFILE_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QAbstractItemModel>

#include "shared_global.h"

class QT_SHARED_EXPORT ResourceFile
{
public:
    ResourceFile(const QString &file_name = QString());

    void setFileName(const QString &file_name) { m_file_name = file_name; }
    QString fileName() const { return m_file_name; }
    bool load();
    bool save();
    QString errorMessage() const { return m_error_message; }
    
    QString resolvePath(const QString &path) const;

    QStringList prefixList() const;
    QStringList fileList(const QString &prefix);
    void addPrefix(const QString &prefix);
    void addFile(const QString &prefix, const QString &file);
    void removePrefix(const QString &prefix);
    void removeFile(const QString &prefix, const QString &file);
    bool contains(const QString &prefix) const;
    bool contains(const QString &prefix, const QString &file) const;
    void changePrefix(const QString &old_prefix, const QString &new_prefix);
    
    int prefixCount() const;
    QString prefix(int idx) const;
    int fileCount(int prefix_idx) const;
    QString file(int prefix_idx, int file_idx) const;

    int indexOfPrefix(const QString &prefix);
    int indexOfFile(int pref_idx, const QString &file);
        
    QString relativePath(const QString &abs_path) const;
    QString absolutePath(const QString &rel_path) const;
        
    static QString fixPrefix(const QString &prefix);
    bool split(const QString &path, QString &prefix, QString &file) const;
    
    bool isEmpty() const { return m_file_name.isEmpty() && m_resource_map.isEmpty(); }

private:
    typedef QMap<QString, QStringList> ResourceMap;
    ResourceMap m_resource_map;
    QString m_file_name;
    QString m_error_message;
};

class QT_SHARED_EXPORT ResourceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ResourceModel(const ResourceFile &resource_file, QObject *parent = 0);
    
    QModelIndex index(int row, int column,
                        const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = DisplayRole) const;

    QString fileName() const { return m_resource_file.fileName(); }
    void setFileName(const QString &file_name) { m_resource_file.setFileName(file_name); }
    void getItem(const QModelIndex &index, QString &prefix, QString &file) const;

    QModelIndex addNewPrefix();
    QModelIndex addFiles(const QModelIndex &idx, const QStringList &file_list);
    void changePrefix(const QModelIndex &idx, const QString &prefix);
    QModelIndex prefixIndex(const QModelIndex &sel_idx) const;
    QModelIndex deleteItem(const QModelIndex &idx);
    QModelIndex getIndex(const QString &prefix, const QString &file);

    QString absolutePath(const QString &path) const { return m_resource_file.absolutePath(path); }

    void reload();
    void save();

    bool dirty() const { return m_dirty; }
    void setDirty(bool b);

signals:
    void dirtyChanged(bool b);
    
private:
    ResourceFile m_resource_file;
    bool m_dirty;
};

#endif // RESOURCEFILE_H
