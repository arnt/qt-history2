#ifndef RESOURCEFILE_H
#define RESOURCEFILE_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>

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

    bool isEmpty() const { return m_file_name.isEmpty() && m_resource_map.isEmpty(); }

private:
    typedef QMap<QString, QStringList> ResourceMap;
    ResourceMap m_resource_map;
    QString m_file_name;
    QString m_error_message;
};

#endif // RESOURCEFILE_H
