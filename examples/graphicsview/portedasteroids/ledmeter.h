/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */

#ifndef __LEDMETER_H__
#define __LEDMETER_H__

#include <q3frame.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <QResizeEvent>


class KALedMeter : public Q3Frame
{
    Q_OBJECT
public:
    KALedMeter( QWidget *parent );

    int range() const { return mRange; }
    void setRange( int r );

    int count() const { return mCount; }
    void setCount( int c );

    int value () const { return mValue; }

    void addColorRange( int pc, const QColor &c );

public slots:
    void setValue( int v );

protected:
    virtual void resizeEvent( QResizeEvent * );
    virtual void drawContents( QPainter * );
    void calcColorRanges();

protected:
    struct ColorRange
    {
	int mPc;
	int mValue;
	QColor mColor;
    };

    int mRange;
    int mCount;
    int mCurrentCount;
    int mValue;
    Q3PtrList<ColorRange> mCRanges;
};

#endif
