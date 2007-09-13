/****************************************************************************
 * ** * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.  * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Patternist_ColoringMessageHandler_h
#define Patternist_ColoringMessageHandler_h

#include <QHash>

#include "ColorOutput.h"
#include "qabstractmessagehandler.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class ColoringMessageHandler : public QAbstractMessageHandler
                             , private ColorOutput
{
public:
    ColoringMessageHandler();

protected:
    virtual void handleMessage(QtMsgType type,
                               const QString &description,
                               const QUrl &identifier,
                               const QSourceLocation &sourceLocation);

private:
    QString colorifyDescription(const QString &in) const;

    enum ColorType
    {
        RunningText,
        Location,
        ErrorCode,
        Keyword,
        Data
    };

    QHash<QString, ColorType> m_classToColor;
};

QT_END_NAMESPACE

QT_END_HEADER 

#endif
