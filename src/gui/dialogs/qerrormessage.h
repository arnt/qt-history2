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

#ifndef QERRORMESSAGE_H
#define QERRORMESSAGE_H

#include <QtGui/qdialog.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_ERRORMESSAGE

class QErrorMessagePrivate;

class Q_GUI_EXPORT QErrorMessage: public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QErrorMessage)
public:
    explicit QErrorMessage(QWidget* parent = 0);
    ~QErrorMessage();

    static QErrorMessage * qtHandler();

public Q_SLOTS:
    void showMessage(const QString &message);
#ifdef QT3_SUPPORT
    inline QT_MOC_COMPAT void message(const QString &text) { showMessage(text); }
#endif

protected:
    void done(int);
    void changeEvent(QEvent *e);

private:
    Q_DISABLE_COPY(QErrorMessage)
};

#endif // QT_NO_ERRORMESSAGE

QT_END_NAMESPACE

QT_END_HEADER

#endif // QERRORMESSAGE_H
