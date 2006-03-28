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

#include "qsvghandler_p.h"

#include "qsvgtinydocument_p.h"
#include "qsvgstructure_p.h"
#include "qsvggraphics_p.h"
#include "qsvgnode_p.h"
#include "qsvgfont_p.h"

#include "qapplication.h"
#include "qwidget.h"
#include "qpen.h"
#include "qpainterpath.h"
#include "qbrush.h"
#include "qcolor.h"
#include "qtextformat.h"
#include "qvector.h"
#include "qdebug.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


static bool parsePathDataFast(const QString &data, QPainterPath &path);

static QPen defaultPen(Qt::black, 1, Qt::NoPen,
                       Qt::FlatCap, Qt::SvgMiterJoin);

static QString xmlSimplify(const QString &str)
{
    QString dummy = str;
    dummy.remove('\n');
    if (dummy.trimmed().isEmpty())
        return QString();
    QString temp;
    QString::const_iterator itr = dummy.constBegin();
    bool wasSpace = false;
    for (;itr != dummy.constEnd(); ++itr) {
        if ((*itr).isSpace()) {
            if (wasSpace || !(*itr).isPrint()) {
                continue;
            }
            temp += *itr;
            wasSpace = true;
        } else {
            temp += *itr;
            wasSpace = false;
        }
    }
    return temp;
}

static QList<qreal> parseNumbersList(QString::const_iterator &itr)
{
    QList<qreal> points;
    QString temp;
    while ((*itr).isSpace())
        ++itr;
    while ((*itr).isNumber() ||
           (*itr) == '-' || (*itr) == '+' || (*itr) == '.') {
        temp = QString();

        if ((*itr) == '-')
            temp += *itr++;
        else if ((*itr) == '+')
            temp += *itr++;
        while ((*itr).isDigit())
            temp += *itr++;
        if ((*itr) == '.')
            temp += *itr++;
        while ((*itr).isDigit())
            temp += *itr++;
        if (( *itr) == 'e') {
            temp += *itr++;
            if ((*itr) == '-' ||
                (*itr) == '+')
                temp += *itr++;
        }
        while ((*itr).isDigit())
            temp += *itr++;
        while ((*itr).isSpace())
            ++itr;
        if ((*itr) == ',')
            ++itr;
        points.append(temp.toDouble());
        //eat the rest of space
        while ((*itr).isSpace())
            ++itr;
    }

    return points;
}

static QList<qreal> parsePercentageList(QString::const_iterator &itr)
{
    QList<qreal> points;
    QString temp;
    while ((*itr).isSpace())
        ++itr;
    while ((*itr).isNumber() ||
           (*itr) == '-' || (*itr) == '+') {
        temp = QString();

        if ((*itr) == '-')
            temp += *itr++;
        else if ((*itr) == '+')
            temp += *itr++;
        while ((*itr).isDigit())
            temp += *itr++;
        if ((*itr) == '.')
            temp += *itr++;
        while ((*itr).isDigit())
            temp += *itr++;
        if (( *itr) == '%') {
            itr++;
        }
        while ((*itr).isSpace())
            ++itr;
        if ((*itr) == ',')
            ++itr;
        points.append(temp.toDouble());
        //eat the rest of space
        while ((*itr).isSpace())
            ++itr;
    }

    return points;
}

static QString idFromUrl(const QString &url)
{
    QString::const_iterator itr = url.constBegin();
    while ((*itr).isSpace())
        ++itr;
    if ((*itr) == '(')
        ++itr;
    while ((*itr).isSpace())
        ++itr;
    if ((*itr) == '#')
        ++itr;
    QString id;
    while ((*itr) != ')') {
        id += *itr;
        ++itr;
    }
    return id;
}

/**
 * returns true when successfuly set the color. false signifies
 * that the color should be inherited
 */
static bool resolveColor(const QString &colorStr, QColor &color, QSvgHandler *handler)
{
    static QHash<QString, QColor> colors;
    QString colorStrTr = colorStr.trimmed();
    if (colors.isEmpty()) {
        colors.insert("black",   QColor(  0,   0,   0));
        colors.insert("green",   QColor(  0, 128,   0));
        colors.insert("silver",  QColor(192, 192, 192));
        colors.insert("lime",    QColor(  0, 255,   0));
        colors.insert("gray",    QColor(128, 128, 128));
        colors.insert("olive",   QColor(128, 128,   0));
        colors.insert("white",   QColor(255, 255, 255));
        colors.insert("yellow",  QColor(255, 255,   0));
        colors.insert("maroon",  QColor(128,   0,   0));
        colors.insert("navy",    QColor(  0,   0, 128));
        colors.insert("red",     QColor(255,   0,   0));
        colors.insert("blue",    QColor(  0,   0, 255));
        colors.insert("purple",  QColor(128,   0, 128));
        colors.insert("teal",    QColor(  0, 128, 128));
        colors.insert("fuchsia", QColor(255,   0, 255));
        colors.insert("aqua",    QColor(  0, 255, 255));
    }
    if (colors.contains(colorStrTr)) {
        color = colors[colorStrTr];
        return color.isValid();
    } else if (colorStr.startsWith("rgb(")) {
        QString::const_iterator itr = colorStr.constBegin();
        ++itr; ++itr; ++itr; ++itr;
        QString::const_iterator itr_back = itr;
        QList<qreal> compo = parseNumbersList(itr);
        //1 means that it failed after reaching non-parsable
        //character which is going to be "%"
        if (compo.size() == 1) {
            itr = itr_back;
            compo = parsePercentageList(itr);
            compo[0] *= 2.55;
            compo[1] *= 2.55;
            compo[2] *= 2.55;
        }

        color = QColor(int(compo[0]),
                       int(compo[1]),
                       int(compo[2]));
        return true;
    } else if (colorStr == QLatin1String("inherited") ||
               colorStr == QLatin1String("inherit"))  {
        return false;
    } else if (colorStr == QLatin1String("currentColor")) {
        color = handler->currentColor();
        return true;
    }

    color = QColor(colorStrTr);
    return color.isValid();
}

static bool constructColor(const QString &colorStr, const QString &opacity,
                           QColor &color, QSvgHandler *handler)
{
    if (!resolveColor(colorStr, color, handler))
        return false;
    if (!opacity.isEmpty()) {
        qreal op = opacity.toDouble();
        if (op <= 1)
            op *= 255;
        color.setAlpha(int(op));
    }
    return true;
}

static qreal parseLength(const QString &str, QSvgHandler::LengthType &type,
                         QSvgHandler *handler)
{
    QString numStr = str.trimmed();
    qreal len = 0;

    if (numStr.endsWith("%")) {
        numStr.chop(1);
        len = numStr.toDouble();
        type = QSvgHandler::PERCENT;
    } else if (numStr.endsWith("px")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = QSvgHandler::PX;
    } else if (numStr.endsWith("pc")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = QSvgHandler::PC;
    } else if (numStr.endsWith("pt")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = QSvgHandler::PT;
    } else if (numStr.endsWith("mm")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = QSvgHandler::MM;
    } else if (numStr.endsWith("cm")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = QSvgHandler::CM;
    } else if (numStr.endsWith("in")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = QSvgHandler::IN;
    } else {
        len = numStr.toDouble();
        type = handler->defaultCoordinateSystem();
        //type = QSvgHandler::OTHER;
    }
    //qDebug()<<"len is "<<len<<", from "<<numStr;
    return len;
}


static bool createSvgGlyph(QSvgFont *font, const QXmlAttributes &attributes)
{
    QString uncStr = attributes.value("unicode");
    QString havStr = attributes.value("horiz-adv-x");
    QString pathStr = attributes.value("d");

    QChar unicode = (uncStr.isEmpty())?0:uncStr.at(0);
    qreal havx = (havStr.isEmpty())?-1:havStr.toDouble();
    QPainterPath path;
    parsePathDataFast(pathStr, path);

    font->addGlyph(unicode, path, havx);

    return true;
}

// this should really be called convertToDefaultCoordinateSystem
// and convert when type != QSvgHandler::defaultCoordinateSystem
static qreal convertToPixels(qreal len, bool isX, QSvgHandler::LengthType type)
{
    QWidgetList widgets = QApplication::topLevelWidgets();
    QWidget *sampleWidget = widgets.isEmpty() ? 0 : widgets.first();

    if (!sampleWidget) {
        qWarning("can't produce matrics without some widget");
        return len;
    }

    qreal dpi    = isX ? sampleWidget->logicalDpiX() : sampleWidget->logicalDpiY();
    qreal mm     = isX ? sampleWidget->widthMM() : sampleWidget->heightMM();
    qreal screen = isX ? sampleWidget->width() : sampleWidget->height();

    switch (type) {
    case QSvgHandler::PERCENT:
        break;
    case QSvgHandler::PX:
        break;
    case QSvgHandler::PC:
        break;
    case QSvgHandler::PT:
        //### inkscape exports it as inkscape:export-[x,y]dpi
        len *= (90/72.0);
        break;
    case QSvgHandler::MM:
        len *= screen/mm;
        break;
    case QSvgHandler::CM:
        len *= dpi/2.54;
        break;
    case QSvgHandler::IN:
        len *= dpi;
        break;
    case QSvgHandler::OTHER:
        break;
    default:
        break;
    }
    return len;
}

static void parseColor(QSvgNode *,
                       const QXmlAttributes &attributes,
                       QSvgHandler *handler)
{
    QString colorStr = attributes.value("color");
    QString opacity  = attributes.value("color-opacity");
    QColor color;
    if (constructColor(colorStr, opacity, color, handler)) {
        handler->pushColor(color);
    }
}

static void parseBrush(QSvgNode *node,
                       const QXmlAttributes &attributes,
                       QSvgHandler *handler)
{
    QString value = attributes.value("fill");
    QString myId = attributes.value("id");
    value = value.trimmed();
    if (!value.isEmpty()) {
        if (value.startsWith("url")) {
            value = value.remove(0, 3);
            QString id = idFromUrl(value);
            QSvgStructureNode *group = 0;
            QSvgNode *dummy = node;
            while (dummy && (dummy->type() != QSvgNode::DOC  &&
                             dummy->type() != QSvgNode::G    &&
                             dummy->type() != QSvgNode::DEFS &&
                             dummy->type() != QSvgNode::SWITCH)) {
                dummy = dummy->parent();
            }
            if (dummy)
                group = static_cast<QSvgStructureNode*>(dummy);
            if (group) {
                QSvgStyleProperty *style =
                    group->scopeStyle(id);
                if (style)
                    node->appendStyleProperty(style,
                                              attributes.value("id"));
                else {
                    qWarning("Couldn't resolve property: %s", qPrintable(id));
                }
            }
        } else if (value != QLatin1String("none")) {
            QString opacity = attributes.value("fill-opacity");
            QString fillRule = attributes.value("fill-rule");

            if (opacity.isEmpty())
                opacity = attributes.value("opacity");
            QColor color;
            if (constructColor(value, opacity, color, handler)) {
                QSvgStyleProperty *prop = new QSvgFillStyle(QBrush(color));
                node->appendStyleProperty(prop, myId);
            }
        } else {
            QSvgStyleProperty *prop = new QSvgFillStyle(QBrush(Qt::NoBrush));
            node->appendStyleProperty(prop, myId);
        }
    }
}


