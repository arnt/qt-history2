#ifndef DIALOG_H
#define DIALOG_H

#include <qmotifdialog.h>


class CustomDialog : public QMotifDialog
{
    Q_OBJECT

public:
    CustomDialog( Widget parent, const char *name = 0,
		  bool modal = FALSE, WFlags flags = 0 );
};

#endif // DIALOG_H
