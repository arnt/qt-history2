/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qalgorithms.h"
#include "qapplication.h"
#include "qboxlayout.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qframe.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpainter.h"
#include "qpushbutton.h"
#include "qset.h"
#include "qstyle.h"
#include "qvarlengtharray.h"
#if defined(Q_WS_MAC)
#include "qmacstyle_mac.h"
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#elif defined(Q_WS_WIN)
#include "qwizard_win_p.h"
#include "qtimer.h"
#endif

#include "qwizard.h"

#include "private/qdialog_p.h"

#include <string.h>     // for memset()

// These fudge terms were needed a few places to obtain pixel-perfect results
const int GapBetweenLogoAndRightEdge = 5;
const int ModernHeaderTopMargin = 2;
const int ClassicHMargin = 4;
const int MacButtonHMargin = 13;

static void changeSpacerSize(QLayout *layout, int index, int width, int height)
{
    QSpacerItem *spacer = layout->itemAt(index)->spacerItem();
    Q_ASSERT(spacer);
    spacer->changeSize(width, height);
}

static QWidget *iWantTheFocus(QWidget *ancestor)
{
    const int MaxIterations = 100;

    QWidget *candidate = ancestor;
    for (int i = 0; i < MaxIterations; ++i) {
        candidate = candidate->nextInFocusChain();
        if (!candidate)
            break;

        if (candidate->focusPolicy() & Qt::TabFocus) {
            if (candidate != ancestor && ancestor->isAncestorOf(candidate))
                return candidate;
        }
    }
    return 0;
}

static bool objectInheritsXAndXIsCloserThanY(const QObject *object, const QByteArray &classX,
                                             const QByteArray &classY)
{
    const QMetaObject *metaObject = object->metaObject();
    while (metaObject) {
        if (metaObject->className() == classX)
            return true;
        if (metaObject->className() == classY)
            return false;
        metaObject = metaObject->superClass();
    }
    return false;
}

const int NFallbackDefaultProperties = 7;

const struct {
    const char *className;
    const char *property;
    const char *changedSignal;
} fallbackProperties[NFallbackDefaultProperties] = {
    // If you modify this list, make sure to update the documentation (and the auto test)
    { "QAbstractButton", "checked", SIGNAL(toggled(bool)) },
    { "QAbstractSlider", "value", SIGNAL(valueChanged(int)) },
    { "QComboBox", "currentIndex", SIGNAL(currentIndexChanged(int)) },
    { "QDateTimeEdit", "dateTime", SIGNAL(dateTimeChanged(QDateTime)) },
    { "QLineEdit", "text", SIGNAL(textChanged(QString)) },
    { "QListWidget", "currentRow", SIGNAL(currentRowChanged(int)) },
    { "QSpinBox", "value", SIGNAL(valueChanged(int)) }
};

class QWizardDefaultProperty
{
public:
    QByteArray className;
    QByteArray property;
    QByteArray changedSignal;

    inline QWizardDefaultProperty() {}
    inline QWizardDefaultProperty(const char *className, const char *property,
                                   const char *changedSignal)
        : className(className), property(property), changedSignal(changedSignal) {}
};

class QWizardField
{
public:
    inline QWizardField() {}
    QWizardField(QWizardPage *page, const QString &spec, QObject *object, const char *property,
                  const char *changedSignal);

    void resolve(const QVector<QWizardDefaultProperty> &defaultPropertyTable);
    void findProperty(const QWizardDefaultProperty *properties, int propertyCount);

    QWizardPage *page;
    QString name;
    bool mandatory;
    QObject *object;
    QByteArray property;
    QByteArray changedSignal;
    QVariant initialValue;
};

QWizardField::QWizardField(QWizardPage *page, const QString &spec, QObject *object,
                           const char *property, const char *changedSignal)
    : page(page), name(spec), mandatory(false), object(object), property(property),
      changedSignal(changedSignal)
{
    if (name.endsWith(QLatin1Char('*'))) {
        name.chop(1);
        mandatory = true;
    }
}

void QWizardField::resolve(const QVector<QWizardDefaultProperty> &defaultPropertyTable)
{
    if (property.isEmpty())
        findProperty(defaultPropertyTable.constData(), defaultPropertyTable.count());
    initialValue = object->property(property);
}

void QWizardField::findProperty(const QWizardDefaultProperty *properties, int propertyCount)
{
    QByteArray className;

    for (int i = 0; i < propertyCount; ++i) {
        if (objectInheritsXAndXIsCloserThanY(object, properties[i].className, className)) {
            className = properties[i].className;
            property = properties[i].property;
            changedSignal = properties[i].changedSignal;
        }
    }
}

class QWizardLayoutInfo
{
public:
    inline QWizardLayoutInfo()
        : topLevelMargin(-1), childMargin(-1), spacing(-1), wizStyle(QWizard::ClassicStyle),
          header(false), watermark(false), title(false), subTitle(false), extension(false) {}

    int topLevelMargin;
    int childMargin;
    int spacing;
    int buttonSpacing;
    QWizard::WizardStyle wizStyle;
    bool header;
    bool watermark;
    bool title;
    bool subTitle;
    bool extension;

    bool operator==(const QWizardLayoutInfo &other);
    inline bool operator!=(const QWizardLayoutInfo &other) { return !operator==(other); }
};

bool QWizardLayoutInfo::operator==(const QWizardLayoutInfo &other)
{
    return topLevelMargin == other.topLevelMargin
           && childMargin == other.childMargin
           && spacing == other.spacing
           && buttonSpacing == other.buttonSpacing
           && wizStyle == other.wizStyle
           && header == other.header
           && watermark == other.watermark
           && title == other.title
           && subTitle == other.subTitle
           && extension == other.extension;
}

class QWizardHeader : public QWidget
{
public:
    enum RulerType { Ruler };

    inline QWizardHeader(RulerType /* ruler */, QWidget *parent = 0)
        : QWidget(parent) { setFixedHeight(2); }
    QWizardHeader(QWidget *parent = 0);

    void setup(const QWizardLayoutInfo &info, const QString &title,
               const QString &subTitle, const QPixmap &logo, const QPixmap &banner,
               Qt::TextFormat titleFormat, Qt::TextFormat subTitleFormat);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QLabel *titleLabel;
    QLabel *subTitleLabel;
    QLabel *logoLabel;
    QGridLayout *layout;
    QPixmap bannerPixmap;
};

QWizardHeader::QWizardHeader(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setBackgroundRole(QPalette::Base);

    titleLabel = new QLabel(this);
    titleLabel->setBackgroundRole(QPalette::Base);

    subTitleLabel = new QLabel(this);
    subTitleLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    subTitleLabel->setWordWrap(true);

    logoLabel = new QLabel(this);

    QFont font = titleLabel->font();
    font.setBold(true);
    titleLabel->setFont(font);

    layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    layout->setRowMinimumHeight(3, 1);
    layout->setRowStretch(4, 1);

    layout->setColumnStretch(2, 1);
    layout->setColumnMinimumWidth(4, 2 * GapBetweenLogoAndRightEdge);
    layout->setColumnMinimumWidth(6, GapBetweenLogoAndRightEdge);

    layout->addWidget(titleLabel, 2, 1, 1, 2);
    layout->addWidget(subTitleLabel, 4, 2);
    layout->addWidget(logoLabel, 1, 5, 5, 1);
}

void QWizardHeader::setup(const QWizardLayoutInfo &info, const QString &title,
                          const QString &subTitle, const QPixmap &logo, const QPixmap &banner,
                          Qt::TextFormat titleFormat, Qt::TextFormat subTitleFormat)
{
    bool modern = (info.wizStyle == QWizard::ModernStyle);

    layout->setRowMinimumHeight(0, modern ? ModernHeaderTopMargin : 0);
    layout->setRowMinimumHeight(1, modern ? info.topLevelMargin - ModernHeaderTopMargin - 1 : 0);
    layout->setRowMinimumHeight(6, (modern ? 3 : GapBetweenLogoAndRightEdge) + 2);

    int minColumnWidth0 = modern ? 2 * info.topLevelMargin : 0;
    int minColumnWidth1 = modern ? 2 * info.topLevelMargin + 1
                                 : info.topLevelMargin + ClassicHMargin;
    layout->setColumnMinimumWidth(0, minColumnWidth0);
    layout->setColumnMinimumWidth(1, minColumnWidth1);

    titleLabel->setTextFormat(titleFormat);
    titleLabel->setText(title);
    logoLabel->setPixmap(logo);

    subTitleLabel->setTextFormat(subTitleFormat);
    subTitleLabel->setText(QLatin1String("Pq\nPq"));
    int desiredSubTitleHeight = subTitleLabel->sizeHint().height();
    subTitleLabel->setText(subTitle);

    if (modern) {
        bannerPixmap = banner;
    } else {
        bannerPixmap = QPixmap();
    }

    if (bannerPixmap.isNull()) {
        /*
            There is no widthForHeight() function, so we simulate it with a loop.
        */
        int candidateSubTitleWidth = qMin(512, 2 * qApp->desktop()->width() / 3);
        int delta = candidateSubTitleWidth >> 1;
        while (delta > 0) {
            if (subTitleLabel->heightForWidth(candidateSubTitleWidth - delta)
                        <= desiredSubTitleHeight)
                candidateSubTitleWidth -= delta;
            delta >>= 1;
        }

        subTitleLabel->setMinimumSize(candidateSubTitleWidth, desiredSubTitleHeight);

        QSize size = layout->totalMinimumSize();
#if QT_VERSION < 0x040200
        // work around bug in QGridLayout
        size += QSize(minColumnWidth0 + (3 * GapBetweenLogoAndRightEdge), 9);
#endif

        setMinimumSize(size);
        setMaximumSize(QWIDGETSIZE_MAX, size.height());
    } else {
        subTitleLabel->setMinimumSize(0, 0);
        setFixedSize(banner.size() + QSize(0, 2));
    }
    updateGeometry();
}

void QWizardHeader::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, bannerPixmap);

    int x = width() - 2;
    int y = height() - 2;
    const QPalette &pal = palette();
    painter.setPen(pal.mid().color());
    painter.drawLine(0, y, x, y);
    painter.setPen(pal.base().color());
    painter.drawPoint(x + 1, y);
    painter.drawLine(0, y + 1, x + 1, y + 1);
}

// We save one vtable by basing QWizardRuler on QWizardHeader
class QWizardRuler : public QWizardHeader
{
public:
    inline QWizardRuler(QWidget *parent = 0)
        : QWizardHeader(Ruler, parent) {}
};

class QWizardPagePrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QWizardPage)

public:
    enum TriState { Tri_Unknown = -1, Tri_False, Tri_True };

    inline QWizardPagePrivate()
        : wizard(0), completeState(Tri_Unknown), explicitlyFinal(false), commit(false) {}

    bool cachedIsComplete() const;
    void _q_maybeEmitCompleteChanged();
    void _q_updateCachedCompleteState();

    QWizard *wizard;
    QString title;
    QString subTitle;
    QPixmap pixmaps[QWizard::NPixmaps];
    QVector<QWizardField> pendingFields;
    mutable TriState completeState;
    bool explicitlyFinal;
    bool commit;
    QMap<int, QString> buttonCustomTexts;
};

bool QWizardPagePrivate::cachedIsComplete() const
{
    Q_Q(const QWizardPage);
    if (completeState == Tri_Unknown)
        completeState = q->isComplete() ? Tri_True : Tri_False;
    return completeState == Tri_True;
}

void QWizardPagePrivate::_q_maybeEmitCompleteChanged()
{
    Q_Q(QWizardPage);
    TriState newState = q->isComplete() ? Tri_True : Tri_False;
    if (newState != completeState)
        emit q->completeChanged();
}

void QWizardPagePrivate::_q_updateCachedCompleteState()
{
    Q_Q(QWizardPage);
    completeState = q->isComplete() ? Tri_True : Tri_False;
}

class QWizardAntiFlickerWidget : public QWidget
{
    QWizard *wizard;
    QWizardPrivate *wizardPrivate;
public:
    QWizardAntiFlickerWidget(QWizard *wizard, QWizardPrivate *wizardPrivate)
        : QWidget(wizard)
        , wizard(wizard)
        , wizardPrivate(wizardPrivate) {}
#if defined(Q_WS_WIN)
protected:
    void paintEvent(QPaintEvent *);
#endif
};

class QWizardPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QWizard)

public:
    typedef QMap<int, QWizardPage *> PageMap;

    enum Direction {
        Backward,
        Forward
    };

    inline QWizardPrivate()
        : start(-1),
          current(-1),
          canContinue(false),
          canFinish(false),
          disableUpdatesCount(0),
          currentPageExplicitlyIncomplete(false),
          currentPageExplicitlyFinal(false),
#if defined(Q_WS_MAC)
          wizStyle(QWizard::MacStyle),
          opts(QWizard::NoDefaultButton | QWizard::NoCancelButton),
#elif defined(Q_WS_WIN)
          wizStyle(QWizard::ModernStyle),
          opts(QWizard::HelpButtonOnRight),
#else
          wizStyle(QWizard::ClassicStyle),
          opts(0),
#endif
          buttonsHaveCustomLayout(false),
          titleFmt(Qt::AutoText),
          subTitleFmt(Qt::AutoText),
          placeholderWidget1(0),
          placeholderWidget2(0),
          headerWidget(0),
          watermarkLabel(0),
          titleLabel(0),
          subTitleLabel(0),
          bottomRuler(0)
#if defined(Q_WS_WIN)
          , wasCompositionEnabled(false)
