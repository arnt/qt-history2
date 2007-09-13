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

#ifndef QCALENDARTEXTNAVIGATOR_P_H
#define QCALENDARTEXTNAVIGATOR_P_H

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

#include <QtCore/qobject.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qbasictimer.h>

QT_BEGIN_NAMESPACE

class QLabel;
class QCalendarDateValidator;
class QFrame;

class QCalendarTextNavigator: public QObject
{
    Q_OBJECT
public:
    QCalendarTextNavigator(QObject *parent = 0)
        : QObject(parent), m_dateText(0), m_dateFrame(0), m_dateValidator(0), m_widget(0), m_editDelay(1500), m_date(QDate::currentDate()) { }

    QWidget *widget() const;
    void setWidget(QWidget *widget);

    int dateEditAcceptDelay() const;
    void setDateEditAcceptDelay(int delay);

    QDate date() const;
    void setDate(const QDate &date);

    bool eventFilter(QObject *o, QEvent *e);
    void timerEvent(QTimerEvent *e);

signals:
    void dateChanged(const QDate &date);
    void editingFinished();

private:
    void applyDate();
    void updateDateLabel();
    void createDateLabel();
    void removeDateLabel();

    QLabel *m_dateText;
    QFrame *m_dateFrame;
    QBasicTimer m_acceptTimer;
    QCalendarDateValidator *m_dateValidator;
    QWidget *m_widget;
    int m_editDelay;

    QDate m_date;
};

QT_END_NAMESPACE

#endif

