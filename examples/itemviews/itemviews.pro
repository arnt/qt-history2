TEMPLATE      = subdirs
SUBDIRS       = basicsortfiltermodel \
                chart \
		coloreditorfactory \
                customsortfiltermodel \
                dirview \
                pixelator \
                puzzle \
                simpledommodel \
                simpletreemodel \
                simplewidgetmapper \
                spinboxdelegate \
                stardelegate

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/itemviews
INSTALLS += sources
DEFINES += QT_USE_USING_NAMESPACE
