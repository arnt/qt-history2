/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.h#18 $
**
** Definition of QSpinBox widget class
**
** Created : 1997
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSPINBOX_H
#define QSPINBOX_H

#ifndef QT_H
#include "qframe.h"
#include "qrangect.h"
#endif // QT_H

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
    virtual const char*	prefix() const;
    virtual const char*	suffix() const;
    virtual QString 	cleanText() const;

    void		setSpecialValueText( const char* text );
    const char* 	specialValueText() const;

    void 		setWrapping( bool on );
    bool 		wrapping() const;

    void		setValidator( QValidator* v );

    QSize 		sizeHint() const;

public slots:
    virtual void	setValue( int value );
    virtual void	setPrefix( const char* text );
    virtual void	setSuffix( const char* text );
    virtual void	stepUp();
    virtual void	stepDown();

signals:
    void		valueChanged( int value );
    void		valueChanged( const char* valueText );

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

    void		paletteChange( const QPalette& );
    void		enabledChange( bool );
    void		fontChange( const QFont& );
    void		styleChange( GUIStyle );

protected slots:
    void		textChanged();

private:
    void initSpinBox();
    struct QSpinBoxData* extra;
    QPushButton* up;
    QPushButton* down;
    QLineEdit* vi;
    QValidator* validator;
    QString pfix;
    QString sfix;
    QString specText;
    bool wrap;
    bool edited;
};


#endif
