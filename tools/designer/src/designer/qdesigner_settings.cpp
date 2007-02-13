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

#include "qdesigner.h"
#include "qdesigner_settings.h"
#include "qdesigner_widgetbox.h"
#include "qdesigner_workbench.h"
#include "qdesigner_propertyeditor.h"
#include "qdesigner_objectinspector.h"

#include <QtCore/QVariant>
#include <QtCore/QDir>

#include <QtGui/QDesktopWidget>
#include <QtGui/QStyle>

#include <QtCore/qdebug.h>

static bool ensurePath(const QString &path)
{
    QDir current(QDir::current());
    if (current.exists(path) || current.mkpath(path))
        return true;

    qWarning() << "Unable to create the template path " << path << '.';
    return false;
}

QDesignerSettings::QDesignerSettings() :
   m_designerPath(QLatin1String("/.designer"))
{
}

QStringList QDesignerSettings::defaultFormTemplatePaths() const
{
    QStringList rc;
    // Ensure default form template paths
    const QString templatePath = QLatin1String("/templates");
    // home
    QString path = QDir::homePath();
    path += m_designerPath;
    path += templatePath;
    if (ensurePath(path))
        rc += path;

    // designer/bin
    path = qDesigner->applicationDirPath();
    path += templatePath;
    if (ensurePath(path))
        rc += path;
    return rc;
}

QStringList QDesignerSettings::formTemplatePaths() const
{
    return value(QLatin1String("FormTemplatePaths"),defaultFormTemplatePaths()).toStringList();
}

void QDesignerSettings::setFormTemplatePaths(const QStringList &paths)
{
    setValue(QLatin1String("FormTemplatePaths"), paths);
}

QString QDesignerSettings::defaultUserWidgetBoxXml() const
{
    QString rc = QDir::homePath();
    rc +=  m_designerPath;
    rc += QLatin1String("/widgetbox.xml");
    return rc;
}

void QDesignerSettings::saveGeometryFor(const QWidget *w)
{
    Q_ASSERT(w && !w->objectName().isEmpty());
    saveGeometryHelper(w, w->objectName());
}

void QDesignerSettings::setGeometryFor(QWidget *w, const QRect &fallBack) const
{
    Q_ASSERT(w && !w->objectName().isEmpty());
    setGeometryHelper(w, w->objectName(),
                      fallBack.isNull() ? QRect(QPoint(0, 0), w->sizeHint()) : fallBack);
}

void QDesignerSettings::saveGeometryHelper(const QWidget *w, const QString &key)
{
    beginGroup(key);
    QPoint pos = w->pos();
    if (!w->isWindow()) // in workspace
        pos = w->parentWidget()->pos();

    setValue(QLatin1String("screen"), QApplication::desktop()->screenNumber(w));
    setValue(QLatin1String("geometry"), QRect(pos, w->size()));
    setValue(QLatin1String("visible"), w->isVisible());
    setValue(QLatin1String("maximized"), w->isMaximized());
    endGroup();
}

void QDesignerSettings::setGeometryHelper(QWidget *w, const QString &key,
                                          const QRect &fallBack) const
{
    const int screen = value(key + QLatin1String("/screen"), 0).toInt();
    QRect g = value(key + QLatin1String("/geometry"), fallBack).toRect();
    const QRect screenRect = QApplication::desktop()->availableGeometry(screen);

    // Do geometry in a couple of steps
    // 1) Make sure the rect is within the specified geometry
    // 2) Make sure the bottom right and top left fit on the screen, move them in.
    // 3) Check again and resize.

    if (w->isWindow() && g.intersect(screenRect).isEmpty())
        g = fallBack;

    // Maybe use center?
    if (!screenRect.contains(g.bottomRight())) {
        g.moveRight(qMax(0 + g.width(), qMin(screenRect.right(), g.right())));
        g.moveBottom(qMax(0 + g.height(), qMin(screenRect.bottom(), g.bottom())));
    }

    if (!screenRect.contains(g.topLeft())) {
        g.moveLeft(qMin(screenRect.right() - g.width(), qMax(screenRect.left(), g.left())));
        g.moveTop(qMin(screenRect.bottom() - g.height(), qMax(screenRect.top(), g.top())));
    }

    if (!screenRect.contains(g.bottomRight())) {
        g.setRight(qMin(screenRect.right(), g.right()));
        g.moveBottom(qMin(screenRect.bottom(), g.bottom()));
    }

    if (!screenRect.contains(g.topLeft())) {
        g.setLeft(qMax(0, qMin(screenRect.left(), g.left())));
        g.moveTop(qMax(0, qMin(screenRect.top(), g.top())));
    }


    if (!w->isWindow()) // in workspace
        w->parentWidget()->move(g.topLeft());
    else
        w->move(g.topLeft());

    if (value(key + QLatin1String("/maximized"), false).toBool()) {
        w->setWindowState(w->windowState() | Qt::WindowMaximized);
    } else {
        w->resize(g.size());
    }

    if (value(key + QLatin1String("/visible"), true).toBool())
        w->show();
}

QStringList QDesignerSettings::recentFilesList() const
{
    return value(QLatin1String("recentFilesList")).toStringList();
}

void QDesignerSettings::setRecentFilesList(const QStringList &sl)
{
    setValue(QLatin1String("recentFilesList"), sl);
}

void QDesignerSettings::setShowNewFormOnStartup(bool showIt)
{
    setValue(QLatin1String("newFormDialog/ShowOnStartup"), showIt);
}

bool QDesignerSettings::showNewFormOnStartup() const
{
    return value(QLatin1String("newFormDialog/ShowOnStartup"), true).toBool();
}

void QDesignerSettings::setUIMode(UIMode mode)
{
    setValue(QLatin1String("UI/currentMode"), mode);
}

UIMode QDesignerSettings::uiMode() const
{
#ifdef Q_WS_WIN
    const UIMode defaultMode = DockedMode;
#else
    const UIMode defaultMode = TopLevelMode;
#endif
    return static_cast<UIMode>(value(QLatin1String("UI/currentMode"), defaultMode).toInt());
}

QByteArray QDesignerSettings::mainWindowState() const
{
    return value(QLatin1String("MainWindowState")).toByteArray();
}

void QDesignerSettings::setMainWindowState(const QByteArray &mainWindowState)
{
    setValue(QLatin1String("MainWindowState"), mainWindowState);
}

void QDesignerSettings::clearBackup()
{
    remove(QLatin1String("backup/fileListOrg"));
    remove(QLatin1String("backup/fileListBak"));
}

void QDesignerSettings::setBackup(const QMap<QString, QString> &map)
{
    const QStringList org = map.keys();
    const QStringList bak = map.values();

    setValue(QLatin1String("backup/fileListOrg"), org);
    setValue(QLatin1String("backup/fileListBak"), bak);
}

QMap<QString, QString> QDesignerSettings::backup() const
{
    const QStringList org = value(QLatin1String("backup/fileListOrg"), QStringList()).toStringList();
    const QStringList bak = value(QLatin1String("backup/fileListBak"), QStringList()).toStringList();

    QMap<QString, QString> map;
    for (int i = 0; i < org.count(); ++i)
        map.insert(org.at(i), bak.at(i));

    return map;
}