static void parseQPen(QPen &pen, QSvgNode *node,
                      const QXmlAttributes &attributes,
                      QSvgHandler *handler)
{
    QString value = attributes.value("stroke");
    QString dashArray  = attributes.value("stroke-dasharray");
    QString dashOffset = attributes.value("stroke-dashoffset");
    QString linecap    = attributes.value("stroke-linecap");
    QString linejoin   = attributes.value("stroke-linejoin");
    QString miterlimit = attributes.value("stroke-miterlimit");
    QString opacity    = attributes.value("stroke-opacity");
    QString width      = attributes.value("stroke-width");
    QString myId       = attributes.value("id");

    if (opacity.isEmpty())
        opacity = attributes.value("opacity");

    if (!value.isEmpty() || !width.isEmpty()) {
        if (value != QLatin1String("none")) {
            if (!value.isEmpty()) {
                if (node && value.startsWith("url")) {
                    value = value.remove(0, 3);
                    QString id = idFromUrl(value);
                    QSvgStructureNode *group = 0;
                    QSvgNode *dummy = node;
                    while (dummy && (dummy->type() != QSvgNode::DOC  &&
                                     dummy->type() != QSvgNode::G    &&
                                     dummy->type() != QSvgNode::DEFS &&
                                     dummy->type() != QSvgNode::SWITCH)) {
                        dummy = dummy->parent();
                    }
                    if (dummy)
                        group = static_cast<QSvgStructureNode*>(dummy);
                    if (group) {
                        QSvgStyleProperty *style =
                            group->scopeStyle(id);
                        if (style->type() == QSvgStyleProperty::GRADIENT) {
                            QBrush b(*((QSvgGradientStyle*)style)->qgradient());
                            pen.setBrush(b);
                        } else if (style->type() == QSvgStyleProperty::SOLID_COLOR) {
                            pen.setColor(
                                ((QSvgSolidColorStyle*)style)->qcolor());
                        }
                    } else {
                        qDebug()<<"QSvgHandler::parsePen no parent group?";
                    }
                } else {
                    QColor color;
                    if (constructColor(value, opacity, color, handler))
                        pen.setColor(color);
                }
                //since we could inherit stroke="none"
                //we need to reset the style of our stroke to something
                pen.setStyle(Qt::SolidLine);
            }
            if (!width.isEmpty()) {
                QSvgHandler::LengthType lt;
                qreal widthF = parseLength(width, lt, handler);
                //### fixme
                if (!widthF) {
                    pen.setStyle(Qt::NoPen);
                    return;
                }
                pen.setWidthF(widthF);
            }
            qreal penw = pen.widthF();

            if (!linejoin.isEmpty()) {
                if (linejoin == "miter")
                    pen.setJoinStyle(Qt::SvgMiterJoin);
                else if (linejoin == "round")
                    pen.setJoinStyle(Qt::RoundJoin);
                else if (linejoin == "bevel")
                    pen.setJoinStyle(Qt::BevelJoin);
            }
            if (!miterlimit.isEmpty()) {
                pen.setMiterLimit(miterlimit.toDouble());
            }

            if (!linecap.isEmpty()) {
                if (linecap == "butt")
                    pen.setCapStyle(Qt::FlatCap);
                else if (linecap == "round")
                    pen.setCapStyle(Qt::RoundCap);
                else if (linecap == "square")
                    pen.setCapStyle(Qt::SquareCap);
            }

            if (!dashArray.isEmpty()) {
                QString::const_iterator itr = dashArray.constBegin();
                QList<qreal> dashes = parseNumbersList(itr);
                QVector<qreal> vec(dashes.size());

                int i = 0;
                foreach(qreal dash, dashes) {
                    vec[i++] = dash/penw;
                }
                pen.setDashPattern(vec);
            }

        } else {
            pen.setStyle(Qt::NoPen);
        }
    }
}

static QMatrix parseTransformationMatrix(const QString &value)
{
    QMatrix matrix;
    QString::const_iterator itr = value.constBegin();

    while (itr != value.constEnd()) {
        if ((*itr) == 'm') {  //matrix
            QString temp("m");
            int remains = 6;
            while (remains--) {
                temp += *itr++;
            }

            while ((*itr).isSpace())
                ++itr;
            ++itr;// '('
            QList<qreal> points = parseNumbersList(itr);
            ++itr; // ')'

            Q_ASSERT(points.count() == 6);
            matrix = matrix * QMatrix(points[0], points[1],
                                      points[2], points[3],
                                      points[4], points[5]);

            //qDebug()<<"matrix is "<<temp;
        } else if ((*itr) == 't') { //translate
            QString trans;
            int remains = 9;
            while (remains--) {
                trans += *itr++;
            }
            while ((*itr).isSpace())
                ++itr;
            ++itr;// '('
            QList<qreal> points = parseNumbersList(itr);
            ++itr; // ')'

            Q_ASSERT(points.count() == 2 ||
                     points.count() == 1);
            if (points.count() == 2)
                matrix.translate(points[0], points[1]);
            else
                matrix.translate(points[0], 0);

            //qDebug()<<"trans is "<<points;
        } else if ((*itr) == 'r') { //rotate
            QString rot;
            int remains = 6;
            while (remains--) {
                rot += *itr++;
            }
            while ((*itr).isSpace())
                ++itr;

            ++itr;// '('
            QList<qreal> points = parseNumbersList(itr);
            ++itr;// ')'
            Q_ASSERT(points.count() == 3 ||
                     points.count() == 1);
            if (points.count() == 3) {
                matrix.translate(points[1], points[2]);
                matrix.rotate(points[0]);
                matrix.translate(-points[1], -points[2]);
            }
            else
                matrix.rotate(points[0]);

            //qDebug()<<"rot is "<<points;
        } else if ((*itr) == 's') { //scale | skewX | skewY
            QString temp;
            int remains = 5;
            while (remains--) {
                temp += *itr++;
            }
            while ((*itr).isSpace())
                ++itr;

            ++itr;// '('
            QList<qreal> points = parseNumbersList(itr);
            ++itr;// ')'
            Q_ASSERT(points.count() == 2 ||
                     points.count() == 1);
            if (temp == QLatin1String("scale")) {
                if (points.count() == 2) {
                    matrix.scale(points[0], points[1]);
                }
                else
                    matrix.scale(points[0], points[0]);
            } else if (temp == QLatin1String("skewX")) {
                const qreal deg2rad = qreal(0.017453292519943295769);
                matrix.shear(tan(points[0]*deg2rad), 0);
            } else if (temp == QLatin1String("skewY")) {
                const qreal deg2rad = qreal(0.017453292519943295769);
                matrix.shear(0, tan(points[0]*deg2rad));
            }
        } else if ((*itr) == ' '  ||
                   (*itr) == '\t' ||
                   (*itr) == '\n') {
            ++itr;
        }
        if (itr != value.constEnd())
            ++itr;
    }
    return matrix;
}

static void parsePen(QSvgNode *node,
                     const QXmlAttributes &attributes,
                     QSvgHandler *handler)
{
    QString value = attributes.value("stroke");
    QString dashArray  = attributes.value("stroke-dasharray");
    QString dashOffset = attributes.value("stroke-dashoffset");
    QString linecap    = attributes.value("stroke-linecap");
    QString linejoin   = attributes.value("stroke-linejoin");
    QString miterlimit = attributes.value("stroke-miterlimit");
    QString opacity    = attributes.value("stroke-opacity");
    QString width      = attributes.value("stroke-width");
    QString myId       = attributes.value("id");

    if (opacity.isEmpty())
        opacity = attributes.value("opacity");

    if (!value.isEmpty() || !width.isEmpty() || !linecap.isEmpty() ||
        linejoin.isEmpty()) {
        if (value != QLatin1String("none")) {
            QSvgStrokeStyle *inherited =
                static_cast<QSvgStrokeStyle*>(node->parent()->styleProperty(
                                                  QSvgStyleProperty::STROKE));
            QPen pen(defaultPen);
            if (inherited)
                pen = inherited->qpen();

            if (!value.isEmpty()) {
                if (value.startsWith("url")) {
                    value = value.remove(0, 3);
                    QString id = idFromUrl(value);
                    QSvgStructureNode *group = 0;
                    QSvgNode *dummy = node;
                    while (dummy && (dummy->type() != QSvgNode::DOC  &&
                                     dummy->type() != QSvgNode::G    &&
                                     dummy->type() != QSvgNode::DEFS &&
                                     dummy->type() != QSvgNode::SWITCH)) {
                        dummy = dummy->parent();
                    }
                    if (dummy)
                        group = static_cast<QSvgStructureNode*>(dummy);
                    if (group) {
                        QSvgStyleProperty *style =
                            group->scopeStyle(id);
                        if (style->type() == QSvgStyleProperty::GRADIENT) {
                            QBrush b(*((QSvgGradientStyle*)style)->qgradient());
                            pen.setBrush(b);
                        } else if (style->type() == QSvgStyleProperty::GRADIENT) {
                            pen.setColor(
                                ((QSvgSolidColorStyle*)style)->qcolor());
                        }
                    } else {
                        qDebug()<<"QSvgHandler::parsePen no parent group?";
                    }
                } else {
                    QColor color;
                    if (constructColor(value, opacity, color, handler))
                        pen.setColor(color);
                }
                //since we could inherit stroke="none"
                //we need to reset the style of our stroke to something
                pen.setStyle(Qt::SolidLine);
            }
            if (!width.isEmpty()) {
                QSvgHandler::LengthType lt;
                qreal widthF = parseLength(width, lt, handler);
                //### fixme
                if (!widthF) {
                    pen.setStyle(Qt::NoPen);
                    return;
                }
                pen.setWidthF(widthF);
            }

            if (!linejoin.isEmpty()) {
                if (linejoin == "miter")
                    pen.setJoinStyle(Qt::SvgMiterJoin);
                else if (linejoin == "round")
                    pen.setJoinStyle(Qt::RoundJoin);
                else if (linejoin == "bevel")
                    pen.setJoinStyle(Qt::BevelJoin);
            }

            if (!linecap.isEmpty()) {
                if (linecap == "butt")
                    pen.setCapStyle(Qt::FlatCap);
                else if (linecap == "round")
                    pen.setCapStyle(Qt::RoundCap);
                else if (linecap == "square")
                    pen.setCapStyle(Qt::SquareCap);
            }

            qreal penw = pen.widthF();
            if (!dashArray.isEmpty()) {
                QString::const_iterator itr = dashArray.constBegin();
                QList<qreal> dashes = parseNumbersList(itr);
                QVector<qreal> vec(dashes.size());

                int i = 0;
                foreach(qreal dash, dashes) {
                    vec[i++] = dash/penw;
                }

                pen.setDashPattern(vec);
            }
            if (!miterlimit.isEmpty()) {
                pen.setMiterLimit(miterlimit.toDouble());
            }

            node->appendStyleProperty(new QSvgStrokeStyle(pen), myId);
        } else {
            QPen pen(defaultPen);
            pen.setStyle(Qt::NoPen);
            node->appendStyleProperty(new QSvgStrokeStyle(pen), myId);
        }
    }
}


