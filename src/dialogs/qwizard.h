/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qwizard.h#1 $
**
** Definition of the QWizard wizard framework
**
** Created : 990101
**
** Copyright (C) 1999 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QWIZARDDIALOG_H
#define QWIZARDDIALOG_H


#ifndef QT_H
#include "qdialog.h"
#endif // QT_H


class QWizardPrivate;
class QWizard;


class Q_EXPORT QWizard : public QDialog
{
    Q_OBJECT
public:
    QWizard( QWidget *parent=0, const char *name=0, bool modal=FALSE,
	     WFlags f=0 );
    ~QWizard();

    void show();
    void setFont( const QFont & font );

    void addPage( QWidget *, const QString & );

    void showPage( QWidget * );

    int count() const;

public slots:
    virtual void setBackEnabled( bool );
    virtual void setNextEnabled( bool );
    virtual void setHelpEnabled( bool );

protected slots:
    virtual void back();
    virtual void next();
    virtual void help();

private:
    QWizardPrivate *d;
};


#endif
