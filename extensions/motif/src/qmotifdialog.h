#ifndef QMOTIFDIALOG_H
#define QMOTIFDIALOG_H

#include <qdialog.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

class QMotifWidget;
class QMotifDialogPrivate;

class QMotifDialog : public QDialog
{
    Q_OBJECT

public:
    enum DialogType {
	Prompt,
	Selection,
	Command,
	FileSelection,
	Template,
	Error,
	Information,
	Message,
	Question,
	Warning,
	Working
    };

    QMotifDialog( DialogType dialogtype, Widget parent = NULL,
                  ArgList args = NULL, Cardinal argcount = 0,
		  const char *name = 0, bool modal = FALSE, WFlags flags = 0 );
    QMotifDialog( Widget = NULL, ArgList = NULL, Cardinal = 0,
		  const char * = 0, bool = FALSE, WFlags = 0 );
    virtual ~QMotifDialog();

    Widget shell() const;
    Widget dialog() const;

    void show();
    void hide();

    static void acceptCallback( Widget, XtPointer, XtPointer );
    static void rejectCallback( Widget, XtPointer, XtPointer );

public slots:
    void accept();
    void reject();

protected:
    bool x11Event( XEvent * );

private:
    QMotifDialogPrivate *d;

    void realize( Widget );
    void insertChild( Widget );
    void deleteChild( Widget );
    void buildWidgetDict( Widget );
    void tearDownWidgetDict( Widget );

    friend void qmotif_dialog_realize( Widget, XtValueMask *, XSetWindowAttributes *);
    friend void qmotif_dialog_insert_child( Widget );
    friend void qmotif_dialog_delete_child( Widget );
    friend void qmotif_dialog_change_managed( Widget );
};

#endif // QMOTIFDIALOG_H