static bool parseQBrush(const QXmlAttributes &attributes, QSvgNode *node,
                        QBrush &brush, QSvgHandler *handler)
{
    QString value = attributes.value("fill");
    QString opacity = attributes.value("fill-opacity");

    if (opacity.isEmpty())
        opacity = attributes.value("opacity");

    QColor color;
    if (!value.isEmpty() || !opacity.isEmpty()) {
        if (value.startsWith("url")) {
            value = value.remove(0, 3);
            QString id = idFromUrl(value);

            QSvgStructureNode *group = 0;
            QSvgNode *dummy = node;
            while (dummy && (dummy->type() != QSvgNode::DOC  &&
                             dummy->type() != QSvgNode::G    &&
                             dummy->type() != QSvgNode::DEFS &&
                             dummy->type() != QSvgNode::SWITCH)) {
                dummy = dummy->parent();
            }
            if (dummy)
                group = static_cast<QSvgStructureNode*>(dummy);
            if (group) {
                QSvgStyleProperty *style =
                    group->scopeStyle(id);
                switch (style->type()) {
                case QSvgStyleProperty::FILL:
                {
                    brush = static_cast<QSvgFillStyle*>(style)->qbrush();
                    break;
                }
                case QSvgStyleProperty::SOLID_COLOR:
                {
                    brush = QBrush(static_cast<QSvgSolidColorStyle*>(style)->qcolor());
                    break;
                }
                case QSvgStyleProperty::GRADIENT:
                {
                    brush = QBrush(*static_cast<QSvgGradientStyle*>(style)->qgradient());
                    break;
                }
                default:
                    qWarning("Couldn't resolve property: %s", qPrintable(id));
                }
            }
        } else if (value != QLatin1String("none")) {
            if (constructColor(value, opacity, color, handler)) {
                brush.setStyle(Qt::SolidPattern);
                brush.setColor(color);
            }
        } else {
            brush = QBrush(Qt::NoBrush);
        }
        return true;
    }
    return false;
}

static bool parseQFont(const QXmlAttributes &attributes,
                       QFont &font, QSvgHandler *handler)
{
    QString family = attributes.value("font-family");
    QString size = attributes.value("font-size");
    QString style = attributes.value("font-style");
    QString weight = attributes.value("font-weight");

    if (!family.isEmpty() || !size.isEmpty() ||
        !style.isEmpty() || !weight.isEmpty()) {

        if (!family.isEmpty()) {
            font.setFamily(family.trimmed());
        }
        if (!size.isEmpty()) {
            QSvgHandler::LengthType type;
            qreal len = parseLength(size, type, handler);
            //len = convertToPixels(len, false, type);
            // ### org_module.svg shows that font size
            // seems to be always in px...
            type  = QSvgHandler::PX;
            if (type == QSvgHandler::PX ||
                type == QSvgHandler::OTHER)
                font.setPixelSize(int(len));
            else
                font.setPointSizeF(len);
        }
        if (!style.isEmpty()) {
            if (style == QLatin1String("normal")) {
                font.setStyle(QFont::StyleNormal);
            } else if (style == QLatin1String("italic")) {
                font.setStyle(QFont::StyleItalic);
            } else if (style == QLatin1String("oblique")) {
                font.setStyle(QFont::StyleOblique);
            } else if (style == QLatin1String("inherit")) {
                //inherited already
            }
        }
        if (!weight.isEmpty()) {
            bool ok = false;
            int weightNum = weight.toInt(&ok);
            if (ok) {
                switch (weightNum) {
                case 100:
                case 200:
                    font.setWeight(QFont::Light);
                    break;
                case 300:
                case 400:
                    font.setWeight(QFont::Normal);
                    break;
                case 500:
                case 600:
                    font.setWeight(QFont::DemiBold);
                    break;
                case 700:
                case 800:
                    font.setWeight(QFont::Bold);
                    break;
                case 900:
                    font.setWeight(QFont::Black);
                    break;
                default:
                    break;
                }
            } else {
                if (weight == QLatin1String("normal")) {
                    font.setWeight(QFont::Normal);
                } else if (weight == QLatin1String("bold")) {
                    font.setWeight(QFont::Bold);
                } else if (weight == QLatin1String("bolder")) {
                    font.setWeight(QFont::DemiBold);
                } else if (weight == QLatin1String("lighter")) {
                    font.setWeight(QFont::Light);
                }
            }
        }
        // QFontInfo fi(font);
        // font.setPointSize(fi.pointSize());
        return true;
    }

    return false;
}

static void parseFont(QSvgNode *node,
                      const QXmlAttributes &attributes,
                      QSvgHandler *handler)
{
    QFont font;
    QSvgFontStyle *inherited =
        static_cast<QSvgFontStyle*>(node->parent()->styleProperty(
                                        QSvgStyleProperty::FONT));
    if (inherited)
        font = inherited->qfont();
    if (parseQFont(attributes, font, handler)) {
        QString myId = attributes.value("id");
        QString anchor = attributes.value("text-anchor");
        QSvgTinyDocument *doc = node->document();
        QSvgFontStyle *fontStyle = 0;
        QString family = (font.family().isEmpty())?myId:font.family();
        if (!family.isEmpty()) {
            QSvgFont *svgFont = doc->svgFont(family);
            if (svgFont) {
                fontStyle = new QSvgFontStyle(svgFont, doc);
                if (font.pixelSize() < 0)
                    fontStyle->setPointSize(font.pointSizeF());
                else
                    fontStyle->setPointSize(font.pixelSize());
            }
        }
        if (!fontStyle)
            fontStyle = new QSvgFontStyle(font, node->document());
        if (!anchor.isEmpty())
            fontStyle->setTextAnchor(anchor);

        node->appendStyleProperty(fontStyle, myId);
    }
}

static void parseTransform(QSvgNode *node,
                           const QXmlAttributes &attributes,
                           QSvgHandler *)
{
    QString value = attributes.value("transform");
    QString myId = attributes.value("id");
    value = value.trimmed();
    QMatrix matrix = parseTransformationMatrix(value);
    if (!matrix.isIdentity()) {
        node->appendStyleProperty(new QSvgTransformStyle(matrix), myId);
    }

}

static void parseVisibility(QSvgNode *node,
                            const QXmlAttributes &attributes,
                            QSvgHandler *)
{
    QString value = attributes.value("visibility");
    QSvgNode *parent = node->parent();

    if (parent && (value.isEmpty() || value == "inherit"))
        node->setVisible(parent->isVisible());
    else if (value == "hidden" || value == "collapse") {
        node->setVisible(false);
    } else
        node->setVisible(true);
}

static void pathArcSegment(QPainterPath &path,
                           qreal xc, qreal yc,
                           qreal th0, qreal th1,
                           qreal rx, qreal ry, qreal xAxisRotation)
{
    qreal sinTh, cosTh;
    qreal a00, a01, a10, a11;
    qreal x1, y1, x2, y2, x3, y3;
    qreal t;
    qreal thHalf;

    sinTh = sin(xAxisRotation * (M_PI / 180.0));
    cosTh = cos(xAxisRotation * (M_PI / 180.0));

    a00 =  cosTh * rx;
    a01 = -sinTh * ry;
    a10 =  sinTh * rx;
    a11 =  cosTh * ry;

    thHalf = 0.5 * (th1 - th0);
    t = (8.0 / 3.0) * sin(thHalf * 0.5) * sin(thHalf * 0.5) / sin(thHalf);
    x1 = xc + cos(th0) - t * sin(th0);
    y1 = yc + sin(th0) + t * cos(th0);
    x3 = xc + cos(th1);
    y3 = yc + sin(th1);
    x2 = x3 + t * sin(th1);
    y2 = y3 - t * cos(th1);

    path.cubicTo(a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
                 a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
                 a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}

// the arc handling code underneath is from XSVG (BSD license)
/*
 * Copyright  2002 USC/Information Sciences Institute
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Information Sciences Institute not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.  Information Sciences Institute
 * makes no representations about the suitability of this software for
 * any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * INFORMATION SCIENCES INSTITUTE DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL INFORMATION SCIENCES
 * INSTITUTE BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
static void pathArc(QPainterPath &path,
                    qreal		rx,
                    qreal		ry,
                    qreal		x_axis_rotation,
                    int		large_arc_flag,
                    int		sweep_flag,
                    qreal		x,
                    qreal		y,
                    qreal curx, qreal cury)
{
    qreal sin_th, cos_th;
    qreal a00, a01, a10, a11;
    qreal x0, y0, x1, y1, xc, yc;
    qreal d, sfactor, sfactor_sq;
    qreal th0, th1, th_arc;
    int i, n_segs;
    qreal dx, dy, dx1, dy1, Pr1, Pr2, Px, Py, check;

    rx = qAbs(rx);
    ry = qAbs(ry);

    sin_th = sin(x_axis_rotation * (M_PI / 180.0));
    cos_th = cos(x_axis_rotation * (M_PI / 180.0));

    dx = (curx - x) / 2.0;
    dy = (cury - y) / 2.0;
    dx1 =  cos_th * dx + sin_th * dy;
    dy1 = -sin_th * dx + cos_th * dy;
    Pr1 = rx * rx;
    Pr2 = ry * ry;
    Px = dx1 * dx1;
    Py = dy1 * dy1;
    /* Spec : check if radii are large enough */
    check = Px / Pr1 + Py / Pr2;
    if (check > 1) {
        rx = rx * sqrt(check);
        ry = ry * sqrt(check);
    }

    a00 =  cos_th / rx;
    a01 =  sin_th / rx;
    a10 = -sin_th / ry;
    a11 =  cos_th / ry;
    x0 = a00 * curx + a01 * cury;
    y0 = a10 * curx + a11 * cury;
    x1 = a00 * x + a01 * y;
    y1 = a10 * x + a11 * y;
    /* (x0, y0) is current point in transformed coordinate space.
       (x1, y1) is new point in transformed coordinate space.

       The arc fits a unit-radius circle in this space.
    */
    d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    sfactor_sq = 1.0 / d - 0.25;
    if (sfactor_sq < 0) sfactor_sq = 0;
    sfactor = sqrt(sfactor_sq);
    if (sweep_flag == large_arc_flag) sfactor = -sfactor;
    xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
    yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
    /* (xc, yc) is center of the circle. */

    th0 = atan2(y0 - yc, x0 - xc);
    th1 = atan2(y1 - yc, x1 - xc);

    th_arc = th1 - th0;
    if (th_arc < 0 && sweep_flag)
        th_arc += 2 * M_PI;
    else if (th_arc > 0 && !sweep_flag)
        th_arc -= 2 * M_PI;

    n_segs = int(ceil(qAbs(th_arc / (M_PI * 0.5 + 0.001))));

    for (i = 0; i < n_segs; i++) {
        pathArcSegment(path, xc, yc,
                       th0 + i * th_arc / n_segs,
                       th0 + (i + 1) * th_arc / n_segs,
                       rx, ry, x_axis_rotation);
    }
}

