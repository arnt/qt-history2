/****************************************************************************
**
** Implementation of the QStyleOption structure.
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

#include "qstyleoption.h"

/*!
    \class QStyleOption qstyleoption.h
    \brief QStyleOption and its sub-classes describe parameters that are used
    by QStyle functions.
    \ingroup appearance

    In previous Qt versions, many QStyle functions required a QWidget parameter
    of a certain type in order for it to work correctly. This coupled the
    QStyle closely to the widgets it was styling. In Qt 4.0, this restriction
    has been removed.  Instead, a QStyleOption or one of its sub-classes supply
    the information the function needs.

    QStyleOption and its sub-classes all have a version parameter. The current
    version is 0.  This version number must be specified explicitly when
    creating the structure.  The version number helps to ensure future
    expansion without ruining compatiblity with older styles.

    Contrary to the rest of Qt, there are few member functions and the access
    to the variables is direct. The "low-level" feel makes the structures use
    straight forward and empahsizes that these are parameters used by the style
    functions, not clases. As a downside it forces developers to be careful to
    initialize all the variables.

    QStyleOption and its sub-classes can be safely checked using the qt_cast()
    functions.
*/

/*!
  \enum QStyleOption::OptionType

  This enum is used internally by the QStyleOption, its sub-classes and
  qt_cast() to determine the type of style option. In general you do not need
  to worry about this unless you want to create your own QStyleOption
  sub-class.

  \value SO_Default Indicates a default QStyleOption
  \value SO_FocusRect Indicates a \l QStyleOptionFocusRect
  \value SO_Button Indicates a \l QStyleOptionButton
  \value SO_Tab Indicates a \l QStyleOptionTab
  \value SO_MenuItem Indicates a \l QStyleOptionMenuItem
  \value SO_Complex Indicates a \l QStyleOptionComplex
  \value SO_Slider Indicates a \l QStyleOptionSlider
  \value SO_Frame Indicates a \l QStyleOptionFrame
  \value SO_ProgressBar Indicates a \l QStyleOptionProgressBar
  \value SO_ListView Indicates a \l QStyleOptionListView
  \value SO_ListViewItem Indicates a \l QStyleOptionListViewItem
  \value SO_Header Indicates a \l QStyleOptionHeader
  \value SO_DockWindow Indicates a \l QStyleOptionDockWindow
  \value SO_SpinBox Indicates a \l QStyleOptionSpinBox
  \value SO_ToolButton Indicates a \l QStyleOptionToolButton
  \value SO_ComboBox Indicates a \l QStyleOptionComboBox
  \value SO_ToolBox Indicates a \l QStyleOptionToolBox
  \value SO_TitleBar Indicates a \l QStyleOptionTitleBar
  \value SO_ViewItem Indicates a \l QStyleOptionViewItem (used in Interviews)
  \value SO_CustomBase This is reserved for custom QStyleOptions,
                        all custom values must be above this value.

*/
/*!
  Construct a QStyleOption with version \a optionversion and type \a optiontype.
  Usually, you will only pass \a optionversion. The \a optiontype parameter is
  mainly used by the subclasses for RTTI information.
*/

QStyleOption::QStyleOption(int optionversion, int optiontype)
    : version(optionversion), type(optiontype), state(QStyle::Style_Default)
{
}

/*!
  A convenience function that intializes some of the variables in the
  QStyleOption structure based on the widget \a w.

  The init() function assigns the rect and palette variables to \a w's rect()
  and palette() respectively. It also initializes the state variable checking
  if the widget is enabled and if it has focus.
*/
void QStyleOption::init(const QWidget *w)
{
    state = QStyle::Style_Default;
    if (w->isEnabled())
        state |= QStyle::Style_Enabled;
    if (w->hasFocus())
        state |= QStyle::Style_HasFocus;
    rect = w->rect();
    palette = w->palette();
}

/*!
    \property QStyleOption::palette

    \brief The palette that should be used in when painting the control
*/

/*!
    \property QStyleOption::rect

    \brief The area that should be used for various calculations and painting.

    This can have overloaded meanings. For example, for \l CE_PushButton it would
    be the rectangle for the entire button, while for \l CE_PushButtonLabel it
    would be just the area for the label.
*/

/*!
    \property QStyleOption::state

    \brief The QStyle::StyleFlags that are used when drawing the control.

    Several flags can be OR'd together. See drawControl and drawPrimitive for
    further information.

    \sa QStyle::drawPrimitive() QStyle::drawControl() QStyle::drawComplexControl() QStyle::StyleFlags
*/

/*!
    \property QStyleOption::type

    \brief The \l OptionType of the QStyleOption
*/

/*!
    \property QStyleOption::version

    \brief The version of the QStyleOption

    The current default is 0.
*/

/*
    \class QStyleOptionFocusRect qstyleoption.h
    \brief QStyleOptionFocusRect describes the parameters for drawing a focus
    rect with QStyle.
    \ingroup appearance
*/

/*!
  \fn QStyleOptionFocusRect::QStyleOptionFocusRect(int version)

  Construct a QStyleOptionFocusRect with version number \a version
*/

/*!
    \property QStyleOptionFocusRect::backgroundColor

    \brief The background color on which the focus rect is being drawn.
*/
