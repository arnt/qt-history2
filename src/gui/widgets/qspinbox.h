#ifndef QDOUBLESPINBOX_H
#define QDOUBLESPINBOX_H

#include <qabstractspinbox.h>
class QSpinBoxPrivate;
class Q_GUI_EXPORT QSpinBox : public QAbstractSpinBox
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSpinBox);

    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum RESET clearMinimum)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum RESET clearMaximum)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep RESET clearSingleStep)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString specialValueText READ specialValueText WRITE setSpecialValueText)


public:
    QSpinBox(QWidget *parent = 0, WFlags f = 0);
    QSpinBox(int min, int max, int step = 1, QWidget *parent = 0, WFlags f = 0);
#ifdef QT_COMPAT
    QSpinBox(QWidget *parent, const char *name);
    QSpinBox(int min, int max, int step, QWidget *parent, const char *name);
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
    void clearSingleStep();

    int minimum() const;
    void setMinimum(int min);
    void clearMinimum();

    int maximum() const;
    void setMaximum(int max);
    void clearMaximum();

#ifdef QT_COMPAT
    inline QT_COMPAT void setRange(int min, int max) { setMinimum(min); setMaximum(max); }
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
    void valueChanged(QString);
};

class QDoubleSpinBoxPrivate;
class Q_GUI_EXPORT QDoubleSpinBox : public QAbstractSpinBox
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDoubleSpinBox);

    Q_PROPERTY(double value READ value WRITE setValue)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum RESET clearMinimum)
    Q_PROPERTY(double minimum READ minimum WRITE setMinimum RESET clearMaximum)
    Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep RESET clearSingleStep)
    Q_PROPERTY(int precision READ precision WRITE setPrecision RESET clearPrecision)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)
    Q_PROPERTY(QString specialValueText READ specialValueText WRITE setSpecialValueText)


public:
    QDoubleSpinBox(QWidget *parent = 0, WFlags f = 0);
    QDoubleSpinBox(double min, double max, double step = 1, int prec = 0, QWidget *parent = 0, WFlags f = 0);

    double value() const;

    QString prefix() const;
    void setPrefix(const QString &p);

    QString suffix() const;
    void setSuffix(const QString &s);

    QString specialValueText() const;
    void setSpecialValueText(const QString &s);

    double singleStep() const;
    void setSingleStep(double val);
    void clearSingleStep();

    double minimum() const;
    void setMinimum(double min);
    void clearMinimum();

    double maximum() const;
    void setMaximum(double max);
    void clearMaximum();

    int precision() const;
    void setPrecision(int prec);
    void clearPrecision();

    virtual QString mapValueToText(double v) const;
    virtual double mapTextToValue(QString *text, QValidator::State *state) const;

public slots:
    void setValue(double val);

signals:
    void valueChanged(double);
    void valueChanged(QString);
};


#endif
