/****************************************************************************
** $Id: //depot/qt/main/examples/validator/motor.h#3 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef MOTOR_H
#define MOTOR_H

#include <qvalidator.h>
#include <qspinbox.h>


class MotorValidator: public QValidator
{
    Q_OBJECT
public:
    MotorValidator( QSpinBox * parent, const char * name );
    ~MotorValidator();

    void setRange( int bottom, int top, int step );

    int bottom() { return b; }
    int top() { return t; }
    int step() { return s; }

    QValidator::State validate( QString &, int & ) const;

private:
    int b, t, s;
};


#endif
