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

#ifndef QDIALOG_P_H
#define QDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qwidget_p.h>
#include <qdialog.h>

class QEventLoop;
class QPushButton;
class QSizeGrip;

class QDialogPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QDialog)
public:

    QDialogPrivate()
        : mainDef(0), orientation(Qt::Horizontal),extension(0), doShowExtension(false),
#ifndef QT_NO_SIZEGRIP
          resizer(0),
#endif
          rescode(0), eventLoop(0)
        {}

    QPushButton* mainDef;
    Qt::Orientation orientation;
    QWidget* extension;
    bool doShowExtension;
    QSize size, min, max;
#ifndef QT_NO_SIZEGRIP
    QSizeGrip* resizer;
#endif
    QPoint lastRMBPress;

    void        setDefault(QPushButton *);
    void        setMainDefault(QPushButton *);
    void        hideDefault();
#ifdef Q_OS_TEMP
    void        hideSpecial();
#endif

    int rescode;

    QEventLoop *eventLoop;
};

#endif // QDIALOG_P_H
