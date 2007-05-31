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
#include "preferences.h"
#include "qdesigner_settings.h"
#include "qdesigner_widgetbox.h"
#include "qdesigner_workbench.h"
#include "qdesigner_propertyeditor.h"
#include "qdesigner_objectinspector.h"

#include <qdesigner_utils_p.h>

#include <QtCore/QVariant>
#include <QtCore/QDir>

#include <QtGui/QDesktopWidget>
#include <QtGui/QStyle>
#include <QtGui/QListView>

#include <QtCore/qdebug.h>

static const char *designerPath = "/.designer";
static const char *newFormShowKey = "newFormDialog/ShowOnStartup";
static const char *mainWindowStateKey = "MainWindowState";
static const char *toolBoxStateKey = "ToolBoxState";
static const char *toolBarsStateKey = "ToolBarsState";
static const char *backupOrgListKey = "backup/fileListOrg";
static const char *backupBakListKey = "backup/fileListBak";
static const char *styleKey = "style";
static const char *appStyleSheetKey = "AppStyleSheet";
static const char *defaultGridKey = "defaultGrid";
static const char *formTemplatePathsKey = "FormTemplatePaths";
static const char *recentFilesListKey = "recentFilesList";
static const char *actionEditorViewModeKey = "ActionEditorViewMode";

static bool checkTemplatePath(const QString &path, bool create)
{
    QDir current(QDir::current());
    if (current.exists(path))
        return true;

    if (!create)
        return false;

    if (current.mkpath(path))
        return true;

    qdesigner_internal::designerWarning(QObject::tr("The template path %1 could not be created.").arg(path));
    return false;
}

QDesignerSettings::QDesignerSettings()
{
}

const QStringList &QDesignerSettings::defaultFormTemplatePaths()
{
    static QStringList rc;
    if (rc.empty()) {
        // Ensure default form template paths
        const QString templatePath = QLatin1String("/templates");
        // home
        QString path = QDir::homePath();
        path += QLatin1String(designerPath);
        path += templatePath;
        if (checkTemplatePath(path, true))
            rc += path;

        // designer/bin: Might be owned by root in some installations, do not force it.
        path = qDesigner->applicationDirPath();
        path += templatePath;
        if (checkTemplatePath(path, false))
            rc += path;
    }
    return rc;
}

QStringList QDesignerSettings::formTemplatePaths() const
{
    return value(QLatin1String(formTemplatePathsKey),defaultFormTemplatePaths()).toStringList();
}

void QDesignerSettings::setFormTemplatePaths(const QStringList &paths)
{
    setValue(QLatin1String(formTemplatePathsKey), paths);
}

QString QDesignerSettings::defaultUserWidgetBoxXml() const
{
    QString rc = QDir::homePath();
    rc += QLatin1String(designerPath);
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
    setValue(QLatin1String("visible"), w->isVisible());
    setValue(QLatin1String("geometry"), w->saveGeometry());
    endGroup();
}

void QDesignerSettings::setGeometryHelper(QWidget *w, const QString &key,
                                          const QRect &fallBack) const
{
    QByteArray ba(value(key + QLatin1String("/geometry")).toByteArray());

    if (ba.isEmpty()) {
        w->move(fallBack.topLeft());
        w->resize(fallBack.size());
    } else {
        w->restoreGeometry(ba);
    }

    if (value(key + QLatin1String("/visible"), true).toBool())
        w->show();
}

QStringList QDesignerSettings::recentFilesList() const
{
    return value(QLatin1String(recentFilesListKey)).toStringList();
}

void QDesignerSettings::setRecentFilesList(const QStringList &sl)
{
    setValue(QLatin1String(recentFilesListKey), sl);
}

void QDesignerSettings::setShowNewFormOnStartup(bool showIt)
{
    setValue(QLatin1String(newFormShowKey), showIt);
}

bool QDesignerSettings::showNewFormOnStartup() const
{
    return value(QLatin1String(newFormShowKey), true).toBool();
}

QByteArray QDesignerSettings::mainWindowState() const
{
    return value(QLatin1String(mainWindowStateKey)).toByteArray();
}

void QDesignerSettings::setMainWindowState(const QByteArray &mainWindowState)
{
    setValue(QLatin1String(mainWindowStateKey), mainWindowState);
}

QByteArray QDesignerSettings::toolBoxState() const
{
    return value(QLatin1String(toolBoxStateKey)).toByteArray();
}