static bool parsePathDataFast(const QString &data, QPainterPath &path)
{
    QString::const_iterator itr = data.constBegin();
    qreal x0 = 0, y0 = 0;              // starting point
    qreal x = 0, y = 0;                // current point
    char lastMode = 0;
    QChar pathElem;
    QPointF ctrlPt;

    while (itr != data.constEnd()) {
        while ((*itr).isSpace())
            ++itr;
        pathElem = *itr;
        ++itr;
        QList<qreal> arg = parseNumbersList(itr);
        if (pathElem == 'z' || pathElem == 'Z')
            arg.append(0);//dummy
        while (!arg.isEmpty()) {
            qreal offsetX = x;        // correction offsets
            qreal offsetY = y;        // for relative commands
            switch (pathElem.toAscii()) {
            case 'm': {
                x = x0 = arg[0] + offsetX;
                y = y0 = arg[1] + offsetY;
                path.moveTo(x0, y0);
                arg.pop_front(); arg.pop_front();
            }
                break;
            case 'M': {
                x = x0 = arg[0];
                y = y0 = arg[1];
                path.moveTo(x0, y0);
                arg.pop_front(); arg.pop_front();
            }
                break;
            case 'z':
            case 'Z': {
                x = x0;
                y = y0;
                path.closeSubpath();
                arg.pop_front();//pop dummy
            }
                break;
            case 'l': {
                x = arg.front() + offsetX;
                arg.pop_front();
                y = arg.front() + offsetY;
                arg.pop_front();
                path.lineTo(x, y);

            }
                break;
            case 'L': {
                x = arg.front(); arg.pop_front();
                y = arg.front(); arg.pop_front();
                path.lineTo(x, y);
            }
                break;
            case 'h': {
                x = arg.front() + offsetX; arg.pop_front();
                path.lineTo(x, y);
            }
                break;
            case 'H': {
                x = arg[0];
                path.lineTo(x, y);
                arg.pop_front();
            }
                break;
            case 'v': {
                y = arg[0] + offsetY;
                path.lineTo(x, y);
                arg.pop_front();
            }
                break;
            case 'V': {
                y = arg[0];
                path.lineTo(x, y);
                arg.pop_front();
            }
                break;
            case 'c': {
                QPointF c1(arg[0]+offsetX, arg[1]+offsetY);
                QPointF c2(arg[2]+offsetX, arg[3]+offsetY);
                QPointF e(arg[4]+offsetX, arg[5]+offsetY);
                path.cubicTo(c1, c2, e);
                ctrlPt = c2;
                x = e.x();
                y = e.y();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                break;
            }
            case 'C': {
                QPointF c1(arg[0], arg[1]);
                QPointF c2(arg[2], arg[3]);
                QPointF e(arg[4], arg[5]);
                path.cubicTo(c1, c2, e);
                ctrlPt = c2;
                x = e.x();
                y = e.y();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                break;
            }
            case 's': {
                QPointF c1;
                if (lastMode == 'c' || lastMode == 'C' ||
                    lastMode == 's' || lastMode == 'S')
                    c1 = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c1 = QPointF(x, y);
                QPointF c2(arg[0]+offsetX, arg[1]+offsetY);
                QPointF e(arg[2]+offsetX, arg[3]+offsetY);
                path.cubicTo(c1, c2, e);
                ctrlPt = c2;
                x = e.x();
                y = e.y();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                break;
            }
            case 'S': {
                QPointF c1;
                if (lastMode == 'c' || lastMode == 'C' ||
                    lastMode == 's' || lastMode == 'S')
                    c1 = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c1 = QPointF(x, y);
                QPointF c2(arg[0], arg[1]);
                QPointF e(arg[2], arg[3]);
                path.cubicTo(c1, c2, e);
                ctrlPt = c2;
                x = e.x();
                y = e.y();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                break;
            }
            case 'q': {
                QPointF c(arg[0]+offsetX, arg[1]+offsetY);
                QPointF e(arg[2]+offsetX, arg[3]+offsetY);
                path.quadTo(c, e);
                ctrlPt = c;
                x = e.x();
                y = e.y();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                break;
            }
            case 'Q': {
                QPointF c(arg[0], arg[1]);
                QPointF e(arg[2], arg[3]);
                path.quadTo(c, e);
                ctrlPt = c;
                x = e.x();
                y = e.y();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                break;
            }
            case 't': {
                QPointF e(arg[0]+offsetX, arg[1]+offsetY);
                QPointF c;
                if (lastMode == 'q' || lastMode == 'Q' ||
                    lastMode == 't' || lastMode == 'T')
                    c = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c = QPointF(x, y);
                path.quadTo(c, e);
                ctrlPt = c;
                x = e.x();
                y = e.y();
                arg.pop_front(); arg.pop_front();
                break;
            }
            case 'T': {
                QPointF e(arg[0], arg[1]);
                QPointF c;
                if (lastMode == 'q' || lastMode == 'Q' ||
                    lastMode == 't' || lastMode == 'T')
                    c = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
                else
                    c = QPointF(x, y);
                path.quadTo(c, e);
                ctrlPt = c;
                x = e.x();
                y = e.y();
                arg.pop_front(); arg.pop_front();
                break;
            }
            case 'a': {
                qreal rx = arg[0];
                qreal ry = arg[1];
                qreal xAxisRotation = arg[2];
                qreal largeArcFlag  = arg[3];
                qreal sweepFlag = arg[4];
                qreal ex = arg[5] + offsetX;
                qreal ey = arg[6] + offsetY;
                qreal curx = x;
                qreal cury = y;
                pathArc(path, rx, ry, xAxisRotation, int(largeArcFlag),
                        int(sweepFlag), ex, ey, curx, cury);

                x = ex;
                y = ey;

                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                arg.pop_front();
            }
                break;
            case 'A': {
                qreal rx = arg[0];
                qreal ry = arg[1];
                qreal xAxisRotation = arg[2];
                qreal largeArcFlag  = arg[3];
                qreal sweepFlag = arg[4];
                qreal ex = arg[5];
                qreal ey = arg[6];
                qreal curx = x;
                qreal cury = y;
                pathArc(path, rx, ry, xAxisRotation, int(largeArcFlag),
                        int(sweepFlag), ex, ey, curx, cury);
                x = ex;
                y = ey;
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                arg.pop_front(); arg.pop_front();
                arg.pop_front();
            }
                break;
            default:
                qDebug()<<"path data is "<<pathElem;
                Q_ASSERT(!"invalid path data");
                break;
            }
            lastMode = pathElem.toAscii();
        }
    }
    return true;
}

static bool parseStyle(QSvgNode *node,
                       const QXmlAttributes &attributes,
                       QSvgHandler *);

static bool parseCSStoXMLAttrs(const QString &css,
                               QXmlAttributes &attributes)
{
    QStringList lst = css.split(";", QString::SkipEmptyParts);
    QRegExp rx("\\/\\*.+\\*\\/");
    rx.setMinimal(true);

    QStringList::iterator itr = lst.begin();
    for ( ; itr != lst.end(); ++itr) {
        QString prop = (*itr).remove(rx).trimmed();
        if (prop.isEmpty())
            continue;
        QStringList props = prop.split(":", QString::SkipEmptyParts);
        attributes.append(props[0], QString(), props[0], props[1]);
    }
    return attributes.count();
}

static bool parseDefaultTextStyle(QSvgNode *node,
                                  const QXmlAttributes &attributes,
                                  bool initial,
                                  QSvgHandler *handler)
{
    Q_ASSERT(node->type() == QSvgText::TEXT);
    QSvgText *textNode = static_cast<QSvgText*>(node);
    QXmlAttributes attrs = attributes;
    QString css = attrs.value("style");
    QString fontFamily = attrs.value("font-family");

    parseCSStoXMLAttrs(css, attrs);

    QString anchor = attrs.value("text-anchor");

    QSvgFontStyle *fontStyle = static_cast<QSvgFontStyle*>(
        node->styleProperty(QSvgStyleProperty::FONT));
    if (fontStyle) {
        QSvgTinyDocument *doc = fontStyle->doc();
        if (doc && fontStyle->svgFont()) {
            parseStyle(node, attrs, handler);
            return true;
        }
    } else if (!fontFamily.isEmpty()) {
        QSvgTinyDocument *doc = node->document();
        QSvgFont *svgFont = doc->svgFont(fontFamily);
        if (svgFont) {
            parseStyle(node, attrs, handler);
            return true;
        }
    }


    QTextCharFormat format;
    QFont font;
    QBrush brush(QColor(0, 0, 0));
    if (!initial) {
        font = textNode->topFormat().font();
        brush = textNode->topFormat().foreground();
    }
    if (initial) {
        QSvgFontStyle *fontStyle = static_cast<QSvgFontStyle*>(
            node->parent()->styleProperty(QSvgStyleProperty::FONT));
        if (fontStyle) {
            font = fontStyle->qfont();
            if (anchor.isEmpty())
                anchor = fontStyle->textAnchor();
        }
    }
    if (parseQFont(attrs, font, handler) || initial) {
        if (font.pixelSize() != -1)
            format.setProperty(QTextFormat::FontPixelSize, font.pixelSize());
        format.setFont(font);
    }

    if (initial) {
        QSvgFillStyle *fillStyle = static_cast<QSvgFillStyle*>(
            node->styleProperty(QSvgStyleProperty::FILL));
        if (fillStyle) {
            brush = fillStyle->qbrush();
        }
    }
    if (parseQBrush(attrs, node, brush, handler) || initial) {
        if (brush.style() != Qt::NoBrush || initial)
            format.setForeground(brush);
    }

    QPen pen(Qt::NoPen);
//     QSvgStrokeStyle *inherited =
//         static_cast<QSvgStrokeStyle*>(node->parent()->styleProperty(
//                                           QSvgStyleProperty::STROKE));
//     if (inherited)
//         pen = inherited->qpen();
    parseQPen(pen, node, attrs, handler);
    if (pen.style() != Qt::NoPen) {
        format.setTextOutline(pen);
    }

    if (initial) {
        Qt::Alignment align = Qt::AlignLeft;
        if (anchor == "middle")
            align = Qt::AlignHCenter;
        else if (anchor == "end")
            align = Qt::AlignRight;
        textNode->setTextAlignment(align);
    }
    parseTransform(node, attributes, handler);

    textNode->insertFormat(format);

    return true;
}