#endif
          , aeroStyleOverride(false)
    {
        for (int i = 0; i < QWizard::NButtons; ++i)
            btns[i] = 0;
    }

    void init();
    void reset();
    void cleanupPagesNotInHistory();
    void addField(const QWizardField &field);
    void switchToPage(int newId, Direction direction);
    QWizardLayoutInfo layoutInfoForCurrentPage();
    void recreateLayout(const QWizardLayoutInfo &info);
    void updateLayout();
    void updateMinMaxSizes(const QWizardLayoutInfo &info);
    bool ensureButton(QWizard::WizardButton which) const;
    void connectButton(QWizard::WizardButton which) const;
    void updateButtonTexts();
    void updateButtons();
    void setButtonLayout(const QWizard::WizardButton *array, int size);
    bool buttonLayoutContains(QWizard::WizardButton which);
    void updatePixmap(QWizard::WizardPixmap which);
#if defined(Q_WS_WIN)
    void handleAeroStyleChange();
#endif
    void disableUpdates();
    void enableUpdates();
    void _q_emitCustomButtonClicked();
    void _q_updateButtonStates();

#ifdef Q_WS_MAC
    static QPixmap findDefaultBackgroundPixmap();
#endif

    PageMap pageMap;
    QVector<QWizardField> fields;
    QMap<QString, int> fieldIndexMap;
    QVector<QWizardDefaultProperty> defaultPropertyTable;
    QList<int> history;
    QSet<int> initialized; // ### remove and move bit to QWizardPage?
    int start;
    int current;
    bool canContinue;
    bool canFinish;
    QWizardLayoutInfo layoutInfo;
    int disableUpdatesCount;
    bool currentPageExplicitlyIncomplete; // ### written but never read -> remove?
    bool currentPageExplicitlyFinal; // ### written but never read -> remove?

    QWizard::WizardStyle wizStyle;
    QWizard::WizardOptions opts;
    QMap<int, QMap<int, QString> > buttonDefaultTexts;
    QMap<int, QString> buttonCustomTexts;
    bool buttonsHaveCustomLayout;
    QList<QWizard::WizardButton> buttonsCustomLayout;
    Qt::TextFormat titleFmt;
    Qt::TextFormat subTitleFmt;
    mutable QPixmap defaultPixmaps[QWizard::NPixmaps];

    union {
        // keep in sync with QWizard::WizardButton
        mutable struct {
            QAbstractButton *back;
            QAbstractButton *next;
            QAbstractButton *commit;
            QAbstractButton *finish;
            QAbstractButton *cancel;
            QAbstractButton *help;
        } btn;
        mutable QAbstractButton *btns[QWizard::NButtons];
    };
    QWizardAntiFlickerWidget *antiFlickerWidget;
    QWidget *placeholderWidget1;
    QWidget *placeholderWidget2;
    QWizardHeader *headerWidget;
    QLabel *watermarkLabel;
    QFrame *pageFrame;
    QLabel *titleLabel;
    QLabel *subTitleLabel;
    QWizardRuler *bottomRuler;

    QVBoxLayout *pageVBoxLayout;
    QHBoxLayout *buttonLayout;
    QGridLayout *mainLayout;

#if defined(Q_WS_WIN)
    QVistaHelper *vistaHelper;
    bool wasCompositionEnabled;
#endif
    bool aeroStyleOverride;
};

void QWizardPrivate::init()
{
    Q_Q(QWizard);

    antiFlickerWidget = new QWizardAntiFlickerWidget(q, this);

    for (int wstyle = 0; wstyle < QWizard::NStyles; ++wstyle) {
        QMap<int, QString> texts; // keep in sync with QWizard::WizardButton
        if (wstyle == QWizard::MacStyle) {
            texts.insert(QWizard::BackButton,   QWizard::tr("Go Back"));
            texts.insert(QWizard::NextButton,   QWizard::tr("Continue"));
            texts.insert(QWizard::CommitButton, QWizard::tr("Commit"));
            texts.insert(QWizard::FinishButton, QWizard::tr("Done"));
            texts.insert(QWizard::CancelButton, QWizard::tr("Quit"));
            texts.insert(QWizard::HelpButton,   QWizard::tr("Help"));
        } else {
            texts.insert(QWizard::BackButton,   QWizard::tr("< &Back"));
            texts.insert(
                QWizard::NextButton,
                QWizard::tr(wstyle == QWizard::AeroStyle ? "&Next" : "&Next >"));
            texts.insert(QWizard::CommitButton, QWizard::tr("Commit"));
            texts.insert(QWizard::FinishButton, QWizard::tr("&Finish"));
            texts.insert(QWizard::CancelButton, QWizard::tr("Cancel"));
            texts.insert(QWizard::HelpButton,   QWizard::tr("&Help"));
        }
        buttonDefaultTexts.insert(wstyle, texts);
    }

    // create these buttons right away; create the other buttons as necessary
    ensureButton(QWizard::BackButton);
    ensureButton(QWizard::NextButton);
    ensureButton(QWizard::CommitButton);
    ensureButton(QWizard::FinishButton);

    pageFrame = new QFrame(antiFlickerWidget);
    pageFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    pageVBoxLayout = new QVBoxLayout(pageFrame);
    pageVBoxLayout->setSpacing(0);
    pageVBoxLayout->addSpacing(0);
    pageVBoxLayout->addStretch(1);

    buttonLayout = new QHBoxLayout;
    buttonLayout->addSpacing(0);
    buttonLayout->addSpacing(0);

    mainLayout = new QGridLayout(antiFlickerWidget);
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);

    updateButtons();

    for (int i = 0; i < NFallbackDefaultProperties; ++i)
        defaultPropertyTable.append(QWizardDefaultProperty(fallbackProperties[i].className,
                                                           fallbackProperties[i].property,
                                                           fallbackProperties[i].changedSignal));

#if defined(Q_WS_WIN)
    vistaHelper = new QVistaHelper(q);
    wasCompositionEnabled = vistaHelper->isCompositionEnabled();
    vistaHelper->backButton()->move(
        0, vistaHelper->topOffset() - qMin(vistaHelper->topOffset(), vistaHelper->topPadding()));
    if (vistaHelper->isCompositionEnabled())
        wizStyle = QWizard::AeroStyle;
    handleAeroStyleChange();
#endif
}

void QWizardPrivate::reset()
{
    Q_Q(QWizard);
    if (current != -1) {
        q->currentPage()->hide();
        cleanupPagesNotInHistory();
        for (int i = history.count() - 1; i >= 0; --i)
            q->cleanupPage(history.at(i));
        history.clear();
        initialized.clear();

        current = -1;
        emit q->currentIdChanged(-1);
    }
}

void QWizardPrivate::cleanupPagesNotInHistory()
{
    Q_Q(QWizard);

    const QSet<int> original = initialized;
    QSet<int>::const_iterator i = original.constBegin();
    QSet<int>::const_iterator end = original.constEnd();

    for (; i != end; ++i) {
        if (!history.contains(*i)) {
            q->cleanupPage(*i);
            initialized.remove(*i);
        }
    }
}

void QWizardPrivate::addField(const QWizardField &field)
{
    QWizardField myField = field;
    myField.resolve(defaultPropertyTable);

    if (fieldIndexMap.contains(myField.name)) {
        qWarning("QWizardPage::registerField: Duplicate field '%s'", qPrintable(myField.name));
        return;
    }

    fieldIndexMap.insert(myField.name, fields.count());
    fields += myField;
    if (myField.mandatory && !myField.changedSignal.isEmpty())
        QObject::connect(myField.object, myField.changedSignal,
                         myField.page, SLOT(_q_maybeEmitCompleteChanged()));
}

void QWizardPrivate::switchToPage(int newId, Direction direction)
{
    Q_Q(QWizard);

    disableUpdates();

    int oldId = current;
    if (QWizardPage *oldPage = q->currentPage()) {
        oldPage->hide();

        if (direction == Backward) {
            if (!(opts & QWizard::IndependentPages)) {
                q->cleanupPage(oldId);
                initialized.remove(oldId);
            }
            Q_ASSERT(history.last() == oldId);
            history.removeLast();
            Q_ASSERT(history.last() == newId);
        }
    }

    current = newId;
    currentPageExplicitlyIncomplete = false;
    currentPageExplicitlyFinal = false;

    QWizardPage *newPage = q->currentPage();
    if (newPage) {
        if (direction == Forward) {
            if (!initialized.contains(current)) {
                initialized.insert(current);
                q->initializePage(current);
            }
            history.append(current);
        }
        newPage->show();
    }

    canContinue = (q->nextId() != -1);
    canFinish = (newPage && newPage->isFinalPage());

    _q_updateButtonStates();
    updateButtonTexts();

    const QWizard::WizardButton nextOrCommit =
        newPage && newPage->isCommitPage() ? QWizard::CommitButton : QWizard::NextButton;
    QAbstractButton *nextOrFinishButton =
        btns[canContinue ? nextOrCommit : QWizard::FinishButton];
    QWidget *candidate = 0;

    /*
        If there is no default button and the Next or Finish button
        is enabled, give focus directly to it as a convenience to the
        user. This is the normal case on Mac OS X.

        Otherwise, give the focus to the new page's first child that
        can handle it. If there is no such child, give the focus to
        Next or Finish.
    */
    if ((opts & QWizard::NoDefaultButton) && nextOrFinishButton->isEnabled()) {
        candidate = nextOrFinishButton;
    } else if (newPage) {
        candidate = iWantTheFocus(newPage);
    }
    if (!candidate)
        candidate = nextOrFinishButton;
    candidate->setFocus();

    if (wizStyle == QWizard::MacStyle)
        q->updateGeometry();

    enableUpdates();
    updateLayout();

    emit q->currentIdChanged(current);
}

// keep in sync with QWizard::WizardButton
static const char * const buttonSlots[QWizard::NStandardButtons] = {
    SLOT(back()), SLOT(next()), SLOT(next()), SLOT(accept()), SLOT(reject()),
    SIGNAL(helpRequested())
};

QWizardLayoutInfo QWizardPrivate::layoutInfoForCurrentPage()
{
    Q_Q(QWizard);
    QStyle *style = q->style();

    QWizardLayoutInfo info;

    info.topLevelMargin = style->pixelMetric(QStyle::PM_DefaultTopLevelMargin);
    info.childMargin = style->pixelMetric(QStyle::PM_DefaultChildMargin);
    info.spacing = style->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    info.buttonSpacing = info.spacing;
#ifdef Q_WS_MAC
    if (qobject_cast<QMacStyle *>(style))
        info.buttonSpacing = 0;
#endif

    info.wizStyle = wizStyle;

    QString titleText;
    QString subTitleText;
    QPixmap backgroundPixmap;
    QPixmap watermarkPixmap;

    if (QWizardPage *page = q->currentPage()) {
        titleText = page->title();
        subTitleText = page->subTitle();
        backgroundPixmap = page->pixmap(QWizard::BackgroundPixmap);
        watermarkPixmap = page->pixmap(QWizard::WatermarkPixmap);
    }

    info.header = (info.wizStyle == QWizard::ClassicStyle || info.wizStyle == QWizard::ModernStyle)
        && !(opts & QWizard::IgnoreSubTitles) && !subTitleText.isEmpty();
    info.watermark = (info.wizStyle != QWizard::MacStyle) && (info.wizStyle != QWizard::AeroStyle)
        && !watermarkPixmap.isNull();
    info.title = !info.header && !titleText.isEmpty();
    info.subTitle = !(opts & QWizard::IgnoreSubTitles) && !info.header && !subTitleText.isEmpty();
    info.extension = info.watermark && (opts & QWizard::ExtendedWatermarkPixmap);

    return info;
}

