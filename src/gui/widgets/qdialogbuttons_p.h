/****************************************************************************
**
** Definition of QDialogButtons class.
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

#ifndef QDIALOGBUTTONS_P_H
#define QDIALOGBUTTONS_P_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_DIALOGBUTTONS
struct QDialogButtonsPrivate;

class
QDialogButtons : public QWidget
{
    Q_OBJECT
public:
    enum Button { None=0, Accept=0x01, Reject=0x02, Help=0x04, Apply=0x08, All=0x10, Abort=0x20, Retry=0x40, Ignore=0x80 };
#ifndef QT_NO_DIALOG
    QDialogButtons(QDialog *parent, bool autoConnect = true, Q_UINT32 buttons = Accept | Reject,
                   Qt::Orientation orient = Qt::Horizontal);
#endif // QT_NO_DIALOG
    QDialogButtons(QWidget *parent, Q_UINT32 buttons = Accept | Reject,
                   Qt::Orientation orient = Qt::Horizontal);
    ~QDialogButtons();

    void setQuestionMode(bool);
    bool questionMode() const;

    void setButtonEnabled(Button button, bool enabled);
    bool isButtonEnabled(Button) const;

    inline void showButton(Button b) { setButtonVisible(b, true) ; }
    inline void hideButton(Button b) { setButtonVisible(b, false); }
    virtual void setButtonVisible(Button, bool visible);
    bool isButtonVisible(Button) const;

    void addWidget(QWidget *);

    virtual void setDefaultButton(Button);
    Button defaultButton() const;

    virtual void setButtonText(Button, const QString &);
    QString buttonText(Button) const;

    void setOrientation(Qt::Orientation);
    Qt::Orientation orientation() const;

    virtual QSize sizeHint(Button) const;
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void layoutButtons();
    virtual QWidget *createButton(Button);

    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *);
    void changeEvent(QEvent *);

private slots:
    void handleClicked();

signals:
    void clicked(Button);
    void acceptClicked();
    void rejectClicked();
    void helpClicked();
    void applyClicked();
    void allClicked();
    void retryClicked();
    void ignoreClicked();
    void abortClicked();

private:
    QDialogButtonsPrivate *d;
    void init(Q_UINT32, Qt::Orientation);
};
#endif //QT_NO_DIALOGBUTTONS
#endif //QDIALOGBUTTONS_P_H
