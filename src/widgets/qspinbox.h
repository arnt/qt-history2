/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.h#35 $
**
** Definition of QSpinBox widget class
**
** Created : 1997
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSPINBOX_H
#define QSPINBOX_H

#ifndef QT_H
#include "qframe.h"
#include "qrangecontrol.h"
#endif // QT_H

#if QT_FEATURE_WIDGETS


class QPushButton;
class QLineEdit;
class QValidator;

struct QSpinBoxPrivate;

class Q_EXPORT QSpinBox: public QFrame, public QRangeControl
{
    Q_OBJECT
    Q_ENUMS( ButtonSymbols )
    Q_PROPERTY( QString text READ text )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString cleanText READ cleanText )
    Q_PROPERTY( QString specialValueText READ specialValueText WRITE setSpecialValueText )
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )
    Q_PROPERTY( ButtonSymbols buttonSymbols READ buttonSymbols WRITE setButtonSymbols )
    Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
    Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY( int lineStep READ lineStep WRITE setLineStep )
    Q_PROPERTY( int value READ value WRITE setValue )
	
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

    enum ButtonSymbols { UpDownArrows, PlusMinus };
    void		setButtonSymbols( ButtonSymbols ); // 3.0: virtual
    ButtonSymbols	buttonSymbols() const;

    virtual void	setValidator( const QValidator* v );
    const QValidator * validator() const;

    QSize 		sizeHint() const;
    QSizePolicy 	sizePolicy() const;

    int	 minValue() const;
    int	 maxValue() const;
    void setMinValue( int );
    void setMaxValue( int );
    int	 lineStep() const;
    void setLineStep( int );
    int  value() const;

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
    void		leaveEvent( QEvent* );

    void		styleChange( QStyle& );

protected slots:
    void		textChanged();

private:
    void initSpinBox();
    struct QSpinBoxPrivate* d;
    QPushButton* up;
    QPushButton* down;
    QLineEdit* vi;
    QValidator* validate;
    QString pfix;
    QString sfix;
    QString specText;
    bool wrap;
    bool edited;

    void arrangeWidgets();
    void updateButtonSymbols();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSpinBox( const QSpinBox& );
    QSpinBox& operator=( const QSpinBox& );
#endif

};

#endif // QT_FEATURE_WIDGETS

#endif // QSPINBOX_H
