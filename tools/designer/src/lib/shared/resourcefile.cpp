/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
TRANSLATOR qdesigner_internal::ResourceModel
*/

#include "resourcefile_p.h"
#include "resourcemimedata_p.h"

#include <QtDesigner/abstractformbuilder.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>

#include <QtGui/QIcon>
#include <QtGui/QApplication>
#include <QtGui/QImageReader>

#include <QtXml/QDomDocument>

namespace qdesigner_internal {

/******************************************************************************
** ResourceFile
*/

ResourceFile::ResourceFile(const QString &file_name)
{
    setFileName(file_name);
}

template <typename T>
static QList<T> uniqueItems(QList<T> list)
{
    QList<T> result;

    qSort(list.begin(), list.end());
    T last;
    bool first = true;
    foreach (const T &t, list) {
        if (first || t != last) {
            result.append(t);
            last = t;
        }
        first = false;
    }

    return result;
}

bool ResourceFile::load()
{
    m_error_message.clear();

    if (m_file_name.isEmpty()) {
        m_error_message = QApplication::translate("Designer", "file name is empty");
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
        m_error_message = QApplication::translate("Designer", "XML error on line %1, col %2: %3")
                    .arg(error_line).arg(error_col).arg(error_msg);
        return false;
    }

    QDomElement root = doc.firstChildElement(QLatin1String("RCC"));
    if (root.isNull()) {
        m_error_message = QApplication::translate("Designer", "no <RCC> root element");
        return false;
    }

    QDomElement relt = root.firstChildElement(QLatin1String("qresource"));
    for (; !relt.isNull(); relt = relt.nextSiblingElement(QLatin1String("qresource"))) {
        FileList file_list;
        QDomElement felt = relt.firstChildElement(QLatin1String("file"));
        for (; !felt.isNull(); felt = felt.nextSiblingElement(QLatin1String("file")))
            file_list.append(File(absolutePath(felt.text()), felt.attribute(QLatin1String("alias"))));

        QString prefix = fixPrefix(relt.attribute(QLatin1String("prefix")));
        if (prefix.isEmpty())
            prefix = QString(QLatin1Char('/'));

        const QString lang = relt.attribute(QLatin1String("lang"));

        const int idx = indexOfPrefix(prefix);
        if (idx == -1) {
            m_prefix_list.append(Prefix(prefix, lang, uniqueItems(file_list)));
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
        m_error_message = QApplication::translate("Designer", "file name is empty");
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

    const QStringList name_list = uniqueItems(prefixList());

    foreach (QString name, name_list) {
        FileList file_list;
        QString lang;
        foreach (Prefix pref, m_prefix_list) {
            if (pref.name == name){
                file_list += pref.file_list;
                lang = pref.lang;
            }
        }
        file_list = uniqueItems(file_list);

        QDomElement relt = doc.createElement(QLatin1String("qresource"));
        root.appendChild(relt);
        relt.setAttribute(QLatin1String("prefix"), name);
        if(!lang.isEmpty())
            relt.setAttribute(QLatin1String("lang"), lang);

        foreach (const File &file, file_list) {
            QDomElement felt = doc.createElement(QLatin1String("file"));
            relt.appendChild(felt);
            const QString conv_file = relativePath(file.name).replace(QDir::separator(), QLatin1Char('/'));
            const QDomText text = doc.createTextNode(conv_file);
            felt.appendChild(text);
            if (!file.alias.isEmpty())
                felt.setAttribute(QLatin1String("alias"), file.alias);
        }
    }

    QTextStream stream(&file);
    doc.save(stream, 4);

    return true;
}

bool ResourceFile::split(const QString &_path, QString *prefix, QString *file) const
{
    prefix->clear();
    file->clear();

    QString path = _path;
    if (!path.startsWith(QLatin1Char(':')))
        return false;
    path = path.mid(1);

    for (int i = 0; i < m_prefix_list.size(); ++i) {
        const Prefix &pref = m_prefix_list.at(i);
        if (!path.startsWith(pref.name))
            continue;

        *prefix = pref.name;
        if (pref.name == QString(QLatin1Char('/')))
            *file = path.mid(1);
        else
            *file = path.mid(pref.name.size() + 1);

        const QString filePath = absolutePath(*file);

        for (int j = 0; j < pref.file_list.count(); j++) {
            const File f = pref.file_list.at(j);
            if (!f.alias.isEmpty()) {
                if (absolutePath(f.alias) == filePath) {
                    *file = f.name;
                    return true;
                }
            } else if (f.name == filePath)
                return true;
        }
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
    const FileList &abs_file_list = m_prefix_list.at(pref_idx).file_list;
    QStringList result;
    foreach (const File &abs_file, abs_file_list)
        result.append(relativePath(abs_file.name));
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

void ResourceFile::replaceLang(int prefix_idx, const QString &lang)
{
    m_prefix_list[prefix_idx].lang = lang;
}

void ResourceFile::replaceAlias(int prefix_idx, int file_idx, const QString &alias)
{
    m_prefix_list[prefix_idx].file_list[file_idx].alias = alias;
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

    QFileInfo fileInfo(m_file_name);
    return fileInfo.absoluteDir().relativeFilePath(abs_path);
}

QString ResourceFile::absolutePath(const QString &rel_path) const
{
    const QFileInfo fi(rel_path);
    if (fi.isAbsolute())
        return rel_path;

    QString rc = QFileInfo(m_file_name).path();
    rc +=  QDir::separator();
    rc += rel_path;
    return QDir::cleanPath(rc);
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
    const QChar slash = QLatin1Char('/');
    QString result = QString(slash);
    for (int i = 0; i < prefix.size(); ++i) {
        const QChar c = prefix.at(i);
        if (c == slash && result.at(result.size() - 1) == slash)
            continue;
        result.append(c);
    }

    if (result.size() > 1 && result.endsWith(slash))
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

QString ResourceFile::lang(int idx) const
{
    return m_prefix_list.at(idx).lang;
}

int ResourceFile::fileCount(int prefix_idx) const
{
    return m_prefix_list.at(prefix_idx).file_list.size();
}

QString ResourceFile::file(int prefix_idx, int file_idx) const
{
    return relativePath(m_prefix_list.at(prefix_idx).file_list.at(file_idx).name);
}

QString ResourceFile::alias(int prefix_idx, int file_idx) const
{
    return m_prefix_list.at(prefix_idx).file_list.at(file_idx).alias;
}

/******************************************************************************
** ResourceModel
*/

ResourceModel::ResourceModel(const ResourceFile &resource_file, QObject *parent)
    : QAbstractItemModel(parent), m_resource_file(resource_file),  m_dirty(false)
{
    // Only action that works for QListWidget and the like.
    setSupportedDragActions(Qt::CopyAction);
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

    const qint32 d = parent.internalId();

    if (!parent.isValid()) {
        if (row < m_resource_file.prefixCount())
            result = createIndex(row, 0, -1);
    } else if (column == 0
                && d == -1
                && parent.row() < m_resource_file.prefixCount()
                && row < m_resource_file.fileCount(parent.row())) {
        result = createIndex(row, 0, parent.row());
    }

    return result;
}

QModelIndex ResourceModel::parent(const QModelIndex &index) const
{
    QModelIndex result;

    const qint32 d = index.internalId();

    if (index.isValid() && d != -1)
        result = createIndex(d, 0, -1);

    return result;
}

int ResourceModel::rowCount(const QModelIndex &parent) const
{
    int result = 0;

    const qint32 d = parent.internalId();

    if (!parent.isValid())
        result = m_resource_file.prefixCount();
    else if (d == -1)
        result = m_resource_file.fileCount(parent.row());

    return result;
}

int ResourceModel::columnCount(const QModelIndex &) const
{
    return 1;
}

bool ResourceModel::hasChildren(const QModelIndex &parent) const
{
    bool result = false;

    const qint32 d = parent.internalId();

    if (!parent.isValid())
        result = m_resource_file.prefixCount() > 0;
    else if (d == -1)
        result = m_resource_file.fileCount(parent.row()) > 0;

    return result;
}

bool ResourceModel::iconFileExtension(const QString &path)
{
    static QStringList ext_list;
    if (ext_list.isEmpty()) {
        const QList<QByteArray> _ext_list = QImageReader::supportedImageFormats();
        foreach (const QByteArray &ext, _ext_list) {
            QString dotExt = QString(QLatin1Char('.'));
            dotExt  += QString::fromAscii(ext);
            ext_list.append(dotExt);
        }
    }

    foreach (QString ext, ext_list) {
        if (path.endsWith(ext, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

static inline void appendParenthesized(const QString &what, QString &s)
{
    s += QLatin1String(" (");
    s += what;
    s += QLatin1Char(')');
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const qint32 d = index.internalId();

    QVariant result;

    switch (role) {
        case Qt::DisplayRole:
            {
                QString stringRes;
                if (d == -1){
                    stringRes = m_resource_file.prefix(index.row());
                    const QString lang = m_resource_file.lang(index.row());
                    if(!lang.isEmpty())
                        appendParenthesized(lang, stringRes);
                } else  {
                    stringRes = QFileInfo(m_resource_file.file(d, index.row())).fileName();

                    const QString alias = m_resource_file.alias(d, index.row());
                    if(!alias.isEmpty())
                        appendParenthesized(alias, stringRes);
                }
                result = stringRes;
            }
            break;
        case Qt::DecorationRole:
            if (d != -1) {
                const QString path = m_resource_file.absolutePath(m_resource_file.file(d, index.row()));
                if (iconFileExtension(path)) {
                    const QIcon icon(path);
                    if (!icon.isNull())
                        result = icon;
                }
            }
            break;
        case Qt::ToolTipRole:
            if (d != -1) {
                QString conv_file = m_resource_file.relativePath(m_resource_file.file(d, index.row()));
                QString stringRes = conv_file.replace(QDir::separator(), QLatin1Char('/'));

                const QString alias_file = m_resource_file.alias(d, index.row());
                if(!alias_file.isEmpty())
                        appendParenthesized(alias_file , stringRes);

                result = stringRes;
            }
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

    const qint32 d = index.internalId();

    if (d == -1) {
        prefix = m_resource_file.prefix(index.row());
    } else {
        prefix = m_resource_file.prefix(d);
        file = m_resource_file.file(d, index.row());
        const QString alias = m_resource_file.alias(d, index.row());
        if (!alias.isEmpty())
            file = alias;
    }
}

QString ResourceModel::lang(const QModelIndex &index) const
{
    if(!index.isValid())
        return QString();

    return m_resource_file.lang(index.row());
}

QString ResourceModel::alias(const QModelIndex &index) const
{
    if(!index.parent().isValid())
        return QString();
    return m_resource_file.alias(index.parent().row(), index.row());
}

QModelIndex ResourceModel::getIndex(const QString &prefixed_file)
{
    QString prefix, file;
    if (!m_resource_file.split(prefixed_file, &prefix, &file))
        return QModelIndex();
    return getIndex(prefix, file);
}

QModelIndex ResourceModel::getIndex(const QString &prefix, const QString &file)
{
    if (prefix.isEmpty())
        return QModelIndex();

    const int pref_idx = m_resource_file.indexOfPrefix(prefix);
    if (pref_idx == -1)
        return QModelIndex();

    const QModelIndex pref_model_idx = index(pref_idx, 0, QModelIndex());
    if (file.isEmpty())
        return pref_model_idx;

    const int file_idx = m_resource_file.indexOfFile(pref_idx, file);
    if (file_idx == -1)
        return QModelIndex();

    return index(file_idx, 0, pref_model_idx);
}

QModelIndex ResourceModel::prefixIndex(const QModelIndex &sel_idx) const
{
    if (!sel_idx.isValid())
        return QModelIndex();
    const QModelIndex parentIndex = parent(sel_idx);
    return parentIndex.isValid() ? parentIndex : sel_idx;
}

QModelIndex ResourceModel::addNewPrefix()
{
    int i = 0;
    QString prefix = tr("/new/prefix1");
    while (m_resource_file.contains(prefix))
        prefix = tr("/new/prefix%1").arg((++i)+1);

    i = rowCount(QModelIndex());
    beginInsertRows(QModelIndex(), i, i);
    m_resource_file.addPrefix(prefix);
    endInsertRows();

    setDirty(true);

    return index(i, 0, QModelIndex());
}

QModelIndex ResourceModel::addFiles(const QModelIndex &model_idx, const QStringList &file_list)
{
    if (!model_idx.isValid())
        return QModelIndex();
    const QModelIndex prefix_model_idx = prefixIndex(model_idx);
    const int prefix_idx = prefix_model_idx.row();

    QStringList unique_list;
    foreach (QString file, file_list) {
        if (!m_resource_file.contains(prefix_idx, file) && !unique_list.contains(file))
            unique_list.append(file);
    }

    if (unique_list.isEmpty())
        return QModelIndex();

    const int cnt = m_resource_file.fileCount(prefix_idx);
    beginInsertRows(prefix_model_idx, cnt, cnt + unique_list.count() - 1); // ### FIXME

    foreach (QString file, file_list)
        m_resource_file.addFile(prefix_idx, file);

    const QFileInfo fi(file_list.last());
    m_lastResourceDir = fi.absolutePath();

    endInsertRows();
    setDirty(true);

    return index(cnt + unique_list.count() - 1, 0, prefix_model_idx);
}

void ResourceModel::changePrefix(const QModelIndex &model_idx, const QString &prefix)
{
    if (!model_idx.isValid())
        return;

    const QModelIndex prefix_model_idx = prefixIndex(model_idx);
    const int prefix_idx = model_idx.row();
    if (m_resource_file.prefix(prefix_idx) == ResourceFile::fixPrefix(prefix))
        return;

    if(m_resource_file.contains(prefix))
        return;

    m_resource_file.replacePrefix(prefix_idx, prefix);
    emit dataChanged(prefix_model_idx, prefix_model_idx);
    setDirty(true);
}

void ResourceModel::changeLang(const QModelIndex &model_idx, const QString &lang)
{
    if (!model_idx.isValid())
        return;

    const QModelIndex prefix_model_idx = prefixIndex(model_idx);
    const int prefix_idx = model_idx.row();
    if (m_resource_file.lang(prefix_idx) == lang)
        return;

    m_resource_file.replaceLang(prefix_idx, lang);
    emit dataChanged(prefix_model_idx, prefix_model_idx);
    setDirty(true);
}

void ResourceModel::changeAlias(const QModelIndex &index, const QString &alias)
{
    if (!index.parent().isValid())
        return;

    if(m_resource_file.alias(index.parent().row(), index.row()) == alias)
        return;
    m_resource_file.replaceAlias(index.parent().row(), index.row(), alias);
    emit dataChanged(index, index);
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

    beginRemoveRows(parent(idx), idx.row(), idx.row());

    if (file.isEmpty()) {
        m_resource_file.removePrefix(prefix_idx);
        if (prefix_idx == m_resource_file.prefixCount())
            --prefix_idx;
    } else {
        m_resource_file.removeFile(prefix_idx, file_idx);
        if (file_idx == m_resource_file.fileCount(prefix_idx))
            --file_idx;
    }

    endRemoveRows();
    setDirty(true);

    if (prefix_idx == -1)
        return QModelIndex();
    const QModelIndex prefix_model_idx = index(prefix_idx, 0, QModelIndex());
    if (file_idx == -1)
        return prefix_model_idx;
    return index(file_idx, 0, prefix_model_idx);
}

bool ResourceModel::reload()
{
    const bool result = m_resource_file.load();
    if (result)
        setDirty(false);
    return result;
}

bool ResourceModel::save()
{
    const bool result = m_resource_file.save();
    if (result)
        setDirty(false);
    return result;
}

QString ResourceModel::lastResourceOpenDirectory() const
{
    if (m_lastResourceDir.isEmpty())
        return absolutePath(QString());
    return m_lastResourceDir;
}

// Create a resource path 'prefix:/file'
QString ResourceModel::resourcePath(const QString &prefix, const QString &file)
{
    QString rc = QString(QLatin1Char(':'));
    rc += prefix;
    rc += QLatin1Char('/');
    rc += file;
    return QDir::cleanPath(rc);
}

QMimeData *ResourceModel::mimeData(const QModelIndexList & indexes) const
{
    if (indexes.size() != 1)
        return 0;

    QString prefix, file;
    getItem(indexes.front(), prefix, file);
    if (prefix.isEmpty() || file.isEmpty())
        return 0;
    ResourceMimeData md(iconFileExtension(file) ?  ResourceMimeData::Image : ResourceMimeData::File);
    md.setQrcPath(m_resource_file.fileName());
    md.setFilePath(resourcePath(prefix, file));
    return md.toMimeData();
}
} // namespace qdesigner_internal
