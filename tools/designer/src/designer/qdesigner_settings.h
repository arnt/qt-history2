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

#ifndef QDESIGNER_SETTINGS_H
#define QDESIGNER_SETTINGS_H

#include <QtCore/QSettings>

class QDesignerSettings: public QObject
{
    Q_OBJECT
public:
    QDesignerSettings(QObject *parent = 0);
    virtual ~QDesignerSettings();

    QStringList formTemplatePaths() const;

// ### protected:
    inline QSettings *settings();

private:
    QSettings m_settings;
};

inline QSettings *QDesignerSettings::settings()
{ return &m_settings; }

#endif // QDESIGNER_SETTINGS_H
