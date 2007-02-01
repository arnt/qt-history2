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

#include "qsimpleresource_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerLanguageExtension>
#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/extrainfo.h>

#include <QtGui/QIcon>
#include <QtGui/QWidget>

namespace qdesigner_internal {

QSimpleResource::QSimpleResource(QDesignerFormEditorInterface *core) :
    QAbstractFormBuilder(),
    m_core(core)
{
    QString workingDirectory = QDir::homePath();
    workingDirectory +=  QDir::separator();
    workingDirectory +=  QLatin1String(".designer");
    setWorkingDirectory(QDir(workingDirectory));
}

QSimpleResource::~QSimpleResource()
{

}

QBrush QSimpleResource::setupBrush(DomBrush *brush)
{
    return QAbstractFormBuilder::setupBrush(brush);
}

DomBrush *QSimpleResource::saveBrush(const QBrush &brush)
{
    return QAbstractFormBuilder::saveBrush(brush);
}

QIcon QSimpleResource::nameToIcon(const QString &filePath, const QString &qrcPath)
{
    QString file_path = filePath;
    QString qrc_path = qrcPath;

    if (qrc_path.isEmpty()) {
        QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core()->extensionManager(), core());
        if (!lang || !lang->isLanguageResource(file_path))
            file_path = workingDirectory().absoluteFilePath(file_path);
    } else {
        qrc_path = workingDirectory().absoluteFilePath(qrc_path);
    }

    return core()->iconCache()->nameToIcon(file_path, qrc_path);
}

QString QSimpleResource::iconToFilePath(const QIcon &pm) const
{
    QString file_path = core()->iconCache()->iconToFilePath(pm);
    QString qrc_path = core()->iconCache()->iconToQrcPath(pm);
    if (qrc_path.isEmpty()) {
        QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core()->extensionManager(), core());
        if (!lang || !lang->isLanguageResource(file_path))
            return workingDirectory().relativeFilePath(file_path);
    }

    return file_path;
}

QString QSimpleResource::iconToQrcPath(const QIcon &pm) const
{
    QString qrc_path = core()->iconCache()->iconToQrcPath(pm);
    if (qrc_path.isEmpty())
        return QString();

    return workingDirectory().relativeFilePath(qrc_path);
}

QPixmap QSimpleResource::nameToPixmap(const QString &filePath, const QString &qrcPath)
{
    QString file_path = filePath;
    QString qrc_path = qrcPath;

    if (qrc_path.isEmpty()) {
        QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core()->extensionManager(), core());
        if (!lang || !lang->isLanguageResource(file_path))
            file_path = workingDirectory().absoluteFilePath(file_path);
    } else {
        qrc_path = workingDirectory().absoluteFilePath(qrc_path);
    }

    return core()->iconCache()->nameToPixmap(file_path, qrc_path);
}

QString QSimpleResource::pixmapToFilePath(const QPixmap &pm) const
{
    QString file_path = core()->iconCache()->pixmapToFilePath(pm);
    QString qrc_path = core()->iconCache()->pixmapToQrcPath(pm);
    if (qrc_path.isEmpty()) {
        QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core()->extensionManager(), core());
        if (!lang || !lang->isLanguageResource(file_path))
            return workingDirectory().relativeFilePath(file_path);
    }

    return file_path;
}

QString QSimpleResource::pixmapToQrcPath(const QPixmap &pm) const
{
    QString qrc_path = core()->iconCache()->pixmapToQrcPath(pm);
    if (qrc_path.isEmpty())
        return QString();

    return workingDirectory().relativeFilePath(qrc_path);
}

void QSimpleResource::addExtensionDataToDOM(QDesignerFormEditorInterface *core, DomWidget *ui_widget, QWidget *widget)
{
    QExtensionManager *emgr = core->extensionManager();
    if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension*>(emgr, widget)) {
        extra->saveWidgetExtraInfo(ui_widget);
    }
}

void QSimpleResource::applyExtensionDataFromDOM(QDesignerFormEditorInterface *core, DomWidget *ui_widget, QWidget *widget)
{
    if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension*>(core->extensionManager(), widget)) {
        extra->loadWidgetExtraInfo(ui_widget);
    }
}
}
