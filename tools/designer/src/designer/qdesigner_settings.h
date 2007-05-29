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

#ifndef QDESIGNER_SETTINGS_H
#define QDESIGNER_SETTINGS_H

#include <QtCore/QMap>
#include <QtCore/QRect>
#include <QtCore/QSettings>
#include <QtCore/QStringList>

struct Preferences;

class QDesignerSettings : public QSettings
{
public:
    QDesignerSettings();

    QStringList formTemplatePaths() const;
    void setFormTemplatePaths(const QStringList &paths);

    QString defaultUserWidgetBoxXml() const;

    void setGeometryFor(QWidget *w, const QRect &fallBack = QRect()) const;
    void saveGeometryFor(const QWidget *w);

    QStringList recentFilesList() const;
    void setRecentFilesList(const QStringList &list);

    void setShowNewFormOnStartup(bool showIt);
    bool showNewFormOnStartup() const;

    void setPreferences(const Preferences&);
    Preferences preferences() const;

    QByteArray mainWindowState() const;
    void setMainWindowState(const QByteArray &mainWindowState);

    QByteArray toolBoxState() const;
    void setToolBoxState(const QByteArray &state);

    QByteArray toolBarsState() const;
    void setToolBarsState(const QByteArray &mainWindowState);

    void clearBackup();
    void setBackup(const QMap<QString, QString> &map);
    QMap<QString, QString> backup() const;

    static const QStringList &defaultFormTemplatePaths();

    QString style() const;
    void setStyle(const QString &style);

    QString appStyleSheet() const;
    void setAppStyleSheet(const QString &styleSheet);

    int actionEditorViewMode() const;
    void setActionEditorViewMode(int vm);

private:
    void setGeometryHelper(QWidget *w, const QString &key, const QRect &fallBack) const;
    void saveGeometryHelper(const QWidget *w, const QString &key);
    QStringList additionalFormTemplatePaths() const;
};

#endif // QDESIGNER_SETTINGS_H
