#include <QtCore/QFile>
#include <QtXml/QDomDocument>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>

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
    setFileName(file_name);
}

bool ResourceFile::load()
{
    if (m_file_name.isEmpty()) {
        qWarning("ResourceFile::load(): file name is empty");
        return false;
    }

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
            file_list.append(absolutePath(felt.text()));
        m_resource_map.insert(relt.attribute(QLatin1String("prefix")), file_list);
    }

    return true;
}

bool ResourceFile::save()
{
    if (m_file_name.isEmpty()) {
        qWarning("ResourceFile::save(): file name is empty");
        return false;
    }
    
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
            QDomText text = doc.createTextNode(relativePath(f));
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
            
        QString result = absolutePath(path.mid(it.key().size() + 1));
        if (it.value().contains(result))
            return result;
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
    QStringList abs_file_list = m_resource_map.value(fixPrefix(prefix));
    QStringList result;
    foreach (QString abs_file, abs_file_list)
        result.append(relativePath(abs_file));
    return result;
}

void ResourceFile::addPrefix(const QString &prefix)
{
    QString fixed_prefix = fixPrefix(prefix);
    if (m_resource_map.contains(fixed_prefix))
        return;
    m_resource_map.insert(fixed_prefix, QStringList());
}

int ResourceFile::indexOfPrefix(const QString &prefix)
{
    QString fixed_prefix = fixPrefix(prefix);
    int i = 0;
    ResourceMap::const_iterator it = m_resource_map.begin();
    for (; it != m_resource_map.end(); ++it, ++i) {
        if (it.key() == fixed_prefix)
            return i;
    }
    return -1;
}

void ResourceFile::addFile(const QString &prefix, const QString &file)
{
    m_resource_map[fixPrefix(prefix)].append(absolutePath(file));
}

void ResourceFile::removePrefix(const QString &prefix)
{
    m_resource_map.remove(fixPrefix(prefix));
}

void ResourceFile::removeFile(const QString &prefix, const QString &file)
{
    QString fixed_prefix = fixPrefix(prefix);
    if (!m_resource_map.contains(fixed_prefix))
        return;
    m_resource_map[fixed_prefix].removeAll(absolutePath(file));
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
    return m_resource_map.contains(fixPrefix(prefix));
}

bool ResourceFile::contains(const QString &prefix, const QString &file) const
{
    QStringList file_list = m_resource_map.value(fixPrefix(prefix));
    return file_list.contains(absolutePath(file));
}

void ResourceFile::changePrefix(const QString &old_prefix, const QString &new_prefix)
{
    QString fixed_old_prefix = fixPrefix(old_prefix);
    QString fixed_new_prefix = fixPrefix(new_prefix);

    if (!m_resource_map.contains(fixed_old_prefix))
        return;
    if (m_resource_map.contains(fixed_new_prefix))
        return;
    QStringList file_list = m_resource_map.value(fixed_old_prefix);
    m_resource_map.remove(fixed_old_prefix);
    m_resource_map.insert(fixed_new_prefix, file_list);
}

QString ResourceFile::fixPrefix(const QString &prefix)
{
    QString result = QLatin1String("/");
    for (int i = 0; i < prefix.size(); ++i) {
        QChar c = prefix.at(i);
        if (c == QLatin1Char('/') && result.at(result.size() - 1) == QLatin1Char('/'))
            continue;
        result.append(c);
    }
        
    if (result.size() > 1 && result.endsWith(QLatin1String("/")))
        result = result.mid(0, result.size() - 1);

    return result;
}

int ResourceFile::prefixCount() const
{
    return m_resource_map.size();
}

QString ResourceFile::prefix(int idx) const
{
    int i = 0;
    ResourceMap::const_iterator it = m_resource_map.begin();
    for (; it != m_resource_map.end(); ++it, ++i) {
        if (i == idx)
            return it.key();
    }
    return QString();
}

int ResourceFile::fileCount(int prefix_idx) const
{
    return m_resource_map.value(prefix(prefix_idx)).size();
}

QString ResourceFile::file(int prefix_idx, int file_idx) const
{
    QStringList list = m_resource_map.value(prefix(prefix_idx));
    if (file_idx >= list.size())
        return QString();
    return relativePath(list.at(file_idx));
}

