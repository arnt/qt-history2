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

#ifndef QABSTRACTSPINBOX_H
#define QABSTRACTSPINBOX_H

#include <QtGui/qwidget.h>
#include <QtGui/qvalidator.h>

class QLineEdit;

class QAbstractSpinBoxPrivate;
class Q_GUI_EXPORT QAbstractSpinBox : public QWidget
{
    Q_OBJECT

    Q_ENUMS(ButtonSymbols)
    Q_PROPERTY(bool wrapping READ wrapping WRITE setWrapping)
    Q_PROPERTY(bool tracking READ tracking WRITE setTracking)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool slider READ slider WRITE setSlider)
    Q_PROPERTY(bool frame READ hasFrame WRITE setFrame)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(ButtonSymbols buttonSymbols READ buttonSymbols WRITE setButtonSymbols)
    Q_PROPERTY(QString text READ text)
    Q_PROPERTY(QString cleanText READ cleanText)

public:
    explicit QAbstractSpinBox(QWidget *parent = 0);
    ~QAbstractSpinBox();

    enum StepEnabledFlag { StepNone = 0x00, StepUpEnabled = 0x01,
                           StepDownEnabled = 0x02 };
    Q_DECLARE_FLAGS(StepEnabled, StepEnabledFlag)

    enum ButtonSymbols { UpDownArrows, PlusMinus };

    ButtonSymbols buttonSymbols() const;
    void setButtonSymbols(ButtonSymbols bs);

    QString text() const;
    QString cleanText() const;

    bool tracking() const;
    void setTracking(bool w);

    bool wrapping() const;
    void setWrapping(bool w);

    bool slider() const;
    void setSlider(bool s);

    void setFrame(bool f);
    bool hasFrame() const;

    void setReadOnly(bool r);
    bool isReadOnly() const;

    void setAlignment(Qt::Alignment flag);
    Qt::Alignment alignment() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void interpretText();
    bool eventFilter(QObject *object, QEvent *event);

    virtual QValidator::State validate(QString &input, int &pos) const;
    virtual void fixup(QString &input) const;

    virtual void stepBy(int steps);
public slots:
    void selectAll();
    virtual void clear();

protected:
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);
    void hideEvent(QHideEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void timerEvent(QTimerEvent *e);
    void paintEvent(QPaintEvent *e);
    void showEvent(QShowEvent *e);

    QLineEdit *lineEdit() const;
    void setLineEdit(QLineEdit *e);

    virtual StepEnabled stepEnabled() const;

protected:
    QAbstractSpinBox(QAbstractSpinBoxPrivate &dd, QWidget *parent = 0);

private:
    Q_PRIVATE_SLOT(d, void editorTextChanged(const QString &))
    Q_PRIVATE_SLOT(d, void editorCursorPositionChanged(int, int))

    Q_DECLARE_PRIVATE(QAbstractSpinBox)
    Q_DISABLE_COPY(QAbstractSpinBox)
};

#endif // QABSTRACTSPINBOX_H
