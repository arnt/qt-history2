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

class QDesignerSettings : public QSettings
{
public:
    QDesignerSettings();
    virtual ~QDesignerSettings();

    QStringList formTemplatePaths() const;
    void setFormTemplatePaths(const QStringList &paths);

    QString defaultUserWidgetBoxXml() const;

    void setGeometryFor(QWidget *w, const QRect &fallBack = QRect()) const;
    void saveGeometryFor(const QWidget *w);

    QStringList recentFilesList() const;
    void setRecentFilesList(const QStringList &list);

    void setShowNewFormOnStartup(bool showIt);
    bool showNewFormOnStartup() const;

    void setUIMode(int mode);
    int uiMode() const;

    QByteArray mainWindowState() const;
    void setMainWindowState(const QByteArray &mainWindowState);

    void clearBackup();
    void setBackup(const QMap<QString, QString> &map);
    QMap<QString, QString> backup() const;

private:
    QStringList defaultFormTemplatePaths() const;

    void setGeometryHelper(QWidget *w, const QString &key, const QRect &fallBack) const;
    void saveGeometryHelper(const QWidget *w, const QString &key);

private:
    QString m_designerPath;
};

#endif // QDESIGNER_SETTINGS_H
