/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "ui3reader.h"
#include "parser.h"
#include "domtool.h"
#include <qregexp.h>
#include <qstringlist.h>
#include <globaldefs.h>
#include <qdebug.h>

/*!
  Creates a set-call for property \a exclusiveProp of the object
  given in \a e.

  If the object does not have this property, the function does nothing.

  Exclusive properties are used to generate the implementation of
  application font or palette change handlers in createFormImpl().

 */
void Ui3Reader::createExclusiveProperty( const QDomElement & e, const QString& exclusiveProp )
{
    QDomElement n;
    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
        return;
    QString objName = getObjectName( e );
#if 0 // it's not clear whether this check should be here or not
    if ( objName.isEmpty() )
        return;
#endif
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
        if ( n.tagName() == "property" ) {
            bool stdset = stdsetdef;
            if ( n.hasAttribute( "stdset" ) )
                stdset = toBool( n.attribute( "stdset" ) );
            QString prop = n.attribute( "name" );
            if ( prop != exclusiveProp )
                continue;
            QString value = setObjectProperty( objClass, objName, prop, n.firstChild().toElement(), stdset );
            if ( value.isEmpty() )
                continue;
            // we assume the property isn't of type 'string'
            out << '\t' << objName << "->setProperty( \"" << prop << "\", QVariant(" << value << ") );" << endl;
        }
    }
}


/*!  Attention: this function has to be in sync with
  Resource::saveProperty() and DomTool::elementToVariant. If you
  change one, change all.
 */