void QWizardPrivate::recreateLayout(const QWizardLayoutInfo &info)
{
    Q_Q(QWizard);

    /*
        Start by undoing the main layout.
    */
    for (int i = mainLayout->count() - 1; i >= 0; --i) {
        QLayoutItem *item = mainLayout->takeAt(i);
        if (item->layout()) {
            item->layout()->setParent(0);
        } else {
            delete item;
        }
    }
    for (int i = mainLayout->columnCount() - 1; i >= 0; --i)
        mainLayout->setColumnMinimumWidth(i, 0);
    for (int i = mainLayout->rowCount() - 1; i >= 0; --i)
        mainLayout->setRowMinimumHeight(i, 0);

    /*
        Now, recreate it.
    */

    bool mac = (info.wizStyle == QWizard::MacStyle);
    bool classic = (info.wizStyle == QWizard::ClassicStyle);
    bool modern = (info.wizStyle == QWizard::ModernStyle);
    bool aero = (info.wizStyle == QWizard::AeroStyle);
    int deltaMargin = info.topLevelMargin - info.childMargin;
    int deltaSpacing = info.topLevelMargin - info.spacing;

    int row = 0;
    int numColumns;
    if (mac) {
        numColumns = 3;
    } else if (info.watermark) {
        numColumns = 2;
    } else {
        numColumns = 1;
    }
    int pageColumn = qMin(1, numColumns - 1);

    if (mac) {
        mainLayout->setMargin(0);
        mainLayout->setSpacing(0);
        buttonLayout->setMargin(0);
        pageVBoxLayout->setMargin(10);
    } else {
        if (modern) {
            mainLayout->setMargin(0);
            mainLayout->setSpacing(0);
            pageVBoxLayout->setMargin(deltaMargin);
            buttonLayout->setMargin(info.topLevelMargin);
        } else {
            mainLayout->setMargin(info.topLevelMargin);
            mainLayout->setSpacing(info.spacing);
            pageVBoxLayout->setMargin(0);
            buttonLayout->setMargin(0);
        }
    }
    buttonLayout->setSpacing(info.buttonSpacing);
    changeSpacerSize(buttonLayout, 0, mac ? MacButtonHMargin : 0, 0);
    changeSpacerSize(buttonLayout, buttonLayout->count() - 1, mac ? MacButtonHMargin : 0, 0);

    if (info.header) {
        if (!headerWidget)
            headerWidget = new QWizardHeader(antiFlickerWidget);
        headerWidget->setAutoFillBackground(modern);
        mainLayout->addWidget(headerWidget, row++, 0, 1, numColumns);
    }
    if (headerWidget)
        headerWidget->setVisible(info.header);

    int watermarkStartRow = row;

    if (mac)
        mainLayout->setRowMinimumHeight(row++, info.topLevelMargin + 1);

    if (info.title) {
        if (!titleLabel) {
            titleLabel = new QLabel(antiFlickerWidget);
            titleLabel->setBackgroundRole(QPalette::Base);
            titleLabel->setWordWrap(true);
        }

        QFont titleFont = q->font();
        titleFont.setPointSize(titleFont.pointSize() + (mac ? 1 : 4));
        titleFont.setBold(true);
        titleLabel->setPalette(QPalette());

        if (aero) {
            // ### hardcoded for now:
            titleFont = QFont("Segoe UI", 12);
            QPalette pal(titleLabel->palette());
            pal.setColor(QPalette::Text, "#003399");
            titleLabel->setPalette(pal);
        }

        titleLabel->setFont(titleFont);
        const int aeroTitleIndent = 25; // ### hardcoded for now - should be calculated somehow
        titleLabel->setIndent(aero ? aeroTitleIndent : mac ? 2 : classic ? info.childMargin : info.topLevelMargin);
        if (modern) {
            if (!placeholderWidget1) {
                placeholderWidget1 = new QWidget(antiFlickerWidget);
                placeholderWidget1->setBackgroundRole(QPalette::Base);
            }
            placeholderWidget1->setFixedHeight(info.topLevelMargin + 2);
            mainLayout->addWidget(placeholderWidget1, row++, pageColumn);
        }
        mainLayout->addWidget(titleLabel, row++, pageColumn);
        if (modern) {
            if (!placeholderWidget2) {
                placeholderWidget2 = new QWidget(antiFlickerWidget);
                placeholderWidget2->setBackgroundRole(QPalette::Base);
            }
            placeholderWidget2->setFixedHeight(5);
            mainLayout->addWidget(placeholderWidget2, row++, pageColumn);
        }
        if (mac)
            mainLayout->setRowMinimumHeight(row++, info.topLevelMargin - 4);
    }
    if (placeholderWidget1)
        placeholderWidget1->setVisible(info.title && modern);
    if (placeholderWidget2)
        placeholderWidget2->setVisible(info.title && modern);

    if (info.subTitle) {
        if (!subTitleLabel) {
            subTitleLabel = new QLabel(pageFrame);
            subTitleLabel->setWordWrap(true);

            // ideally, the same margin should be used on the right side as well
            subTitleLabel->setIndent(info.childMargin);

            pageVBoxLayout->insertWidget(1, subTitleLabel);
        }
    }

    changeSpacerSize(pageVBoxLayout, 0, 0, info.subTitle ? info.childMargin : 0);

    int hMargin = mac ? 1 : 0;
    int vMargin = hMargin;

    pageFrame->setFrameStyle(mac ? (QFrame::Box | QFrame::Raised) : QFrame::NoFrame);
    pageFrame->setLineWidth(0);
    pageFrame->setMidLineWidth(hMargin);

    if (info.header) {
        if (modern) {
            hMargin = info.topLevelMargin;
            vMargin = deltaMargin;
        } else if (classic) {
            hMargin = deltaMargin + ClassicHMargin;
            vMargin = 0;
        }
    }

    if (aero) {
        int leftMargin   = 18; // ### hardcoded for now - should be calculated somehow
        int topMargin    = vMargin;
        int rightMargin  = hMargin; // ### for now
        int bottomMargin = vMargin;
        pageFrame->setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);
    } else {
        pageFrame->setContentsMargins(hMargin, vMargin, hMargin, vMargin);
    }

    if (info.watermark && !watermarkLabel) {
        watermarkLabel = new QLabel(antiFlickerWidget);
        watermarkLabel->setBackgroundRole(QPalette::Base);
        watermarkLabel->setMinimumHeight(1);
        watermarkLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        watermarkLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }

    //bool wasSemiTransparent = pageFrame->testAttribute(Qt::WA_SetPalette);
    const bool wasSemiTransparent =
        pageFrame->palette().brush(QPalette::Window).color().alpha() < 255
        || pageFrame->palette().brush(QPalette::Base).color().alpha() < 255;
    if (mac) {
        if (!wasSemiTransparent) {
            QPalette pal = pageFrame->palette();
            pal.setBrush(QPalette::Window, QColor(255, 255, 255, 153));
            // ### The next line is required to ensure visual semitransparency when
            // ### switching from ModernStyle to MacStyle. See TAG1 below.
            pal.setBrush(QPalette::Base, QColor(255, 255, 255, 153));
            pageFrame->setPalette(pal);
            pageFrame->setAutoFillBackground(true);
            antiFlickerWidget->setAutoFillBackground(false);
        }
    } else {
        if (wasSemiTransparent)
            pageFrame->setPalette(QPalette());

        bool baseBackground = (modern && !info.header); // ### TAG1
        pageFrame->setBackgroundRole(baseBackground ? QPalette::Base : QPalette::Window);

        if (titleLabel)
            titleLabel->setAutoFillBackground(baseBackground);
        pageFrame->setAutoFillBackground(baseBackground);
        if (watermarkLabel)
            watermarkLabel->setAutoFillBackground(baseBackground);
        if (placeholderWidget1)
            placeholderWidget1->setAutoFillBackground(baseBackground);
        if (placeholderWidget2)
            placeholderWidget2->setAutoFillBackground(baseBackground);

        if (aero) {
            QPalette pal = pageFrame->palette();
            pal.setBrush(QPalette::Window, QColor(255, 255, 255));
            pageFrame->setPalette(pal);
            pageFrame->setAutoFillBackground(true);
            pal = antiFlickerWidget->palette();
            pal.setBrush(QPalette::Window, QColor(255, 255, 255));
            antiFlickerWidget->setPalette(pal);
            antiFlickerWidget->setAutoFillBackground(true);
        }
    }

    mainLayout->addWidget(pageFrame, row++, pageColumn);

    int watermarkEndRow = row;
    if (classic)
        mainLayout->setRowMinimumHeight(row++, deltaSpacing);

    if (mac)
        mainLayout->setRowMinimumHeight(row, 2 + 2 * deltaSpacing);

    int buttonStartColumn = info.extension ? 1 : 0;
    int buttonNumColumns = info.extension ? 1 : numColumns;

    if (classic || modern) {
        if (!bottomRuler)
            bottomRuler = new QWizardRuler(antiFlickerWidget);
        mainLayout->addWidget(bottomRuler, row++, buttonStartColumn, 1, buttonNumColumns);
    }

    if (classic)
        mainLayout->setRowMinimumHeight(row++, deltaSpacing);

    if (mac)
        mainLayout->setRowMinimumHeight(row++, info.childMargin);
    mainLayout->addLayout(buttonLayout, row++, buttonStartColumn, 1, buttonNumColumns);
    if (mac)
        mainLayout->setRowMinimumHeight(row++, info.childMargin);

    if (info.watermark) {
        if (info.extension)
            watermarkEndRow = row;
        mainLayout->addWidget(watermarkLabel, watermarkStartRow, 0,
                              watermarkEndRow - watermarkStartRow, 1);
    }

    mainLayout->setColumnMinimumWidth(0, mac && !info.watermark ? 181 : 0);
    if (mac)
        mainLayout->setColumnMinimumWidth(2, 21);

    if (headerWidget)
        headerWidget->setVisible(info.header);
    if (titleLabel)
        titleLabel->setVisible(info.title);
    if (subTitleLabel)
        subTitleLabel->setVisible(info.subTitle);
    if (bottomRuler)
        bottomRuler->setVisible(classic || modern);
    if (watermarkLabel)
        watermarkLabel->setVisible(info.watermark);

    layoutInfo = info;
}

void QWizardPrivate::updateLayout()
{
    Q_Q(QWizard);

    disableUpdates();

    QWizardLayoutInfo info = layoutInfoForCurrentPage();
    if (layoutInfo != info)
        recreateLayout(info);

    QWizardPage *page = q->currentPage();

    if (info.header) {
        Q_ASSERT(page);
        headerWidget->setup(info, page->title(), page->subTitle(),
                            page->pixmap(QWizard::LogoPixmap), page->pixmap(QWizard::BannerPixmap),
                            titleFmt, subTitleFmt);
    }

    if (info.watermark) {
        Q_ASSERT(page);
        watermarkLabel->setPixmap(page->pixmap(QWizard::WatermarkPixmap));
    }
    if (info.title) {
        Q_ASSERT(page);
        titleLabel->setTextFormat(titleFmt);
        titleLabel->setText(page->title());
    }
    if (info.subTitle) {
        Q_ASSERT(page);
        subTitleLabel->setTextFormat(subTitleFmt);
        subTitleLabel->setText(page->subTitle());
    }

    enableUpdates();
    updateMinMaxSizes(info);
}

void QWizardPrivate::updateMinMaxSizes(const QWizardLayoutInfo &info)
{
    Q_Q(QWizard);
    q->setMinimumSize(mainLayout->totalMinimumSize());
    q->setMaximumSize(mainLayout->totalMaximumSize());
    if (info.header && headerWidget->maximumWidth() != QWIDGETSIZE_MAX)
        q->setFixedWidth(headerWidget->maximumWidth());
    if (info.watermark)
        q->setFixedHeight(mainLayout->totalSizeHint().height());
}

bool QWizardPrivate::ensureButton(QWizard::WizardButton which) const
{
    if (uint(which) >= QWizard::NButtons)
        return false;

    if (!btns[which]) {
        QPushButton *pushButton = new QPushButton(antiFlickerWidget);
#ifdef Q_WS_MAC
        pushButton->setAutoDefault(false);
#endif
        pushButton->hide();
        btns[which] = pushButton;
        if (which < QWizard::NStandardButtons)
            pushButton->setText(buttonDefaultTexts.value(wizStyle).value(which));
        connectButton(which);
    }
    return true;
}

void QWizardPrivate::connectButton(QWizard::WizardButton which) const
{
    Q_Q(const QWizard);
    if (which < QWizard::NStandardButtons) {
        QObject::connect(btns[which], SIGNAL(clicked()), q, buttonSlots[which]);
    } else {
        QObject::connect(btns[which], SIGNAL(clicked()), q, SLOT(_q_emitCustomButtonClicked()));
    }
}

void QWizardPrivate::updateButtonTexts()
{
    Q_Q(QWizard);
    for (int i = 0; i < QWizard::NButtons; ++i) {
        if (btns[i]) {
            if (q->currentPage() && (q->currentPage()->d_func()->buttonCustomTexts.contains(i)))
                btns[i]->setText(q->currentPage()->d_func()->buttonCustomTexts.value(i));
            else if (buttonCustomTexts.contains(i))
                btns[i]->setText(buttonCustomTexts.value(i));
            else if (i < QWizard::NStandardButtons)
                btns[i]->setText(buttonDefaultTexts.value(wizStyle).value(i));
        }
    }
}

void QWizardPrivate::updateButtons()
{
    if (buttonsHaveCustomLayout) {
        QVarLengthArray<QWizard::WizardButton> array(buttonsCustomLayout.count());
        for (int i = 0; i < buttonsCustomLayout.count(); ++i)
            array[i] = buttonsCustomLayout.at(i);
        setButtonLayout(array.constData(), array.count());
    } else {
        // Positions:
        //     Help Stretch Custom1 Custom2 Custom3 Cancel Back Next Commit Finish Cancel Help

        const int ArraySize = 12;
        QWizard::WizardButton array[ArraySize];
        memset(array, -1, sizeof(array));
        Q_ASSERT(array[0] == QWizard::NoButton);

        if (opts & QWizard::HaveHelpButton) {
            int i = (opts & QWizard::HelpButtonOnRight) ? 11 : 0;
            array[i] = QWizard::HelpButton;
        }
        array[1] = QWizard::Stretch;
        if (opts & QWizard::HaveCustomButton1)
            array[2] = QWizard::CustomButton1;
        if (opts & QWizard::HaveCustomButton2)
            array[3] = QWizard::CustomButton2;
        if (opts & QWizard::HaveCustomButton3)
            array[4] = QWizard::CustomButton3;

        if (!(opts & QWizard::NoCancelButton)) {
            int i = (opts & QWizard::CancelButtonOnLeft) ? 5 : 10;
            array[i] = QWizard::CancelButton;
        }
        array[6] = QWizard::BackButton;
        array[7] = QWizard::NextButton;
        array[8] = QWizard::CommitButton;
        array[9] = QWizard::FinishButton;

        setButtonLayout(array, ArraySize);
    }
}

void QWizardPrivate::setButtonLayout(const QWizard::WizardButton *array, int size)
{
    QWidget *prev = pageFrame;

    // we deliberately skip the first and the last item, which are spacers
    for (int i = buttonLayout->count() - 2; i >= 1; --i) {
        QLayoutItem *item = buttonLayout->takeAt(i);
        if (QWidget *widget = item->widget())
            widget->hide();
        delete item;
    }

    int pos = 1;

    for (int i = 0; i < size; ++i) {
        QWizard::WizardButton which = array[i];
        if (which == QWizard::Stretch) {
            buttonLayout->insertStretch(pos++, 1);
        } else if (which != QWizard::NoButton) {
            ensureButton(which);
            buttonLayout->insertWidget(pos++, btns[which]);

            // Back, Next, Commit, and Finish are handled in _q_updateButtonStates()
            if (which != QWizard::BackButton && which != QWizard::NextButton
                && which != QWizard::CommitButton && which != QWizard::FinishButton)
                btns[which]->show();

            if (prev)
                QWidget::setTabOrder(prev, btns[which]);
            prev = btns[which];
        }
    }

    _q_updateButtonStates();
}

