#include <QtCore/QFile>
#include <QtXml/QDomDocument>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>
#include <QtGui/QMessageBox>
#include <QtGui/QIcon>

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

/******************************************************************************
** ResourceFile
*/

ResourceFile::ResourceFile(const QString &file_name)
{
    setFileName(file_name);
}

bool ResourceFile::load()
{
    m_error_message.clear();
    
    if (m_file_name.isEmpty()) {
        m_error_message = QObject::tr("file name is empty");
        return false;
    }

    QFile file(m_file_name);
    if (!file.open(QIODevice::ReadOnly)) {
        m_error_message = file.errorString();
        return false;
    }

    m_resource_map.clear();

    QDomDocument doc;

    QString error_msg;
    int error_line, error_col;
    if (!doc.setContent(&file, &error_msg, &error_line, &error_col)) {
        m_error_message = QObject::tr("XML error on line %1, col %2: %3")
                    .arg(error_line).arg(error_col).arg(error_msg);
        return false;
    } 

    QDomElement root = doc.firstChildElement(QLatin1String("RCC"));
    if (root.isNull()) {
        m_error_message = QObject::tr("no <RCC> root element");
        return false;
    }

    QDomElement relt = root.firstChildElement(QLatin1String("qresource"));
    for (; !relt.isNull(); relt = relt.nextSiblingElement(QLatin1String("qresource"))) {
        QStringList file_list;
        QDomElement felt = relt.firstChildElement(QLatin1String("file"));
        for (; !felt.isNull(); felt = felt.nextSiblingElement(QLatin1String("file")))
            file_list.append(absolutePath(felt.text()));
        m_resource_map.insert(fixPrefix(relt.attribute(QLatin1String("prefix"))), file_list);
    }

    return true;
}

