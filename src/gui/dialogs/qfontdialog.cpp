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

#include "qwindowdefs.h"

#ifndef QT_NO_FONTDIALOG

#include "qfontdialog.h"

#include "qevent.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qcheckbox.h"
#include "qcombobox.h"
#include "qlayout.h"
#include "qgroupbox.h"
#include "qlabel.h"
#include "qapplication.h"
#include "qfontdatabase.h"
#include "qstyle.h"
#include <private/qfontdata_p.h>
#include <qvalidator.h>
#include <qabstractitemmodel.h>
#include <qgenerictreeview.h>
#include <qgenericheader.h>

class QFontListModel : public QAbstractItemModel
{
public:
    QFontListModel(QObject *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    QStringList lst;
};

QFontListModel::QFontListModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

int QFontListModel::rowCount(const QModelIndex &) const
{
    return lst.count();
}

int QFontListModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant QFontListModel::data(const QModelIndex &index, int) const
{
    if (index.isValid() && index.row() < (int)lst.count() && index.column() == 0)
        return lst.at(index.row());
    return QVariant();
}

class QFontListView : public QGenericTreeView
{
    Q_OBJECT
public:
    QFontListView(QWidget *parent);
    inline QFontListModel *model() const { return static_cast<QFontListModel *>(QGenericTreeView::model()); }
    inline void setCurrentItem(int item) {
        QGenericTreeView::setCurrentItem(model()->index(item, 0));
    }
    inline int currentItem() const {
        return QGenericTreeView::currentItem().row();
    }
    inline int count() const {
        return model()->rowCount();
    }
    inline QString currentText() const {
        int row = QGenericTreeView::currentItem().row();
        return row < 0 ? QString() : model()->lst.at(row);
    }
    void currentChanged(const QModelIndex &, const QModelIndex &current) {
        emit highlighted(current.row());
    }
    QString text(int i) const {
        return model()->lst.at(i);
    }
signals:
    void highlighted(int);
};

QFontListView::QFontListView(QWidget *parent)
    : QGenericTreeView(parent)
{
    setModel(new QFontListModel(parent));
    setRootIsDecorated(false);
    QGenericHeader *h = header();
    h->setResizeMode(QGenericHeader::Stretch, 0);
    h->hide();
}

/*!
  \class QFontDialog qfontdialog.h
  \ingroup dialogs
  \mainclass
  \brief The QFontDialog class provides a dialog widget for selecting a font.

  The usual way to use this class is to call one of the static convenience
  functions, e.g. getFont().

  Examples:

  \code
    bool ok;
    QFont font = QFontDialog::getFont(
                    &ok, QFont("Helvetica [Cronyx]", 10), this);
    if (ok) {
        // the user clicked OK and font is set to the font the user selected
    } else {
        // the user canceled the dialog; font is set to the initial
        // value, in this case Helvetica [Cronyx], 10
    }
  \endcode

    The dialog can also be used to set a widget's font directly:
  \code
    myWidget.setFont(QFontDialog::getFont(0, myWidget.font()));
  \endcode
  If the user clicks OK the font they chose will be used for myWidget,
  and if they click Cancel the original font is used.

  \img qfontdlg-w.png Screenshot in Windows style

  \sa QFont, QFontInfo, QFontMetrics
*/

class QFontDialogPrivate
{
public:
    QFontDialogPrivate() : script(QFontPrivate::defaultScript) {};
    QLabel * familyAccel;
    QLineEdit * familyEdit;
    QFontListView * familyList;

    QLabel * styleAccel;
    QLineEdit * styleEdit;
    QFontListView * styleList;

    QLabel * sizeAccel;
    QLineEdit * sizeEdit;
    QFontListView * sizeList;

    QGroupBox * effects;
    QCheckBox * strikeout;
    QCheckBox * underline;
    QComboBox * color;

    QGroupBox * sample;
    QLineEdit * sampleEdit;

    QLabel * scriptAccel;
    QComboBox * scriptCombo;

    QPushButton * ok;
    QPushButton * cancel;

    QBoxLayout * buttonLayout;
    QBoxLayout * effectsLayout;
    QBoxLayout * sampleLayout;
    QBoxLayout * sampleEditLayout;

    QFontDatabase fdb;

    QString       family;
    QFont::Script script;
    QString       style;
    int           size;

