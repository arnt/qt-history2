#ifndef QMOTIFDIALOG_H
#define QMOTIFDIALOG_H

#include <qdialog.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

class QMotifDialogPrivate;

class QMotifDialog : public QDialog
{
    Q_OBJECT

public:
    // The DialogType covers the predefined Motif dialog types.
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

    // Creates a predefined Motif dialog with a Motif widget parent.
    //
    // Creates a Shell widget which is a special subclass of XmDialogShell.
    // This allows applications to use the QDialog API with existing Motif
    // dialogs.  This allows appilications to have proper modality handling
    // through the QMotif extension.  You can access the Shell widget with
    // the shell() member function.
    //
    // Creates a dialog widget with the Shell widget as it's parent.  The
    // type of the dialog created is specified by the dialogtype argument.
    // see the DialogType enum for a list of available dialog types.  You
    // can access the dialog widget with the dialog() member function.
    QMotifDialog( DialogType dialogtype, Widget parent = NULL,
                  ArgList args = NULL, Cardinal argcount = 0,
		  const char *name = 0, bool modal = FALSE, WFlags flags = 0 );

    // Creates a QMotifDialog for use in a custom Motif dialog.
    //
    // Creates a Shell widget which is a special subclass of XmDialogShell.
    // This allows applications to use the QDialog API with existing Motif
    // dialogs.  This allows appilications to have proper modality handling
    // through the QMotif extension.  You can access the Shell widget with
    // the shell() member function.
    //
    // A dialog widget is not created by this constructor.  Instead, you
    // should create the dialog widget as a child of this dialog.  Once
    // you do this, QMotifDialog will take over ownership of your custom
    // dialog, and you can access it with the dialog() member function.
    //
    // NOTE: When QMotifDialog is destroyed, the Shell widget and the dialog
    // widget are destroyed.  You should not destroy the dialog widget yourself.
    QMotifDialog( Widget = NULL, ArgList = NULL, Cardinal = 0,
		  const char * = 0, bool = FALSE, WFlags = 0 );

    // for predefined motif dialogs with QWidget parents
    // NOTE: not yet implemented
    QMotifDialog( DialogType, QWidget * = 0, ArgList = NULL, Cardinal = 0,
		  const char * = 0, bool = FALSE, WFlags = 0 );
    // for custom motif dialogs with QWidget parents
    // NOTE: not yet implemented
    QMotifDialog( QWidget *parent = 0, ArgList = NULL, Cardinal = 0,
		  const char * = 0, bool = FALSE, WFlags = 0 );

    // Destroys the QDialog, dialog widget and shell widget.
    virtual ~QMotifDialog();

    // Returns the Shell widget embedded in this dialog.
    Widget shell() const;
    // Returns the Motif widget embedded in this dialog.
    Widget dialog() const;

    // Manages the dialog widget and shows the widget.
    void show();
    // Unmanages the dialog and shows the widget.
    void hide();

    // convenient callbacks for motif to accept/reject the QMotifDialog
    // NOTE: not yet implemented - the idea is to add Ok and Cancel callbacks
    // to call these methods, which will then automatically close the dialog
    // without having to explicitly call accept() or reject().
    static void acceptCallback( Widget, XtPointer, XtPointer );
    static void rejectCallback( Widget, XtPointer, XtPointer );

public slots:
    // The accept() and reject() slots are reimplemented only to bring them
    // into public access.  Ok and Cancel callbacks can all these methods
    // to close the dialog.
    void accept();
    void reject();

protected:
    // Delivers events to both Xt/Motif and Qt.
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
