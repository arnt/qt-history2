/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner.h"
#include "qdesigner_settings.h"
#include "qdesigner_widgetbox.h"
#include "qdesigner_propertyeditor.h"
#include "qdesigner_objectinspector.h"

#include <QtCore/QVariant>
#include <QtCore/QDir>

#include <QtGui/QDesktopWidget>

QDesignerSettings::QDesignerSettings()
    : QSettings()
{
    m_designerPath = QLatin1String("/.designer");
}

QDesignerSettings::~QDesignerSettings()
{
}

QStringList QDesignerSettings::formTemplatePaths() const
{
    return value("FormTemplatePaths", defaultFormTemplatePaths()).toStringList();
}

void QDesignerSettings::setFormTemplatePaths(const QStringList &paths)
{
    setValue("FormTemplatePaths", paths);
}

QStringList QDesignerSettings::defaultFormTemplatePaths() const
{
    QStringList paths;

    QString templatePath = QLatin1String("/templates");

    paths.append(QDir::homePath() + m_designerPath + templatePath);
    paths.append(qDesigner->applicationDirPath() + templatePath);

    return paths;
}

void QDesignerSettings::saveGeometryFor(const QWidget *w)
{
    Q_ASSERT(w && !w->objectName().isNull());
    const QWidget *widgetToPass = w;
    if (w->parentWidget()) {
        widgetToPass = w->parentWidget();
    }

    saveGeometryHelper(widgetToPass, w->objectName());
}

void QDesignerSettings::setGeometryFor(QWidget *w, const QRect &fallBack) const
{
    Q_ASSERT(w && !w->objectName().isNull());
    QWidget *widgetToPass = w;
    if (w->parentWidget())
        widgetToPass = w->parentWidget();

    setGeometryHelper(widgetToPass, w->objectName(),
                      fallBack.isNull() ? QRect(QPoint(0, 0), w->sizeHint()) : fallBack);
}

void QDesignerSettings::saveGeometryHelper(const QWidget *w, const QString &key)
{
    beginGroup(key);
    setValue("screen", QApplication::desktop()->screenNumber(w));
    setValue("geometry", w->geometry());
    setValue("visible", w->isVisible());
    endGroup();

}

void QDesignerSettings::setGeometryHelper(QWidget *w, const QString &key,
                                          const QRect &fallBack) const
{
//    beginGroup();
    QRect g = value(key + "/geometry", fallBack).toRect();
    if (g.intersect(QApplication::desktop()->availableGeometry()).isEmpty())
        g = fallBack;
    w->resize(g.size());
    w->move(g.topLeft());
    if (value(key + "/visible", true).toBool())
        w->show();
//    endGroup();
}
