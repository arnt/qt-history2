/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.h#31 $
**
** Definition of QSpinBox widget class
**
** Created : 1997
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSPINBOX_H
#define QSPINBOX_H

#ifndef QT_H
#include "qframe.h"
#include "qrangecontrol.h"
#endif // QT_H

class QPushButton;
class QLineEdit;
class QValidator;
struct QSpinBoxData;


class Q_EXPORT QSpinBox: public QFrame, public QRangeControl
{
    Q_OBJECT
public:
    QSpinBox( QWidget* parent = 0, const char *name = 0 );
    QSpinBox( int minValue, int maxValue, int step = 1,
	      QWidget* parent = 0, const char* name = 0 );
    ~QSpinBox();

    QString 		text() const;
    virtual QString 	prefix() const;
    virtual QString 	suffix() const;
    virtual QString 	cleanText() const;

    virtual void	setSpecialValueText( const QString &text );
    QString 		specialValueText() const;

    virtual void 	setWrapping( bool on );
    bool 		wrapping() const;

    virtual void	setValidator( const QValidator* v );
    const QValidator * validator() const;

    QSize 		sizeHint() const;
    QSizePolicy 	sizePolicy() const;

public slots:
    virtual void	setValue( int value );
    virtual void	setPrefix( const QString &text );
    virtual void	setSuffix( const QString &text );
    virtual void	stepUp();
    virtual void	stepDown();
    virtual void	setEnabled( bool );

signals:
    void		valueChanged( int value );
    void		valueChanged( const QString &valueText );

protected:
    virtual QString	mapValueToText( int value );
    virtual int		mapTextToValue( bool* ok );
    QString		currentValueText();

    virtual void	updateDisplay();
    virtual void	interpretText();

    QPushButton*	upButton() const;
    QPushButton*	downButton() const;
    QLineEdit*		editor() const;

    virtual void	valueChange();
    virtual void	rangeChange();

    bool		eventFilter( QObject* obj, QEvent* ev );
    void		resizeEvent( QResizeEvent* ev );
    void		wheelEvent( QWheelEvent * );

    void		styleChange( GUIStyle ); //#?

protected slots:
    void		textChanged();

private:
    void initSpinBox();
    struct QSpinBoxData* extra;
    QPushButton* up;
    QPushButton* down;
    QLineEdit* vi;
    QValidator* validate;
    QString pfix;
    QString sfix;
    QString specText;
    bool wrap;
    bool edited;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSpinBox( const QSpinBox& );
    QSpinBox& operator=( const QSpinBox& );
#endif

};


#endif
