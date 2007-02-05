/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#ifdef Q_WS_WIN

#include <qaxfactory.h>

class AutoTestWidget : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{411fb496-79d9-4c9c-b740-c498b0c00455}")
    Q_CLASSINFO("InterfaceID", "{7fa37398-a095-4adb-a4f1-c2091485f4de}")
    Q_CLASSINFO("EventsID", "{21512b78-9ed7-4a9f-8fc2-423c5abcc016}")

public:
    AutoTestWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
    }

public slots:
    void defaultEnumArgument(Qt::FocusPolicy pol = Qt::ClickFocus)
    {
    }
    void defaultIntArgument(int pol = 5)
    {
    }
};

#include "tst_qaxserver.moc"

QAXFACTORY_BEGIN("{ae67488a-e1aa-42a7-96fd-e202ba2d75a7}", "{45cbbb77-1055-4c70-98f4-10fc7f0cd909}")
    QAXCLASS(AutoTestWidget)
QAXFACTORY_END()

#else
QTEST_NOOP_MAIN
#endif // Q_WS_WIN
