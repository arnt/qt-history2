#ifndef __QDIALOGBUTTONS_H__
#define __QDIALOGBUTTONS_H__

#ifndef QT_H
#ifndef QT_H
#include <qwidget.h>
#endif // QT_H
#endif

#ifndef QT_NO_DIALOGBUTTONS
struct QDialogButtonsPrivate;

class
QDialogButtons : public QWidget
{
    Q_OBJECT
public:
    enum Button { None=0, Accept=0x01, Reject=0x02, Help=0x04, Apply=0x08, All=0x10, Abort=0x20, Retry=0x40, Ignore=0x80 };
#ifndef QT_NO_DIALOG
    QDialogButtons(QDialog *parent, bool autoConnect = TRUE, Q_UINT32 buttons = Accept | Reject,
		   Orientation orient = Horizontal, const char *name = NULL);
#endif // QT_NO_DIALOG
    QDialogButtons(QWidget *parent, Q_UINT32 buttons = Accept | Reject, 
		   Orientation orient = Horizontal, const char *name = NULL);
    ~QDialogButtons();

    void setQuestionMode(bool);
    bool questionMode() const;

    void setButtonEnabled(Button button, bool enabled);
    bool isButtonEnabled(Button) const;

    inline void showButton(Button b) { setButtonVisible(b, TRUE) ; }
    inline void hideButton(Button b) { setButtonVisible(b, FALSE); }
    virtual void setButtonVisible(Button, bool visible);
    bool isButtonVisible(Button) const;

    void addWidget(QWidget *);

    virtual void setDefaultButton(Button);
    Button defaultButton() const;

    virtual void setButtonText(Button, const QString &);
    QString buttonText(Button) const;

    void setOrientation(Orientation);
    Orientation orientation() const;

    virtual QSize sizeHint(Button) const;
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void layoutButtons();
    virtual QWidget *createButton(Button);

    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *);
    void styleChanged(QStyle &);

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
    void init(Q_UINT32, Orientation);
};
#endif //QT_NO_DIALOGBUTTONS
#endif /* __QDIALOGBUTTONS_H__ */
