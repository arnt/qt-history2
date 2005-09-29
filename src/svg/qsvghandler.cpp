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

#include "qapplication.h"
#include "qwidget.h"
#include "qpen.h"
#include "qpainterpath.h"
#include "qbrush.h"
#include "qtextformat.h"
#include "qdebug.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const QPen defaultPen(Qt::black, 1, Qt::SolidLine,
                             Qt::FlatCap, Qt::MiterJoin);

static QString xmlSimplify(const QString &str)
{
    QString dummy = str;
    dummy.remove('\n');
    if (dummy.trimmed().isEmpty())
        return QString();
    QString temp;
    QString::const_iterator itr = dummy.begin();
    bool wasSpace = false;
    for (;itr != dummy.end(); ++itr) {
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
    QString::const_iterator itr = url.begin();
    while ((*itr).isSpace())
        ++itr;
    if ((*itr) == '(')
        ++itr;
    while ((*itr).isSpace())
        ++itr;
    if ((*itr) == '#')
        ++itr;
    QString id;
    while ((*itr).isLetterOrNumber()) {
        id += *itr;
        ++itr;
    }
    return id;
}

/**
 * returns true when successfuly set the color. false signifies
 * that the color should be inherited
 */
static bool resolveColor(const QString &colorStr, QColor &color)
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
        QString::const_iterator itr = colorStr.begin();
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
               colorStr == QLatin1String("inherit") ||
               colorStr == QLatin1String("currentColor")) {
        return false;
    }

    color = QColor(colorStrTr);
    return color.isValid();
}

static bool constructColor(const QString &colorStr, const QString &opacity,
                           QColor &color)
{
    if (!resolveColor(colorStr, color))
        return false;
    if (!opacity.isEmpty()) {
        qreal op = opacity.toDouble();
        if (op <= 1)
            op *= 255;
        color.setAlpha(int(op));
    }
    return true;
}

enum LengthType
{
    PERCENT,
    PX,
    PC,
    PT,
    MM,
    CM,
    IN,
    OTHER
};

static qreal parseLength(const QString &str, LengthType &type)
{
    QString numStr = str.trimmed();
    qreal len = 0;

    if (numStr.endsWith("%")) {
        numStr.chop(1);
        len = numStr.toDouble();
        type = PERCENT;
    } else if (numStr.endsWith("px")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = PX;
    } else if (numStr.endsWith("pc")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = PC;
    } else if (numStr.endsWith("pt")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = PT;
    } else if (numStr.endsWith("mm")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = MM;
    } else if (numStr.endsWith("cm")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = CM;
    } else if (numStr.endsWith("in")) {
        numStr.chop(2);
        len = numStr.toDouble();
        type = IN;
    } else {
        len = numStr.toDouble();
        type = OTHER;
    }
    //qDebug()<<"len is "<<len<<", from "<<numStr;
    return len;
}

static qreal convertToPixels(qreal len, bool isX, LengthType type)
{
    QWidgetList widgets = QApplication::topLevelWidgets();
    QWidget *sampleWidget = widgets.first();

    if (!sampleWidget) {
        qWarning()<<"can't produce matrics without some widget";
        return 0;
    }

    qreal dpi    = isX ? sampleWidget->logicalDpiX() : sampleWidget->logicalDpiY();
    qreal mm     = isX ? sampleWidget->widthMM() : sampleWidget->heightMM();
    qreal screen = isX ? sampleWidget->width() : sampleWidget->height();

    switch (type) {
    case PERCENT:
        break;
    case PX:
        break;
    case PC:
        break;
    case PT:
        break;
    case MM:
        len *= screen/mm;
        break;
    case CM:
        len *= dpi/2.54;
        break;
    case IN:
        len *= dpi;
        break;
    case OTHER:
        break;
    default:
        break;
    }
    return len;
}


