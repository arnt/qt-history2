#ifndef RESOURCEFILE_H
#define RESOURCEFILE_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QFile>

class ResourceFile
{
public:
    ResourceFile();

    bool load(QFile &file);
    void save(QFile &file);
    
    QString resolvePath(const QString &path) const;

    QStringList prefixList() const;
    QStringList fileList(const QString &prefix);
    void addPrefix(const QString &prefix);
    void addFile(const QString &prefix, const QString &file);
    
private:
    typedef QMap<QString, QStringList> ResourceMap;
    ResourceMap m_resource_map;
};

#endif // RESOURCEFILE_H
