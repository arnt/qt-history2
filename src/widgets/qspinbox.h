/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.h#13 $
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

    virtual void 	setWrapping( bool on );
    bool 		wrapping() const;

    QSize 		sizeHint() const;

public slots:
    virtual void	setValue( int value );
    virtual void	setSuffix( const char* text );
    virtual void	stepUp();
    virtual void	stepDown();

signals:
    void		valueChanged( int value );

protected:
    bool		eventFilter( QObject* obj, QEvent* ev );
    void		resizeEvent( QResizeEvent* ev );

    virtual void	valueChange();
    virtual void	rangeChange();

    void		setValidator( QValidator* v );
    virtual void	updateDisplay();
    virtual void	interpretText();

    QPushButton*	upButton() const;
    QPushButton*	downButton() const;
    QLineEdit*		editor() const;

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
    bool wrap;
    bool edited;
};


#endif
