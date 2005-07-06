TEMPLATE      = subdirs
SUBDIRS       = blockingfortuneclient \
		broadcastreceiver \
                broadcastsender \
                fortuneclient \
                fortuneserver \
                ftp \
                http \
                loopback \
                threadedfortuneserver \
		torrent

# install
target.path = $$[QT_INSTALL_EXAMPLES]/network
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS network.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/network
INSTALLS += target sources