    bool smoothScalable;
};


/*!
  \internal
  Constructs a standard font dialog.

  Use setFont() to set the initial font attributes.

  The \a parent, \a name, \a modal and \a f parameters are passed to
  the QDialog constructor.

  \sa getFont()
*/

QFontDialog::QFontDialog(QWidget *parent, const char *name,
                          bool modal, Qt::WFlags f)
    : QDialog(parent, f)
{
    setObjectName(name);
    setModal(modal);
    setSizeGripEnabled(true);
    d = new QFontDialogPrivate;
    // grid
    d->familyEdit = new QLineEdit(this);
    d->familyEdit->setReadOnly(true);
    d->familyList = new QFontListView(this);
    d->familyEdit->setFocusProxy(d->familyList);

    d->familyAccel = new QLabel(tr("&Font"), this);
    d->familyAccel->setBuddy(d->familyList);
    d->familyAccel->setIndent(2);

    d->styleEdit = new QLineEdit(this);
    d->styleEdit->setReadOnly(true);
    d->styleList = new QFontListView(this);
    d->styleEdit->setFocusProxy(d->styleList);

    d->styleAccel = new QLabel(tr("Font st&yle"), this);
    d->styleAccel->setBuddy(d->styleList);
    d->styleAccel->setIndent(2);

    d->sizeEdit = new QLineEdit(this);
    d->sizeEdit->setFocusPolicy(Qt::ClickFocus);
    QIntValidator *validator = new QIntValidator(1, 512, this);
    d->sizeEdit->setValidator(validator);
    d->sizeList = new QFontListView(this);

    d->sizeAccel = new QLabel(tr("&Size"), this);
    d->sizeAccel->setBuddy(d->sizeEdit);
    d->sizeAccel->setIndent(2);

    // effects box
    d->effects = new QGroupBox(tr("Effects"), this);
    QVBoxLayout *vbox = new QVBoxLayout(d->effects);
    d->strikeout = new QCheckBox(d->effects);
    d->strikeout->setText(tr("Stri&keout"));
    vbox->addWidget(d->strikeout);
    d->underline = new QCheckBox(d->effects);
    d->underline->setText(tr("&Underline"));
    vbox->addWidget(d->underline);

    d->sample = new QGroupBox(tr("Sample"), this);
    QHBoxLayout *hbox = new QHBoxLayout(d->sample);
    d->sampleEdit = new QLineEdit(d->sample);
    d->sampleEdit->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    d->sampleEdit->setAlignment(Qt::AlignCenter);
    // Note that the sample text is *not* translated with tr(), as the
    // characters used depend on the charset encoding.
    d->sampleEdit->setText("AaBbYyZz");
    hbox->addWidget(d->sampleEdit);

    d->scriptCombo = new QComboBox(this);

    d->scriptAccel = new QLabel(tr("Scr&ipt"), this);
    d->scriptAccel->setBuddy(d->scriptCombo);
    d->scriptAccel->setIndent(2);

    d->size = 0;
    d->smoothScalable = false;

    connect(d->scriptCombo, SIGNAL(activated(int)),
             SLOT(scriptHighlighted(int)));
    connect(d->familyList, SIGNAL(highlighted(int)),
             SLOT(familyHighlighted(int)));
    connect(d->styleList, SIGNAL(highlighted(int)),
             SLOT(styleHighlighted(int)));
    connect(d->sizeList, SIGNAL(highlighted(int)),
             SLOT(sizeHighlighted(int)));
    connect(d->sizeEdit, SIGNAL(textChanged(QString)),
             SLOT(sizeChanged(QString)));

    connect(d->strikeout, SIGNAL(clicked()),
             SLOT(updateSample()));
    connect(d->underline, SIGNAL(clicked()),
             SLOT(updateSample()));

    for (int i = 0; i < QFont::NScripts; i++) {
        QString scriptname = QFontDatabase::scriptName((QFont::Script) i);
        if (!scriptname.isEmpty())
            d->scriptCombo->insertItem(scriptname);
    }

    updateFamilies();
    if (d->familyList->count() != 0)
        d->familyList->setCurrentItem(0);

    // grid layout
    QGridLayout * mainGrid = new QGridLayout(this);
    mainGrid->setMargin(12);
    mainGrid->setSpacing(0);

    mainGrid->addWidget(d->familyAccel, 0, 0);
    mainGrid->addWidget(d->familyEdit, 1, 0);
    mainGrid->addWidget(d->familyList, 2, 0);

    mainGrid->addWidget(d->styleAccel, 0, 2);
    mainGrid->addWidget(d->styleEdit, 1, 2);
    mainGrid->addWidget(d->styleList, 2, 2);

    mainGrid->addWidget(d->sizeAccel, 0, 4);
    mainGrid->addWidget(d->sizeEdit, 1, 4);
    mainGrid->addWidget(d->sizeList, 2, 4);

    mainGrid->setColumnStretch(0, 38);
    mainGrid->setColumnStretch(2, 24);
    mainGrid->setColumnStretch(4, 10);

    mainGrid->setColumnSpacing(1, 6);
    mainGrid->setColumnSpacing(3, 6);
    mainGrid->setColumnSpacing(5, 6);

    mainGrid->setRowSpacing(3, 12);

    mainGrid->addWidget(d->effects, 4, 0);

    mainGrid->addWidget(d->sample, 4, 2, 4, 3);

    mainGrid->addWidget(d->scriptAccel, 5, 0);
    mainGrid->setRowSpacing(6, 2);
    mainGrid->addWidget(d->scriptCombo, 7, 0);

    mainGrid->setRowSpacing(8, 12);

    QHBoxLayout *buttonBox = new QHBoxLayout;
    mainGrid->addItem(buttonBox, 9, 0, 1, 5);

    buttonBox->addStretch(1);
    QString okt = modal ? tr("OK") : tr("Apply");
    d->ok = new QPushButton(okt, this);
    buttonBox->addWidget(d->ok);
    if (modal)
        connect(d->ok, SIGNAL(clicked()), SLOT(accept()));
    d->ok->setDefault(true);

    buttonBox->addSpacing(12);

    QString cancelt = modal ? tr("Cancel") : tr("Close");
    d->cancel = new QPushButton(cancelt, this);
    buttonBox->addWidget(d->cancel);
    connect(d->cancel, SIGNAL(clicked()), SLOT(reject()));

    resize(500, 360);

    d->sizeEdit->installEventFilter(this);
    d->familyList->installEventFilter(this);
    d->styleList->installEventFilter(this);
    d->sizeList->installEventFilter(this);

    d->familyList->setFocus();
}

/*!
  \internal
 Destroys the font dialog and frees up its storage.
*/

QFontDialog::~QFontDialog()
{
    delete d;
    d = 0;
}

/*!
  Executes a modal font dialog and returns a font.

  If the user clicks OK, the selected font is returned. If the user
  clicks Cancel, the \a initial font is returned.

  The dialog is called \a name, with the parent \a parent.
  \a initial is the initially selected font.
  If the \a ok parameter is not-null, \e *\a ok is set to true if the
  user clicked OK, and set to false if the user clicked Cancel.

  This static function is less flexible than the full QFontDialog
  object, but is convenient and easy to use.

  Examples:
  \code
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QFont("Times", 12), this);
    if (ok) {
        // font is set to the font the user selected
    } else {
        // the user canceled the dialog; font is set to the initial
        // value, in this case Times, 12.
    }
  \endcode

    The dialog can also be used to set a widget's font directly:
  \code
    myWidget.setFont(QFontDialog::getFont(0, myWidget.font()));
  \endcode
  In this example, if the user clicks OK the font they chose will be
  used, and if they click Cancel the original font is used.
*/
QFont QFontDialog::getFont(bool *ok, const QFont &initial,
                            QWidget *parent, const char* name)
{
    return getFont(ok, &initial, parent, name);
}

/*!
    \overload

  Executes a modal font dialog and returns a font.

  If the user clicks OK, the selected font is returned. If the user
  clicks Cancel, the Qt default font is returned.

  The dialog is called \a name, with parent \a parent.
  If the \a ok parameter is not-null, \e *\a ok is set to true if the
  user clicked OK, and false if the user clicked Cancel.

  This static function is less functional than the full QFontDialog
  object, but is convenient and easy to use.

  Example:
  \code
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    if (ok) {
        // font is set to the font the user selected
    } else {
        // the user canceled the dialog; font is set to the default
        // application font, QApplication::font()
    }
  \endcode

*/
QFont QFontDialog::getFont(bool *ok, QWidget *parent,const char* name)
{
    return getFont(ok, 0, parent, name);
}

QFont QFontDialog::getFont(bool *ok, const QFont *def,
                            QWidget *parent, const char* name)
{
    QFont result;
    if (def)
        result = *def;

    QFontDialog *dlg = new QFontDialog(parent, name, true);

    dlg->setFont((def ? *def : QFont()));
#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg->setWindowTitle(tr("Select Font"));
#endif

    bool res = (dlg->exec() == QDialog::Accepted);
    if (res)
        result = dlg->font();
    if (ok)
        *ok = res;
    delete dlg;
    return result;
}


/*!
    \internal
    An event filter to make the Up, Down, PageUp and PageDown keys work
    correctly in the line edits. The source of the event is the object
    \a o and the event is \a e.
*/

bool QFontDialog::eventFilter(QObject * o , QEvent * e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent * k = (QKeyEvent *)e;
        if (o == d->sizeEdit &&
        (k->key() == Qt::Key_Up ||
             k->key() == Qt::Key_Down ||
         k->key() == Qt::Key_Prior ||
         k->key() == Qt::Key_Next)) {

            int ci = d->sizeList->currentItem();
            (void)QApplication::sendEvent(d->sizeList, k);

            if (ci != d->sizeList->currentItem() &&
                style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this))
                d->sizeEdit->selectAll();
            return true;
        } else if ((o == d->familyList || o == d->styleList) &&
                    (k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter)) {
            k->accept();
        accept();
            return true;
        }
    } else if (e->type() == QEvent::FocusIn &&
                style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this)) {
        if (o == d->familyList)
            d->familyEdit->selectAll();
        else if (o == d->styleList)
            d->styleEdit->selectAll();
        else if (o == d->sizeList)
            d->sizeEdit->selectAll();
    } else if (e->type() == QEvent::MouseButtonPress && o == d->sizeList) {
            d->sizeEdit->setFocus();
    }
    return QDialog::eventFilter(o, e);
}