void QDesignerSettings::setToolBoxState(const QByteArray &state)
{
    setValue(QLatin1String(toolBoxStateKey), state);
}

QByteArray QDesignerSettings::toolBarsState() const
{
    return value(QLatin1String(toolBarsStateKey)).toByteArray();
}

void QDesignerSettings::setToolBarsState(const QByteArray &toolBarsState)
{
    setValue(QLatin1String(toolBarsStateKey), toolBarsState);
}

void QDesignerSettings::clearBackup()
{
    remove(QLatin1String(backupOrgListKey));
    remove(QLatin1String(backupBakListKey));
}

void QDesignerSettings::setBackup(const QMap<QString, QString> &map)
{
    const QStringList org = map.keys();
    const QStringList bak = map.values();

    setValue(QLatin1String(backupOrgListKey), org);
    setValue(QLatin1String(backupBakListKey), bak);
}

QMap<QString, QString> QDesignerSettings::backup() const
{
    const QStringList org = value(QLatin1String(backupOrgListKey), QStringList()).toStringList();
    const QStringList bak = value(QLatin1String(backupBakListKey), QStringList()).toStringList();

    QMap<QString, QString> map;
    for (int i = 0; i < org.count(); ++i)
        map.insert(org.at(i), bak.at(i));

    return map;
}

QString QDesignerSettings::style() const
{
    return value(QLatin1String(styleKey), QString()).toString();
}

void QDesignerSettings::setStyle(const QString &style)
{
    setValue(QLatin1String(styleKey), style);
}

QString QDesignerSettings::appStyleSheet() const
{
    return value(QLatin1String(appStyleSheetKey), QString()).toString();
}

void QDesignerSettings::setAppStyleSheet(const QString &styleSheet)
{
     setValue(QLatin1String(appStyleSheetKey), styleSheet);
}

void QDesignerSettings::setPreferences(const Preferences& p)
{
    beginGroup(QLatin1String("UI"));
    setValue(QLatin1String("currentMode"), p.m_uiMode);
    setValue(QLatin1String("font"), p.m_font);
    setValue(QLatin1String("useFont"), p.m_useFont);
    setValue(QLatin1String("writingSystem"), p.m_writingSystem);
    endGroup();
    // grid
    setValue(QLatin1String(defaultGridKey), p.m_defaultGrid.toVariantMap());
    setStyle(p.m_style);
    setAppStyleSheet(p.m_appStyleSheet);
    // merge template paths
    QStringList templatePaths = defaultFormTemplatePaths();
    templatePaths += p.m_additionalTemplatePaths;
    setFormTemplatePaths(templatePaths);
}

Preferences QDesignerSettings::preferences() const
{
    Preferences rc;
#ifdef Q_WS_WIN
    const UIMode defaultMode = DockedMode;
#else
    const UIMode defaultMode = TopLevelMode;
#endif
    rc.m_uiMode = static_cast<UIMode>(value(QLatin1String("UI/currentMode"), defaultMode).toInt());
    rc.m_writingSystem = static_cast<QFontDatabase::WritingSystem>(value(QLatin1String("UI/writingSystem"), QFontDatabase::Any).toInt());
    rc.m_font = qVariantValue<QFont>(value(QLatin1String("UI/font")));
    rc.m_useFont = value(QLatin1String("UI/useFont"), QVariant(false)).toBool();
    const QVariantMap defaultGridMap = value(QLatin1String(defaultGridKey), QVariantMap()).toMap();
    if (!defaultGridMap.empty())
        rc.m_defaultGrid.fromVariantMap(defaultGridMap);
    rc.m_additionalTemplatePaths = additionalFormTemplatePaths();
    rc.m_style = style();
    rc.m_appStyleSheet = appStyleSheet();
    return rc;
}

QStringList QDesignerSettings::additionalFormTemplatePaths() const
{
    // get template paths excluding internal ones
    QStringList rc = formTemplatePaths();
    foreach (QString internalTemplatePath, defaultFormTemplatePaths()) {
        const int index = rc.indexOf(internalTemplatePath);
        if (index != -1)
            rc.removeAt(index);
    }
    return rc;
}

int QDesignerSettings::actionEditorViewMode() const
{
    return value(QLatin1String(actionEditorViewModeKey), 0).toInt();
}

void QDesignerSettings::setActionEditorViewMode(int vm)
{
    setValue(QLatin1String(actionEditorViewModeKey), vm);
}