QString Ui3Reader::setObjectProperty( const QString& objClass, const QString& obj, const QString &prop, const QDomElement &e, bool stdset )
{
    QString v;
    if ( e.tagName() == "rect" ) {
        QDomElement n3 = e.firstChild().toElement();
        int x = 0, y = 0, w = 0, h = 0;
        while ( !n3.isNull() ) {
            if ( n3.tagName() == "x" )
                x = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "y" )
                y = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "width" )
                w = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "height" )
                h = n3.firstChild().toText().data().toInt();
            n3 = n3.nextSibling().toElement();
        }
        v = "QRect( %1, %2, %3, %4 )";
        v = v.arg(x).arg(y).arg(w).arg(h);

    } else if ( e.tagName() == "point" ) {
        QDomElement n3 = e.firstChild().toElement();
        int x = 0, y = 0;
        while ( !n3.isNull() ) {
            if ( n3.tagName() == "x" )
                x = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "y" )
                y = n3.firstChild().toText().data().toInt();
            n3 = n3.nextSibling().toElement();
        }
        v = "QPoint( %1, %2 )";
        v = v.arg(x).arg(y);
    } else if ( e.tagName() == "size" ) {
        QDomElement n3 = e.firstChild().toElement();
        int w = 0, h = 0;
        while ( !n3.isNull() ) {
            if ( n3.tagName() == "width" )
                w = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "height" )
                h = n3.firstChild().toText().data().toInt();
            n3 = n3.nextSibling().toElement();
        }
        v = "QSize( %1, %2 )";
        v = v.arg(w).arg(h);
    } else if ( e.tagName() == "color" ) {
        QDomElement n3 = e.firstChild().toElement();
        int r = 0, g = 0, b = 0;
        while ( !n3.isNull() ) {
            if ( n3.tagName() == "red" )
                r = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "green" )
                g = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "blue" )
                b = n3.firstChild().toText().data().toInt();
            n3 = n3.nextSibling().toElement();
        }
        v = "QColor( %1, %2, %3 )";
        v = v.arg(r).arg(g).arg(b);
    } else if ( e.tagName() == "font" ) {
        QDomElement n3 = e.firstChild().toElement();
        QString attrname = e.parentNode().toElement().attribute( "name", "font" );
        QString fontname;
        if ( !obj.isEmpty() ) {
            fontname = registerObject( obj + "_" + attrname );
            out << indent << "QFont "  << fontname << "(  " << obj << "->font() );" << endl;
        } else {
            fontname = registerObject( "f" );
            out << indent << "QFont "  << fontname << "( font() );" << endl;
        }
        while ( !n3.isNull() ) {
            if ( n3.tagName() == "family" )
                out << indent << fontname << ".setFamily( \"" << n3.firstChild().toText().data() << "\" );" << endl;
            else if ( n3.tagName() == "pointsize" )
                out << indent << fontname << ".setPointSize( " << n3.firstChild().toText().data() << " );" << endl;
            else if ( n3.tagName() == "bold" )
                out << indent << fontname << ".setBold( " << mkBool( n3.firstChild().toText().data() ) << " );" << endl;
            else if ( n3.tagName() == "italic" )
                out << indent << fontname << ".setItalic( " << mkBool( n3.firstChild().toText().data() ) << " );" << endl;
            else if ( n3.tagName() == "underline" )
                out << indent << fontname << ".setUnderline( " << mkBool( n3.firstChild().toText().data() ) << " );" << endl;
            else if ( n3.tagName() == "strikeout" )
                out << indent << fontname << ".setStrikeOut( " << mkBool( n3.firstChild().toText().data() ) << " );" << endl;
            n3 = n3.nextSibling().toElement();
        }

        if ( prop == "font" ) {
            if ( !obj.isEmpty() )
                out << indent << obj << "->setFont( " << fontname << " ); " << endl;
            else
                out << indent << "setFont( " << fontname << " ); " << endl;
        } else {
            v = fontname;
        }
    } else if ( e.tagName() == "string" ) {
        QString txt = e.firstChild().toText().data();
        QString com = getComment( e.parentNode() );

        if ( prop == "toolTip" && objClass != "QAction" && objClass != "QActionGroup" ) {
            if ( !obj.isEmpty() )
                trout << indent << "QToolTip::add( " << obj << ", "
                      << trcall( txt, com ) << " );" << endl;
            else
                trout << indent << "QToolTip::add( this, "
                      << trcall( txt, com ) << " );" << endl;
        } else if ( prop == "whatsThis" && objClass != "QAction" && objClass != "QActionGroup" ) {
            if ( !obj.isEmpty() )
                trout << indent << "QWhatsThis::add( " << obj << ", "
                      << trcall( txt, com ) << " );" << endl;
            else
                trout << indent << "QWhatsThis::add( this, "
                      << trcall( txt, com ) << " );" << endl;
        } else {
            v = trcall( txt, com );
        }
    } else if ( e.tagName() == "cstring" ) {
            v = "\"%1\"";
            v = v.arg( e.firstChild().toText().data() );
    } else if ( e.tagName() == "number" ) {
        v = "%1";
        v = v.arg( e.firstChild().toText().data() );
    } else if ( e.tagName() == "bool" ) {
        v = QString("%1").arg( mkBool( e.firstChild().toText().data() ) );
    } else if ( e.tagName() == "pixmap" ) {
        v = e.firstChild().toText().data();
        if ( !pixmapLoaderFunction.isEmpty() ) {
            v.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
            v.append( QString( externPixmaps ? "\"" : "" ) + " )" );
        }
    } else if ( e.tagName() == "iconset" ) {
        v = "QIconSet( %1 )";
        QString s = e.firstChild().toText().data();
        if ( !pixmapLoaderFunction.isEmpty() ) {
            s.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
            s.append( QString( externPixmaps ? "\"" : "" ) + " )" );
        }
        v = v.arg( s );
    } else if ( e.tagName() == "image" ) {
        v = e.firstChild().toText().data() + ".convertToImage()";
    } else if ( e.tagName() == "enum" ) {
        if ( stdset )
            v = "%1::%2";
        else
            v = "\"%1\"";
        QString oc = objClass;
        QString ev = e.firstChild().toText().data();
        if ( oc == "QListView" && ev == "Manual" ) // #### workaround, rename QListView::Manual in 4.0
            oc = "QScrollView";
        if ( stdset )
            v = v.arg( oc ).arg( ev );
        else
            v = v.arg( ev );
    } else if ( e.tagName() == "set" ) {
        QString keys( e.firstChild().toText().data() );
        QStringList lst = keys.split('|');
        v = "int( ";
        QStringList::Iterator it = lst.begin();
        while ( it != lst.end() ) {
            v += objClass + "::" + *it;
            ++it;

            if ( it != lst.end() )
                v += " | ";
        }
        v += " )";
    } else if ( e.tagName() == "sizepolicy" ) {
        QDomElement n3 = e.firstChild().toElement();
        SizePolicy sp;
        sp.init();

        while (!n3.isNull()) {
            if (n3.tagName() == "hsizetype")
                sp.hsizetype = n3.firstChild().toText().data().toInt();
            else if (n3.tagName() == "vsizetype")
                sp.vsizetype = n3.firstChild().toText().data().toInt();
            else if (n3.tagName() == "horstretch")
                sp.horstretch = n3.firstChild().toText().data().toInt();
            else if (n3.tagName() == "verstretch")
                sp.verstretch = n3.firstChild().toText().data().toInt();
            n3 = n3.nextSibling().toElement();
        }
        QString tmp;
        if ( !obj.isEmpty() )
            tmp = obj + "->";
        v = "QSizePolicy( (QSizePolicy::SizeType)%1, (QSizePolicy::SizeType)%2, %3, %4, " + tmp + "sizePolicy().hasHeightForWidth() )";
        v = v.arg(sp.hsizetype).arg(sp.vsizetype).arg(sp.horstretch).arg(sp.horstretch);
    } else if ( e.tagName() == "palette" ) {
        ColorGroup activeColorGroup, inactiveColorGroup, disabledColorGroup;
        bool no_pixmaps = e.elementsByTagName( "pixmap" ).count() == 0;
        QDomElement n;
        if ( no_pixmaps ) {
            n = e.firstChild().toElement();
            while ( !n.isNull() ) {
                ColorGroup cg;
                if ( n.tagName() == "active" ) {
                    activeColorGroup = loadColorGroup( n );
                } else if ( n.tagName() == "inactive" ) {
                    inactiveColorGroup = loadColorGroup( n );
                } else if ( n.tagName() == "disabled" ) {
                    disabledColorGroup = loadColorGroup( n );
                }
                n = n.nextSibling().toElement();
            }
        }
#if 0 /// ### implement me!! (urgent)
        if ( no_pixmaps && pal == QPalette( pal.active().button(), pal.active().background() ) ) {
            v = "QPalette( QColor( %1, %2, %3 ), QColor( %1, %2, %3 ) )";
            v = v.arg( pal.active().button().red() ).arg( pal.active().button().green() ).arg( pal.active().button().blue() );
            v = v.arg( pal.active().background().red() ).arg( pal.active().background().green() ).arg( pal.active().background().blue() );
        } else {
            QString palette = "pal";
            if ( !pal_used ) {
                out << indent << "QPalette " << palette << ";" << endl;
                pal_used = TRUE;
            }
            QString cg = "cg";
            if ( !cg_used ) {
                out << indent << "QColorGroup " << cg << ";" << endl;
                cg_used = TRUE;
            }
            n = e.firstChild().toElement();
            while ( !n.isNull() && n.tagName() != "active" )
                n = n.nextSibling().toElement();
            createColorGroupImpl( cg, n );
            out << indent << palette << ".setActive( " << cg << " );" << endl;

            n = e.firstChild().toElement();
            while ( !n.isNull() && n.tagName() != "inactive" )
                n = n.nextSibling().toElement();
            createColorGroupImpl( cg, n );
            out << indent << palette << ".setInactive( " << cg << " );" << endl;

            n = e.firstChild().toElement();
            while ( !n.isNull() && n.tagName() != "disabled" )
                n = n.nextSibling().toElement();
            createColorGroupImpl( cg, n );
            out << indent << palette << ".setDisabled( " << cg << " );" << endl;
            v = palette;
        }
#endif

    } else
     if ( e.tagName() == "cursor" ) {
        v = "QCursor( %1 )";
        v = v.arg( e.firstChild().toText().data() );
    } else if ( e.tagName() == "date" ) {
        QDomElement n3 = e.firstChild().toElement();
        int y, m, d;
        y = m = d = 0;
        while ( !n3.isNull() ) {
            if ( n3.tagName() == "year" )
                y = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "month" )
                m = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "day" )
                d = n3.firstChild().toText().data().toInt();
            n3 = n3.nextSibling().toElement();
        }
        v = "QDate( %1, %2, %3 )";
        v = v.arg(y).arg(m).arg(d);
    } else if ( e.tagName() == "time" ) {
        QDomElement n3 = e.firstChild().toElement();
        int h, m, s;
        h = m = s = 0;
        while ( !n3.isNull() ) {
            if ( n3.tagName() == "hour" )
                h = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "minute" )
                m = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "second" )
                s = n3.firstChild().toText().data().toInt();
            n3 = n3.nextSibling().toElement();
        }
        v = "QTime( %1, %2, %3 )";
        v = v.arg(h).arg(m).arg(s);
    } else if ( e.tagName() == "datetime" ) {
        QDomElement n3 = e.firstChild().toElement();
        int h, mi, s, y, mo, d;
        h = mi = s = y = mo = d = 0;
        while ( !n3.isNull() ) {
            if ( n3.tagName() == "hour" )
                h = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "minute" )
                mi = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "second" )
                s = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "year" )
                y = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "month" )
                mo = n3.firstChild().toText().data().toInt();
            else if ( n3.tagName() == "day" )
                d = n3.firstChild().toText().data().toInt();
            n3 = n3.nextSibling().toElement();
        }
        v = "QDateTime( QDate( %1, %2, %3 ), QTime( %4, %5, %6 ) )";
        v = v.arg(y).arg(mo).arg(d).arg(h).arg(mi).arg(s);
    } else if ( e.tagName() == "stringlist" ) {
        QStringList l;
        QDomElement n3 = e.firstChild().toElement();
        QString listname = "l";
        if ( !obj.isEmpty() ) {
            listname = obj + "_stringlist";
            listname = registerObject( listname );
            out << indent << "QStringList "  << listname << ";" << endl;
        } else {
            listname = registerObject( listname );
            out << indent << "QStringList "  << listname << ";" << endl;
        }
        while ( !n3.isNull() ) {
            if ( n3.tagName() == "string" )
                out << indent << listname << " << \"" << n3.firstChild().toText().data().simplified() << "\";" << endl;
            n3 = n3.nextSibling().toElement();
        }
        v = listname;
    }
    return v;
}

/*! Extracts a named object property from \a e.
 */
QDomElement Ui3Reader::getObjectProperty( const QDomElement& e, const QString& name )
{
    QDomElement n;
    for ( n = e.firstChild().toElement();
          !n.isNull();
          n = n.nextSibling().toElement() ) {
        if ( n.tagName() == "property"  && n.toElement().attribute("name") == name )
            return n;
    }
    return n;
}