/*!
  \internal
    Updates the contents of the "font family" list box. This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateFamilies()
{
    d->familyList->blockSignals(true);

    enum match_t { MATCH_NONE=0, MATCH_LAST_RESORT=1, MATCH_APP=2, MATCH_FALLBACK, MATCH_FAMILY=3 };

    QStringList familyNames = d->fdb.families(d->script);
    {
        // merge the unicode/unknown family list with the above list.
        QStringList l = d->fdb.families(QFont::Unicode) +
                        d->fdb.families(QFont::UnknownScript);
        QStringList::ConstIterator it = l.begin(), end = l.end();
        for (; it != end; ++it) {
            if (! familyNames.contains(*it))
                familyNames << *it;
        }
    }

    familyNames.sort();

    d->familyList->model()->lst = familyNames;

    QString foundryName1, familyName1, foundryName2, familyName2;
    int bestFamilyMatch = -1;
    match_t bestFamilyType = MATCH_NONE;

    QFont f;

    // ##### do the right thing for a list of family names in the font.
    QFontDatabase::parseFontName(d->family, foundryName1, familyName1);

    QStringList::Iterator it = familyNames.begin();
    int i = 0;
    for(; it != familyNames.end(); ++it, ++i) {

        QFontDatabase::parseFontName(*it, foundryName2, familyName2);

        //try to match..
        if (familyName1 == familyName2) {
            bestFamilyType = MATCH_FAMILY;
            if (foundryName1 == foundryName2) {
                bestFamilyMatch = i;
                break;
            }
            if (bestFamilyMatch < MATCH_FAMILY)
                bestFamilyMatch = i;
        }

        //and try some fall backs
        match_t type = MATCH_NONE;
        if (bestFamilyType <= MATCH_NONE && familyName2 == f.lastResortFamily())
                type = MATCH_LAST_RESORT;
        if (bestFamilyType <= MATCH_LAST_RESORT && familyName2 == f.family())
                type = MATCH_APP;
        // ### add fallback for script
        if (type != MATCH_NONE) {
            bestFamilyType = type;
            bestFamilyMatch = i;
        }
    }

    if (i != -1 && bestFamilyType != MATCH_NONE)
        d->familyList->setCurrentItem(bestFamilyMatch);
    else
        d->familyList->setCurrentItem(0);
    d->familyEdit->setText(d->familyList->currentText());
    if (style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
         d->familyList->hasFocus())
        d->familyEdit->selectAll();

    d->familyList->blockSignals(false);
    updateStyles();
}

/*!
    \internal
    Updates the contents of the "font style" list box. This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateStyles()
{
    d->styleList->blockSignals(true);


    QStringList styles = d->fdb.styles(d->familyList->currentText());
    d->styleList->model()->lst = styles;

    if (styles.isEmpty()) {
        d->styleEdit->clear();
        d->smoothScalable = false;
    } else {
        if (!d->style.isEmpty()) {
            bool found = false;
            bool first = true;
            QString cstyle = d->style;
        redo:
            for ( int i = 0 ; i < (int)d->styleList->count() ; i++ ) {
                if ( cstyle == d->styleList->text(i) ) {
                     d->styleList->setCurrentItem( i );
                     found = true;
                     break;
                 }
            }
            if (!found && first) {
                if (cstyle.contains("Italic")) {
                    cstyle.replace("Italic", "Oblique");
                    first = false;
                    goto redo;
                } else if (cstyle.contains("Oblique")) {
                    cstyle.replace("Oblique", "Italic");
                    first = false;
                    goto redo;
                }
            }
            if (!found)
                d->styleList->setCurrentItem(0);
        }

        d->styleEdit->setText(d->styleList->currentText());
        if (style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
            d->styleList->hasFocus())
            d->styleEdit->selectAll();

        d->smoothScalable = d->fdb.isSmoothlyScalable(d->familyList->currentText(), d->styleList->currentText());
    }

    d->styleList->blockSignals(false);

    updateSizes();
}

/*!
    \internal
    Updates the contents of the "font size" list box. This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateSizes()
{
    d->sizeList->blockSignals(true);

    if (!d->familyList->currentText().isEmpty()) {
        QList<int> sizes = d->fdb.pointSizes(d->familyList->currentText(), d->styleList->currentText());

        int i = 0;
        int current = -1;
        QStringList str_sizes;
        for(QList<int>::const_iterator it = sizes.constBegin() ; it != sizes.constEnd(); ++it) {
            str_sizes.append(QString::number(*it));
            if (current == -1 && *it >= d->size)
                current = i;
            ++i;
        }
        d->sizeList->model()->lst = str_sizes;
        if (current == -1)
            // we request a size bigger than the ones in the list, select the biggest one
            current = d->sizeList->count() - 1;
        d->sizeList->setCurrentItem(current);

        d->sizeEdit->blockSignals(true);
        d->sizeEdit->setText((d->smoothScalable ? QString::number(d->size) : d->sizeList->currentText()));
        if (style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
             d->sizeList->hasFocus())
            d->sizeEdit->selectAll();
        d->sizeEdit->blockSignals(false);
    } else {
        d->sizeEdit->clear();
    }

    d->sizeList->blockSignals(false);
    updateSample();
}

void QFontDialog::updateSample()
{
    if (d->familyList->currentText().isEmpty())
        d->sampleEdit->clear();
    else
        d->sampleEdit->setFont(font());
}

/*!
    \internal
*/
void QFontDialog::scriptHighlighted(int index)
{
    d->script = (QFont::Script)index;
    d->sampleEdit->setText(d->fdb.scriptSample(d->script));
    updateFamilies();
}

