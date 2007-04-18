
AccessibleSlider::QAccessibleSlider(QWidget *w)
: QAccessibleAbstractSlider(w)
{
    Q_ASSERT(slider());
    addControllingSignal(QLatin1String("valueChanged(int)"));
}

/*! Returns the slider. */
QSlider *QAccessibleSlider::slider() const
{
    return qobject_cast<QSlider*>(object());
}

/*! \reimp */
QRect QAccessibleSlider::rect(int child) const
{
    QRect rect;
    if (!slider()->isVisible())
        return rect;
    const QStyleOptionSlider option = qt_qsliderStyleOption(slider());
    QRect srect = slider()->style()->subControlRect(QStyle::CC_Slider, &option,
                                                    QStyle::SC_SliderHandle, slider());

    switch (child) {
    case PageLeft:
        if (slider()->orientation() == Qt::Vertical)
            rect = QRect(0, 0, slider()->width(), srect.y());
        else
            rect = QRect(0, 0, srect.x(), slider()->height());
        break;
    case Position:
        rect = srect;
        break;
    case PageRight:
        if (slider()->orientation() == Qt::Vertical)
            rect = QRect(0, srect.y() + srect.height(), slider()->width(), slider()->height()- srect.y() - srect.height());
        else
            rect = QRect(srect.x() + srect.width(), 0, slider()->width() - srect.x() - srect.width(), slider()->height());
        break;
    default:
        return QAccessibleAbstractSlider::rect(child);
    }

    QPoint tp = slider()->mapToGlobal(QPoint(0,0));
    return QRect(tp.x() + rect.x(), tp.y() + rect.y(), rect.width(), rect.height());
}

/*! \reimp */
int QAccessibleSlider::childCount() const
{
    if (!slider()->isVisible())
        return 0;
    return PageRight;
}

/*! \reimp */
QString QAccessibleSlider::text(Text t, int child) const
{
    if (!slider()->isVisible())
        return QString();
    switch (t) {
    case Value:
        if (!child || child == 2)
            return QString::number(slider()->value());
        return QString();
    case Name:
        switch (child) {
        case PageLeft:
            return slider()->orientation() == Qt::Horizontal ?
                QSlider::tr("Page left") : QSlider::tr("Page up");
        case Position:
            return QSlider::tr("Position");
        case PageRight:
            return slider()->orientation() == Qt::Horizontal ?
                QSlider::tr("Page right") : QSlider::tr("Page down");
        }
        break;
    default:
        break;
    }
    return QAccessibleAbstractSlider::text(t, child);
}

/*! \reimp */
QAccessible::Role QAccessibleSlider::role(int child) const
{
    switch (child) {
    case PageLeft:
    case PageRight:
        return PushButton;
    case Position:
        return Indicator;
    default:
        return Slider;
    }
}

/*! \reimp */
QAccessible::State QAccessibleSlider::state(int child) const
{
    const State parentState = QAccessibleAbstractSlider::state(0);

    if (child == 0)
        return parentState;

    // Inherit the Invisible state from parent.
    State state = parentState & QAccessible::Invisible;

    // Disable left/right if we are at the minimum/maximum.
    const QSlider * const slider = QAccessibleSlider::slider();
    switch (child) {
    case PageLeft:
        if (slider->value() <= slider->minimum())
            state |= Unavailable;
        break;
    case PageRight:
        if (slider->value() >= slider->maximum())
            state |= Unavailable;
        break;
    case Position:
    default:
        break;
    }

    return state;
}

/*!
    \fn int QAccessibleSlider::defaultAction(int child) const

    Returns the default action for the given \a child. The base class
    implementation returns 0.
*/
int QAccessibleSlider::defaultAction(int child) const
{
    switch (child) {
        case SliderSelf:
            return SetFocus;
        case PageLeft:
            return Press;
        case PageRight:
            return Press;
    }

    return 0;
}

// Name, Description, Value, Help, Accelerator
static const char * const actionTexts[][5] =
{
    {"Press", "Decreases the value of the slider", "", "", "Ctrl+L"},
    {"Press", "Increaces the value of the slider", "", "", "Ctrl+R"}
};

QString QAccessibleSlider::actionText(int action, Text text, int child) const
{
    if (action != Press || child < 1 || child > 2)
        return QAccessibleAbstractSlider::actionText(action, text, child);        

    return actionTexts[child - 1][t];
}

bool QAccessibleSlider::doAction(int action, int child)
{
    if (action != Press || child < 1 || child > 2)
        return false;
    
    if (child == PageLeft)
        slider()->setValue(slider()->value() - slider()->pageStep());
    else
        slider()->setValue(slider()->value() + slider()->pageStep());
}

QAccessibleAbstractSlider::QAccessibleAbstractSlider(QWidget *w, Role r)
    : QAccessibleWidgetEx(w, r)
{
    Q_ASSERT(qobject_cast<QAbstractSlider *>(w));
}

QVariant QAccessibleAbstractSlider::invokeMethodEx(Method method, int child, const QVariantList &params)
{
    switch (method) {
    case ListSupportedMethods: {
        QSet<QAccessible::Method> set;
        set << ListSupportedMethods;
        return qVariantFromValue(set | qvariant_cast<QSet<QAccessible::Method> >(
                    QAccessibleWidgetEx::invokeMethodEx(method, child, params)));
    }
    default:
        return QAccessibleWidgetEx::invokeMethodEx(method, child, params);
    }
}

QVariant QAccessibleAbstractSlider::currentValue()
{
    return abstractSlider()->value();
}

void QAccessibleAbstractSlider::setCurrentValue(const QVariant &value)
{
    abstractSlider()->setValue(value.toInt());
}

QVariant QAccessibleAbstractSlider::maximumValue()
{
    return abstractSlider()->maximum();
}

QVariant QAccessibleAbstractSlider::minimumValue()
{
    return abstractSlider()->minimum();
}

QAbstractSlider *QAccessibleAbstractSlider::abstractSlider() const
{
    return static_cast<QAbstractSlider *>(object());
}
