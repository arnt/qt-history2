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

#ifndef QMDISUBWINDOW_H
#define QMDISUBWINDOW_H

#include <QWidget>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QMdiSubWindowPrivate;
class Q_GUI_EXPORT QMdiSubWindow : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int keyboardSingleStep READ keyboardSingleStep WRITE setKeyboardSingleStep)
    Q_PROPERTY(int keyboardPageStep READ keyboardPageStep WRITE setKeyboardPageStep)
public:
    enum SubWindowOption {
        AllowSubWindowsOutsideArea = 0x1,
        TransparentResize = 0x2,
        TransparentMove = 0x4,
    };
    Q_DECLARE_FLAGS(SubWindowOptions, SubWindowOption);

    QMdiSubWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~QMdiSubWindow();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setWidget(QWidget *widget);
    QWidget *widget() const;

    QWidget *maximizedButtonsWidget() const; // internal
    QWidget *maximizedSystemMenuIconWidget() const; // internal

    bool isShaded() const;
    QSize iconSize() const;

    void setOption(SubWindowOption option, bool on = true);
    bool testOption(SubWindowOption) const;

    void setKeyboardSingleStep(int step);
    int keyboardSingleStep() const;

    void setKeyboardPageStep(int step);
    int keyboardPageStep() const;

Q_SIGNALS:
    void windowStateChanged(Qt::WindowStates oldState, Qt::WindowStates newState);
    void aboutToBecomeActive();

public Q_SLOTS:
    void showMenu();
    void showShaded();

protected:
    bool eventFilter(QObject *object, QEvent *event);
    bool event(QEvent *event);
    void showEvent(QShowEvent *showEvent);
    void changeEvent(QEvent *changeEvent);
    void closeEvent(QCloseEvent *closeEvent);
    void leaveEvent(QEvent *leaveEvent);
    void resizeEvent(QResizeEvent *resizeEvent);
    void paintEvent(QPaintEvent *paintEvent);
    void mousePressEvent(QMouseEvent *mouseEvent);
    void mouseDoubleClickEvent(QMouseEvent *mouseEvent);
    void mouseReleaseEvent(QMouseEvent *mouseEvent);
    void mouseMoveEvent(QMouseEvent *mouseEvent);
    void keyPressEvent(QKeyEvent *keyEvent);
    void contextMenuEvent(QContextMenuEvent *contextMenuEvent);

private:
    Q_DECLARE_PRIVATE(QMdiSubWindow)
    Q_PRIVATE_SLOT(d_func(), void _q_updateStaysOnTopHint());
    Q_PRIVATE_SLOT(d_func(), void _q_enterInteractiveMode());
    Q_PRIVATE_SLOT(d_func(), void _q_processFocusChanged(QWidget *, QWidget *));
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMdiSubWindow::SubWindowOptions);

QT_END_HEADER

#endif // QMDISUBWINDOW_H