static bool parseCSSStyle(QSvgNode *node,
                          const QString &css,
                          QSvgHandler *handler)
{
    QXmlAttributes attributes;
    if (parseCSStoXMLAttrs(css, attributes))
        parseStyle(node, attributes, handler);

    return true;
}

static inline QStringList stringToList(const QString &str)
{
    QStringList lst = str.split(",", QString::SkipEmptyParts);
    return lst;
}

static bool parseCoreNode(QSvgNode *node,
                          const QXmlAttributes &attributes)
{
    QString featuresStr   = attributes.value("requiredFeatures");
    QString extensionsStr = attributes.value("requiredExtensions");
    QString languagesStr  = attributes.value("systemLanguage");
    QString formatsStr    = attributes.value("requiredFormats");
    QString fontsStr      = attributes.value("requiredFonts");

    QStringList features = stringToList(featuresStr);
    QStringList extensions = stringToList(extensionsStr);
    QStringList languages = stringToList(languagesStr);
    QStringList formats = stringToList(formatsStr);
    QStringList fonts = stringToList(fontsStr);

    node->setRequiredFeatures(features);
    node->setRequiredExtensions(extensions);
    node->setRequiredLanguages(languages);
    node->setRequiredFormats(formats);
    node->setRequiredFonts(fonts);

    return true;
}

static bool parseStyle(QSvgNode *node,
                       const QXmlAttributes &attributes,
                       QSvgHandler *handler)
{
    if (!attributes.value("style").isEmpty()) {
        parseCSSStyle(node, attributes.value("style"), handler);
    }
    parseColor(node, attributes, handler);
    parseBrush(node, attributes, handler);
    parsePen(node, attributes, handler);
    parseFont(node, attributes, handler);
    parseTransform(node, attributes, handler);
    parseVisibility(node, attributes, handler);


#if 0
    value = attributes.value("audio-level");

    value = attributes.value("color-rendering");

    value = attributes.value("display");

    value = attributes.value("display-align");

    value = attributes.value("image-rendering");

    value = attributes.value("line-increment");

    value = attributes.value("pointer-events");

    value = attributes.value("shape-rendering");

    value = attributes.value("solid-color");

    value = attributes.value("solid-opacity");

    value = attributes.value("text-rendering");

    value = attributes.value("vector-effect");

    value = attributes.value("viewport-fill");

    value = attributes.value("viewport-fill-opacity");
#endif
    return true;
}

