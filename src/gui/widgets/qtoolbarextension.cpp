#include "qtoolbarextension_p.h"

// ### move this into the style code and make the extension stylable
// ### (sizeHint() as well)
static const char * const arrow_v_xpm[] = {
    "7 9 3 1",
    "            c None",
    ".            c #000000",
    "+            c none",
    ".+++++.",
    "..+++..",
    "+..+..+",
    "++...++",
    ".++.++.",
    "..+++..",
    "+..+..+",
    "++...++",
    "+++.+++"
};

static const char * const arrow_h_xpm[] = {
    "9 7 3 1",
    "            c None",
    ".            c #000000",
    "+            c none",
    "..++..+++",
    "+..++..++",
    "++..++..+",
    "+++..++..",
    "++..++..+",
    "+..++..++",
    "..++..+++"
};

QToolBarExtension::QToolBarExtension(QWidget *parent)
    : QToolButton(parent)
{
    setAutoRaise(true);
    setOrientation(Qt::Horizontal);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void QToolBarExtension::setOrientation(Qt::Orientation o)
{
    if (o == Qt::Horizontal) {
        setIcon(QPixmap((const char **)arrow_h_xpm));
    } else {
        setIcon(QPixmap((const char **)arrow_v_xpm));
   }
}

QSize QToolBarExtension::sizeHint() const
{
    return QSize(14, 14);
}
