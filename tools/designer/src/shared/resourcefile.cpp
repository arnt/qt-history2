#include <QtCore/QFile>
#include <QtXml/QDomDocument>
#include <QtCore/QTextStream>
#include <QtCore/QDir>

#include "resourcefile.h"

static QString relativePath(const QString &dir, const QString &file)
{
    QString result;
    QStringList dir_elts = dir.split(QDir::separator(), QString::SkipEmptyParts);
    QStringList file_elts = file.split(QDir::separator(), QString::SkipEmptyParts);

    int i = 0;
    while (i < dir_elts.size() && i < file_elts.size() && dir_elts.at(i) == file_elts.at(i))
        ++i;

    for (int j = 0; j < dir_elts.size() - i; ++j)
        result += QLatin1String("..") + QDir::separator();

    for (int j = i; j < file_elts.size(); ++j) {
        result += file_elts.at(j);
        if (j < file_elts.size() - 1)
        result += QDir::separator();
    }

    return result;
}

ResourceFile::ResourceFile(const QString &file_name)
{
    m_file_name = file_name;
}

bool ResourceFile::load()
{
    QFile file(m_file_name);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("ResourceFile::ResourceFile(): failed to open qrc file \"%s\":\n%s",
                    m_file_name.toLatin1().constData(),
                    file.errorString().toLatin1().constData());
        return false;
    }

    m_resource_map.clear();

    QDomDocument doc;

    QString error_msg;
    int error_line, error_col;
    if (!doc.setContent(&file, &error_msg, &error_line, &error_col)) {
        qWarning("ResourceFile::ResourceFile(): failed to parse qrc file:\n%s %d:%d:%s",
                    file.fileName().toLatin1().constData(), error_line, error_col,
                    error_msg.toLatin1().constData());
        return false;
    } 

    QDomElement root = doc.firstChildElement(QLatin1String("RCC"));
    if (root.isNull()) {
        qWarning("ResourceFile::ResourceFile(): bad root element in '%s'",
                    file.fileName().toLatin1().constData());
        return false;
    }

    QDomElement relt = root.firstChildElement(QLatin1String("qresource"));
    for (; !relt.isNull(); relt = relt.nextSiblingElement(QLatin1String("qresource"))) {
        QStringList file_list;
        QDomElement felt = relt.firstChildElement(QLatin1String("file"));
        for (; !felt.isNull(); felt = felt.nextSiblingElement(QLatin1String("file")))
            file_list.append(felt.text());
        m_resource_map.insert(relt.attribute(QLatin1String("prefix")), file_list);
    }

    return true;
}

bool ResourceFile::save()
{
    QFile file(m_file_name);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("ResourceFile::ResourceFile(): failed to open qrc file \"%s\":\n%s",
                    m_file_name.toLatin1().constData(),
                    file.errorString().toLatin1().constData());
        return false;
    }
    
    QDomDocument doc;
    QDomElement root = doc.createElement(QLatin1String("RCC"));
    doc.appendChild(root);

    ResourceMap::const_iterator it = m_resource_map.begin();
    for (; it != m_resource_map.end(); ++it) {
        QDomElement relt = doc.createElement(QLatin1String("qresource"));
        root.appendChild(relt);
        relt.setAttribute(QLatin1String("prefix"), it.key());

        foreach (QString f, it.value()) {
            QDomElement felt = doc.createElement(QLatin1String("file"));
            relt.appendChild(felt);
            QDomText text = doc.createTextNode(f);
            felt.appendChild(text);
        }
    }

    QTextStream stream(&file);
    doc.save(stream, 4);

    return true;
}

QString ResourceFile::resolvePath(const QString &_path) const
{
    QString path = _path;
    
    if (!path.startsWith(QLatin1String(":")))
        return QString();
    path = path.mid(1);

    ResourceMap::const_iterator it = m_resource_map.begin();
    for (; it != m_resource_map.end(); ++it) {
        if (!path.startsWith(it.key()))
            continue;
            
        QString result = path.mid(it.key().size() + 1);
        if (it.value().contains(result))
            return QDir::cleanPath(QFileInfo(m_file_name).path() + QDir::separator() + result);
        else
            return QString();
    }

    return QString();
}

QStringList ResourceFile::prefixList() const
{
    return m_resource_map.keys();
}

QStringList ResourceFile::fileList(const QString &prefix)
{
    return m_resource_map.value(prefix);
}

void ResourceFile::addPrefix(const QString &prefix)
{
    m_resource_map.insert(prefix, QStringList());
}

void ResourceFile::addFile(const QString &prefix, const QString &file)
{
    m_resource_map[prefix].append(relativePath(file));
}

void ResourceFile::removePrefix(const QString &prefix)
{
    m_resource_map.remove(prefix);
}

void ResourceFile::removeFile(const QString &prefix, const QString &file)
{
    if (!m_resource_map.contains(prefix))
        return;
    m_resource_map[prefix].removeAll(file);
}

QString ResourceFile::relativePath(const QString &abs_path) const
{
    if (QFileInfo(abs_path).isRelative())
        return abs_path;
        
    return ::relativePath(QFileInfo(m_file_name).path(), abs_path);
}

QString ResourceFile::absolutePath(const QString &rel_path) const
{
    QFileInfo fi(rel_path);
    if (fi.isAbsolute())
        return rel_path;

    return QDir::cleanPath(QFileInfo(m_file_name).path() + QDir::separator() + rel_path);
}

bool ResourceFile::contains(const QString &prefix) const
{
    return m_resource_map.contains(prefix);
}

bool ResourceFile::contains(const QString &prefix, const QString &file) const
{
    QStringList file_list = m_resource_map.value(prefix);
    return file_list.contains(file);
}

void ResourceFile::changePrefix(const QString &old_prefix, const QString &new_prefix)
{
    if (!m_resource_map.contains(old_prefix))
        return;
    if (m_resource_map.contains(new_prefix))
        return;
    QStringList file_list = m_resource_map.value(old_prefix);
    m_resource_map.remove(old_prefix);
    m_resource_map.insert(new_prefix, file_list);
}
