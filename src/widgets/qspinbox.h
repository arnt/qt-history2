/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.h#14 $
**
** Definition of QSpinBox widget class
**
** Created : 940206
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSPINBOX_H
#define QSPINBOX_H

#include <qframe.h>
#include <qrangect.h>
#include <qstring.h>

class QPushButton;
class QLineEdit;
class QValidator;
struct QSpinBoxData;


class QSpinBox: public QFrame, public QRangeControl
{
    Q_OBJECT
public:
    QSpinBox( QWidget* parent = 0, const char* name = 0 );
    QSpinBox( int minValue, int maxValue, int step = 1,
	      QWidget* parent = 0, const char* name = 0 );
    ~QSpinBox();

    const char* 	text() const;
    virtual const char*	suffix() const;
    virtual QString 	textWithoutSuffix() const;

    void		setMinValueText( const char* text );
    const char* 	minValueText() const;

    void 		setWrapping( bool on );
    bool 		wrapping() const;

    void		setValidator( QValidator* v );

    QSize 		sizeHint() const;
    void		setPalette( const QPalette& p );

public slots:
    virtual void	setValue( int value );
    virtual void	setSuffix( const char* text );
    virtual void	stepUp();
    virtual void	stepDown();

signals:
    void		valueChanged( int value );

protected:
    virtual QString	mapValueToText( int value );
    virtual int		mapTextToValue( bool* ok );

    virtual void	updateDisplay();
    virtual void	interpretText();

    QPushButton*	upButton() const;
    QPushButton*	downButton() const;
    QLineEdit*		editor() const;

    virtual void	valueChange();
    virtual void	rangeChange();

    bool		eventFilter( QObject* obj, QEvent* ev );
    void		resizeEvent( QResizeEvent* ev );

protected slots:
    void		textChanged();

private:
    void initSpinBox();
    struct QSpinBoxData* extra;
    QPushButton* up;
    QPushButton* down;
    QLineEdit* vi;
    QValidator* validator;
    QString sfix;
    QString minText;
    bool wrap;
    bool edited;
};


#endif
