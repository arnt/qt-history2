/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qwizard.h#7 $
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

class QHBoxLayout;


class Q_EXPORT QWizard : public QDialog
{
    Q_OBJECT
public:
    QWizard( QWidget *parent=0, const char *name=0, bool modal=FALSE,
	     WFlags f=0 );
    ~QWizard();

    void show();
    void setFont( const QFont & font );

    virtual void addPage( QWidget *, const QString & );
    virtual void removePage( QWidget * );

    QString title( QWidget * ) const;

    void showPage( QWidget * );

    QWidget * currentPage() const;

    virtual bool appropriate( QWidget * ) const;
    virtual void setApproprate( QWidget *, bool );

    //bool backEnabled( QWidget * ) const;
    //bool nextEnabled( QWidget * ) const;
    //bool helpEnabled( QWidget * ) const;

    QPushButton * backButton() const;
    QPushButton * nextButton() const;
    QPushButton * finishButton() const;
    QPushButton * helpButton() const;

    bool eventFilter( QObject *, QEvent * );

public slots:
    virtual void setBackEnabled( QWidget *, bool );
    virtual void setNextEnabled( QWidget *, bool );
    virtual void setFinishEnabled( QWidget *, bool );

    virtual void setHelpEnabled( QWidget *, bool );

    virtual void setFinish(  QWidget *, bool );

protected slots:
    virtual void back();
    virtual void next();
    virtual void finish();
    virtual void help();

signals:
    void helpClicked();

protected:
    virtual void layOutButtonRow( QHBoxLayout * );
    virtual void layOutTitleRow( QHBoxLayout *, const QString & );

private:
    void setBackEnabled( bool );
    void setNextEnabled( bool );

    void setHelpEnabled( bool );

    void setNextPage( QWidget * );

    void updateButtons();

    int count() const;

    void layOut();
    QWizardPrivate *d;
};


#endif