static void parseColor(QSvgNode *node,
                       const QXmlAttributes &attributes)
{
    QString colorStr = attributes.value("color");
    QString opacity  = attributes.value("color-opacity");
    QColor color;
    if (constructColor(colorStr, opacity, color)) {
        QSvgStyleProperty *prop = new QSvgFillStyle(QBrush(color));
        node->appendStyleProperty(prop, QString());
        // SVG 1.1 Conformance test painting-fill-02-t.svg
        // makes it seem color only sets the fill not the stroke
        //prop = new QSvgStrokeStyle(QPen(color));
        //node->appendStyleProperty(prop, QString());
    }
}

static void parseBrush(QSvgNode *node,
                       const QXmlAttributes &attributes)
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
            while (dummy && (dummy->type() != QSvgNode::DOC &&
                             dummy->type() != QSvgNode::G   &&
                             dummy->type() != QSvgNode::DEFS)) {
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
                    qWarning()<<"Couldn't resolve property: "<<id;
                }
            }
        } else if (value != QLatin1String("none")) {
            QString opacity = attributes.value("fill-opacity");
            QString fillRule = attributes.value("fill-rule");
            QColor color;
            if (constructColor(value, opacity, color)) {
                QSvgStyleProperty *prop = new QSvgFillStyle(QBrush(color));
                node->appendStyleProperty(prop, myId);
            }
        } else {
            QSvgStyleProperty *prop = new QSvgFillStyle(QBrush(Qt::NoBrush));
            node->appendStyleProperty(prop, myId);
        }
    }
}

static void parsePen(QSvgNode *node,
                     const QXmlAttributes &attributes)
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
    if (!value.isEmpty() || !width.isEmpty()) {
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
                    while (dummy && (dummy->type() != QSvgNode::DOC &&
                                     dummy->type() != QSvgNode::G   &&
                                     dummy->type() != QSvgNode::DEFS)) {
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
                    if (constructColor(value, opacity, color))
                        pen.setColor(color);
                }
                //since we could inherit stroke="none"
                //we need to reset the style of our stroke to something
                pen.setStyle(Qt::SolidLine);
            }
            if (!width.isEmpty())
                pen.setWidthF(width.toDouble());

            if (!linejoin.isEmpty()) {
                if (linejoin == "miter")
                    pen.setJoinStyle(Qt::MiterJoin);
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

            // ### we need custom pen styles for this to work
            // 100% correctly
            if (!dashArray.isEmpty()) {
                QString::const_iterator itr = dashArray.begin();
                QList<qreal> dashes = parseNumbersList(itr);
                qreal pen_width = pen.widthF();
                int scale =  qRound(pen_width < 1 ? 1 : pen_width);
                //int space = (pen_width < 1 ? 1 : (2 * scale));
                int dot = 1 * scale;
                int dash = 4 * scale;
                if (dashes.size() == 6) {
                    if (dashes.at(0) != dashes.at(2))
                        pen.setStyle(Qt::DashDotDotLine);
                    else {
                        qreal dotDif = qAbs(dashes.at(0) - dot);
                        qreal dashDif = qAbs(dashes.at(0) - dash);
                        if (dotDif < dashDif)
                            pen.setStyle(Qt::DashLine);
                        else
                            pen.setStyle(Qt::DotLine);
                    }
                } else if (dashes.size() >= 2) {
                    qreal dotDif = qAbs(dashes.at(0) - dot);
                    qreal dashDif = qAbs(dashes.at(0) - dash);
                    if (dotDif < dashDif)
                        pen.setStyle(Qt::DashLine);
                    else
                        pen.setStyle(Qt::DotLine);
                }
            }

            node->appendStyleProperty(new QSvgStrokeStyle(pen), myId);
        } else {
            QPen pen(defaultPen);
            pen.setStyle(Qt::NoPen);
            node->appendStyleProperty(new QSvgStrokeStyle(pen), myId);
        }
    }
}


static bool parseQBrush(const QXmlAttributes &attributes,
                        QBrush &brush)
{
    QString value = attributes.value("fill");
    QString opacity = attributes.value("fill-opacity");
    QColor color;
    if (!value.isEmpty() || !opacity.isEmpty()) {
        if (constructColor(value, opacity, color)) {
            brush.setStyle(Qt::SolidPattern);
            brush.setColor(color);
            return true;
        }
    }
    return false;
}

