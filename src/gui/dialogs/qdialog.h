/****************************************************************************
**
** Definition of QDialog class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDIALOG_H
#define QDIALOG_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H
#ifndef QT_NO_DIALOG
#if 0
Q_OBJECT
#endif

class QPushButton;
class QDialogPrivate;

class Q_GUI_EXPORT QDialog : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE( QDialog );
    friend class QPushButton;

    Q_PROPERTY( bool sizeGripEnabled READ isSizeGripEnabled WRITE setSizeGripEnabled )
    Q_PROPERTY( bool modal READ isModal WRITE setModal )

public:
    QDialog(QWidget* parent = 0, WFlags f = 0);
    QDialog( QWidget* parent, const char* name, bool modal=false, WFlags f=0 ); // deprecated
    ~QDialog();

    enum DialogCode { Rejected, Accepted };

    int		result() const { return rescode; }

    void	show();
    void	hide();

    void	setOrientation( Orientation orientation );
    Orientation	orientation() const;

    void	setExtension( QWidget* extension );
    QWidget*	extension() const;

    QSize	sizeHint() const;
    QSize	minimumSizeHint() const;

    void setSizeGripEnabled( bool );
    bool isSizeGripEnabled() const;

    void setModal( bool modal );
    bool isModal() const;
#ifdef Q_OS_TEMP
    bool	event( QEvent * );
#endif

public slots:
    int exec();

protected slots:
    virtual void done( int );
    virtual void accept();
    virtual void reject();

    void	showExtension( bool );

protected:
    void	setResult( int r )	{ rescode = r; }
    void	keyPressEvent( QKeyEvent * );
    void	closeEvent( QCloseEvent * );
    void	showEvent(QShowEvent *);
    void	resizeEvent( QResizeEvent * );
    void	contextMenuEvent( QContextMenuEvent * );
    bool	eventFilter( QObject *, QEvent * );
    void	adjustPosition( QWidget*);

private:
    void	setDefault( QPushButton * );
    void	setMainDefault( QPushButton * );
    void	hideDefault();
#ifdef Q_OS_TEMP
    void	hideSpecial();
#endif

    int		rescode;
    uint	in_loop: 1;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDialog( const QDialog & );
    QDialog &operator=( const QDialog & );
#endif
};

#endif // QT_NO_DIALOG
#endif // QDIALOG_H
