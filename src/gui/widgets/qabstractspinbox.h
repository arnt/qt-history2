#ifndef QSPINBOX_H
#define QSPINBOX_H

#include <qwidget.h>
#include <qvalidator.h>

class QLineEdit;

class QAbstractSpinBoxPrivate;
class Q_GUI_EXPORT QAbstractSpinBox : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractSpinBox);
    Q_FLAGS(StepEnabledFlags)
    Q_ENUMS(ButtonSymbols)
    Q_PROPERTY(bool wrapping READ wrapping WRITE setWrapping)
    Q_PROPERTY(bool tracking READ tracking WRITE setTracking)
    Q_PROPERTY(Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(ButtonSymbols buttonSymbols READ buttonSymbols WRITE setButtonSymbols)
    QDOC_PROPERTY(QString text READ text)
    QDOC_PROPERTY(QString cleanText READ cleanText)

public:
    QAbstractSpinBox(QWidget *parent = 0, WFlags f = 0);
    ~QAbstractSpinBox();

    enum StepEnabledFlags { StepUpEnabled = 0x100, StepDownEnabled = 0x200 };
    Q_DECLARE_FLAGS(StepEnabled, StepEnabledFlags);

    enum ButtonSymbols { UpDownArrows, PlusMinus };

    ButtonSymbols buttonSymbols() const;
    void setButtonSymbols(ButtonSymbols bs);

    QString text() const;
    QString cleanText() const;

    bool tracking() const;
    void setTracking(bool w);

    bool wrapping() const;
    void setWrapping(bool w);

    void setAlignment(Alignment flag);
    Alignment alignment() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void interpretText();
protected:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);
    void changeEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void timerEvent(QTimerEvent *e);
    void paintEvent(QPaintEvent *e);
    void showEvent(QShowEvent *e);

    QLineEdit *lineEdit() const;

    virtual void stepBy(int steps);
    virtual StepEnabled stepEnabled() const;

private:
    Q_PRIVATE_SLOT(void editorTextChanged(const QString &))
    Q_PRIVATE_SLOT(void editorCursorPositionChanged(int, int));

protected:
    QAbstractSpinBox(QAbstractSpinBoxPrivate &dd, QWidget *parent, WFlags f);
};

#endif