static bool parseQFont(const QXmlAttributes &attributes,
                       QFont &font)
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
            LengthType type;
            qreal len = parseLength(size, type);
            //len = convertToPixels(len, false, type);
            if (type == PX || type == OTHER)
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
                      const QXmlAttributes &attributes)
{
    QFont font;
    QSvgFontStyle *inherited =
        static_cast<QSvgFontStyle*>(node->parent()->styleProperty(
                                        QSvgStyleProperty::FONT));
    if (inherited)
        font = inherited->qfont();
    if (parseQFont(attributes, font)) {
        QString myId = attributes.value("id");
        node->appendStyleProperty(new QSvgFontStyle(font), myId);
    }
}

static void parseTransform(QSvgNode *node,
                           const QXmlAttributes &attributes)
{
    QString value = attributes.value("transform");
    QString myId = attributes.value("id");
    value = value.trimmed();
    QMatrix matrix;
    QString::const_iterator itr = value.begin();

    while (itr != value.end()) {
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
        if (itr != value.end())
            ++itr;
    }

    if (!matrix.isIdentity()) {
        node->appendStyleProperty(new QSvgTransformStyle(matrix), myId);
    }

}


static void path_arc_segment(QPainterPath &path,
                             double xc, double yc,
                             double th0, double th1,
                             double rx, double ry, double x_axis_rotation)
{
    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x1, y1, x2, y2, x3, y3;
    double t;
    double th_half;

    sin_th = sin (x_axis_rotation * (M_PI / 180.0));
    cos_th = cos (x_axis_rotation * (M_PI / 180.0));

    a00 = cos_th * rx;
    a01 = -sin_th * ry;
    a10 = sin_th * rx;
    a11 = cos_th * ry;

    th_half = 0.5 * (th1 - th0);
    t = (8.0 / 3.0) * sin (th_half * 0.5) * sin (th_half * 0.5) / sin (th_half);
    x1 = xc + cos (th0) - t * sin (th0);
    y1 = yc + sin (th0) + t * cos (th0);
    x3 = xc + cos (th1);
    y3 = yc + sin (th1);
    x2 = x3 + t * sin (th1);
    y2 = y3 - t * cos (th1);

    path.cubicTo(a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
                 a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
                 a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}

// the code underneath is from XSVG
static void path_arc(QPainterPath &path,
                     double		rx,
                     double		ry,
                     double		x_axis_rotation,
                     int		large_arc_flag,
                     int		sweep_flag,
                     double		x,
                     double		y,
                     double curx, double cury)
{
    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x0, y0, x1, y1, xc, yc;
    double d, sfactor, sfactor_sq;
    double th0, th1, th_arc;
    int i, n_segs;
    double dx, dy, dx1, dy1, Pr1, Pr2, Px, Py, check;

    rx = fabs (rx);
    ry = fabs (ry);

    sin_th = sin (x_axis_rotation * (M_PI / 180.0));
    cos_th = cos (x_axis_rotation * (M_PI / 180.0));

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
    if(check > 1)
    {
        rx = rx * sqrt(check);
        ry = ry * sqrt(check);
    }

    a00 = cos_th / rx;
    a01 = sin_th / rx;
    a10 = -sin_th / ry;
    a11 = cos_th / ry;
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
    sfactor = sqrt (sfactor_sq);
    if (sweep_flag == large_arc_flag) sfactor = -sfactor;
    xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
    yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
    /* (xc, yc) is center of the circle. */

    th0 = atan2 (y0 - yc, x0 - xc);
    th1 = atan2 (y1 - yc, x1 - xc);

    th_arc = th1 - th0;
    if (th_arc < 0 && sweep_flag)
        th_arc += 2 * M_PI;
    else if (th_arc > 0 && !sweep_flag)
        th_arc -= 2 * M_PI;

    n_segs = int(ceil (fabs (th_arc / (M_PI * 0.5 + 0.001))));

    for (i = 0; i < n_segs; i++) {
        path_arc_segment(path, xc, yc,
                         th0 + i * th_arc / n_segs,
                         th0 + (i + 1) * th_arc / n_segs,
                         rx, ry, x_axis_rotation);
    }

}

static bool parsePathDataFast(const QString &data, QPainterPath &path)
{
    QString::const_iterator itr = data.begin();
    double x0 = 0, y0 = 0;              // starting point
    double x = 0, y = 0;                // current point
    char lastMode = 0;
    QChar pathElem;
    QPointF ctrlPt;

    while (itr != data.end()) {
        while ((*itr).isSpace())
            ++itr;
        pathElem = *itr;
        ++itr;
        QList<qreal> arg = parseNumbersList(itr);
        if (pathElem == 'z' || pathElem == 'Z')
            arg.append(0);//dummy
        while (!arg.isEmpty()) {
            double offsetX = x;        // correction offsets
            double offsetY = y;        // for relative commands
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
                double rx = arg[0];
                double ry = arg[1];
                double xAxisRotation = arg[2];
                double largeArcFlag  = arg[3];
                double sweepFlag = arg[4];
                double ex = arg[5] + offsetX;
                double ey = arg[6] + offsetY;
                double curx = x;
                double cury = y;
                path_arc(path, rx, ry, xAxisRotation, int(largeArcFlag),
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
                double rx = arg[0];
                double ry = arg[1];
                double xAxisRotation = arg[2];
                double largeArcFlag  = arg[3];
                double sweepFlag = arg[4];
                double ex = arg[5];
                double ey = arg[6];
                double curx = x;
                double cury = y;
                path_arc(path, rx, ry, xAxisRotation, int(largeArcFlag),
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
                       const QXmlAttributes &attributes);

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
                                  bool initial)
{
    Q_ASSERT(node->type() == QSvgText::TEXT);
    QSvgText *textNode = static_cast<QSvgText*>(node);
    QXmlAttributes attrs = attributes;
    QString css = attrs.value("style");
    parseCSStoXMLAttrs(css, attrs);

    QString anchor = attrs.value("text-anchor");

    QTextCharFormat format;
    QFont font;
    QBrush brush;
    if (!initial) {
        font = textNode->topFormat().font();
        brush = textNode->topFormat().foreground();
    }
    if (initial) {
        QSvgFontStyle *fontStyle = static_cast<QSvgFontStyle*>(
            node->parent()->styleProperty(QSvgStyleProperty::FONT));
        if (fontStyle)
            font = fontStyle->qfont();
    }
    if (parseQFont(attrs, font) || initial) {
        if (font.pixelSize() != -1)
            format.setProperty(QTextFormat::FontPixelSize, font.pixelSize());
        format.setFont(font);
    }

    if (initial) {
        QSvgFillStyle *fillStyle = static_cast<QSvgFillStyle*>(
            node->styleProperty(QSvgStyleProperty::FILL));
        if (fillStyle)
            fillStyle->qbrush();
    }
    if (parseQBrush(attrs, brush) || initial) {
        format.setForeground(brush);
    }

    if (initial) {
        Qt::Alignment align = Qt::AlignLeft;
        if (anchor == "middle")
            align = Qt::AlignHCenter;
        else if (anchor == "end")
            align = Qt::AlignRight;
        textNode->setTextAlignment(align);
    }
    parseTransform(node, attributes);

    textNode->insertFormat(format);

    return true;
}

static bool parseCSSStyle(QSvgNode *node,
                          const QString &css)
{
    QXmlAttributes attributes;
    if (parseCSStoXMLAttrs(css, attributes))
        parseStyle(node, attributes);

    return true;
}

static bool parseStyle(QSvgNode *node,
                       const QXmlAttributes &attributes)
{
    if (!attributes.value("style").isEmpty()) {
        parseCSSStyle(node, attributes.value("style"));
    }
    parseColor(node, attributes);
    parseBrush(node, attributes);
    parsePen(node, attributes);
    parseFont(node, attributes);
    parseTransform(node, attributes);

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

    value = attributes.value("visibility");
#endif
    return true;
}

static bool parseAnchorNode(QSvgNode *parent,
                            const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseAnimateNode(QSvgNode *parent,
                             const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseAnimateColorNode(QSvgNode *parent,
                                  const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseAimateMotionNode(QSvgNode *parent,
                                  const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseAnimateTransformNode(QSvgNode *parent,
                                      const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode * createAnimationNode(QSvgNode *parent,
                                      const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return 0;
}

static bool parseAudioNode(QSvgNode *parent,
                           const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createCircleNode(QSvgNode *parent,
                                  const QXmlAttributes &attributes)
{
    QString cx      = attributes.value("cx");
    QString cy      = attributes.value("cy");
    QString r       = attributes.value("r");
    double ncx = cx.toDouble();
    double ncy = cy.toDouble();
    double nr  = r.toDouble();

    QRectF rect(ncx-nr, ncy-nr, nr*2, nr*2);
    QSvgNode *circle = new QSvgCircle(parent, rect);
    return circle;
}

static QSvgNode *createDefsNode(QSvgNode *parent,
                                const QXmlAttributes &attributes)
{
    Q_UNUSED(attributes);
    QSvgDefs *defs = new QSvgDefs(parent);
    return defs;
}

static bool parseDescNode(QSvgNode *parent,
                          const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseDiscardNode(QSvgNode *parent,
                             const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createEllipseNode(QSvgNode *parent,
                                   const QXmlAttributes &attributes)
{
    QString cx      = attributes.value("cx");
    QString cy      = attributes.value("cy");
    QString rx      = attributes.value("rx");
    QString ry      = attributes.value("ry");
    double ncx = cx.toDouble();
    double ncy = cy.toDouble();
    double nrx = rx.toDouble();
    double nry = ry.toDouble();

    QRectF rect(ncx-nrx, ncy-nry, nrx*2, nry*2);
    QSvgNode *ellipse = new QSvgEllipse(parent, rect);
    return ellipse;
}

static QSvgStyleProperty *createFontNode(QSvgNode *parent,
                                        const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return 0;
}

static bool parseFontFaceNode(QSvgStyleProperty *parent,
                              const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseFontFaceNameNode(QSvgStyleProperty *parent,
                                  const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseFontFaceSrcNode(QSvgStyleProperty *parent,
                                 const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseFontFaceUriNode(QSvgStyleProperty *parent,
                                 const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseForeignObjectNode(QSvgNode *parent,
                                   const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createGNode(QSvgNode *parent,
                             const QXmlAttributes &attributes)
{
    Q_UNUSED(attributes);
    QSvgG *node = new QSvgG(parent);
    return node;
}

static bool parseGlyphNode(QSvgStyleProperty *parent,
                           const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseHandlerNode(QSvgNode *parent,
                             const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseHkernNode(QSvgNode *parent,
                           const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createImageNode(QSvgNode *parent,
                                 const QXmlAttributes &attributes)
{
    QString x = attributes.value("x");
    QString y = attributes.value("y");
    QString width  = attributes.value("width");
    QString height = attributes.value("height");
    QString filename = attributes.value("xlink:href");
    double nx = x.toDouble();
    double ny = y.toDouble();
    LengthType type;
    double nwidth = parseLength(width, type);
    nwidth = convertToPixels(nwidth, true, type);

    double nheight = parseLength(width, type);
    nheight = convertToPixels(nwidth, false, type);
    QImage image(filename);

    if (image.isNull())
        return 0;

    QSvgNode *img = new QSvgImage(parent,
                                  image,
                                  QRect(int(nx),
                                        int(ny),
                                        int(nwidth),
                                        int(nheight)));
    return img;
}

static QSvgNode *createLineNode(QSvgNode *parent,
                                const QXmlAttributes &attributes)
{
    QString x1 = attributes.value("x1");
    QString y1 = attributes.value("y1");
    QString x2 = attributes.value("x2");
    QString y2 = attributes.value("y2");
    double nx1 = x1.toDouble();
    double ny1 = y1.toDouble();
    double nx2 = x2.toDouble();
    double ny2 = y2.toDouble();

    QLineF lineBounds(nx1, ny1, nx2, ny2);
    QSvgNode *line = new QSvgLine(parent, lineBounds);
    return line;
}

static QSvgStyleProperty *createLinearGradientNode(QSvgNode *node,
                                                   const QXmlAttributes &attributes)
{
    QString x1 = attributes.value("x1");
    QString y1 = attributes.value("y1");
    QString x2 = attributes.value("x2");
    QString y2 = attributes.value("y2");
    QString link = attributes.value("xlink:href");
    qreal nx1 = x1.toDouble();
    qreal ny1 = y1.toDouble();
    qreal nx2 = x2.toDouble();
    qreal ny2 = y2.toDouble();
    bool  needsResolving = (nx2==0 && ny2==0);

    QSvgNode *itr = node;
    while (itr && itr->type() != QSvgNode::DOC) {
        itr = itr->parent();
    }
    QSvgTinyDocument *doc = static_cast<QSvgTinyDocument*>(itr);
    Q_ASSERT(doc);
    if (nx2 == 0)
        nx2 = doc->viewBox().width();
    if (ny2 == 0)
        ny2 = doc->viewBox().height();

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
                              const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseMissingGlyphNode(QSvgStyleProperty *parent,
                                  const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseMpathNode(QSvgNode *parent,
                           const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createPathNode(QSvgNode *parent,
                                const QXmlAttributes &attributes)
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
                                   const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    QString pointsStr  = attributes.value("points");

    //same QPolygon parsing is in createPolylineNode
    QString::const_iterator sitr = pointsStr.begin();
    QList<qreal> points = parseNumbersList(sitr);
    QPolygonF poly(points.count()/2);
    int i = 0;
    QList<qreal>::const_iterator itr = points.begin();
    while (itr != points.end()) {
        qreal one = *itr; ++itr;
        qreal two = *itr; ++itr;
        poly[i] = QPointF(one, two);
        ++i;
    }
    QSvgNode *polygon = new QSvgPolygon(parent, poly);
    return polygon;
}

static QSvgNode *createPolylineNode(QSvgNode *parent,
                                    const QXmlAttributes &attributes)
{
    QString pointsStr  = attributes.value("points");

    //same QPolygon parsing is in createPolygonNode
    QString::const_iterator sitr = pointsStr.begin();
    QList<qreal> points = parseNumbersList(sitr);
    QPolygonF poly(points.count()/2);
    int i = 0;
    QList<qreal>::const_iterator itr = points.begin();
    while (itr != points.end()) {
        qreal one = *itr; ++itr;
        qreal two = *itr; ++itr;
        poly[i] = QPointF(one, two);
        ++i;
    }

    QSvgNode *line = new QSvgPolyline(parent, poly);
    return line;
}

static bool parsePrefetchNode(QSvgNode *parent,
                              const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgStyleProperty *createRadialGradientNode(QSvgNode *,
                                                   const QXmlAttributes &attributes)
{
    QString cx = attributes.value("cx");
    QString cy = attributes.value("cy");
    QString r  = attributes.value("r");
    QString fx = attributes.value("fx");
    QString fy = attributes.value("fy");

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

    QRadialGradient *grad = new QRadialGradient(ncx, ncy, nr, nfx, nfy);
    QSvgStyleProperty *prop = new QSvgGradientStyle(grad);
    return prop;
}

static QSvgNode *createRectNode(QSvgNode *parent,
                                const QXmlAttributes &attributes)
{
    QString x      = attributes.value("x");
    QString y      = attributes.value("y");
    QString width  = attributes.value("width");
    QString height = attributes.value("height");
    QString rx      = attributes.value("rx");
    QString ry      = attributes.value("ry");

    LengthType type;
    qreal nwidth = parseLength(width, type);
    nwidth = convertToPixels(nwidth, true, type);

    qreal nheight = parseLength(height, type);
    nheight = convertToPixels(nheight, true, type);
    double nrx = rx.toDouble();
    double nry = ry.toDouble();

    QRectF bounds(x.toDouble(), y.toDouble(),
                  nwidth, nheight);

    if (nrx && !nry)
        nry = nrx;
    else if (nry && !nrx)
        nrx = nry;
    if (nrx > bounds.width()/2)
        nrx = bounds.width()/2;
    else if (nry > bounds.height()/2)
        nry = bounds.height()/2;
    QSvgNode *rect = new QSvgRect(parent, bounds,
                                  int(nrx),
                                  int(nry));
    return rect;
}

static bool parseScriptNode(QSvgNode *parent,
                            const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseSetNode(QSvgNode *parent,
                         const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgStyleProperty *createSolidColorNode(QSvgNode *parent,
                                              const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    QString solidColorStr = attributes.value("solid-color");
    QString solidOpacityStr = attributes.value("solid-opacity");
    QColor color;
    if (!constructColor(solidColorStr, solidOpacityStr, color))
        return 0;
    QSvgSolidColorStyle *style = new QSvgSolidColorStyle(color);
    return style;
}

static bool parseStopNode(QSvgStyleProperty *parent,
                          const QXmlAttributes &attributes)
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
    LengthType type;
    qreal offset = parseLength(offsetStr, type);
    offset = convertToPixels(offset, true, type);
    if (type == PERCENT) {
        offset = offset/100.0;
    }
    bool colorOK = constructColor(colorStr, opacityStr, color);
    QGradient *grad = style->qgradient();
//    qDebug()<<"set color at"<<offset<<color;
    grad->setColorAt(offset, color);
    if (!colorOK)
        style->addResolve(offset);
    return true;
}

static bool parseStyleNode(QSvgNode *parent,
                           const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createSvgNode(QSvgNode *parent,
                               const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);

    QString baseProfile = attributes.value("baseProfile");

    if (baseProfile.isEmpty() && baseProfile != QLatin1String("tiny")) {
        qWarning()<<"Profile is "<<baseProfile<<", while we only support tiny!";
    }

    QSvgTinyDocument *node = new QSvgTinyDocument();
    QString widthStr  = attributes.value("width");
    QString heightStr = attributes.value("height");
    QString viewBoxStr = attributes.value("viewBox");


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
    }
    LengthType type;
    if (!widthStr.isEmpty()) {
        qreal width = parseLength(widthStr, type);
        width = convertToPixels(width, true, type);
        node->setWidth(int(width), type == PERCENT);
    }
    if (!heightStr.isEmpty()) {
        qreal height = parseLength(heightStr, type);
        height = convertToPixels(height, false, type);
        node->setHeight(int(height), type == PERCENT);
    }

    return node;
}

static bool parseSwitchNode(QSvgNode *parent,
                            const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseTbreakNode(QSvgNode *parent,
                                  const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createTextNode(QSvgNode *parent,
                                const QXmlAttributes &attributes)
{
    QString x = attributes.value("x");
    QString y = attributes.value("y");
    //### editable and rotate not handled
    LengthType type;
    qreal nx = parseLength(x, type);
    nx = convertToPixels(nx, true, type);
    qreal ny = parseLength(y, type);
    ny = convertToPixels(ny, true, type);

    QSvgNode *text = new QSvgText(parent, QPointF(nx, ny));
    return text;
}

static QSvgNode *createTextAreaNode(QSvgNode *parent,
                                    const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return 0;
}

static bool parseTitleNode(QSvgNode *parent,
                           const QXmlAttributes &attributes)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseTspanNode(QSvgNode *parent,
                           const QXmlAttributes &attributes)
{
    return parseDefaultTextStyle(parent, attributes, false);
}

static QSvgNode *createUseNode(QSvgNode *parent,
                               const QXmlAttributes &attributes)
{
    QString linkId = attributes.value("xlink:href").remove(0, 1);
    QSvgStructureNode *group = 0;
    switch (parent->type()) {
    case QSvgNode::DOC:
    case QSvgNode::DEFS:
    case QSvgNode::G:
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

    qWarning()<<"link "<<linkId<<" hasn't been detected!";
    return 0;
}

static QSvgNode *createVideoNode(QSvgNode *parent,
                                 const QXmlAttributes &attributes)
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
    : m_doc(0), m_style(0)
{
    if (s_groupFactory.isEmpty())
        init();
}

bool QSvgHandler::startElement(const QString &namespaceURI,
                               const QString &localName,
                               const QString &,
                               const QXmlAttributes &attributes)
{
    Q_UNUSED(namespaceURI);
    QSvgNode *node = 0;
    //qDebug()<<"localName = "<<localName;
    if (s_groupFactory.contains(localName)) {
        //group
        m_style = 0;
        node = s_groupFactory[localName](
            m_doc ? m_nodes.top() : 0, attributes);
        Q_ASSERT(node);
        if (!m_doc) {
            Q_ASSERT(node->type() == QSvgNode::DOC);
            m_doc = static_cast<QSvgTinyDocument*>(node);
        } else {
            switch (m_nodes.top()->type()) {
            case QSvgNode::DOC:
            case QSvgNode::G:
            case QSvgNode::DEFS: {
                QSvgStructureNode *group =
                    static_cast<QSvgStructureNode*>(m_nodes.top());
                group->addChild(node, attributes.value("id"));
            }
                break;
            default:
                break;
            }
        }
        parseStyle(node, attributes);
    } else if (s_graphicsFactory.contains(localName)) {
        //rendering element
        Q_ASSERT(!m_nodes.isEmpty());
        m_style = 0;
        node = s_graphicsFactory[localName](m_nodes.top(), attributes);
        if (node) {
            switch (m_nodes.top()->type()) {
            case QSvgNode::DOC:
            case QSvgNode::G:
            case QSvgNode::DEFS: {
                QSvgStructureNode *group =
                    static_cast<QSvgStructureNode*>(m_nodes.top());
                group->addChild(node, attributes.value("id"));
            }
                break;
            default:
                Q_ASSERT(!"not a grouping element is the parent");
            }
            if (node->type() != QSvgNode::TEXT)
                parseStyle(node, attributes);
            else
                parseDefaultTextStyle(node, attributes, true);
        }
    } else if (s_utilFactory.contains(localName)) {
        Q_ASSERT(!m_nodes.isEmpty());
        m_style = 0;
        if (!s_utilFactory[localName](m_nodes.top(), attributes)) {
            qWarning()<<"Problem parsing "<<localName;
        }
    } else if (s_styleFactory.contains(localName)) {
        QSvgStyleProperty *prop = s_styleFactory[localName](
            m_nodes.top(), attributes);
        if (prop) {
            QString id = attributes.value("id");
            m_nodes.top()->appendStyleProperty(prop, id, true);
            m_style = prop;
        } else {
            qWarning()<<"Couldn't parse node: "<<localName;
        }
    } else if (s_styleUtilFactory.contains(localName)) {
        if (m_style) {
            if (!s_styleUtilFactory[localName](m_style, attributes)) {
                qWarning()<<"Problem parsing "<<localName;
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
    qWarning() << "Fatal error on line" << exception.lineNumber()
               << ", column" << exception.columnNumber() << ":"
               << exception.message();

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
    s_utilFactory.insert("switch", (ParseMethod) parseSwitchNode);
    s_utilFactory.insert("tBreak", (ParseMethod) parseTbreakNode);
    s_utilFactory.insert("title", (ParseMethod) parseTitleNode);
    s_utilFactory.insert("tspan", (ParseMethod) (ParseMethod) parseTspanNode);

    s_groupFactory.insert("svg", (FactoryMethod) createSvgNode);
    s_groupFactory.insert("g", (FactoryMethod) createGNode);
    s_groupFactory.insert("defs", (FactoryMethod) createDefsNode);

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
