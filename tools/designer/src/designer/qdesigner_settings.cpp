/****************************************************************************
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
#include "qdesigner_workbench.h"
#include "qdesigner_propertyeditor.h"
#include "qdesigner_objectinspector.h"

#include <QtCore/QVariant>
#include <QtCore/QDir>

#include <QtGui/QDesktopWidget>
#include <QtGui/QHeaderView>

#include <QtCore/qdebug.h>

QDesignerSettings::QDesignerSettings()
    : QSettings()
{
    m_designerPath = QLatin1String("/.designer");

    QStringList paths = defaultFormTemplatePaths();
    foreach (QString path, paths) {
        if (!QDir::current().exists(path))
            QDir::current().mkpath(path);
    }
}

QDesignerSettings::~QDesignerSettings()
{
}

QStringList QDesignerSettings::formTemplatePaths() const
{
    return value(QLatin1String("FormTemplatePaths"),
                 defaultFormTemplatePaths()).toStringList();
}

void QDesignerSettings::setFormTemplatePaths(const QStringList &paths)
{
    setValue(QLatin1String("FormTemplatePaths"), paths);
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
//    beginGroup();
    int screen = value(key + QLatin1String("/screen"), 0).toInt();
    QRect g = value(key + QLatin1String("/geometry"), fallBack).toRect();

    if (w->isWindow() && g.intersect(QApplication::desktop()->availableGeometry(screen)).isEmpty()) {
        g = fallBack;
    }

    if (value(key + QLatin1String("/maximized"), false).toBool()) {
        w->setWindowState(w->windowState() | Qt::WindowMaximized);
    } else {
        if (!w->isWindow()) // in workspace
            w->parentWidget()->move(g.topLeft());
        else
            w->move(g.topLeft());

        w->resize(g.size());
    }
    if (value(key + QLatin1String("/visible"), true).toBool())
        w->show();
//    endGroup();
}

void QDesignerSettings::setHeaderSizesFor(QHeaderView *hv) const
{
    if (!hv)
        return;

    QString key;
    QWidget *w = hv;
    while (key.isEmpty() && w) {
        key = w->objectName();
        w = w->parentWidget();
    }
    Q_ASSERT(!key.isEmpty());
    QList<QVariant> sizeHints;
    for (int i = 0; i < hv->count(); ++i)
        sizeHints.append(hv->sectionSizeHint(i));
    setHeaderSizesForHelper(hv, key, sizeHints);
}

void QDesignerSettings::saveHeaderSizesFor(const QHeaderView *hv)
{
    if (!hv)
        return;

    const QWidget *w = hv;
    QString key;
    while (key.isEmpty() && w) {
        key = w->objectName();
        w = w->parentWidget();
    }
    Q_ASSERT(!key.isEmpty());
    saveHeaderSizesForHelper(hv, key);
}

void QDesignerSettings::setHeaderSizesForHelper(QHeaderView *hv, const QString &key,
                                               const QList<QVariant> &hints) const
{
    QList<QVariant> sizes = value(key + QLatin1String("/columnSizes"), hints).toList();
    int i;
    // Make sure the list of the sizes is correct.
    if (sizes.size() < hints.size()) {
        for (i = sizes.size(); i < hints.size(); ++i)
            sizes.append(hints.at(i));
    }

    for (i = 0; i < sizes.size(); ++i)
        hv->resizeSection(i, sizes.at(i).toInt());
}

void QDesignerSettings::saveHeaderSizesForHelper(const QHeaderView *hv, const QString &key)
{
    QList<QVariant> sizes;
    for (int i = 0; i < hv->count(); ++i)
        sizes.append(hv->sectionSize(i));
    QDesignerSettings settings;
    settings.setValue(key + QLatin1String("/columnSizes"), sizes);
}

QStringList QDesignerSettings::recentFilesList() const
{
    return value("recentFilesList").toStringList();
}

void QDesignerSettings::setRecentFilesList(const QStringList &sl)
{
    setValue("recentFilesList", sl);
}

void QDesignerSettings::setShowNewFormOnStartup(bool showIt)
{
    setValue("newFormDialog/ShowOnStartup", showIt);
}

bool QDesignerSettings::showNewFormOnStartup() const
{
    return value("newFormDialog/ShowOnStartup", true).toBool();
}

void QDesignerSettings::setUIMode(int mode)
{
    setValue("UI/currentMode", mode);
}

int QDesignerSettings::uiMode() const
{
    return value("UI/currentMode", QDesignerWorkbench::TopLevelMode).toInt();
}

void QDesignerSettings::setUseBigIcons(bool useBig)
{
    setValue("UI/useBigIcons", useBig);
}

bool QDesignerSettings::useBigIcons() const
{
    return value("UI/useBigIcons",
#ifdef Q_WS_MAC
                 true
#else
                 false
#endif
            ).toBool();
}
