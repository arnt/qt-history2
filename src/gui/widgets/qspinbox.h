/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSPINBOX_H
#define QSPINBOX_H

#include <QtGui/qabstractspinbox.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_SPINBOX

class QSpinBoxPrivate;
class Q_GUI_EXPORT QSpinBox : public QAbstractSpinBox
{
    Q_OBJECT

    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString cleanText READ cleanText)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged USER true)

public:
    explicit QSpinBox(QWidget *parent = 0);
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QSpinBox(QWidget *parent, const char *name);
    QT3_SUPPORT_CONSTRUCTOR QSpinBox(int min, int max, int step, QWidget *parent,
                                     const char *name = 0);
#endif

    int value() const;

    QString prefix() const;
    void setPrefix(const QString &prefix);

    QString suffix() const;
    void setSuffix(const QString &suffix);

    QString cleanText() const;

    int singleStep() const;
    void setSingleStep(int val);

    int minimum() const;
    void setMinimum(int min);

    int maximum() const;
    void setMaximum(int max);

    void setRange(int min, int max);

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT void setLineStep(int step) { setSingleStep(step); }
    inline QT3_SUPPORT void setMaxValue(int val) { setMaximum(val); }
    inline QT3_SUPPORT void setMinValue(int val) { setMinimum(val); }
    inline QT3_SUPPORT int maxValue() const { return maximum(); }
    inline QT3_SUPPORT int minValue() const { return minimum(); }
#endif

protected:
    bool event(QEvent *event);
    virtual QValidator::State validate(QString &input, int &pos) const;
    virtual int valueFromText(const QString &text) const;
    virtual QString textFromValue(int val) const;
    virtual void fixup(QString &str) const;


public Q_SLOTS:
    void setValue(int val);

Q_SIGNALS:
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

    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString cleanText READ cleanText)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged USER true)
public:
    explicit QDoubleSpinBox(QWidget *parent = 0);

    double value() const;

    QString prefix() const;
    void setPrefix(const QString &prefix);

    QString suffix() const;
    void setSuffix(const QString &suffix);

    QString cleanText() const;

    double singleStep() const;
    void setSingleStep(double val);

    double minimum() const;
    void setMinimum(double min);

    double maximum() const;
    void setMaximum(double max);

    void setRange(double min, double max);

    int decimals() const;
    void setDecimals(int prec);

    virtual QValidator::State validate(QString &input, int &pos) const;
    virtual double valueFromText(const QString &text) const;
    virtual QString textFromValue(double val) const;
    virtual void fixup(QString &str) const;

public Q_SLOTS:
    void setValue(double val);

Q_SIGNALS:
    void valueChanged(double);
    void valueChanged(const QString &);

private:
    Q_DISABLE_COPY(QDoubleSpinBox)
    Q_DECLARE_PRIVATE(QDoubleSpinBox)
};

#endif // QT_NO_SPINBOX

QT_END_HEADER

#endif // QSPINBOX_H
