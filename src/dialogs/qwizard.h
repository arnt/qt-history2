/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qwizard.h#16 $
**
** Definition of the QWizard class.
**
** Created : 990101
**
** Copyright (C) 1999 by Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

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

    virtual void showPage( QWidget * );

    QWidget * currentPage() const;

    QWidget* page( int pos ) const;
    int pageCount() const;

    virtual bool appropriate( QWidget * ) const;
    virtual void setAppropriate( QWidget *, bool );

    QPushButton * backButton() const;
    QPushButton * nextButton() const;
    QPushButton * finishButton() const;
    QPushButton * cancelButton() const;
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

    void layOut();
    QWizardPrivate *d;
};


#endif // QWIZARD_H
