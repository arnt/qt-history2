/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qwizard.h#3 $
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
class QWizardPagePrivate;
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

    QWidget * currentPage() const;

    virtual bool appropriate( QWidget * ) const;
    virtual void setApproprate( QWidget *, bool );

    bool backEnabled( QWidget * ) const;
    bool nextEnabled( QWidget * ) const;
    bool helpEnabled( QWidget * ) const;

    bool finish( QWidget * );

public slots:
    virtual void setBackEnabled( QWidget *, bool );
    virtual void setNextEnabled( QWidget *, bool );

    virtual void setHelpEnabled( QWidget *, bool );

    virtual void setFinish(  QWidget *, bool );

protected slots:
    virtual void back();
    virtual void next();
    virtual void help();

signals:
    void helpClicked();

private:
    void setBackEnabled( bool );
    void setNextEnabled( bool );

    void setHelpEnabled( bool );

    void setNextPage( QWidget * );

    void updateButtons() const;

    int count() const;

    QWizardPrivate *d;
};


/*

class QWizardPage: public QWidget
{
    Q_OBJECT
public:
    QWizardPage( QWizard * parent, const char * name = 0 );
    ~QWizardPage();

    bool backEnabled() const;
    bool nextEnabled() const;
    bool helpEnabled() const;

    bool finish();

public slots:
    virtual void setBackEnabled( bool );
    virtual void setNextEnabled( bool );

    void setNextPage( QWidget * );

    virtual void setHelpEnabled( bool );

    virtual void setFinish( bool );

signals:
    void helpClicked();

private:
    QWizardPagePrivate * d;

    friend QWizard;
};

*/


#endif
