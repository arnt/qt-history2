#ifndef RESOURCEFILE_H
#define RESOURCEFILE_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QFile>

class ResourceFile
{
public:
    ResourceFile(const QString &file_name);

    bool load();
    bool save();
    
    QString resolvePath(const QString &path) const;

    QStringList prefixList() const;
    QStringList fileList(const QString &prefix);
    void addPrefix(const QString &prefix);
    void addFile(const QString &prefix, const QString &file);
    void removePrefix(const QString &prefix);
    void removeFile(const QString &prefix, const QString &file);

    QString relativePath(const QString &abs_path) const;
    QString absolutePath(const QString &rel_path) const;
        
private:
    typedef QMap<QString, QStringList> ResourceMap;
    ResourceMap m_resource_map;
    QString m_file_name;
};

#endif // RESOURCEFILE_H
