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

#ifndef QDIALOGBUTTONS_P_H
#define QDIALOGBUTTONS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qwidget.h"

struct QDialogButtonsPrivate;

class
QDialogButtons : public QWidget
{
    Q_OBJECT
public:
    enum Button { None=0, Accept=0x01, Reject=0x02, Help=0x04, Apply=0x08, All=0x10, Abort=0x20, Retry=0x40, Ignore=0x80 };
    explicit QDialogButtons(QDialog *parent, bool autoConnect = true,
                            quint32 buttons = Accept | Reject,
                            Qt::Orientation orient = Qt::Horizontal);
    explicit QDialogButtons(QWidget *parent,
                            quint32 buttons = Accept | Reject,
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
    void init(quint32, Qt::Orientation);
};


#endif //QDIALOGBUTTONS_P_H