bool QWizardPrivate::buttonLayoutContains(QWizard::WizardButton which)
{
    return !buttonsHaveCustomLayout || buttonsCustomLayout.contains(which);
}

void QWizardPrivate::updatePixmap(QWizard::WizardPixmap which)
{
    Q_Q(QWizard);
    if (which == QWizard::BackgroundPixmap) {
        if (wizStyle == QWizard::MacStyle) {
            q->update();
            q->updateGeometry();
        }
    } else {
        updateLayout();
    }
}

#if defined(Q_WS_WIN)
void QWizardPrivate::handleAeroStyleChange()
{
    Q_Q(QWizard);
    if (wizStyle == QWizard::AeroStyle) {
        vistaHelper->setDWMTitleBar(QVistaHelper::ExtendedTitleBar);
        q->installEventFilter(vistaHelper);
        q->setMouseTracking(true); // due to faked title bar
        antiFlickerWidget->move(0, vistaHelper->titleBarSize() + vistaHelper->topOffset());
        QObject::connect(
            vistaHelper->backButton(), SIGNAL(clicked()), q, buttonSlots[QWizard::BackButton]);
        vistaHelper->backButton()->show();
    } else {
        vistaHelper->setDWMTitleBar(QVistaHelper::NormalTitleBar);
        q->removeEventFilter(vistaHelper);
        q->setMouseTracking(false); // ### reset to original value (which could be true!)
        antiFlickerWidget->move(0, 0);
        vistaHelper->backButton()->disconnect();
        vistaHelper->backButton()->hide();
    }

    _q_updateButtonStates();
    updateButtonTexts(); // ### assume this is required - 2 B TESTED!

    if (q->isVisible())
        vistaHelper->setWindowPosHack();
}
#endif

void QWizardPrivate::disableUpdates()
{
    Q_Q(QWizard);
    if (disableUpdatesCount++ == 0) {
        q->setUpdatesEnabled(false);
        antiFlickerWidget->hide();
    }
}

void QWizardPrivate::enableUpdates()
{
    Q_Q(QWizard);
    if (--disableUpdatesCount == 0) {
        antiFlickerWidget->show();
        q->setUpdatesEnabled(true);
    }
}

void QWizardPrivate::_q_emitCustomButtonClicked()
{
    Q_Q(QWizard);
    QObject *button = q->sender();
    for (int i = QWizard::NStandardButtons; i < QWizard::NButtons; ++i) {
        if (btns[i] == button) {
            emit q->customButtonClicked(QWizard::WizardButton(i));
            break;
        }
    }
}

void QWizardPrivate::_q_updateButtonStates()
{
    Q_Q(QWizard);

    disableUpdates();

    const QWizardPage *page = q->currentPage();
    bool complete = page && page->isComplete();

    btn.back->setEnabled(history.count() > 1
                         && !q->page(history.at(history.count() - 2))->isCommitPage()
                         && (!canFinish || !(opts & QWizard::DisabledBackButtonOnLastPage)));
    btn.next->setEnabled(canContinue && complete);
    btn.finish->setEnabled(canFinish && complete);

    const bool backButtonVisible = buttonLayoutContains(QWizard::BackButton)
        && (history.count() > 1 || !(opts & QWizard::NoBackButtonOnStartPage))
        && (canContinue || !(opts & QWizard::NoBackButtonOnLastPage));
    bool commitPage = page && page->isCommitPage();
    btn.back->setVisible(backButtonVisible);
    btn.next->setVisible(buttonLayoutContains(QWizard::NextButton) && !commitPage
                         && (canContinue || (opts & QWizard::HaveNextButtonOnLastPage)));
    btn.commit->setVisible(buttonLayoutContains(QWizard::CommitButton) && commitPage
                           && canContinue);
    btn.finish->setVisible(buttonLayoutContains(QWizard::FinishButton)
                           && (canFinish || (opts & QWizard::HaveFinishButtonOnEarlyPages)));

    bool useDefault = !(opts & QWizard::NoDefaultButton);
    if (QPushButton *nextPush = qobject_cast<QPushButton *>(btn.next))
        nextPush->setDefault(canContinue && useDefault && !commitPage);
    if (QPushButton *commitPush = qobject_cast<QPushButton *>(btn.commit))
        commitPush->setDefault(canContinue && useDefault && commitPage);
    if (QPushButton *finishPush = qobject_cast<QPushButton *>(btn.finish))
        finishPush->setDefault(!canContinue && useDefault);

#if defined(Q_WS_WIN)
    if (wizStyle == QWizard::AeroStyle) {
        vistaHelper->backButton()->setEnabled(btn.back->isEnabled());
        vistaHelper->backButton()->setVisible(backButtonVisible);
        btn.back->setVisible(false);
    }
#endif

    enableUpdates();
}

#ifdef Q_WS_MAC
QPixmap QWizardPrivate::findDefaultBackgroundPixmap()
{
    // ### Simplify this code if this ever goes into Qt
    CFURLRef url;
    const uint ExpectedFileSize = 62932;
    const int ExpectedImageWidth = 242;
    const int ExpectedImageHeight = 414;
    if (LSFindApplicationForInfo(kLSUnknownCreator, CFSTR("com.apple.KeyboardSetupAssistant"),
                                 0, 0, &url) == noErr) {
        CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, url);
        CFRelease(url);
        if (bundle) {
            url = CFBundleCopyResourceURL(bundle, CFSTR("Background"), CFSTR("tif"), 0);
            if (url) {
                CFRelease(bundle);
                FSRef fsref;
                CFURLGetFSRef(url, &fsref);
                CFRelease(url);
                FSSpec fsspec;
                FSCatalogInfo catalog;
                memset(&catalog, 0, sizeof(FSCatalogInfo));
                FSGetCatalogInfo(&fsref, kFSCatInfoGettableInfo, &catalog, 0, &fsspec, 0);
                if (catalog.dataLogicalSize == ExpectedFileSize) {
                    GraphicsImportComponent importer;
                    GetGraphicsImporterForFile(&fsspec, &importer);
                    CGImageRef image = 0;
                    GraphicsImportCreateCGImage(importer, &image,
                                                kGraphicsImportCreateCGImageUsingCurrentSettings);
                    if (image) {
                        int width = CGImageGetWidth(image);
                        int height = CGImageGetHeight(image);
                        if (width == ExpectedImageWidth && height == ExpectedImageHeight) {
                            QPixmap pixmap(ExpectedImageWidth, ExpectedImageHeight);
                            pixmap.fill(Qt::transparent);
                            CGContextRef context = qt_mac_cg_context(&pixmap);
                            HIRect rect = CGRectMake(0., 0., width, height);
                            HIViewDrawCGImage(context, &rect, image);
                            CFRelease(context);
                            CFRelease(image);
                            return pixmap;
                        } else {
                            CFRelease(image);
                        }
                    }
                }
            }
        }
    }
    return QPixmap();

}
#endif

#if defined(Q_WS_WIN)
void QWizardAntiFlickerWidget::paintEvent(QPaintEvent *)
{
    if (wizard->wizardStyle() == QWizard::AeroStyle) {
        QFrame *pageFrame = wizardPrivate->pageFrame;
        Q_ASSERT(pageFrame->parent() == this);
        const int pageFrameBottom =
            pageFrame->mapToParent(QPoint(0, pageFrame->geometry().height())).y();
        QPainter painter(this);
        const QBrush brush(QColor(240, 240, 240)); // ### hardcoded for now
        painter.fillRect(0, pageFrameBottom, width(), height() - pageFrameBottom, brush);
        painter.setPen(QPen(QBrush(QColor(223, 223, 223)), 0)); // ### hardcoded for now
        painter.drawLine(0, pageFrameBottom, width(), pageFrameBottom);
    }
}
#endif

