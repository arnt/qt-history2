/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSPINBOX_H
#define QSPINBOX_H

#include <qabstractspinbox.h>

class QSpinBoxPrivate;
class Q_GUI_EXPORT QSpinBox : public QAbstractSpinBox
{
    Q_OBJECT

    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString specialValueText READ specialValueText WRITE setSpecialValueText)

public:
    QSpinBox(QWidget *parent = 0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QSpinBox(QWidget *parent, const char *name);
    QT_COMPAT_CONSTRUCTOR QSpinBox(int min, int max, int step, QWidget *parent,
                                   const char *name = 0);
#endif

    int value() const;

    QString prefix() const;
    void setPrefix(const QString &p);

    QString suffix() const;
    void setSuffix(const QString &s);

    QString specialValueText() const;
    void setSpecialValueText(const QString &s);

    int singleStep() const;
    void setSingleStep(int val);

    int minimum() const;
    void setMinimum(int min);

    int maximum() const;
    void setMaximum(int max);

    void setRange(int min, int max);

#ifdef QT_COMPAT
    inline QT_COMPAT void setLineStep(int step) { setSingleStep(step); }
    inline QT_COMPAT void setMaxValue(int val) { setMaximum(val); }
    inline QT_COMPAT void setMinValue(int val) { setMinimum(val); }
    inline QT_COMPAT int maxValue() const { return maximum(); }
    inline QT_COMPAT int minValue() const { return minimum(); }
#endif

protected:
    virtual QString mapValueToText(int v) const;
    virtual int mapTextToValue(QString *text, QValidator::State *state) const;

public slots:
    void setValue(int val);

signals:
    void valueChanged(int);
    void valueChanged(const QString &);

private:
    Q_DISABLE_COPY(QSpinBox)
    Q_DECLARE_PRIVATE(QSpinBox)
};

class QDoubleSpinBoxPrivate;
class Q_GUI_EXPORT QDoubleSpinBox : public QAbstractSpinBox
{
    Q_OBJECT

    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(int precision READ precision WRITE setPrecision)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString specialValueText READ specialValueText WRITE setSpecialValueText)

public:
    QDoubleSpinBox(QWidget *parent = 0);

    double value() const;

    QString prefix() const;
    void setPrefix(const QString &p);

    QString suffix() const;
    void setSuffix(const QString &s);

    QString specialValueText() const;
    void setSpecialValueText(const QString &s);

    double singleStep() const;
    void setSingleStep(double val);

    double minimum() const;
    void setMinimum(double min);

    double maximum() const;
    void setMaximum(double max);

    void setRange(double min, double max);

    int precision() const;
    void setPrecision(int prec);

    virtual QString mapValueToText(double v) const;
    virtual double mapTextToValue(QString *text, QValidator::State *state) const;

public slots:
    void setValue(double val);

signals:
    void valueChanged(double);
    void valueChanged(const QString &);

private:
    Q_DISABLE_COPY(QDoubleSpinBox)
    Q_DECLARE_PRIVATE(QDoubleSpinBox)
};

#endif
