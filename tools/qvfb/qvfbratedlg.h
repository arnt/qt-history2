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

#ifndef QVFBRATEDLG_H
#define QVFBRATEDLG_H

#include <QDialog>

class QLabel;
class QSlider;

class QVFbRateDialog : public QDialog
{
    Q_OBJECT
public:
    QVFbRateDialog( int value, QWidget *parent=0 );

signals:
    void updateRate( int r );

protected slots:
    void rateChanged( int r );
    void cancel();
    void ok();

private:
    QLabel *rateLabel;
    QSlider *rateSlider;
    int oldRate;
};

#endif
