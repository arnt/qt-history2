/****************************************************************************
** $Id: //depot/qt/main/examples/movies/main.cpp#4 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qnp.h>
#include <qpainter.h>
#include <qmovie.h>


class MovieView : public QNPWidget {
    Q_OBJECT
    QMovie& movie;

public:
    MovieView(QMovie& m) :
        QNPWidget(),
	movie(m)
    {
        // No background needed, since we draw on the whole widget.
        movie.setBackgroundColor(backgroundColor());
        setBackgroundMode(NoBackground);

        // Get the movie to tell use when interesting things happen.
        movie.connectUpdate(this, SLOT(movieUpdated(const QRect&)));
        movie.connectResize(this, SLOT(movieResized(const QSize&)));
        movie.connectStatus(this, SLOT(movieStatus(int)));
    }


protected:

    // Draw the contents of the QFrame - the movie and on-screen-display
    void paintEvent(QPaintEvent*)
    {
	QPainter p(this);

	if ( movie.isNull() )
	    return;

        // Get the current movie frame.
        QPixmap pm = movie.framePixmap();

	if ( pm.isNull() )
	    return;

        // Get the area we have to draw in.
        QRect r = rect() & movie.getValidRect();
	if ( r != rect() )
	    p.eraseRect(rect());

        // Only rescale is we need to - it can take CPU!
        if ( r.size() != pm.size() ) {
            QWMatrix m;
            m.scale((double)r.width()/pm.width(),
                    (double)r.height()/pm.height());
            pm = pm.xForm(m);
        }

        // Draw the [possibly scaled] frame.  movieUpdated() below calls
        // repaint with only the changed area, so clipping will ensure we
        // only do the minimum amount of rendering.
        //
        p.drawPixmap(r.x(), r.y(), pm);


        // The on-screen display

        const char* message = 0;

        if (movie.paused()) {
            message = "PAUSED";
        } else if (movie.finished()) {
            message = "THE END";
        } else if (movie.steps() > 0) {
            message = "FF >>";
        }

        if (message) {
            // Find a good font size...
            p.setFont(QFont("Helvetica", 24));

            QFontMetrics fm = p.fontMetrics();
            if ( fm.width(message) > r.width()-10 )
                p.setFont(QFont("Helvetica", 18));

            fm = p.fontMetrics();
            if ( fm.width(message) > r.width()-10 )
                p.setFont(QFont("Helvetica", 14));

            fm = p.fontMetrics();
            if ( fm.width(message) > r.width()-10 )
                p.setFont(QFont("Helvetica", 12));

            fm = p.fontMetrics();
            if ( fm.width(message) > r.width()-10 )
                p.setFont(QFont("Helvetica", 10));

            // "Shadow" effect.
            p.setPen(black);
            p.drawText(1, 1, width()-1, height()-1, AlignCenter, message);
            p.setPen(white);
            p.drawText(0, 0, width()-1, height()-1, AlignCenter, message);
        }
    }

    void mouseReleaseEvent(QMouseEvent* event)
    {
        // Do what the Help says...

        if (event->state() & ShiftButton) {
            movie.restart();
        } else if (!movie.paused()) {
            movie.pause();
        } else {
            if (event->button() & LeftButton)
                movie.step((event->state() & ControlButton) ? 10 : 1);
            else if (event->button() & (MidButton|RightButton))
                movie.unpause();
        }

        repaint(); // To hide/show "PAUSED".
    }

private slots:
    void movieUpdated(const QRect& area)
    {
        if (!isVisible())
            show();

        // The given area of the movie has changed.

        QRect r = rect();

        if ( r.size() != movie.framePixmap().size() ) {
            // Need to scale - redraw whole frame.
            repaint( r );
        } else {
            // Only redraw the changed area of the frame
            repaint( area.x()+r.x(), area.y()+r.x(),
                     area.width(), area.height() );
        }
    }

    void movieResized(const QSize& size)
    {
        // The movie changed size, probably from its initial zero size.

        resize( size.width(), size.height() );
    }

    void movieStatus(int status)
    {
        // The movie has sent us a status message.

        if (status < 0) {
	    // #### Give message?
        } else if (status == QMovie::Paused || status == QMovie::EndOfMovie) {
            repaint(); // Ensure status text is displayed
        }
    }
};


class MovieLoader : public QNPInstance {
    MovieView* v;
    QMovie* movie;

public:
    MovieLoader() : v(0), movie(0)
    {
    }

    QNPWidget* newWindow()
    {
	if ( !movie ) movie = new QMovie(4096);
	v = new MovieView(*movie);
	return v;
    }

    int writeReady(QNPStream*)
    {
	return movie ? movie->pushSpace() : 0;
    }

    int write(QNPStream* /*str*/, int /*offset*/, int len, void* buffer)
    {
	if ( !movie ) return 0;
	int l = movie->pushSpace();
	if ( l > len ) l = len;
	movie->pushData((const uchar*)buffer,l);
	return l;
    }
};

class MoviePlugin : public QNPlugin {

public:
    MoviePlugin()
    {
    }

    QNPInstance* newInstance()
    {
	return new MovieLoader;
    }

    const char* getMIMEDescription() const
    {
	return "video/x-mng:mng:MNG animation;"
	       "video/mng::MNG animation;"
	       "image/x-jng:jng:MNG animation;"
	       "image/jng::JNG animation";
    }

    const char * getPluginNameString() const
    {
	return "MNG plugin (libmng + Qt)";
    }

    const char * getPluginDescriptionString() const
    {
	return "Supports all movie formats supported by Qt";
    }
};

QNPlugin* QNPlugin::create()
{
    return new MoviePlugin;
}

#include "main.moc"
