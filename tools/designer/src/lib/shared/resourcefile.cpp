#include <QtCore/QFile>
#include <QtXml/QDomDocument>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>
#include <QtGui/QMessageBox>
#include <QtGui/QIcon>

#include "resourcefile.h"

static QString relativePath(const QString &_dir, const QString &_file)
{
    QString dir = QDir::cleanPath(_dir);
    QString file = QDir::cleanPath(_file);

#ifdef Q_OS_WIN
    QString root_path = QDir(dir).rootPath();
    if (root_path != QDir(QFileInfo(file).path()).rootPath()) {
        return file;
    } else {
        dir.remove(0, root_path.size() - 1);
        file.remove(0, root_path.size() - 1);
    }
    // QDir::cleanPath return always a '/' as separator.
    // Stupid workarround for Windows for now.
    dir = dir.replace("/", "\\");
    file = file.replace("/", "\\");
#endif

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

static QStringList uniqueItems(QStringList list)
{
    QStringList result;
    
    list.sort();
    QString last;
    bool first = true;
    foreach (QString s, list) {
        if (first || s != last) {
            result.append(s);
            last = s;
        }
        first = false;
    }

    return result;
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

    m_prefix_list.clear();

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

        QString prefix = fixPrefix(relt.attribute(QLatin1String("prefix")));
        int idx = indexOfPrefix(prefix);
        if (idx == -1) {
            m_prefix_list.append(Prefix(prefix, uniqueItems(file_list)));
        } else {
            Prefix &pref = m_prefix_list[idx];
            pref.file_list += file_list;
            pref.file_list = uniqueItems(pref.file_list);
        }
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

    QStringList name_list = uniqueItems(prefixList());
    
    foreach (QString name, name_list) {
        QStringList file_list;
        foreach (Prefix pref, m_prefix_list) {
            if (pref.name == name)
                file_list += pref.file_list;
        }
        file_list = uniqueItems(file_list);
                
        QDomElement relt = doc.createElement(QLatin1String("qresource"));
        root.appendChild(relt);
        relt.setAttribute(QLatin1String("prefix"), name);

        foreach (QString file, file_list) {
            QDomElement felt = doc.createElement(QLatin1String("file"));
            relt.appendChild(felt);
            QDomText text = doc.createTextNode(relativePath(file));
            felt.appendChild(text);
        }
    }

    QTextStream stream(&file);
    doc.save(stream, 4);

    return true;
}

int ResourceFile::matchPrefix(const QString &_path) const
{
    QString path = _path;
    
    if (!path.startsWith(QLatin1String(":")))
        return -1;
    
    path = path.mid(1);

    for (int i = 0; i < m_prefix_list.size(); ++i) {
        const Prefix &prefix = m_prefix_list.at(i);
        if (path.startsWith(prefix.name))
            return i;
    }
    
    return -1;        
}

bool ResourceFile::split(const QString &_path, QString *prefix, QString *file) const
{
    prefix->clear();
    file->clear();

    QString path = _path;
    if (!path.startsWith(QLatin1String(":")))
        return false;
    path = path.mid(1);
    
    for (int i = 0; i < m_prefix_list.size(); ++i) {
        const Prefix &pref = m_prefix_list.at(i);
        if (!path.startsWith(pref.name))
            continue;
    
        *prefix = pref.name;
        *file = path.mid(pref.name.size() + 1);
        
        if (pref.file_list.contains(absolutePath(*file)))
            return true;
    }
    
    return false;
}

QString ResourceFile::resolvePath(const QString &path) const
{
    QString prefix, file;
    if (split(path, &prefix, &file))
        return absolutePath(file);
    
    return QString();
}

QStringList ResourceFile::prefixList() const
{
    QStringList result;
    for (int i = 0; i < m_prefix_list.size(); ++i)
        result.append(m_prefix_list.at(i).name);
    return result;
}

bool ResourceFile::isEmpty() const
{
    return m_file_name.isEmpty() && m_prefix_list.isEmpty();
}

QStringList ResourceFile::fileList(int pref_idx) const
{
    const QStringList &abs_file_list = m_prefix_list.at(pref_idx).file_list;
    QStringList result;
    foreach (QString abs_file, abs_file_list)
        result.append(relativePath(abs_file));
    return result;
}

void ResourceFile::addFile(int prefix_idx, const QString &file)
{
    m_prefix_list[prefix_idx].file_list.append(absolutePath(file));
}

void ResourceFile::addPrefix(const QString &prefix)
{
    QString fixed_prefix = fixPrefix(prefix);
    if (indexOfPrefix(fixed_prefix) != -1)
        return;
    m_prefix_list.append(fixed_prefix);
}

void ResourceFile::removePrefix(int prefix_idx)
{
    m_prefix_list.removeAt(prefix_idx);
}

void ResourceFile::removeFile(int prefix_idx, int file_idx)
{
    m_prefix_list[prefix_idx].file_list.removeAt(file_idx);
}

void ResourceFile::replacePrefix(int prefix_idx, const QString &prefix)
{
    m_prefix_list[prefix_idx].name = fixPrefix(prefix);
}

void ResourceFile::replaceFile(int pref_idx, int file_idx, const QString &file)
{
    m_prefix_list[pref_idx].file_list[file_idx] = file;
}

int ResourceFile::indexOfPrefix(const QString &prefix) const
{
    QString fixed_prefix = fixPrefix(prefix);
    for (int i = 0; i < m_prefix_list.size(); ++i) {
        if (m_prefix_list.at(i).name == fixed_prefix)
            return i;
    }
    return -1;
}

int ResourceFile::indexOfFile(int pref_idx, const QString &file) const
{
    return m_prefix_list.at(pref_idx).file_list.indexOf(absolutePath(file));
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

bool ResourceFile::contains(const QString &prefix, const QString &file) const
{
    int pref_idx = indexOfPrefix(prefix);
    if (pref_idx == -1)
        return false;
    if (file.isEmpty())
        return true;
    return m_prefix_list.at(pref_idx).file_list.contains(absolutePath(file));
}

bool ResourceFile::contains(int pref_idx, const QString &file) const
{
    return m_prefix_list.at(pref_idx).file_list.contains(absolutePath(file));
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
    return m_prefix_list.size();
}

QString ResourceFile::prefix(int idx) const
{
    return m_prefix_list.at(idx).name;
}

int ResourceFile::fileCount(int prefix_idx) const
{
    return m_prefix_list.at(prefix_idx).file_list.size();
}

QString ResourceFile::file(int prefix_idx, int file_idx) const
{
    return m_prefix_list.at(prefix_idx).file_list.at(file_idx);
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

QModelIndex ResourceModel::addFiles(const QModelIndex &model_idx, const QStringList &file_list)
{
    if (!model_idx.isValid())
        return QModelIndex();
    QModelIndex prefix_model_idx = prefixIndex(model_idx);
    int prefix_idx = prefix_model_idx.row();

    int added = 0;
    foreach (QString file, file_list) {
        if (!m_resource_file.contains(prefix_idx, file)) {
            m_resource_file.addFile(prefix_idx, file);
            ++added;
        }
    }

    int cnt = m_resource_file.fileCount(prefix_idx);
    if (added > 0) {
        emit rowsInserted(prefix_model_idx, cnt - added, cnt - 1);
        setDirty(true);
    }
    
    return index(cnt - 1, 0, prefix_model_idx);
}

void ResourceModel::changePrefix(const QModelIndex &model_idx, const QString &prefix)
{
    if (!model_idx.isValid())
        return;

    QModelIndex prefix_model_idx = prefixIndex(model_idx);
    int prefix_idx = model_idx.row();
    if (m_resource_file.prefix(prefix_idx) == ResourceFile::fixPrefix(prefix))
        return;
        
    m_resource_file.replacePrefix(prefix_idx, prefix);
    emit dataChanged(prefix_model_idx, prefix_model_idx);
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
        m_resource_file.removePrefix(prefix_idx);
        if (prefix_idx == m_resource_file.prefixCount())
            --prefix_idx;
    } else {
        m_resource_file.removeFile(prefix_idx, file_idx);
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