/*!
    \class QWizard
    \brief The QWizard class provides a framework for wizards.

    A wizard (also called an assistant on Mac OS X) is a special type
    of input dialog that consists of a sequence of pages. A wizard's
    purpose is to guide the user through a process step by step.
    Wizards are useful for complex or infrequent tasks that users may
    find difficult to learn.

    QWizard inherits QDialog and represents a wizard. Each page is a
    QWizardPage (a QWidget subclass). To create your own wizards, you
    can use these classes directly, or you can subclass them for more
    control.

    Topics:

    \tableofcontents

    \section1 A Trivial Example

    The following example illustrates how to create wizard pages and
    add them to a wizard. For more advanced examples, see
    \l{classwizard}{Class Wizard} and \l{licensewizard}{License
    Wizard}.

    \quotefromfile trivialwizard/trivialwizard.cpp
    \skipto createIntroPage()
    \printuntil createRegistrationPage()
    \printline {
    \dots
    \skipto /^\}/
    \printline }
    \printuntil createConclusionPage()
    \printline {
    \dots
    \skipto /^\}/
    \printline /^\}/
    \printline main(
    \printuntil /^\}/

    \section1 Wizard Look and Feel

    QWizard supports three wizard looks:

    \list
    \o ClassicStyle is the default on X11 and with Qtopia Core.
    \o ModernStyle is the default on Windows (other than Vista).
    \o MacStyle is the default on Mac OS X.
    \o AeroStyle is the default on Windows Vista.
    \endlist

    You can explicitly set the look to use using setWizardStyle()
    (e.g., if you want the same look on all platforms).

    \table
    \header \o ClassicStyle
            \o ModernStyle
            \o MacStyle
            \o AeroStyle
    \row    \o \inlineimage qtwizard-classic1.png
            \o \inlineimage qtwizard-modern1.png
            \o \inlineimage qtwizard-mac1.png
            \o \inlineimage qtwizard-aero1.png
    \row    \o \inlineimage qtwizard-classic2.png
            \o \inlineimage qtwizard-modern2.png
            \o \inlineimage qtwizard-mac2.png
            \o \inlineimage qtwizard-aero2.png
    \endtable

    Note: AeroStyle has effect only on a Windows Vista system with alpha compositing enabled.
    ModernStyle is used as a fallback when this condition is not met.

    In addition to the wizard style, there are several options that
    control the look and feel of the wizard. These can be set using
    setOption() or setOptions(). For example, HaveHelpButton makes
    QWizard show a \gui Help button along with the other wizard
    buttons.

    You can even change the order of the wizard buttons to any
    arbitrary order using setButtonLayout(), and you can add up to
    three custom buttons (e.g., a \gui Print button) to the button
    row. This is achieved by calling setButton() or setButtonText()
    with CustomButton1, CustomButton2, or CustomButton3 to set up the
    button, and by enabling the HaveCustomButton1, HaveCustomButton2,
    or HaveCustomButton3 options. Whenever the user clicks a custom
    button, customButtonClicked() is emitted. For example:

    \quotefromfile licensewizard/licensewizard.cpp
    \skipto setButtonText
    \printuntil printButtonClicked

    \section1 Elements of a Wizard Page

    Wizards consist of a sequence of \l{QWizardPage}s. At any time,
    only one page is shown. A page has the following attributes:

    \list
    \o A \l{QWizardPage::}{title}.
    \o A \l{QWizardPage::}{subTitle}.
    \o A set of pixmaps, which may or may not be honored, depending
       on the wizard's style:
        \list
        \o WatermarkPixmap (used by ClassicStyle and ModernStyle)
        \o BannerPixmap (used by ModernStyle)
        \o LogoPixmap (used by ClassicStyle and ModernStyle)
        \o BackgroundPixmap (used by MacStyle)
        \endlist
    \endlist

    The diagram belows showns how QWizard renders these attributes,
    assuming they are all present and ModernStyle is used:

    \image qtwizard-nonmacpage.png

    When a \l{QWizardPage::}{subTitle} is set, QWizard displays it
    in a header, in which case it also uses the BannerPixmap and the
    LogoPixmap to decorate the header. The WatermarkPixmap is
    displayed on the left side, below the header. At the bottom,
    there is a row of buttons allowing the user to navigate through
    the pages.

    The page itself (the \l{QWizardPage} widget) occupies the area
    between the header, the watermark, and the button row. Typically,
    the page is a QWizardPage on which a QGridLayout is installed,
    with standard child widgets (\l{QLabel}s, \l{QLineEdit}s, etc.).

    If the wizard's style is MacStyle, the page looks radically
    different:

    \image qtwizard-macpage.png

    The watermark, banner, and logo pixmaps are ignored by the
    MacStyle. If the BackgroundPixmap is set, it is used as the
    background for the wizard; otherwise, a default "assistant" image
    is used.

    The title and subtitle are set by calling
    QWizardPage::setTitle() and QWizardPage::setSubTitle() on the
    individual pages. They may be plain text or HTML (see titleFormat
    and subTitleFormat). The pixmaps can be set globally for the
    entire wizard using setPixmap(), or on a per-page basis using
    QWizardPage::setPixmap().

    \target field mechanism
    \section1 Registering and Using Fields

    In many wizards, the contents of a page may affect the default
    values of the fields of a later page. To make it easy to
    communicate between pages, QWizard supports a "field" mechanism
    that allows you to register a field (e.g., a QLineEdit) on a page
    and to access its value from any page. It is also possible to
    specify mandatory fields (i.e., fields that must be filled before
    the user can advance to the next page).

    To register a field, call QWizardPage::registerField() field.
    For example:

    \quotefromfile classwizard/classwizard.cpp
    \skipto ::ClassInfoPage
    \printuntil {
    \dots
    \skipto classNameLabel
    \printto groupBox =
    \skipto className
    \printuntil qobjectMacro
    \dots
    \skipto /^\}/
    \printline }

    The above code registers three fields, \c className, \c
    baseClass, and \c qobjectMacro, which are associated with three
    child widgets. The asterisk (\c *) next to \c className denotes a
    mandatory field.

    \target initialize page
    The fields of any page are accessible from any other page. For
    example:

    \skipto OutputFilesPage::initializePage
    \printuntil /^\}/

    Here, we call QWizardPage::field() to access the contents of the
    \c className field (which was defined in the \c ClassInfoPage)
    and use it to initialize the \c OuputFilePage. The field's
    contents is returned as a QVariant.

    When we create a field using QWizardPage::registerField(), we
    pass a unique field name and a widget. We can also provide a Qt
    property name and a "changed" signal (a signal that is emitted
    when the property changes) as third and fourth arguments;
    however, this is not necessary for the most common Qt widgets,
    such as QLineEdit, QCheckBox, and QComboBox, because QWizard
    knows which properties to look for.

    \target mandatory fields

    If an asterisk (\c *) is appended to the name when the property
    is registered, the field is a \e{mandatory field}. When a page has
    mandatory fields, the \gui Next and/or \gui Finish buttons are
    enabled only when all mandatory fields are filled.

    To consider a field "filled", QWizard simply checks that the
    field's current value doesn't equal the original value (the value
    it had when initializePage() was called). For QLineEdit, QWizard
    also checks that
    \l{QLineEdit::hasAcceptableInput()}{hasAcceptableInput()} returns
    true, to honor any validator or mask.

    QWizard's mandatory field mechanism is provided for convenience.
    A more powerful (but also more cumbersome) alternative is to
    reimplement QWizardPage::isComplete() and to emit the
    QWizardPage::completeChanged() signal whenever the page becomes
    complete or incomplete.

    The enabled/disabled state of the \gui Next and/or \gui Finish
    buttons is one way to perform validation on the user input.
    Another way is to reimplement validateCurrentPage() (or
    QWizardPage::validatePage()) to perform some last-minute
    validation (and show an error message if the user has entered
    incomplete or invalid information). If the function returns true,
    the next page is shown (or the wizard finishes); otherwise, the
    current page stays up.

    \section1 Creating Linear Wizards

    Most wizards have a linear structure, with page 1 followed by
    page 2 and so on until the last page. The \l{classwizard}{Class
    Wizard} example is such a wizard. With QWizard, linear wizards
    are created by instantiating the \l{QWizardPage}s and inserting
    them using addPage(). By default, the pages are shown in the
    order in which they were added. For example:

    \quotefromfile classwizard/classwizard.cpp
    \skipto ::ClassWizard
    \printuntil addPage(new Conclu
    \dots
    \skipto /^\}/
    \printline }

    When a page is about to be shown, QWizard calls initializePage()
    (which in turn calls QWizardPage::initializePage()) to fill the
    page with default values. By default, this function does nothing,
    but it can be reimplemented to initialize the page's contents
    based on other pages' fields (see the \l{initialize page}{example
    above}).

    If the user presses \gui Back, cleanupPage() is called (which in
    turn calls QWizardPage::cleanupPage()). The default
    implementation resets the page's fields to their original values
    (the values they had before initializePage() was called). If you
    want the \gui Back button to be non-destructive and keep the
    values entered by the user, simply enable the IndependentPages
    option.

    \section1 Creating Non-Linear Wizards

    Some wizards are more complex in that they allow different
    traversal paths based on the information provided by the user.
    The \l{licensewizard}{License Wizard} example illustrates this.
    It provides five wizard pages; depending on which options are
    selected, the user can reach different pages.

    \image licensewizard-flow.png

    In complex wizards, pages are identified by IDs. These IDs are
    typically defined using an enum. For example:

    \quotefromfile licensewizard/licensewizard.h
    \skipto : public QWizard
    \printuntil {
    \dots
    \skipto enum {
    \printuntil };
    \dots
    \skipto /^\};/
    \printline }

    The pages are inserted using setPage(), which takes an ID and an
    instance of QWizardPage (or of a subclass):

    \quotefromfile licensewizard/licensewizard.cpp
    \skipto ::LicenseWizard
    \printuntil setPage(Page_Conclusion
    \dots
    \skipto /^\}/
    \printline }

    By default, the pages are shown in increasing ID order. To
    provide a dynamic order that depends on the options chosen by the
    user, we must reimplement QWizardPage::nextId(). For example:

    \skipto IntroPage::nextId
    \printuntil /^\}/

    \skipto EvaluatePage::nextId
    \printuntil /^\}/

    \skipto RegisterPage::nextId
    \printuntil /^\}/

    \skipto DetailsPage::nextId
    \printuntil /^\}/

    \skipto ConclusionPage::nextId
    \printuntil /^\}/

    It would also be possible to put all the logic in one place, in a
    QWizard::nextId() reimplementation. For example:

    \code
        int LicenseWizard::nextId() const
        {
            switch (currentId()) {
            case Page_Intro:
                if (field("intro.evaluate").toBool()) {
                    return Page_Evaluate;
                } else {
                    return Page_Register;
                }
            case Page_Evaluate:
                return Page_Conclusion;
            case Page_Register:
                if (field("register.upgradeKey").toString().isEmpty()) {
                    return Page_Details;
                } else {
                    return Page_Conclusion;
                }
            case Page_Details:
                return Page_Conclusion;
            case Page_Conclusion:
            default:
                return -1;
            }
        }
    \endcode

    To start at another page than the page with the lowest ID, call
    setStartId().

    To test whether a page has been visited or not, call
    hasVisitedPage(). For example:

    \skipto ConclusionPage::initializePage
    \printuntil /^\}/

    \sa QWizardPage, {Class Wizard Example}, {License Wizard Example}
*/

/*!
    \enum QWizard::WizardButton

    This enum specifies the buttons in a wizard.

    \value BackButton  The \gui Back button (\gui {Go Back} on Mac OS X)
    \value NextButton  The \gui Next button (\gui Continue on Mac OS X)
    \value CommitButton  The \gui Commit button
    \value FinishButton  The \gui Finish button (\gui Done on Mac OS X)
    \value CancelButton  The \gui Cancel button (see also NoCancelButton)
    \value HelpButton    The \gui Help button (see also HaveHelpButton)
    \value CustomButton1  The first user-defined button (see also HaveCustomButton1)
    \value CustomButton2  The second user-defined button (see also HaveCustomButton2)
    \value CustomButton3  The third user-defined button (see also HaveCustomButton3)

    The following value is only useful when calling setButtonLayout():

    \value Stretch  A horizontal stretch in the button layout

    \omitvalue NoButton
    \omitvalue NStandardButtons
    \omitvalue NButtons

    \sa setButton(), setButtonText(), setButtonLayout(), customButtonClicked()
*/

/*!
    \enum QWizard::WizardPixmap

    This enum specifies the pixmaps that can be associated with a page.

    \value WatermarkPixmap  The tall pixmap on the left side of a ClassicStyle or ModernStyle page
    \value LogoPixmap  The small pixmap on the right side of a ClassicStyle or ModernStyle page header
    \value BannerPixmap  The pixmap that occupies the background of a ModernStyle page header
    \value BackgroundPixmap  The pixmap that occupies the background of a MacStyle wizard

    \omitvalue NPixmaps

    \sa setPixmap(), QWizardPage::setPixmap(), {Elements of a Wizard Page}
*/

/*!
    \enum QWizard::WizardStyle

    This enum specifies the different looks supported by QWizard.

    \value ClassicStyle  Classic Windows look (default on X11 and Qtopia)
    \value ModernStyle  Modern Windows look (default on Windows)
    \value MacStyle  Mac OS X look (default on Mac OS X)
    \value AeroStyle  Windows Aero look (default on Windows Vista)

    \sa setWizardStyle(), WizardOption, {Wizard Look and Feel}
*/

/*!
    \enum QWizard::WizardOption

    This enum specifies various options that affect the look and feel
    of a wizard.

    \value IndependentPages  The pages are independent of each other
                             (i.e., they don't derive values from each
                             other).
    \value IgnoreSubTitles  Don't show any subtitles, even if they are set.
    \value ExtendedWatermarkPixmap  Extend any WatermarkPixmap all the
                                    way down to the window's edge.
    \value NoDefaultButton  Don't make the \gui Next or \gui Finish button the
                            dialog's \l{QPushButton::setDefault()}{default button}.
    \value NoBackButtonOnStartPage  Don't show the \gui Back button on the start page.
    \value NoBackButtonOnLastPage   Don't show the \gui Back button on the last page.
    \value DisabledBackButtonOnLastPage  Disable the \gui Back button on the last page.
    \value HaveNextButtonOnLastPage  Show the (disabled) \gui Next button on the last page.
    \value HaveFinishButtonOnEarlyPages  Show the (disabled) \gui Finish button on non-final pages.
    \value NoCancelButton  Don't show the \gui Cancel button.
    \value CancelButtonOnLeft  Put the \gui Cancel button on the left of \gui Back (rather than on
                               the right of \gui Finish or \gui Next).
    \value HaveHelpButton  Show the \gui Help button.
    \value HelpButtonOnRight  Put the \gui Help button on the far right of the button layout
                              (rather than on the far left).
    \value HaveCustomButton1  Show the first user-defined button (CustomButton1).
    \value HaveCustomButton2  Show the second user-defined button (CustomButton2).
    \value HaveCustomButton3  Show the third user-defined button (CustomButton3).

    \sa setOptions(), setOption(), testOption()
*/