/*!
    \internal
*/
void QFontDialog::familyHighlighted(int i)
{
    d->family = d->familyList->text(i);
    d->familyEdit->setText(d->family);
    if (style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
         d->familyList->hasFocus())
        d->familyEdit->selectAll();

    updateStyles();
}


/*!
    \internal
*/

void QFontDialog::styleHighlighted(int index)
{
    QString s = d->styleList->text(index);
    d->styleEdit->setText(s);
    if (style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
         d->styleList->hasFocus())
        d->styleEdit->selectAll();

    d->style = s;

    updateSizes();
}


/*!
    \internal
*/

void QFontDialog::sizeHighlighted(int index)
{
    QString s = d->sizeList->text(index);
    d->sizeEdit->setText(s);
    if (style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
         d->sizeEdit->hasFocus())
        d->sizeEdit->selectAll();

    d->size = s.toInt();
    updateSample();
}

/*!
    \internal
    This slot is called if the user changes the font size.
    The size is passed in the \a s argument as a \e string.
*/

void QFontDialog::sizeChanged(const QString &s)
{
    // no need to check if the conversion is valid, since we have an QIntValidator in the size edit
    int size = s.toInt();
    if (d->size == size)
        return;

    d->size = size;
    if (d->sizeList->count() != 0) {
        int i;
        for (i = 0 ; i < (int)d->sizeList->count() - 1 ; i++) {
            if (d->sizeList->text(i).toInt() >= d->size)
                break;
        }
        d->sizeList->blockSignals(true);
        d->sizeList->setCurrentItem(i);
        d->sizeList->blockSignals(false);
    }
    updateSample();
}

/*!
  \internal
  Sets the font highlighted in the QFontDialog to font \a f.

  \sa font()
*/

void QFontDialog::setFont(const QFont &f)
{
    d->family = f.family();
    d->style = d->fdb.styleString(f);
    d->size = f.pointSize();
    if (d->size == -1) {
            QFontInfo fi(f);
            d->size = fi.pointSize();
    }
    d->strikeout->setChecked(f.strikeOut());
    d->underline->setChecked(f.underline());

    updateFamilies();
}

/*!
  \internal
  Returns the font which the user has chosen.

  \sa setFont()
*/

QFont QFontDialog::font() const
{
    int pSize = d->sizeEdit->text().toInt();

    QFont f = d->fdb.font(d->familyList->currentText(), d->style, pSize);
    f.setStrikeOut(d->strikeout->isChecked());
    f.setUnderline(d->underline->isChecked());
    return f;
}

#include "qfontdialog.moc"

#endif