bool ResourceFile::save()
{
    m_error_message.clear();

    if (m_file_name.isEmpty()) {
        m_error_message = QObject::tr("file name is empty");
        return false;
    }
    
    QFile file(m_file_name);
    if (!file.open(QIODevice::WriteOnly)) {
        m_error_message = file.errorString();
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

bool ResourceFile::split(const QString &_path, QString &prefix, QString &file) const
{
    prefix.clear();
    file.clear();

    QString path = _path;
    
    if (!path.startsWith(QLatin1String(":")))
        return false;
    path = path.mid(1);

    ResourceMap::const_iterator it = m_resource_map.begin();
    for (; it != m_resource_map.end(); ++it) {
        if (!path.startsWith(it.key()))
            continue;

        prefix = it.key();
        file = path.mid(it.key().size() + 1);
        return it.value().contains(absolutePath(file));
    }

    return false;
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

int ResourceFile::indexOfFile(int pref_idx, const QString &file)
{
    QString prefix = this->prefix(pref_idx);
    if (prefix.isEmpty())
        return -1;
    return m_resource_map.value(prefix).indexOf(absolutePath(file));
}

void ResourceFile::addFile(const QString &prefix, const QString &file)
{
    QString abs_file = absolutePath(file);
    QString fixed_prefix = fixPrefix(prefix);

    m_resource_map[fixed_prefix].append(abs_file);
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
    if (m_file_name.isEmpty() || QFileInfo(abs_path).isRelative())
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
    QString path = list.at(file_idx);
    QString rel_path = relativePath(path);
    return rel_path;
}

/******************************************************************************
** ResourceModel
*/

ResourceModel::ResourceModel(const ResourceFile &resource_file, QObject *parent)
    : QAbstractItemModel(parent), m_resource_file(resource_file)
{
    m_dirty = false;
}

void ResourceModel::setDirty(bool b)
{
    if (b == m_dirty)
        return;
    m_dirty = b;
    emit dirtyChanged(b);
}

QModelIndex ResourceModel::index(int row, int column,
                                    const QModelIndex &parent) const
{
    QModelIndex result;

    qint64 d = reinterpret_cast<qint64>(parent.data());
    
    if (!parent.isValid()) {
        if (row < m_resource_file.prefixCount())
            result = createIndex(row, 0, reinterpret_cast<void*>(-1));
    } else if (column == 0
                && d == -1
                && parent.row() < m_resource_file.prefixCount()
                && row < m_resource_file.fileCount(parent.row())) {
        result = createIndex(row, 0, reinterpret_cast<void*>(parent.row()));
    }

//    qDebug() << "ResourceModel::index(" << row << column << parent << "):" << result;
    
    return result;
}

QModelIndex ResourceModel::parent(const QModelIndex &index) const
{
    QModelIndex result;

    qint64 d = reinterpret_cast<qint64>(index.data());
    
    if (index.isValid() && d != -1)
        result = createIndex(d, 0, reinterpret_cast<void*>(-1));

//    qDebug() << "ResourceModel::parent(" << index << "):" << result;
    
    return result;
}

int ResourceModel::rowCount(const QModelIndex &parent) const
{
    int result = 0;
    
    qint64 d = reinterpret_cast<qint64>(parent.data());
    
    if (!parent.isValid())
        result = m_resource_file.prefixCount();
    else if (d == -1)
        result = m_resource_file.fileCount(parent.row());

//    qDebug() << "ResourceModel::rowCount(" << parent << "):" << result;
    
    return result;
}

int ResourceModel::columnCount(const QModelIndex &) const
{
    return 1;
}

bool ResourceModel::hasChildren(const QModelIndex &parent) const
{
    bool result = false;
    
    qint64 d = reinterpret_cast<qint64>(parent.data());
    
    if (!parent.isValid())
        result = m_resource_file.prefixCount() > 0;
    else if (d == -1)
        result = m_resource_file.fileCount(parent.row()) > 0;

//    qDebug() << "ResourceModel::hasChildren(" << parent << "):" << result;
    
    return result;
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    qint64 d = reinterpret_cast<qint64>(index.data());

    QVariant result;

    switch (role) {
        case DisplayRole:
            if (d == -1)
                result = m_resource_file.prefix(index.row());
            else
                result = QFileInfo(m_resource_file.file(d, index.row())).fileName();
            break;
        case DecorationRole:
            if (d != -1) {
                QIcon icon(m_resource_file.absolutePath(m_resource_file.file(d, index.row())));
                if (!icon.isNull())
                    result = icon;
            }
            break;
        case ToolTipRole:
            if (d != -1)
                result = m_resource_file.relativePath(m_resource_file.file(d, index.row()));
            break;
            
        default:
            break;
    };

    return result;
}

void ResourceModel::getItem(const QModelIndex &index, QString &prefix, QString &file) const
{
    prefix.clear();
    file.clear();

    if (!index.isValid())
        return;

    qint64 d = reinterpret_cast<qint64>(index.data());

    if (d == -1) {
        prefix = m_resource_file.prefix(index.row());
    } else {
        prefix = m_resource_file.prefix(d);
        file = m_resource_file.file(d, index.row());
    }
}

QModelIndex ResourceModel::getIndex(const QString &prefix, const QString &file)
{
    if (prefix.isEmpty())
        return QModelIndex();

    int pref_idx = m_resource_file.indexOfPrefix(prefix);
    if (pref_idx == -1)
        return QModelIndex();

    QModelIndex pref_model_idx = index(pref_idx, 0, QModelIndex());
    if (file.isEmpty())
        return pref_model_idx;
        
    int file_idx = m_resource_file.indexOfFile(pref_idx, file);
    if (file_idx == -1)
        return QModelIndex();

    return index(file_idx, 0, pref_model_idx);
}

QModelIndex ResourceModel::prefixIndex(const QModelIndex &sel_idx) const
{
    if (!sel_idx.isValid())
        return QModelIndex();
    QModelIndex parent = this->parent(sel_idx);
    return parent.isValid() ? parent : sel_idx;
}

QModelIndex ResourceModel::addNewPrefix()
{
    int i = 1;
    QString prefix;
    do prefix = tr("/new/prefix%1").arg(i++);
    while (m_resource_file.contains(prefix));

    m_resource_file.addPrefix(prefix);
    i = m_resource_file.indexOfPrefix(prefix);
    emit rowsInserted(QModelIndex(), i, i);

    setDirty(true);
    
    return index(i, 0, QModelIndex());
}

QModelIndex ResourceModel::addFiles(const QModelIndex &idx, const QStringList &file_list)
{
    if (!idx.isValid())
        return QModelIndex();
    QModelIndex prefix_idx = prefixIndex(idx);
    QString prefix = m_resource_file.prefix(prefix_idx.row());
    Q_ASSERT(!prefix.isEmpty());

    int added = 0;
    foreach (QString file, file_list) {
        if (m_resource_file.contains(prefix, file))
            continue;
        m_resource_file.addFile(prefix, file);
        ++added;
    }

    int cnt = m_resource_file.fileCount(prefix_idx.row());
    if (added > 0) {
        emit rowsInserted(prefix_idx, cnt - added, cnt - 1);
        setDirty(true);
    }
    
    return index(cnt - 1, 0, prefix_idx);
}

void ResourceModel::changePrefix(const QModelIndex &idx, const QString &prefix)
{
    if (!idx.isValid())
        return;

    QString fixed_prefix = ResourceFile::fixPrefix(prefix);
    if (m_resource_file.contains(fixed_prefix))
        return;

    QModelIndex prefix_idx = prefixIndex(idx);

    QString old_prefix = m_resource_file.prefix(prefix_idx.row());
    Q_ASSERT(!old_prefix.isEmpty());

    m_resource_file.changePrefix(old_prefix, fixed_prefix);
    emit dataChanged(prefix_idx, prefix_idx);
    setDirty(true);
}

QModelIndex ResourceModel::deleteItem(const QModelIndex &idx)
{
    if (!idx.isValid())
        return QModelIndex();

    QString prefix, file;
    getItem(idx, prefix, file);
    Q_ASSERT(!prefix.isEmpty());
    int prefix_idx = m_resource_file.indexOfPrefix(prefix);
    int file_idx = m_resource_file.indexOfFile(prefix_idx, file);

    emit rowsAboutToBeRemoved(parent(idx), idx.row(), idx.row());

    if (file.isEmpty()) {
        m_resource_file.removePrefix(prefix);
        if (prefix_idx == m_resource_file.prefixCount())
            --prefix_idx;
    } else {
        m_resource_file.removeFile(prefix, file);
        if (file_idx == m_resource_file.fileCount(prefix_idx))
            --file_idx;
    }
    
    setDirty(true);

    if (prefix_idx == -1)
        return QModelIndex();
    QModelIndex prefix_model_idx = index(prefix_idx, 0, QModelIndex());
    if (file_idx == -1)
        return prefix_model_idx;
    return index(file_idx, 0, prefix_model_idx);
}

void ResourceModel::reload()
{
    if (!m_resource_file.load()) {
        QMessageBox::warning(0, tr("Error opening resource file"),
                                tr("Failed to open \"%1\":\n%2")
                                    .arg(m_resource_file.fileName())
                                    .arg(m_resource_file.errorMessage()),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }
    emit reset();
    setDirty(false);
}

void ResourceModel::save()
{
    if (!m_resource_file.save()) {
        QMessageBox::warning(0, tr("Error saving resource file"),
                                tr("Failed to save \"%1\":\n%2")
                                    .arg(m_resource_file.fileName())
                                    .arg(m_resource_file.errorMessage()),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }
    setDirty(false);
}