/*!
    Constructs a wizard with the given \a parent and window \a flags.

    \sa parent(), windowFlags()
*/
QWizard::QWizard(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(*new QWizardPrivate, parent, flags)
{
    Q_D(QWizard);
    d->init();
}

/*!
    Destroys the wizard and its pages, releasing any allocated resources.
*/
QWizard::~QWizard()
{
    Q_D(QWizard);
    delete d->buttonLayout;
}

/*!
    Adds the given \a page to the wizard, and returns the page's ID.

    The ID is guaranteed to be larger than any other ID in the
    QWizard so far.

    \sa setPage(), page()
*/
int QWizard::addPage(QWizardPage *page)
{
    Q_D(QWizard);
    int id = 0;
    if (!d->pageMap.isEmpty())
        id = (d->pageMap.constEnd() - 1).key() + 1;
    setPage(id, page);
    return id;
}

/*!
    Adds the given \a page to the wizard with the given \a id.

    \sa addPage(), page()
*/
void QWizard::setPage(int id, QWizardPage *page)
{
    Q_D(QWizard);

    if (!page) {
        qWarning("QWizard::setPage: Cannot insert null page");
        return;
    }

    if (id == -1) {
        qWarning("QWizard::setPage: Cannot insert page with ID -1");
        return;
    }

    if (d->pageMap.contains(id)) {
        qWarning("QWizard::setPage: Page with duplicate ID %d ignored", id);
        return;
    }

    page->setParent(d->pageFrame);

    QVector<QWizardField> &pendingFields = page->d_func()->pendingFields;
    for (int i = 0; i < pendingFields.count(); ++i)
        d->addField(pendingFields.at(i));
    pendingFields.clear();

    connect(page, SIGNAL(completeChanged()), this, SLOT(_q_updateButtonStates()));

    d->pageMap.insert(id, page);
    page->d_func()->wizard = this;

    int n = d->pageVBoxLayout->count();

    // disable layout to prevent layout updates while adding
    bool pageVBoxLayoutEnabled = d->pageVBoxLayout->isEnabled();
    d->pageVBoxLayout->setEnabled(false);

    d->pageVBoxLayout->insertWidget(n - 1, page);

    /*
        If the page can stretch, let it stretch; otherwise, we stretch
        "infinitely" more than the page itself.
    */
    int stretch = 32768
                  * int(d->pageVBoxLayout->itemAt(n - 1)->expandingDirections() & Qt::Vertical);
    d->pageVBoxLayout->setStretchFactor(page, stretch);

    // hide new page and reset layout to old status
    page->hide();
    d->pageVBoxLayout->setEnabled(pageVBoxLayoutEnabled);
}

/*!
    Returns the page with the given \a id, or 0 if there is no such
    page.

    \sa addPage(), setPage()
*/
QWizardPage *QWizard::page(int id) const
{
    Q_D(const QWizard);
    return d->pageMap.value(id);
}

/*!
    Returns true if the page history contains page \a id; otherwise,
    returns false.

    Pressing \gui Back marks the current page as "unvisited" again.

    \sa visitedPages()
*/
bool QWizard::hasVisitedPage(int id) const
{
    Q_D(const QWizard);
    return d->history.contains(id);
}

/*!
    Returns the list of visited pages, in the order in which they
    were visited.

    Pressing \gui Back marks the current page as "unvisited" again.

    \sa hasVisitedPage()
*/
QList<int> QWizard::visitedPages() const
{
    Q_D(const QWizard);
    return d->history;
}

/*!
    \property QWizard::startId
    \brief the ID of the first page

    If this property isn't explicitly set, this property defaults to
    the lowest page ID in this wizard, or -1 if no page has been
    inserted yet.

    \sa restart(), nextId()
*/
void QWizard::setStartId(int id)
{
    Q_D(QWizard);
    if (!d->pageMap.contains(id)) {
        qWarning("QWizard::setStartId: Invalid page ID %d", id);
        return;
    }
    d->start = id;
}

int QWizard::startId() const
{
    Q_D(const QWizard);
    if (d->start != -1)
        return d->start;
    if (!d->pageMap.isEmpty())
        return d->pageMap.constBegin().key();
    return -1;
}

/*!
    Returns a pointer to the current page, or 0 if there is no current
    page (e.g., before the wizard is shown).

    This is equivalent to calling page(currentId()).

    \sa page(), currentId(), restart()
*/
QWizardPage *QWizard::currentPage() const
{
    Q_D(const QWizard);
    return page(d->current);
}

/*!
    \property QWizard::currentId
    \brief the ID of the current page

    This property cannot be set directly. To change the current page,
    call next(), back(), or restart().

    \sa currentIdChanged(), currentPage()
*/
int QWizard::currentId() const
{
    Q_D(const QWizard);
    return d->current;
}

/*!
    Sets the value of the field called \a name to \a value.

    This function can be used to set fields on any page of the wizard.

    \sa QWizardPage::registerField(), QWizardPage::setField(), field()
*/
void QWizard::setField(const QString &name, const QVariant &value)
{
    Q_D(QWizard);

    int index = d->fieldIndexMap.value(name, -1);
    if (index != -1) {
        const QWizardField &field = d->fields.at(index);
        if (!field.object->setProperty(field.property, value))
            qWarning("QWizard::setField: Couldn't write to property '%s'",
                     field.property.constData());
        return;
    }

    qWarning("QWizard::setField: No such field '%s'", qPrintable(name));
}

/*!
    Returns the value of the field called \a name.

    This function can be used to access fields on any page of the wizard.

    \sa QWizardPage::registerField(), QWizardPage::field(), setField()
*/
QVariant QWizard::field(const QString &name) const
{
    Q_D(const QWizard);

    int index = d->fieldIndexMap.value(name, -1);
    if (index != -1) {
        const QWizardField &field = d->fields.at(index);
        return field.object->property(field.property);
    }

    qWarning("QWizard::field: No such field '%s'", qPrintable(name));
    return QVariant();
}

/*!
    \property QWizard::wizardStyle
    \brief the look and feel of the wizard

    By default, QWizard uses the ClassicStyle on X11 and QWS
    (Qtopia Core), the ModernStyle on Windows, and the MacStyle on
    Mac OS X.

    \sa {Wizard Look and Feel}, options
*/
void QWizard::setWizardStyle(WizardStyle style)
{
    Q_D(QWizard);

    bool styleChange = false;

#if defined(Q_WS_WIN)
    bool aeroStyleChange = true;

    if (style == AeroStyle) {
        if (d->vistaHelper->isCompositionEnabled()) {
            if (d->wizStyle != AeroStyle) {
                styleChange = true;
                d->aeroStyleOverride = false;
            }
        } else {
            style = ModernStyle;
            styleChange = d->wizStyle != ModernStyle;
            aeroStyleChange = d->wizStyle == AeroStyle; 
            d->aeroStyleOverride = true;
        }
    } else if (d->wizStyle == AeroStyle) {
        styleChange = true;
        d->aeroStyleOverride = false;
    } else if (style == ModernStyle && d->aeroStyleOverride) {
        Q_ASSERT(d->wizStyle == ModernStyle);
        d->aeroStyleOverride = false;
    } else if (style != d->wizStyle) {
        styleChange = true;
        aeroStyleChange = false;
        d->aeroStyleOverride = false;
    }
#else
    Q_ASSERT(d->wizStyle != AeroStyle);
    styleChange = style != d->wizStyle;
    if (style == AeroStyle) {
        style = ModernStyle;
        d->aeroStyleOverride = true;
    } else {
        d->aeroStyleOverride = false;
    }
#endif

    if (styleChange) {
        d->disableUpdates();
        d->wizStyle = style;
#if defined(Q_WS_WIN)
        if (aeroStyleChange)
            d->handleAeroStyleChange();
#endif
        d->updateButtonTexts();
        d->updateLayout();
        updateGeometry();
        d->enableUpdates();
    }
}

QWizard::WizardStyle QWizard::wizardStyle() const
{
    Q_D(const QWizard);
    return d->aeroStyleOverride ? AeroStyle : d->wizStyle;
}

/*!
    Sets the given \a option to be enabled if \a on is true;
    otherwise, clears the given \a option.

    \sa options, testOption(), setWizardStyle()
*/
void QWizard::setOption(WizardOption option, bool on)
{
    Q_D(QWizard);
    if (!(d->opts & option) != !on)
        setOptions(d->opts ^ option);
}

/*!
    Returns true if the given \a option is enabled; otherwise, returns
    false.

    \sa options, setOption(), setWizardStyle()
*/
bool QWizard::testOption(WizardOption option) const
{
    Q_D(const QWizard);
    return (d->opts & option) != 0;
}

/*!
    \property QWizard::options
    \brief the various options that affect the look and feel of the wizard

    By default, the following options are set (depending on the platform):

    \list
    \o Windows: HelpButtonOnRight.
    \o Mac OS X: NoDefaultButton and NoCancelButton.
    \o X11 and QWS (Qtopia Core): none.
    \endlist

    \sa wizardStyle
*/
void QWizard::setOptions(WizardOptions options)
{
    Q_D(QWizard);

    WizardOptions changed = (options ^ d->opts);
    if (!changed)
        return;

    d->disableUpdates();

    d->opts = options;
    if ((changed & IndependentPages) && !(d->opts & IndependentPages))
        d->cleanupPagesNotInHistory();

    if (changed & (NoDefaultButton | HaveHelpButton | HelpButtonOnRight | NoCancelButton
                   | CancelButtonOnLeft | HaveCustomButton1 | HaveCustomButton2
                   | HaveCustomButton3)) {
        d->updateButtons();
    } else if (changed & (NoBackButtonOnStartPage | NoBackButtonOnLastPage
                          | HaveNextButtonOnLastPage | HaveFinishButtonOnEarlyPages
                          | DisabledBackButtonOnLastPage)) {
        d->_q_updateButtonStates();
    }

    d->enableUpdates();

    if (changed & (IgnoreSubTitles | ExtendedWatermarkPixmap))
        d->updateLayout();
    else
        d->updateMinMaxSizes(d->layoutInfoForCurrentPage());
}

QWizard::WizardOptions QWizard::options() const
{
    Q_D(const QWizard);
    return d->opts;
}

/*!
    Sets the text on button \a which to be \a text.

    By default, the text on buttons depends on the wizardStyle. For
    example, on Mac OS X, the \gui Next button is called \gui
    Continue.

    To add extra buttons to the wizard (e.g., a \gui Print button),
    one way is to call setButtonText() with CustomButton1,
    CustomButton2, or CustomButton3 to set their text, and make the
    buttons visible using the HaveCustomButton1, HaveCustomButton2,
    and/or HaveCustomButton3 options.

    Button texts may also be set on a per-page basis using QWizardPage::setButtonText().

    \sa setButton(), button(), setButtonLayout(), setOptions(), QWizardPage::setButtonText()
*/
void QWizard::setButtonText(WizardButton which, const QString &text)
{
    Q_D(QWizard);

    if (!d->ensureButton(which))
        return;

    d->buttonCustomTexts.insert(which, text);

    if (!currentPage() || !currentPage()->d_func()->buttonCustomTexts.contains(which))
        d->btns[which]->setText(text);
}

/*!
    Returns the text on button \a which.

    If a text has ben set using setButtonText(), this text is returned.

    By default, the text on buttons depends on the wizardStyle. For
    example, on Mac OS X, the \gui Next button is called \gui
    Continue.

    \sa button(), setButton(), setButtonText(), QWizardPage::buttonText(),
    QWizardPage::setButtonText()
*/
QString QWizard::buttonText(WizardButton which) const
{
    Q_D(const QWizard);

    if (!d->ensureButton(which))
        return QString();

    if (d->buttonCustomTexts.contains(which))
        return d->buttonCustomTexts.value(which);

    if (d->buttonDefaultTexts.value(d->wizStyle).contains(which))
        return d->buttonDefaultTexts.value(d->wizStyle).value(which);

    return d->btns[which]->text();
}

/*!
    Sets the order in which buttons are displayed to \a layout, where
    \a layout is a list of \l{WizardButton}s.

    The default layout depends on the options (e.g., whether
    HelpButtonOnRight) that are set. You can call this function if
    you need more control over the buttons' layout than what \l
    options already provides.

    You can specify horizontal stretches in the layout using \l
    Stretch.

    Example:

    \code
        MyWizard::MyWizard(QWidget *parent)
            : QWizard(parent)
        {
            ...
            QList<QWizard::WizardButton> layout;
            layout << QWizard::Stretch << QWizard::BackButton << QWizard::CloseButton
                   << QWizard::NextButton << QWizard::FinishButton;
            setButtonLayout(layout);
            ...
        }
    \endcode

    \sa setButton(), setButtonText(), setOptions()
*/
void QWizard::setButtonLayout(const QList<WizardButton> &layout)
{
    Q_D(QWizard);

    for (int i = 0; i < layout.count(); ++i) {
        WizardButton button1 = layout.at(i);

        if (button1 == NoButton || button1 == Stretch)
            continue;
        if (!d->ensureButton(button1))
            return;

        // O(n^2), but n is very small
        for (int j = 0; j < i; ++j) {
            WizardButton button2 = layout.at(j);
            if (button2 == button1) {
                qWarning("QWizard::setButtonLayout: Duplicate button in layout");
                return;
            }
        }
    }

    d->buttonsHaveCustomLayout = true;
    d->buttonsCustomLayout = layout;
    d->updateButtons();
}

/*!
    Sets the button corresponding to role \a which to \a button.

    To add extra buttons to the wizard (e.g., a \gui Print button),
    one way is to call setButton() with CustomButton1 to
    CustomButton3, and make the buttons visible using the
    HaveCustomButton1 to HaveCustomButton3 options.

    \sa setButtonText(), setButtonLayout(), options
*/
void QWizard::setButton(WizardButton which, QAbstractButton *button)
{
    Q_D(QWizard);

    if (uint(which) >= NButtons || d->btns[which] == button)
        return;

    if (QAbstractButton *oldButton = d->btns[which]) {
        d->buttonLayout->removeWidget(oldButton);
        delete oldButton;
    }

    d->btns[which] = button;
    if (button) {
        button->setParent(d->antiFlickerWidget);
        d->buttonCustomTexts.insert(which, button->text());
        d->connectButton(which);
    } else {
        d->buttonCustomTexts.remove(which); // ### what about page-specific texts set for 'which'
        d->ensureButton(which);             // (QWizardPage::setButtonText())? Clear them as well?
    }

    d->updateButtons();
}

/*!
    Returns the button corresponding to role \a which.

    \sa setButton(), setButtonText()
*/
QAbstractButton *QWizard::button(WizardButton which) const
{
    Q_D(const QWizard);
    if (!d->ensureButton(which))
        return 0;
    return d->btns[which];
}

/*!
    \property QWizard::titleFormat
    \brief the text format used by page titles

    The default format is Qt::AutoText.

    \sa QWizardPage::title, subTitleFormat
*/
void QWizard::setTitleFormat(Qt::TextFormat format)
{
    Q_D(QWizard);
    d->titleFmt = format;
    d->updateLayout();
}

Qt::TextFormat QWizard::titleFormat() const
{
    Q_D(const QWizard);
    return d->titleFmt;
}

/*!
    \property QWizard::subTitleFormat
    \brief the text format used by page subtitles

    The default format is Qt::AutoText.

    \sa QWizardPage::title, titleFormat
*/
void QWizard::setSubTitleFormat(Qt::TextFormat format)
{
    Q_D(QWizard);
    d->subTitleFmt = format;
    d->updateLayout();
}

Qt::TextFormat QWizard::subTitleFormat() const
{
    Q_D(const QWizard);
    return d->subTitleFmt;
}

/*!
    Sets the pixmap for role \a which to \a pixmap.

    The pixmaps are used by QWizard when displaying a page. Which
    pixmaps are actually used depend on the \l{Wizard Look and
    Feel}{wizard style}.

    Pixmaps can also be set for a specific page using
    QWizardPage::setPixmap().

    \sa QWizardPage::setPixmap(), {Elements of a Wizard Page}
*/
void QWizard::setPixmap(WizardPixmap which, const QPixmap &pixmap)
{
    Q_D(QWizard);
    Q_ASSERT(uint(which) < NPixmaps);
    d->defaultPixmaps[which] = pixmap;
    d->updatePixmap(which);
}

/*!
    Returns the pixmap set for role \a which.

    By default, the only pixmap that is set is the BackgroundPixmap on
    Mac OS X.

    \sa QWizardPage::pixmap(), {Elements of a Wizard Page}
*/
QPixmap QWizard::pixmap(WizardPixmap which) const
{
    Q_D(const QWizard);
    Q_ASSERT(uint(which) < NPixmaps);
#ifdef Q_WS_MAC
    if (which == BackgroundPixmap && d->defaultPixmaps[BackgroundPixmap].isNull())
        d->defaultPixmaps[BackgroundPixmap] = d->findDefaultBackgroundPixmap();
#endif
    return d->defaultPixmaps[which];
}

/*!
    Sets the default property for \a className to be \a property,
    and the associated change signal to be \a changedSignal.

    The default property is used when an instance of \a className (or
    of one of its subclasses) is passed to
    QWizardPage::registerField() and no property is specified.

    QWizard knows the most common Qt widgets. For these (or their
    subclasses), you don't need to specify a \a property or a \a
    changedSignal. The table below lists these widgets:

    \table
    \header \o Widget          \o Property                            \o Change Notification Signal
    \row    \o QAbstractButton \o bool \l{QAbstractButton::}{checked} \o \l{QAbstractButton::}{toggled()}
    \row    \o QAbstractSlider \o int \l{QAbstractSlider::}{value}    \o \l{QAbstractSlider::}{valueChanged()}
    \row    \o QComboBox       \o int \l{QComboBox::}{currentIndex}   \o \l{QComboBox::}{currentIndexChanged()}
    \row    \o QDateTimeEdit   \o QDateTime \l{QDateTimeEdit::}{dateTime} \o \l{QDateTimeEdit::}{dateTimeChanged()}
    \row    \o QLineEdit       \o QString \l{QLineEdit::}{text}       \o \l{QLineEdit::}{textChanged()}
    \row    \o QListWidget     \o int \l{QListWidget::}{currentRow}   \o \l{QListWidget::}{currentRowChanged()}
    \row    \o QSpinBox        \o int \l{QSpinBox::}{value}           \o \l{QSpinBox::}{valueChanged()}
    \endtable

    \sa QWizardPage::registerField()
*/
void QWizard::setDefaultProperty(const char *className, const char *property,
                                 const char *changedSignal)
{
    Q_D(QWizard);
    for (int i = d->defaultPropertyTable.count() - 1; i >= 0; --i) {
        if (qstrcmp(d->defaultPropertyTable.at(i).className, className) == 0) {
            d->defaultPropertyTable.remove(i);
            break;
        }
    }
    d->defaultPropertyTable.append(QWizardDefaultProperty(className, property, changedSignal));
}

/*!
    \reimp
*/
void QWizard::setVisible(bool visible)
{
    Q_D(QWizard);
    if (visible) {
        if (d->current == -1)
            restart();
    }
    QDialog::setVisible(visible);
}

/*!
    \reimp
*/
QSize QWizard::sizeHint() const
{
    Q_D(const QWizard);
    QSize result = d->mainLayout->totalSizeHint();
    QSize extra(500, 360);
    if (d->wizStyle == MacStyle && d->current != -1) {
        QSize pixmap(currentPage()->pixmap(BackgroundPixmap).size());
        extra.setWidth(616);
        if (!pixmap.isNull()) {
            extra.setHeight(pixmap.height());

            /*
                The width isn't always reliable as a size hint, as
                some wizard backgrounds just cover the leftmost area.
                Use a rule of thumb to determine if the width is
                reliable or not.
            */
            if (pixmap.width() >= pixmap.height())
                extra.setWidth(pixmap.width());
        }
    }
    return result.expandedTo(extra);
}

/*!
    \fn void QWizard::currentIdChanged(int id)

    This signal is emitted when the current page changes, with the new
    current \a id.

    \sa currentId(), currentPage()
*/

/*!
    \fn void QWizard::helpRequested()

    This signal is emitted when the user clicks the \gui Help button.

    By default, no \gui Help button is shown. Call
    setOption(HaveHelpButton, true) to have one.

    Example:

    \quotefromfile licensewizard/licensewizard.cpp
    \skipto ::LicenseWizard
    \printuntil {
    \dots
    \skipto HaveHelpButton
    \printline HaveHelpButton
    \skipto helpRequested
    \printuntil ;
    \dots
    \skipto /^\}/
    \printline }
    \skipto ::showHelp
    \printuntil break;
    \dots
    \skipto default:
    \printuntil }
    \skipto QMessageBox::information
    \printuntil ;
    \skipto /^\}/
    \printline }

    \sa customButtonClicked()
*/

/*!
    \fn void QWizard::customButtonClicked(int which)

    This signal is emitted when the user clicks a custom button. \a
    which can be CustomButton1, CustomButton2, or CustomButton3.

    By default, no custom button is shown. Call setOption() with
    HaveCustomButton1, HaveCustomButton2, or HaveCustomButton3 to have
    one, and use setButtonText() or setButton() to configure it.

    \sa helpRequested()
*/

/*!
    Goes back to the previous page.

    This is equivalent to pressing the \gui Back button.

    \sa next(), accept(), reject(), restart()
*/
void QWizard::back()
{
    Q_D(QWizard);
    int n = d->history.count() - 2;
    if (n < 0)
        return;
    d->switchToPage(d->history.at(n), QWizardPrivate::Backward);
}

/*!
    Advances to the next page.

    This is equivalent to pressing the \gui Next or \gui Commit button.

    \sa nextId(), back(), accept(), reject(), restart()
*/
void QWizard::next()
{
    Q_D(QWizard);

    if (d->current == -1)
        return;

    if (validateCurrentPage()) {
        int next = nextId();
        if (next != -1) {
            if (d->history.contains(next)) {
                qWarning("QWizard::next: Page %d already met", next);
                return;
            }
            if (!d->pageMap.contains(next)) {
                qWarning("QWizard::next: No such page %d", next);
                return;
            }
            d->switchToPage(next, QWizardPrivate::Forward);
        }
    }
}

/*!
    Restarts the wizard at the start page.

    \sa startId()
*/
void QWizard::restart()
{
    Q_D(QWizard);
    d->disableUpdates();
    d->reset();
    d->switchToPage(startId(), QWizardPrivate::Forward);
    d->enableUpdates();
}

/*!
    \reimp
*/
bool QWizard::event(QEvent *event)
{
    Q_D(QWizard);
    if (event->type() == QEvent::StyleChange) {
        d->updateLayout();
    }
#if defined(Q_WS_WIN)
    else if (d->wizStyle == AeroStyle) {
        d->vistaHelper->mouseEvent(event);
    }
#endif
    return QDialog::event(event);
}

/*!
    \reimp
*/
void QWizard::resizeEvent(QResizeEvent *event)
{
    Q_D(QWizard);
    int heightOffset = 0;
#if defined(Q_WS_WIN)
    if (d->wizStyle == AeroStyle)
        heightOffset = d->vistaHelper->titleBarSize() + d->vistaHelper->topOffset();
#endif
    d->antiFlickerWidget->resize(event->size().width(), event->size().height() - heightOffset);
#if defined(Q_WS_WIN)
    if (d->wizStyle == AeroStyle)
        d->vistaHelper->resizeEvent(event);
#endif
}

/*!
    \reimp
*/
void QWizard::paintEvent(QPaintEvent * event)
{
    Q_D(QWizard);
    if (d->wizStyle == MacStyle && currentPage()) {
        QPixmap backgroundPixmap = currentPage()->pixmap(BackgroundPixmap);
        if (backgroundPixmap.isNull())
            return;

        QPainter painter(this);
        painter.drawPixmap(0, (height() - backgroundPixmap.height()) / 2, backgroundPixmap);
    }
#if defined(Q_WS_WIN)
    else if (d->wizStyle == AeroStyle) {
        d->vistaHelper->paintEvent(event);
    }
#else
    Q_UNUSED(event);
#endif
}

#if defined(Q_WS_WIN)
bool QWizard::winEvent(MSG *message, long *result)
{
    Q_D(QWizard);
    const bool enterCompositionMode =
        d->vistaHelper->isCompositionEnabled() && !d->wasCompositionEnabled;
    const bool leaveCompositionMode =
        !d->vistaHelper->isCompositionEnabled() && d->wasCompositionEnabled;
    d->wasCompositionEnabled = d->vistaHelper->isCompositionEnabled();
    if (wizardStyle() == AeroStyle) {
        if (enterCompositionMode) {
            Q_ASSERT(d->aeroStyleOverride);
            setWizardStyle(AeroStyle);
            Q_ASSERT(!d->aeroStyleOverride);
        } else if (leaveCompositionMode) {
            Q_ASSERT(!d->aeroStyleOverride);
            setWizardStyle(AeroStyle);
            Q_ASSERT(d->aeroStyleOverride);
        }
    }
    return d->vistaHelper->handleWinEvent(message, result);
}
#endif

/*!
    \reimp
*/
void QWizard::done(int result)
{
    Q_D(QWizard);
    // canceling leaves the wizard in a known state
    if (result == Rejected) {
        d->reset();
    } else {
        if (!validateCurrentPage())
            return;
    }
    QDialog::done(result);
}

/*!
    This virtual function is called by QWizard to prepare page \a id
    just before it is shown. (However, if the \l
    QWizard::IndependentPages option is set, this function is only
    called the first time the page is shown.)

    By reimplementing this function, you can ensure that the page's
    fields are properly initialized based on fields from previous
    pages.

    The default implementation calls QWizardPage::initializePage() on
    page(\a id).

    \sa QWizardPage::initializePage(), cleanupPage()
*/
void QWizard::initializePage(int id)
{
    QWizardPage *page = this->page(id);
    if (page)
        page->initializePage();
}

/*!
    This virtual function is called by QWizard when the user clicks
    \gui Back (unless the QWizard::IndependentPages option is set).

    The default implementation calls QWizardPage::cleanupPage() on
    page(\a id).

    \sa QWizardPage::cleanupPage(), initializePage()
*/
void QWizard::cleanupPage(int id)
{
    QWizardPage *page = this->page(id);
    if (page)
        page->cleanupPage();
}

/*!
    This virtual function is called by QWizard when the user clicks
    \gui Next or \gui Finish to perform some last-minute validation.
    If it returns true, the next page is shown (or the wizard
    finishes); otherwise, the current page stays up.

    The default implementation calls QWizardPage::validatePage() on
    the currentPage().

    When possible, it is usually better style to disable the \gui
    Next or \gui Finish button (by specifying \l{mandatory fields} or
    by reimplementing QWizardPage::isComplete()) than to reimplement
    validateCurrentPage().

    \sa QWizardPage::validatePage(), currentPage()
*/
bool QWizard::validateCurrentPage()
{
    QWizardPage *page = currentPage();
    if (!page)
        return true;

    return page->validatePage();
}

/*!
    This virtual function is called by QWizard to find out which page
    to show when the user clicks the \gui Next button.

    The default implementation calls QWizardPage::nextId() on the
    currentPage().

    \sa QWizardPage::nextId(), currentPage()
*/
int QWizard::nextId() const
{
    const QWizardPage *page = currentPage();
    if (!page)
        return -1;

    return page->nextId();
}

/*!
    \class QWizardPage
    \brief The QWizardPage class is the base class for wizard pages.

    QWizard represents a wizard. Each page is a QWizardPage. When
    you create your own wizards, you can use QWizardPage directly,
    or you can subclass it for more control.

    A page has the following attributes, which are rendered by
    QWizard: a \l title, a \l subTitle, and a \l{setPixmap()}{set of
    pixmaps}. See \l{Elements of a Wizard Page} for details. Once a
    page is added to the wizard (using QWizard::addPage() or
    QWizard::setPage()), wizard() returns a pointer to the
    associated QWizard object.

    Page provides five virtual functions that can be reimplemented to
    provide custom behavior:

    \list
    \o initializePage() is called to initialize the page's contents
       when the user clicks the wizard's \gui Next button. If you
       want to derive the page's default from what the user entered
       on previous pages, this is the function to reimplement.
    \o cleanupPage() is called to reset the page's contents when the
       user clicks the wizard's \gui Back button.
    \o validatePage() validates the page when the user clicks \gui
       Next or \gui Finish. It is often used to show an error message
       if the user has entered incomplete or invalid information.
    \o nextId() returns the ID of the next page. It is useful when
       \l{creating non-linear wizards}, which allow different
       traversal paths based on the information provided by the user.
    \o isComplete() is called to determine whether the \gui Next
       and/or \gui Finish button should be enabled or disabled. If
       you reimplement isComplete(), also make sure that
       completeChanged() is emitted whenever the complete state
       changes.
    \endlist

    Normally, the \gui Next button and the \gui Finish button of a
    wizard are mutually exclusive. If isFinalPage() returns true, \gui
    Finish is available; otherwise, \gui Next is available. By
    default, isFinalPage() is true only when nextId() returns -1. If
    you want to show \gui Next and \gui Final simultaneously for a
    page (letting the user perform an "early finish"), call
    setFinalPage(true) on that page. For wizards that support early
    finishes, you might also want to set the
    \l{QWizard::}{HaveNextButtonOnLastPage} and
    \l{QWizard::}{HaveFinishButtonOnEarlyPages} options on the
    wizard.

    In many wizards, the contents of a page may affect the default
    values of the fields of a later page. To make it easy to
    communicate between pages, QWizard supports a \l{Registering and
    Using Fields}{"field" mechanism} that allows you to register a
    field (e.g., a QLineEdit) on a page and to access its value from
    any page. Fields are global to the entire wizard and make it easy
    for any single page to access information stored by another page,
    without having to put all the logic in QWizard or having the
    pages know explicitly about each other. Fields are registered
    using registerField() and can be accessed at any time using
    field() and setField().

    \sa QWizard, {Class Wizard Example}, {License Wizard Example}
*/

/*!
    Constructs a wizard page with the given \a parent.

    When the page is inserted into a wizard using QWizard::addPage()
    or QWizard::setPage(), the parent is automatically set to be the
    wizard.

    \sa wizard()
*/
QWizardPage::QWizardPage(QWidget *parent)
    : QWidget(*new QWizardPagePrivate, parent, 0)
{
    connect(this, SIGNAL(completeChanged()), this, SLOT(_q_updateCachedCompleteState()));
}

/*!
    \property QWizardPage::title
    \brief the title of the page

    The title is shown by the QWizard, above the actual page. All
    pages should have a title.

    The title may be plain text or HTML, depending on the value of the
    \l{QWizard::titleFormat} property.

    \sa subTitle, {Elements of a Wizard Page}
*/
void QWizardPage::setTitle(const QString &title)
{
    Q_D(QWizardPage);
    d->title = title;
    if (d->wizard && d->wizard->currentPage() == this)
        d->wizard->d_func()->updateLayout();
}

QString QWizardPage::title() const
{
    Q_D(const QWizardPage);
    return d->title;
}

/*!
    \property QWizardPage::subTitle
    \brief the subtitle of the page

    The subtitle is shown by the QWizard, between the title and the
    actual page. Subtitles are optional. In
    \l{QWizard::ClassicStyle}{ClassicStyle} and
    \l{QWizard::ModernStyle}{ModernStyle}, using subtitles is
    necessary to make the header appear. In
    \l{QWizard::MacStyle}{MacStyle}, the subtitle is shown as a text
    label just above the actual page.

    The subtitle may be plain text or HTML, depending on the value of
    the \l{QWizard::subTitleFormat} property.

    \sa title, QWizard::IgnoreSubTitles, {Elements of a Wizard Page}
*/
void QWizardPage::setSubTitle(const QString &subTitle)
{
    Q_D(QWizardPage);
    d->subTitle = subTitle;
    if (d->wizard && d->wizard->currentPage() == this)
        d->wizard->d_func()->updateLayout();
}

QString QWizardPage::subTitle() const
{
    Q_D(const QWizardPage);
    return d->subTitle;
}

/*!
    Sets the pixmap for role \a which to \a pixmap.

    The pixmaps are used by QWizard when displaying a page. Which
    pixmaps are actually used depend on the \l{Wizard Look and
    Feel}{wizard style}.

    Pixmaps can also be set for the entire wizard using
    QWizard::setPixmap(), in which case they apply for all pages that
    don't specify a pixmap.

    \sa QWizard::setPixmap(), {Elements of a Wizard Page}
*/
void QWizardPage::setPixmap(QWizard::WizardPixmap which, const QPixmap &pixmap)
{
    Q_D(QWizardPage);
    Q_ASSERT(uint(which) < QWizard::NPixmaps);
    d->pixmaps[which] = pixmap;
    if (d->wizard && d->wizard->currentPage() == this)
        d->wizard->d_func()->updatePixmap(which);
}

/*!
    Returns the pixmap set for role \a which.

    Pixmaps can also be set for the entire wizard using
    QWizard::setPixmap(), in which case they apply for all pages that
    don't specify a pixmap.

    \sa QWizard::pixmap(), {Elements of a Wizard Page}
*/
QPixmap QWizardPage::pixmap(QWizard::WizardPixmap which) const
{
    Q_D(const QWizardPage);
    Q_ASSERT(uint(which) < QWizard::NPixmaps);

    const QPixmap &pixmap = d->pixmaps[which];
    if (!pixmap.isNull())
        return pixmap;

    if (wizard())
        return wizard()->pixmap(which);

    return pixmap;
}

/*!
    This virtual function is called by QWizard::initializePage() to
    prepare the page just before it is shown. (However, if the \l
    QWizard::IndependentPages option is set, this function is only
    called the first time the page is shown.)

    By reimplementing this function, you can ensure that the page's
    fields are properly initialized based on fields from previous
    pages. For example:

    \quotefromfile classwizard/classwizard.cpp
    \skipto OutputFilesPage::initializePage
    \printuntil }

    The default implementation does nothing.

    \sa QWizard::initializePage(), cleanupPage(), QWizard::IndependentPages
*/
void QWizardPage::initializePage()
{
}

/*!
    This virtual function is called by QWizard::cleanupPage() when
    the user clicks \gui Back (unless the QWizard::IndependentPages
    option is set).

    The default implementation resets the page's fields to their
    original values (the values they had before initializePage() was
    called).

    \sa QWizard::cleanupPage(), initializePage(), QWizard::IndependentPages
*/
void QWizardPage::cleanupPage()
{
    Q_D(QWizardPage);
    if (d->wizard) {
        QVector<QWizardField> &fields = d->wizard->d_func()->fields;
        for (int i = 0; i < fields.count(); ++i) {
            const QWizardField &field = fields.at(i);
            if (field.page == this)
                field.object->setProperty(field.property, field.initialValue);
        }
    }
}

/*!
    This virtual function is called by QWizard::validateCurrentPage()
    when the user clicks \gui Next or \gui Finish to perform some
    last-minute validation. If it returns true, the next page is shown
    (or the wizard finishes); otherwise, the current page stays up.

    The default implementation returns true.

    When possible, it is usually better style to disable the \gui
    Next or \gui Finish button (by specifying \l{mandatory fields} or
    reimplementing isComplete()) than to reimplement validatePage().

    \sa QWizard::validateCurrentPage(), isComplete()
*/
bool QWizardPage::validatePage()
{
    return true;
}

/*!
    This virtual function is called by QWizard to determine whether
    the \gui Next or \gui Finish button should be enabled or
    disabled.

    The default implementation returns true if all \l{mandatory
    fields} are filled; otherwise, it returns false.

    If you reimplement this function, make sure to emit
    completeChanged() whenever the value of isComplete() changes, to
    ensure that QWizard updates the enabled or disabled state of its
    buttons.

    \sa completeChanged(), isFinalPage()
*/
bool QWizardPage::isComplete() const
{
    Q_D(const QWizardPage);

    if (!d->wizard)
        return true;

    const QVector<QWizardField> &wizardFields = d->wizard->d_func()->fields;
    for (int i = wizardFields.count() - 1; i >= 0; --i) {
        const QWizardField &field = wizardFields.at(i);
        if (field.page == this && field.mandatory) {
            QVariant value = field.object->property(field.property);
            if (value == field.initialValue)
                return false;

            if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(field.object)) {
                if (!lineEdit->hasAcceptableInput())
                    return false;
            }
        }
    }
    return true;
}

