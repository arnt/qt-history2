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

#ifndef QDESIGNER_SESSION_H
#define QDESIGNER_SESSION_H

#include <QtCore/QSettings>

class QDesignerSession: public QObject
{
    Q_OBJECT
public:
    QDesignerSession(QObject *parent = 0);
    virtual ~QDesignerSession();

    inline QSettings *settings();

private:
    QSettings m_settings;
};

inline QSettings *QDesignerSession::settings()
{ return &m_settings; }

#endif // QDESIGNER_SESSION_H
