/****************************************************************************
**
** Definition of QSpinBox widget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSPINBOX_H
#define QSPINBOX_H

#ifndef QT_H
#include "qwidget.h"
#include "qrangecontrol.h"
#endif // QT_H

#ifndef QT_NO_SPINBOX

class QLineEdit;
class QValidator;
class QSpinBoxPrivate;

class Q_GUI_EXPORT QSpinBox: public QWidget, public QRangeControl
{
    Q_OBJECT
    Q_ENUMS(ButtonSymbols)
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString cleanText READ cleanText)
    Q_PROPERTY(QString specialValueText READ specialValueText WRITE setSpecialValueText)
    Q_PROPERTY(bool wrapping READ wrapping WRITE setWrapping)
    Q_PROPERTY(ButtonSymbols buttonSymbols READ buttonSymbols WRITE setButtonSymbols)
    Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue)
    Q_PROPERTY(int minValue READ minValue WRITE setMinValue)
    Q_PROPERTY(int lineStep READ lineStep WRITE setLineStep)
    Q_PROPERTY(int value READ value WRITE setValue)

public:
    QSpinBox(QWidget* parent=0, const char* name=0);
    QSpinBox(int minValue, int maxValue, int step = 1,
              QWidget* parent=0, const char* name=0);
    ~QSpinBox();

    QString                text() const;

    virtual QString        prefix() const;
    virtual QString        suffix() const;
    virtual QString        cleanText() const;

    virtual void        setSpecialValueText(const QString &text);
    QString                specialValueText() const;

    virtual void        setWrapping(bool on);
    bool                wrapping() const;

    enum ButtonSymbols { UpDownArrows, PlusMinus };
    virtual void        setButtonSymbols(ButtonSymbols);
    ButtonSymbols        buttonSymbols() const;

    virtual void        setValidator(const QValidator* v);
    const QValidator * validator() const;

    QSize                sizeHint() const;
    QSize                minimumSizeHint() const;

    int         minValue() const;
    int         maxValue() const;
    void setMinValue(int);
    void setMaxValue(int);
    int         lineStep() const;
    void setLineStep(int);
    int  value() const;

    QRect                upRect() const;
    QRect                downRect() const;

public slots:
    virtual void        setValue(int value);
    virtual void        setPrefix(const QString &text);
    virtual void        setSuffix(const QString &text);
    virtual void        stepUp();
    virtual void        stepDown();
    virtual void         setEnabled(bool enabled);
    virtual void         selectAll();

signals:
    void                valueChanged(int value);
    void                valueChanged(const QString &valueText);

protected:
    virtual QString        mapValueToText(int value);
    virtual int                mapTextToValue(bool* ok);
    QString                currentValueText();

    virtual void        updateDisplay();
    virtual void        interpretText();

    QLineEdit*                editor() const;

    virtual void        valueChange();
    virtual void        rangeChange();

    void keyPressEvent(QKeyEvent *);
    void focusOutEvent(QFocusEvent *);
    void                resizeEvent(QResizeEvent* ev);
#ifndef QT_NO_WHEELEVENT
    void                wheelEvent(QWheelEvent *);
#endif

    void                changeEvent(QEvent *);

protected slots:
    void                textChanged();

private:
    void initSpinBox();
    QSpinBoxPrivate* d;
    QLineEdit* vi;
    QValidator* validate;
    QString pfix;
    QString sfix;
    QString specText;

    uint wrap                : 1;
    uint edited                : 1;

    void arrangeWidgets();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSpinBox(const QSpinBox&);
    QSpinBox& operator=(const QSpinBox&);
#endif

};

#endif // QT_NO_SPINBOX

#endif // QSPINBOX_H