/*!
    Explicitly sets this page to be final if \a final is true.

    After calling setFinalPage(true), isFinalPage() returns true and the \gui
    Finish button is visible (and enabled if isComplete() returns
    true).

    After calling setFinalPage(false), isFinalPage() returns true if
    nextId() returns -1; otherwise, it returns false.

    \sa isComplete(), QWizard::HaveFinishButtonOnEarlyPages
*/
void QWizardPage::setFinalPage(bool finalPage)
{
    Q_D(QWizardPage);
    d->explicitlyFinal = finalPage;
}

/*!
    This function is called by QWizard to determine whether the \gui
    Finish button should be shown for this page or not.

    By default, it returns true if there is no next page
    (i.e., nextId() returns -1); otherwise, it returns false.

    By explicitly calling setFinalPage(true), you can let the user perform an
    "early finish".

    \sa isComplete(), QWizard::HaveFinishButtonOnEarlyPages
*/
bool QWizardPage::isFinalPage() const
{
    Q_D(const QWizardPage);
    return d->explicitlyFinal || nextId() == -1;
}

/*!
    Sets whether this page is a commit page.

    A commit page is a page that represents an action which cannot be undone
    by clicking \gui Back or \gui Cancel.

    A \gui Commit button replaces the \gui Next button on a commit page. Clicking this
    button simply calls QWizard::next() just like clicking \gui Next does.

    A page entered directly from a commit page has its \gui Back button disabled.

    \sa isCommitPage()
*/
void QWizardPage::setCommitPage(bool commitPage)
{
    Q_D(QWizardPage);
    d->commit = commitPage;
}