static bool parseAnchorNode(QSvgNode *parent,
                            const QXmlAttributes &attributes,
                            QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseAnimateNode(QSvgNode *parent,
                             const QXmlAttributes &attributes,
                             QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseAnimateColorNode(QSvgNode *parent,
                                  const QXmlAttributes &attributes,
                                  QSvgHandler *handler)
{
    QString typeStr    = attributes.value("type");
    QString fromStr    = attributes.value("from");
    QString toStr      = attributes.value("to");
    QString valuesStr  = attributes.value("values");
    QString beginStr   = attributes.value("begin");
    QString durStr     = attributes.value("dur");
    QString targetStr  = attributes.value("attributeName");
    QString repeatStr  = attributes.value("repeatCount");
    QString fillStr    = attributes.value("fill");

    QList<QColor> colors;
    if (valuesStr.isEmpty()) {
        QColor startColor, endColor;
        constructColor(fromStr, QString(), startColor, handler);
        constructColor(toStr, QString(), endColor, handler);
        colors.append(startColor);
        colors.append(endColor);
    } else {
        QStringList str = valuesStr.split(';');
        QStringList::const_iterator itr;
        for (itr = str.constBegin(); itr != str.constEnd(); ++itr) {
            QColor color;
            constructColor(*itr, QString(), color, handler);
            colors.append(color);
        }
    }

    int ms = 1000;
    beginStr = beginStr.trimmed();
    if (beginStr.endsWith("ms")) {
        beginStr.chop(2);
        ms = 1;
    } else if (beginStr.endsWith("s")) {
        beginStr.chop(1);
    }
    durStr = durStr.trimmed();
    if (durStr.endsWith("ms")) {
        durStr.chop(2);
        ms = 1;
    } else if (durStr.endsWith("s")) {
        durStr.chop(1);
    }
    int begin = static_cast<int>(beginStr.toDouble() * ms);
    int end   = static_cast<int>((durStr.toDouble() + begin) * ms);

    QSvgAnimateColor *anim = new QSvgAnimateColor(begin, end, 0);
    anim->setArgs((targetStr == "fill"), colors);
    anim->setFreeze(fillStr == "freeze");
    anim->setRepeatCount(
        (repeatStr == "indefinite")?-1:repeatStr.toDouble());

    parent->appendStyleProperty(anim, attributes.value("id"));
    parent->document()->setAnimated(true);
    return true;
}

static bool parseAimateMotionNode(QSvgNode *parent,
                                  const QXmlAttributes &attributes,
                                  QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseAnimateTransformNode(QSvgNode *parent,
                                      const QXmlAttributes &attributes,
                                      QSvgHandler *)
{
    QString typeStr    = attributes.value("type");
    QString values     = attributes.value("values");
    QString beginStr   = attributes.value("begin");
    QString durStr     = attributes.value("dur");
    QString targetStr  = attributes.value("attributeName");
    QString repeatStr  = attributes.value("repeatCount");
    QString fillStr    = attributes.value("fill");
    QString fromStr    = attributes.value("from");
    QString toStr      = attributes.value("to");

    QList<qreal> vals;
    if (values.isEmpty()) {
        QString::const_iterator itr = fromStr.constBegin();
        QList<qreal> lst = parseNumbersList(itr);
        while (lst.count() < 3)
            lst.append(0.0);
        vals << lst;

        itr = toStr.constBegin();
        lst = parseNumbersList(itr);
        while (lst.count() < 3)
            lst.append(0.0);
        vals << lst;
    } else {
        QString::const_iterator itr = values.constBegin();
        while (itr != values.constEnd()) {
            QList<qreal> tmpVals = parseNumbersList(itr);
            while (tmpVals.count() < 3)
                tmpVals.append(0.0);

            vals << tmpVals;
            if (itr == values.constEnd())
                break;
            ++itr;
        }
    }

    int ms = 1000;
    beginStr = beginStr.trimmed();
    if (beginStr.endsWith("ms")) {
        beginStr.chop(2);
        ms = 1;
    } else if (beginStr.endsWith("s")) {
        beginStr.chop(1);
    }
    durStr = durStr.trimmed();
    if (durStr.endsWith("ms")) {
        durStr.chop(2);
        ms = 1;
    } else if (durStr.endsWith("s")) {
        durStr.chop(1);
    }
    int begin = static_cast<int>(beginStr.toDouble() * ms);
    int end   = static_cast<int>(durStr.toDouble()*ms) + begin;
    QSvgAnimateTransform::TransformType type = QSvgAnimateTransform::Empty;
    if (typeStr == "translate") {
        type = QSvgAnimateTransform::Translate;
    } else if (typeStr == "scale") {
        type = QSvgAnimateTransform::Scale;
    } else if (typeStr == "rotate") {
        type = QSvgAnimateTransform::Rotate;
    } else if (typeStr == "skewX") {
        type = QSvgAnimateTransform::SkewX;
    } else if (typeStr == "skewY") {
        type = QSvgAnimateTransform::SkewY;
    }

    QSvgAnimateTransform *anim = new QSvgAnimateTransform(begin, end, 0);
    anim->setArgs(type, vals);
    anim->setFreeze(fillStr == "freeze");
    anim->setRepeatCount(
        (repeatStr == "indefinite")?-1:repeatStr.toDouble());

    parent->appendStyleProperty(anim, attributes.value("id"));
    parent->document()->setAnimated(true);
    return true;
}

static QSvgNode * createAnimationNode(QSvgNode *parent,
                                      const QXmlAttributes &attributes,
                                      QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return 0;
}

static bool parseAudioNode(QSvgNode *parent,
                           const QXmlAttributes &attributes,
                           QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createCircleNode(QSvgNode *parent,
                                  const QXmlAttributes &attributes,
                                  QSvgHandler *)
{
    QString cx      = attributes.value("cx");
    QString cy      = attributes.value("cy");
    QString r       = attributes.value("r");
    qreal ncx = cx.toDouble();
    qreal ncy = cy.toDouble();
    qreal nr  = r.toDouble();

    QRectF rect(ncx-nr, ncy-nr, nr*2, nr*2);
    QSvgNode *circle = new QSvgCircle(parent, rect);
    return circle;
}

static QSvgNode *createDefsNode(QSvgNode *parent,
                                const QXmlAttributes &attributes,
                                QSvgHandler *)
{
    Q_UNUSED(attributes);
    QSvgDefs *defs = new QSvgDefs(parent);
    return defs;
}

static bool parseDescNode(QSvgNode *parent,
                          const QXmlAttributes &attributes,
                          QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseDiscardNode(QSvgNode *parent,
                             const QXmlAttributes &attributes,
                             QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createEllipseNode(QSvgNode *parent,
                                   const QXmlAttributes &attributes,
                                   QSvgHandler *)
{
    QString cx      = attributes.value("cx");
    QString cy      = attributes.value("cy");
    QString rx      = attributes.value("rx");
    QString ry      = attributes.value("ry");
    qreal ncx = cx.toDouble();
    qreal ncy = cy.toDouble();
    qreal nrx = rx.toDouble();
    qreal nry = ry.toDouble();

    QRectF rect(ncx-nrx, ncy-nry, nrx*2, nry*2);
    QSvgNode *ellipse = new QSvgEllipse(parent, rect);
    return ellipse;
}

static QSvgStyleProperty *createFontNode(QSvgNode *parent,
                                         const QXmlAttributes &attributes,
                                         QSvgHandler *)
{
    QString hax      = attributes.value("horiz-adv-x");
    QString myId     = attributes.value("id");

    qreal horizAdvX = hax.toDouble();

    while (parent && parent->type() != QSvgNode::DOC) {
        parent = parent->parent();
    }

    if (parent) {
        QSvgTinyDocument *doc = static_cast<QSvgTinyDocument*>(parent);
        QSvgFont *font = new QSvgFont(horizAdvX);
        font->setFamilyName(myId);
        if (!font->familyName().isEmpty()) {
            if (!doc->svgFont(font->familyName()))
                doc->addSvgFont(font);
        }
        return new QSvgFontStyle(font, doc);
    }
    return 0;
}

static bool parseFontFaceNode(QSvgStyleProperty *parent,
                              const QXmlAttributes &attributes,
                              QSvgHandler *)
{
    if (parent->type() != QSvgStyleProperty::FONT) {
        return false;
    }

    QSvgFontStyle *style = static_cast<QSvgFontStyle*>(parent);
    QSvgFont *font = style->svgFont();
    QString name   = attributes.value("font-family");
    QString unitsPerEmStr   = attributes.value("units-per-em");

    qreal unitsPerEm = unitsPerEmStr.toDouble();
    if (!unitsPerEm)
        unitsPerEm = 1000;

    if (!name.isEmpty())
        font->setFamilyName(name);
    font->setUnitsPerEm(unitsPerEm);

    if (!font->familyName().isEmpty())
        if (!style->doc()->svgFont(font->familyName()))
            style->doc()->addSvgFont(font);

    return true;
}

static bool parseFontFaceNameNode(QSvgStyleProperty *parent,
                                  const QXmlAttributes &attributes,
                                  QSvgHandler *)
{
    if (parent->type() != QSvgStyleProperty::FONT) {
        return false;
    }

    QSvgFontStyle *style = static_cast<QSvgFontStyle*>(parent);
    QSvgFont *font = style->svgFont();
    QString name   = attributes.value("name");

    if (!name.isEmpty())
        font->setFamilyName(name);

    if (!font->familyName().isEmpty())
        if (!style->doc()->svgFont(font->familyName()))
            style->doc()->addSvgFont(font);

    return true;
}

static bool parseFontFaceSrcNode(QSvgStyleProperty *parent,
                                 const QXmlAttributes &attributes,
                                 QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseFontFaceUriNode(QSvgStyleProperty *parent,
                                 const QXmlAttributes &attributes,
                                 QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseForeignObjectNode(QSvgNode *parent,
                                   const QXmlAttributes &attributes,
                                   QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createGNode(QSvgNode *parent,
                             const QXmlAttributes &attributes,
                             QSvgHandler *)
{
    Q_UNUSED(attributes);
    QSvgG *node = new QSvgG(parent);
    return node;
}

static bool parseGlyphNode(QSvgStyleProperty *parent,
                           const QXmlAttributes &attributes,
                           QSvgHandler *)
{
    if (parent->type() != QSvgStyleProperty::FONT) {
        return false;
    }

    QSvgFontStyle *style = static_cast<QSvgFontStyle*>(parent);
    QSvgFont *font = style->svgFont();
    createSvgGlyph(font, attributes);
    return true;
}

static bool parseHandlerNode(QSvgNode *parent,
                             const QXmlAttributes &attributes,
                             QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseHkernNode(QSvgNode *parent,
                           const QXmlAttributes &attributes,
                           QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createImageNode(QSvgNode *parent,
                                 const QXmlAttributes &attributes,
                                 QSvgHandler *handler)
{
    QString x = attributes.value("x");
    QString y = attributes.value("y");
    QString width  = attributes.value("width");
    QString height = attributes.value("height");
    QString filename = attributes.value("xlink:href");
    qreal nx = x.toDouble();
    qreal ny = y.toDouble();
    QSvgHandler::LengthType type;
    qreal nwidth = parseLength(width, type, handler);
    nwidth = convertToPixels(nwidth, true, type);

    qreal nheight = parseLength(height, type, handler);
    nheight = convertToPixels(nheight, false, type);


    filename = filename.trimmed();
    QImage image;
    if (filename.startsWith("data")) {
        int idx = filename.lastIndexOf("base64,");
        if (idx != -1) {
            idx += 7;
            QString dataStr = filename.mid(idx);
            QByteArray data = QByteArray::fromBase64(dataStr.toAscii());
            image = QImage::fromData(data);
        } else {
            qDebug()<<"QSvgHandler::createImageNode: Unrecognized inline image format!";
        }
    } else
        image = QImage(filename);

    if (image.isNull()) {
        qDebug()<<"couldn't create image from "<<filename;
        return 0;
    }

    QSvgNode *img = new QSvgImage(parent,
                                  image,
                                  QRect(int(nx),
                                        int(ny),
                                        int(nwidth),
                                        int(nheight)));
    return img;
}

static QSvgNode *createLineNode(QSvgNode *parent,
                                const QXmlAttributes &attributes,
                                QSvgHandler *)
{
    QString x1 = attributes.value("x1");
    QString y1 = attributes.value("y1");
    QString x2 = attributes.value("x2");
    QString y2 = attributes.value("y2");
    qreal nx1 = x1.toDouble();
    qreal ny1 = y1.toDouble();
    qreal nx2 = x2.toDouble();
    qreal ny2 = y2.toDouble();

    QLineF lineBounds(nx1, ny1, nx2, ny2);
    QSvgNode *line = new QSvgLine(parent, lineBounds);
    return line;
}

static QSvgStyleProperty *createLinearGradientNode(QSvgNode *node,
                                                   const QXmlAttributes &attributes,
                                                   QSvgHandler *)
{
    QString x1 = attributes.value("x1");
    QString y1 = attributes.value("y1");
    QString x2 = attributes.value("x2");
    QString y2 = attributes.value("y2");
    QString link = attributes.value("xlink:href");
    QString trans = attributes.value("gradientTransform");
    QString units = attributes.value("gradientUnits");
    qreal nx1 = x1.toDouble();
    qreal ny1 = y1.toDouble();
    qreal nx2 = x2.toDouble();
    qreal ny2 = y2.toDouble();
    bool  needsResolving = true;

    if (nx2==0 && ny2==0) {
        nx2 = 1;
        ny2 = 1;
    } else if (units == "userSpaceOnUse") {
        needsResolving = false;
    }

    QSvgNode *itr = node;
    while (itr && itr->type() != QSvgNode::DOC) {
        itr = itr->parent();
    }


    if (!trans.isEmpty()) {
        QMatrix matrix;
        matrix = parseTransformationMatrix(trans);
        QPointF pt(nx1, ny1);
        pt = matrix.map(pt);
        nx1 = pt.x();
        ny1 = pt.y();
        pt = matrix.map(QPointF(nx2, ny2));
        nx2 = pt.x();
        ny2 = pt.y();
    }

    QLinearGradient *grad = new QLinearGradient(nx1, ny1, nx2, ny2);
    if (!link.isEmpty()) {
        QSvgStyleProperty *prop = node->styleProperty(link);
        //qDebug()<<"inherited "<<prop<<" ("<<link<<")";
        if (prop && prop->type() == QSvgStyleProperty::GRADIENT) {
            QSvgGradientStyle *inherited =
                static_cast<QSvgGradientStyle*>(prop);
            grad->setStops(inherited->qgradient()->stops());
        }
    }

    QSvgStyleProperty *prop = new QSvgGradientStyle(grad, needsResolving);
    return prop;
}

static bool parseMetadataNode(QSvgNode *parent,
                              const QXmlAttributes &attributes,
                              QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseMissingGlyphNode(QSvgStyleProperty *parent,
                                  const QXmlAttributes &attributes,
                                  QSvgHandler *)
{
    if (parent->type() != QSvgStyleProperty::FONT) {
        return false;
    }

    QSvgFontStyle *style = static_cast<QSvgFontStyle*>(parent);
    QSvgFont *font = style->svgFont();
    createSvgGlyph(font, attributes);
    return true;
}

static bool parseMpathNode(QSvgNode *parent,
                           const QXmlAttributes &attributes,
                           QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createPathNode(QSvgNode *parent,
                                const QXmlAttributes &attributes,
                                QSvgHandler *)
{
    QString data      = attributes.value("d");
    QString fillRule  = attributes.value("fill-rule");

    QPainterPath qpath;
    if (!fillRule.isEmpty()) {
        if (fillRule == QLatin1String("nonzero")) {
            qpath.setFillRule(Qt::WindingFill);
        }
    }

    //XXX do error handling
    parsePathDataFast(data, qpath);
    QSvgNode *path = new QSvgPath(parent, qpath);
    return path;
}

static QSvgNode *createPolygonNode(QSvgNode *parent,
                                   const QXmlAttributes &attributes,
                                   QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    QString pointsStr  = attributes.value("points");

    //same QPolygon parsing is in createPolylineNode
    QString::const_iterator sitr = pointsStr.constBegin();
    QList<qreal> points = parseNumbersList(sitr);
    QPolygonF poly(points.count()/2);
    int i = 0;
    QList<qreal>::const_iterator itr = points.constBegin();
    while (itr != points.constEnd()) {
        qreal one = *itr; ++itr;
        qreal two = *itr; ++itr;
        poly[i] = QPointF(one, two);
        ++i;
    }
    QSvgNode *polygon = new QSvgPolygon(parent, poly);
    return polygon;
}

static QSvgNode *createPolylineNode(QSvgNode *parent,
                                    const QXmlAttributes &attributes,
                                    QSvgHandler *)
{
    QString pointsStr  = attributes.value("points");

    //same QPolygon parsing is in createPolygonNode
    QString::const_iterator sitr = pointsStr.constBegin();
    QList<qreal> points = parseNumbersList(sitr);
    QPolygonF poly(points.count()/2);
    int i = 0;
    QList<qreal>::const_iterator itr = points.constBegin();
    while (itr != points.constEnd()) {
        qreal one = *itr; ++itr;
        qreal two = *itr; ++itr;
        poly[i] = QPointF(one, two);
        ++i;
    }

    QSvgNode *line = new QSvgPolyline(parent, poly);
    return line;
}

static bool parsePrefetchNode(QSvgNode *parent,
                              const QXmlAttributes &attributes,
                              QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgStyleProperty *createRadialGradientNode(QSvgNode *node,
                                                   const QXmlAttributes &attributes,
                                                   QSvgHandler *)
{
    QString cx = attributes.value("cx");
    QString cy = attributes.value("cy");
    QString r  = attributes.value("r");
    QString fx = attributes.value("fx");
    QString fy = attributes.value("fy");
    QString link = attributes.value("xlink:href");
    QString trans = attributes.value("gradientTransform");
    QString units = attributes.value("gradientUnits");
    bool needsResolving = true;

    qreal ncx = 0.5;
    qreal ncy = 0.5;
    qreal nr  = 0.5;
    if (!cx.isEmpty())
        ncx = cx.toDouble();
    if (!cy.isEmpty())
        ncy = cy.toDouble();
    if (!r.isEmpty())
        nr = r.toDouble();
    qreal nfx = fx.toDouble();
    qreal nfy = fy.toDouble();

    if (units == "userSpaceOnUse") {
        needsResolving = false;
    }

    if (!trans.isEmpty()) {
        QMatrix matrix;
        matrix = parseTransformationMatrix(trans);
        QPointF pt(ncx, ncy);
        pt = matrix.map(pt);
        ncx = pt.x();
        ncy = pt.y();
        pt = matrix.map(QPointF(nfx, nfy));
        nfx = pt.x();
        nfy = pt.y();
    }

    QRadialGradient *grad = new QRadialGradient(ncx, ncy, nr, nfx, nfy);
    if (!link.isEmpty()) {
        QSvgStyleProperty *prop = node->styleProperty(link);
        //qDebug()<<"inherited "<<prop<<" ("<<link<<")";
        if (prop && prop->type() == QSvgStyleProperty::GRADIENT) {
            QSvgGradientStyle *inherited =
                static_cast<QSvgGradientStyle*>(prop);
            grad->setStops(inherited->qgradient()->stops());
        }
    }
    QSvgGradientStyle *prop = new QSvgGradientStyle(grad, needsResolving);
    return prop;
}

static QSvgNode *createRectNode(QSvgNode *parent,
                                const QXmlAttributes &attributes,
                                QSvgHandler *handler)
{
    QString x      = attributes.value("x");
    QString y      = attributes.value("y");
    QString width  = attributes.value("width");
    QString height = attributes.value("height");
    QString rx      = attributes.value("rx");
    QString ry      = attributes.value("ry");

    QSvgHandler::LengthType type;
    qreal nwidth = parseLength(width, type, handler);
    nwidth = convertToPixels(nwidth, true, type);

    qreal nheight = parseLength(height, type, handler);
    nheight = convertToPixels(nheight, true, type);
    qreal nrx = rx.toDouble();
    qreal nry = ry.toDouble();

    QRectF bounds(x.toDouble(), y.toDouble(),
                  nwidth, nheight);

    //9.2 The 'rect'  element clearly specifies it
    // but the case might in fact be handled because
    // we draw rounded rectangles differently
    if (nrx > bounds.width()/2)
        nrx = bounds.width()/2;
    if (nry > bounds.height()/2)
        nry = bounds.height()/2;

    if (nrx && !nry)
        nry = nrx;
    else if (nry && !nrx)
        nrx = nry;

    //we draw rounded rect from 0...99
    //svg from 0...bounds.width()/2 so we're adjusting the
    //coordinates
    nrx *= (100/(bounds.width()/2));
    nry *= (100/(bounds.height()/2));

    QSvgNode *rect = new QSvgRect(parent, bounds,
                                  int(nrx),
                                  int(nry));
    return rect;
}

static bool parseScriptNode(QSvgNode *parent,
                            const QXmlAttributes &attributes,
                            QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseSetNode(QSvgNode *parent,
                         const QXmlAttributes &attributes,
                         QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgStyleProperty *createSolidColorNode(QSvgNode *parent,
                                               const QXmlAttributes &attributes,
                                               QSvgHandler *handler)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    QString solidColorStr = attributes.value("solid-color");
    QString solidOpacityStr = attributes.value("solid-opacity");

    if (solidOpacityStr.isEmpty())
        solidOpacityStr = attributes.value("opacity");

    QColor color;
    if (!constructColor(solidColorStr, solidOpacityStr, color, handler))
        return 0;
    QSvgSolidColorStyle *style = new QSvgSolidColorStyle(color);
    return style;
}

static bool parseStopNode(QSvgStyleProperty *parent,
                          const QXmlAttributes &attributes,
                          QSvgHandler *handler)
{
    if (parent->type() != QSvgStyleProperty::GRADIENT)
        return false;

    QXmlAttributes attrs = attributes;
    QString cssStyle = attrs.value("style");
    if (!cssStyle.isEmpty()) {
        parseCSStoXMLAttrs(cssStyle, attrs);
    }

    QSvgGradientStyle *style =
        static_cast<QSvgGradientStyle*>(parent);
    QString offsetStr   = attrs.value("offset");
    QString colorStr    = attrs.value("stop-color");
    QString opacityStr  = attrs.value("stop-opacity");
    QColor color;
    QSvgHandler::LengthType type;
    qreal offset = parseLength(offsetStr, type, handler);
    //offset = convertToPixels(offset, true, type);
    if (type == QSvgHandler::PERCENT) {
        offset = offset/100.0;
    }
    bool colorOK = constructColor(colorStr, opacityStr, color, handler);
    QGradient *grad = style->qgradient();
    //qDebug()<<"set color at"<<offset<<color;
    grad->setColorAt(offset, color);
    if (!colorOK)
        style->addResolve(offset);
    return true;
}

static bool parseStyleNode(QSvgNode *parent,
                           const QXmlAttributes &attributes,
                           QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createSvgNode(QSvgNode *parent,
                               const QXmlAttributes &attributes,
                               QSvgHandler *handler)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);

    QString baseProfile = attributes.value("baseProfile");

    if (baseProfile.isEmpty() && baseProfile != QLatin1String("tiny")) {
        qWarning("Profile is %s while we only support tiny!",
                 qPrintable(baseProfile));
    }

    QSvgTinyDocument *node = new QSvgTinyDocument();
    QString widthStr  = attributes.value("width");
    QString heightStr = attributes.value("height");
    QString viewBoxStr = attributes.value("viewBox");

    QSvgHandler::LengthType type = QSvgHandler::PX; // FIXME: is the default correct?
    qreal width = 0;
    if (!widthStr.isEmpty()) {
        width = parseLength(widthStr, type, handler);
        if (type != QSvgHandler::PT)
            width = convertToPixels(width, true, type);
        node->setWidth(int(width), type == QSvgHandler::PERCENT);
    }
    qreal height = 0;
    if (!heightStr.isEmpty()) {
        height = parseLength(heightStr, type, handler);
        if (type != QSvgHandler::PT)
            height = convertToPixels(height, false, type);
        node->setHeight(int(height), type == QSvgHandler::PERCENT);
    }


    if (!viewBoxStr.isEmpty()) {
        QStringList lst = viewBoxStr.split(" ", QString::SkipEmptyParts);
        if (lst.count() != 4)
            lst = viewBoxStr.split(",", QString::SkipEmptyParts);
        QString x      = lst.at(0).trimmed();
        QString y      = lst.at(1).trimmed();
        QString width  = lst.at(2).trimmed();
        QString height = lst.at(3).trimmed();
        node->setViewBox(QRect(x.toInt(), y.toInt(),
                               width.toInt(), height.toInt()));
    } else if (width && height){
        if (type == QSvgHandler::PT) {
            width = convertToPixels(width, false, type);
            height = convertToPixels(height, false, type);
        }

        node->setViewBox(QRect(0, 0, (int)width, (int)height));
    }


    if (type == QSvgHandler::PT) {
        handler->setDefaultCoordinateSystem(type);
    }
    else {
        handler->setDefaultCoordinateSystem(QSvgHandler::PX);
    }

    return node;
}

static QSvgNode *createSwitchNode(QSvgNode *parent,
                                  const QXmlAttributes &attributes,
                                  QSvgHandler *)
{
    Q_UNUSED(attributes);
    QSvgSwitch *node = new QSvgSwitch(parent);
    return node;
}

static bool parseTbreakNode(QSvgNode *parent,
                            const QXmlAttributes &attributes,
                            QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createTextNode(QSvgNode *parent,
                                const QXmlAttributes &attributes,
                                QSvgHandler *handler)
{
    QString x = attributes.value("x");
    QString y = attributes.value("y");
    //### editable and rotate not handled
    QSvgHandler::LengthType type;
    qreal nx = parseLength(x, type, handler);
    qreal ny = parseLength(y, type, handler);

    //### not to pixels but to the default coordinate system
    //    and text should be already in the correct coordinate
    //    system here
    //nx = convertToPixels(nx, true, type);
    //ny = convertToPixels(ny, true, type);

    QSvgNode *text = new QSvgText(parent, QPointF(nx, ny));
    return text;
}

static QSvgNode *createTextAreaNode(QSvgNode *parent,
                                    const QXmlAttributes &attributes,
                                    QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return 0;
}

static bool parseTitleNode(QSvgNode *parent,
                           const QXmlAttributes &attributes,
                           QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseTspanNode(QSvgNode *parent,
                           const QXmlAttributes &attributes,
                           QSvgHandler *handler)
{
    return parseDefaultTextStyle(parent, attributes, false, handler);
}

static QSvgNode *createUseNode(QSvgNode *parent,
                               const QXmlAttributes &attributes,
                               QSvgHandler *)
{
    QString linkId = attributes.value("xlink:href").remove(0, 1);
    QSvgStructureNode *group = 0;
    switch (parent->type()) {
    case QSvgNode::DOC:
    case QSvgNode::DEFS:
    case QSvgNode::G:
    case QSvgNode::SWITCH:
        group = static_cast<QSvgStructureNode*>(parent);
        break;
    default:
        break;
    }

    if (group) {
        QSvgNode *link = group->scopeNode(linkId);
        if (link) {
            //delay link resolving till the first draw call on
            //use nodes, link might have not been created yet
            QSvgUse *node = new QSvgUse(parent, link);
            return node;
        }
    }

    qWarning("link %s hasn't been detected!", qPrintable(linkId));
    return 0;
}

static QSvgNode *createVideoNode(QSvgNode *parent,
                                 const QXmlAttributes &attributes,
                                 QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return 0;
}



QHash<QString, FactoryMethod> QSvgHandler::s_groupFactory;
QHash<QString, FactoryMethod> QSvgHandler::s_graphicsFactory;
QHash<QString, ParseMethod>   QSvgHandler::s_utilFactory;
QHash<QString, StyleFactoryMethod> QSvgHandler::s_styleFactory;
QHash<QString, StyleParseMethod>   QSvgHandler::s_styleUtilFactory;

//static const char *SVG_NAMESPACE = "http://www.w3.org/2000/svg";

QSvgHandler::QSvgHandler()
    : m_doc(0), m_style(0), m_defaultCoords(PX)
{
    if (s_groupFactory.isEmpty()) {
        defaultPen.setMiterLimit(4);
        init();
    }
}

bool QSvgHandler::startElement(const QString &namespaceURI,
                               const QString &localName,
                               const QString &,
                               const QXmlAttributes &attributes)
{
    Q_UNUSED(namespaceURI);
    QSvgNode *node = 0;
    //qDebug()<<"localName = "<<localName;

    if (m_colorTagCount.count()) {
        int top = m_colorTagCount.pop();
        ++top;
        m_colorTagCount.push(top);
    }

    if (s_groupFactory.contains(localName)) {
        //group
        m_style = 0;
        node = s_groupFactory[localName](
            m_doc ? m_nodes.top() : 0, attributes, this);
        Q_ASSERT(node);
        if (!m_doc) {
            Q_ASSERT(node->type() == QSvgNode::DOC);
            m_doc = static_cast<QSvgTinyDocument*>(node);
        } else {
            switch (m_nodes.top()->type()) {
            case QSvgNode::DOC:
            case QSvgNode::G:
            case QSvgNode::DEFS:
            case QSvgNode::SWITCH:
            {
                QSvgStructureNode *group =
                    static_cast<QSvgStructureNode*>(m_nodes.top());
                group->addChild(node, attributes.value("id"));
            }
                break;
            default:
                break;
            }
        }
        parseCoreNode(node, attributes);
        parseStyle(node, attributes, this);
    } else if (s_graphicsFactory.contains(localName)) {
        //rendering element
        Q_ASSERT(!m_nodes.isEmpty());
        m_style = 0;
        node = s_graphicsFactory[localName](m_nodes.top(), attributes, this);
        if (node) {
            switch (m_nodes.top()->type()) {
            case QSvgNode::DOC:
            case QSvgNode::G:
            case QSvgNode::DEFS:
            case QSvgNode::SWITCH:
            {
                QSvgStructureNode *group =
                    static_cast<QSvgStructureNode*>(m_nodes.top());
                group->addChild(node, attributes.value("id"));
            }
                break;
            default:
                Q_ASSERT(!"not a grouping element is the parent");
            }

            parseCoreNode(node, attributes);
            if (node->type() != QSvgNode::TEXT)
                parseStyle(node, attributes, this);
            else
                parseDefaultTextStyle(node, attributes, true, this);
        }
    } else if (s_utilFactory.contains(localName)) {
        Q_ASSERT(!m_nodes.isEmpty());
        m_style = 0;
        if (!s_utilFactory[localName](m_nodes.top(), attributes, this)) {
            qWarning("Problem parsing %s", qPrintable(localName));
        }
    } else if (s_styleFactory.contains(localName)) {
        QSvgStyleProperty *prop = s_styleFactory[localName](
            m_nodes.top(), attributes, this);
        if (prop) {
            QString id = attributes.value("id");
            if (id.isEmpty())
                id = attributes.value("xml:id");
            m_style = prop;
            m_nodes.top()->appendStyleProperty(prop, id, true);
        } else {
            qWarning("Couldn't parse node: %s", qPrintable(localName));
        }
    } else if (s_styleUtilFactory.contains(localName)) {
        if (m_style) {
            if (!s_styleUtilFactory[localName](m_style, attributes, this)) {
                qWarning("Problem parsing %s", qPrintable(localName));
            }
        }
    } else {
        //qWarning()<<"Skipping unknown element!"<<namespaceURI<<"::"<<localName;
        m_skipNodes.push(Unknown);
        return true;
    }

    if (node) {
        m_nodes.push(node);
        m_skipNodes.push(Graphics);
    } else {
        //qDebug()<<"Skipping "<<localName;
        m_skipNodes.push(Style);
    }
    return true;
}

bool QSvgHandler::endElement(const QString &namespaceURI,
                             const QString &qName,
                             const QString &)
{
    Q_UNUSED(namespaceURI); Q_UNUSED(qName);
    CurrentNode node = m_skipNodes.top();
    m_skipNodes.pop();

    if (m_colorTagCount.count()) {
        int top = m_colorTagCount.pop();
        --top;
        if (!top) {
            m_colorStack.pop();
        } else {
            m_colorTagCount.push(top);
        }
    }

    if (node == Unknown) {
        //qDebug()<<"Skipping "<< qName<< ":"<<namespaceURI;
        return true;
    }

    if (m_nodes.top()->type() == QSvgNode::TEXT) {
        QSvgText *node = static_cast<QSvgText*>(m_nodes.top());
        node->popFormat();
    }

    if (node == Graphics)
        m_nodes.pop();
    return true;
}

bool QSvgHandler::characters(const QString &str)
{
    if (m_skipNodes.top() == Unknown)
        return true;

    QString text = xmlSimplify(str);
    if (text.isEmpty())
        return true;

    if (m_nodes.top()->type() == QSvgNode::TEXT) {
        QSvgText *node = static_cast<QSvgText*>(m_nodes.top());
        node->insertText(text);
    }

    return true;
}

bool QSvgHandler::fatalError(const QXmlParseException &exception)
{
    qWarning("Fatal error on line %i, column %i: %s",
             exception.lineNumber(), exception.columnNumber(),
             qPrintable(exception.message()));

    return true;
}

QString QSvgHandler::errorString() const
{
    return QString("error");
}

void QSvgHandler::init()
{
    Q_ASSERT(s_groupFactory.isEmpty());

    s_utilFactory.insert("a", (ParseMethod) parseAnchorNode);
    s_utilFactory.insert("animate", (ParseMethod) parseAnimateNode);
    s_utilFactory.insert("animateColor", (ParseMethod) parseAnimateColorNode);
    s_utilFactory.insert("animateMotion", (ParseMethod) parseAimateMotionNode);
    s_utilFactory.insert("animateTransform", (ParseMethod) parseAnimateTransformNode);
    s_utilFactory.insert("audio", (ParseMethod) parseAudioNode);
    s_utilFactory.insert("desc", (ParseMethod) parseDescNode);
    s_utilFactory.insert("discard", (ParseMethod) parseDiscardNode);
    s_utilFactory.insert("foreignObject", (ParseMethod) parseForeignObjectNode);
    s_utilFactory.insert("handler", (ParseMethod) parseHandlerNode);
    s_utilFactory.insert("hkern", (ParseMethod) parseHkernNode);
    s_utilFactory.insert("metadata", (ParseMethod) parseMetadataNode);
    s_utilFactory.insert("mpath", (ParseMethod) parseMpathNode);
    s_utilFactory.insert("prefetch", (ParseMethod) parsePrefetchNode);
    s_utilFactory.insert("script", (ParseMethod) parseScriptNode);
    s_utilFactory.insert("set", (ParseMethod) parseSetNode);
    s_utilFactory.insert("style", (ParseMethod) parseStyleNode);
    s_utilFactory.insert("tBreak", (ParseMethod) parseTbreakNode);
    s_utilFactory.insert("title", (ParseMethod) parseTitleNode);
    s_utilFactory.insert("tspan", (ParseMethod) (ParseMethod) parseTspanNode);

    s_groupFactory.insert("svg", (FactoryMethod) createSvgNode);
    s_groupFactory.insert("g", (FactoryMethod) createGNode);
    s_groupFactory.insert("defs", (FactoryMethod) createDefsNode);
    s_groupFactory.insert("switch", (FactoryMethod) createSwitchNode);

    s_graphicsFactory.insert("animation", (FactoryMethod) createAnimationNode);
    s_graphicsFactory.insert("circle", (FactoryMethod) createCircleNode);
    s_graphicsFactory.insert("ellipse", (FactoryMethod) createEllipseNode);
    s_graphicsFactory.insert("image", (FactoryMethod) createImageNode);
    s_graphicsFactory.insert("line", (FactoryMethod) createLineNode);
    s_graphicsFactory.insert("path", (FactoryMethod) createPathNode);
    s_graphicsFactory.insert("polygon", (FactoryMethod) createPolygonNode);
    s_graphicsFactory.insert("polyline", (FactoryMethod) createPolylineNode);
    s_graphicsFactory.insert("rect", (FactoryMethod) createRectNode);
    s_graphicsFactory.insert("text", (FactoryMethod) createTextNode);
    s_graphicsFactory.insert("textArea", (FactoryMethod) createTextAreaNode);
    s_graphicsFactory.insert("use", (FactoryMethod) createUseNode);
    s_graphicsFactory.insert("video", (FactoryMethod) createVideoNode);

    s_styleFactory.insert("linearGradient", (StyleFactoryMethod) createLinearGradientNode);
    s_styleFactory.insert("radialGradient", (StyleFactoryMethod) createRadialGradientNode);
    s_styleFactory.insert("font", (StyleFactoryMethod) createFontNode);
    s_styleFactory.insert("solidColor", (StyleFactoryMethod) createSolidColorNode);

    s_styleUtilFactory.insert("font-face", (StyleParseMethod) parseFontFaceNode);
    s_styleUtilFactory.insert("font-face-name", (StyleParseMethod) parseFontFaceNameNode);
    s_styleUtilFactory.insert("font-face-src", (StyleParseMethod) parseFontFaceSrcNode);
    s_styleUtilFactory.insert("font-face-uri", (StyleParseMethod) parseFontFaceUriNode);
    s_styleUtilFactory.insert("glyph", (StyleParseMethod) parseGlyphNode);
    s_styleUtilFactory.insert("missing-glyph", (StyleParseMethod) parseMissingGlyphNode);
    s_styleUtilFactory.insert("stop", (StyleParseMethod) parseStopNode);
}

QSvgTinyDocument * QSvgHandler::document() const
{
    return m_doc;
}

QSvgHandler::LengthType QSvgHandler::defaultCoordinateSystem() const
{
    return m_defaultCoords;
}

void QSvgHandler::setDefaultCoordinateSystem(LengthType type)
{
    m_defaultCoords = type;
}

void QSvgHandler::pushColor(const QColor &color)
{
    m_colorStack.push(color);
    m_colorTagCount.push(1);
}

QColor QSvgHandler::currentColor() const
{
    if (!m_colorStack.isEmpty())
        return m_colorStack.top();
    else
        return QColor(0, 0, 0);
}