/*!
    Returns whether this page is a commit page.

    \sa setCommitPage()
*/
bool QWizardPage::isCommitPage() const
{
    Q_D(const QWizardPage);
    return d->commit;
}

/*!
    Sets the text on button \a which to be \a text on this page.

    By default, the text on buttons depends on the QWizard::wizardStyle,
    but may be redefined for the wizard as a whole using QWizard::setButtonText().

    \sa buttonText(), QWizard::setButtonText(), QWizard::buttonText()
*/
void QWizardPage::setButtonText(QWizard::WizardButton which, const QString &text)
{
    Q_D(QWizardPage);
    d->buttonCustomTexts.insert(which, text);
    if (wizard() && wizard()->currentPage() == this && wizard()->d_func()->btns[which])
        wizard()->d_func()->btns[which]->setText(text);
}

/*!
    Returns the text on button \a which on this page.

    If a text has ben set using setButtonText(), this text is returned.
    Otherwise, if a text has been set using QWizard::setButtonText(),
    this text is returned.

    By default, the text on buttons depends on the QWizard::wizardStyle.
    For example, on Mac OS X, the \gui Next button is called \gui
    Continue.

    \sa setButtonText(), QWizard::buttonText(), QWizard::setButtonText()
*/
QString QWizardPage::buttonText(QWizard::WizardButton which) const
{
    Q_D(const QWizardPage);

    if (d->buttonCustomTexts.contains(which))
        return d->buttonCustomTexts.value(which);

    if (wizard())
        return wizard()->buttonText(which);

    return QString();
}

/*!
    This virtual function is called by QWizard::nextId() to find
    out which page to show when the user clicks the \gui Next button.

    By default, this function returns the page with the following ID
    in the QWizard, or -1 if there is no such page.

    By reimplementing this function, you can specify a dynamic page
    order. For example:

    \quotefromfile licensewizard/licensewizard.cpp
    \skipto IntroPage::nextId
    \printuntil /^\}/

    \sa QWizard::nextId()
*/
int QWizardPage::nextId() const
{
    Q_D(const QWizardPage);

    if (!d->wizard)
        return -1;

    bool foundCurrentPage = false;

    const QWizardPrivate::PageMap &pageMap = d->wizard->d_func()->pageMap;
    QWizardPrivate::PageMap::const_iterator i = pageMap.constBegin();
    QWizardPrivate::PageMap::const_iterator end = pageMap.constEnd();

    for (; i != end; ++i) {
        if (i.value() == this) {
            foundCurrentPage = true;
        } else if (foundCurrentPage) {
            return i.key();
        }
    }
    return -1;
}

/*!
    \fn void QWizardPage::completeChanged()

    This signal is emitted whenever the complete state of the page
    (i.e., the value of isComplete()) changes.

    If you reimplement isComplete(), make sure to emit
    completeChanged() whenever the value of isComplete() changes, to
    ensure that QWizard updates the enabled or disabled state of its
    buttons.

    \sa isComplete()
*/

/*!
    Sets the value of the field called \a name to \a value.

    This function can be used to set fields on any page of the wizard.
    It is equivalent to calling
    wizard()->\l{QWizard::setField()}{setField(\a name, \a value)}.

    \sa QWizard::setField(), field(), registerField()
*/
void QWizardPage::setField(const QString &name, const QVariant &value)
{
    Q_D(QWizardPage);
    if (!d->wizard)
        return;
    d->wizard->setField(name, value);
}

/*!
    Returns the value of the field called \a name.

    This function can be used to access fields on any page of the
    wizard. It is equivalent to calling
    wizard()->\l{QWizard::field()}{field(\a name)}.

    Example:

    \quotefromfile classwizard/classwizard.cpp
    \skipto OutputFilesPage::initializePage
    \printuntil }

    \sa QWizard::field(), setField(), registerField()
*/
QVariant QWizardPage::field(const QString &name) const
{
    Q_D(const QWizardPage);
    if (!d->wizard)
        return QVariant();
    return d->wizard->field(name);
}

/*!
    Creates a field called \a name associated with the given \a
    property of the given \a widget. From then on, that property
    becomes accessible using field() and setField().

    Fields are global to the entire wizard and make it easy for any
    single page to access information stored by another page, without
    having to put all the logic in QWizard or having the pages know
    explicitly about each other.

    If \a name ends with an asterisk (\c *), the field is a mandatory
    field. When a page has mandatory fields, the \gui Next and/or
    \gui Finish buttons are enabled only when all mandatory fields
    are filled. This requires a \a changedSignal to be specified, to
    tell QWizard to recheck the value stored by the mandatory field.

    QWizard knows the most common Qt widgets. For these (or their
    subclasses), you don't need to specify a \a property or a \a
    changedSignal. The table below lists these widgets:

    \table
    \header \o Widget          \o Property                            \o Change Notification Signal
    \row    \o QAbstractButton \o bool \l{QAbstractButton::}{checked} \o \l{QAbstractButton::}{toggled()}
    \row    \o QAbstractSlider \o int \l{QAbstractSlider::}{value}    \o \l{QAbstractSlider::}{valueChanged()}
    \row    \o QComboBox       \o int \l{QComboBox::}{currentIndex}   \o \l{QComboBox::}{currentIndexChanged()}
    \row    \o QDateTimeEdit   \o QDateTime \l{QDateTimeEdit::}{dateTime} \o \l{QDateTimeEdit::}{dateTimeChanged()}
    \row    \o QLineEdit       \o QString \l{QLineEdit::}{text}       \o \l{QLineEdit::}{textChanged()}
    \row    \o QListWidget     \o int \l{QListWidget::}{currentRow}   \o \l{QListWidget::}{currentRowChanged()}
    \row    \o QSpinBox        \o int \l{QSpinBox::}{value}           \o \l{QSpinBox::}{valueChanged()}
    \endtable

    You can use QWizard::setDefaultProperty() to add entries to this
    table or to override existing entries.

    To consider a field "filled", QWizard simply checks that their
    current value doesn't equal their original value (the value they
    had before initializePage() was called). For QLineEdit, it also
    checks that
    \l{QLineEdit::hasAcceptableInput()}{hasAcceptableInput()} returns
    true, to honor any validator or mask.

    QWizard's mandatory field mechanism is provided for convenience.
    It can be bypassed by reimplementing QWizardPage::isComplete().

    \sa field(), setField(), QWizard::setDefaultProperty()
*/
void QWizardPage::registerField(const QString &name, QWidget *widget, const char *property,
                                const char *changedSignal)
{
    Q_D(QWizardPage);
    QWizardField field(this, name, widget, property, changedSignal);
    if (d->wizard) {
        d->wizard->d_func()->addField(field);
    } else {
        d->pendingFields += field;
    }
}

/*!
    Returns the wizard associated with this page, or 0 if this page
    hasn't been inserted into a QWizard yet.

    \sa QWizard::addPage(), QWizard::setPage()
*/
QWizard *QWizardPage::wizard() const
{
    Q_D(const QWizardPage);
    return d->wizard;
}

#include "moc_qwizard.cpp"
