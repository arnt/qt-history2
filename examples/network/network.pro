TEMPLATE      = subdirs
SUBDIRS       = blockingfortuneclient \
 		broadcastreceiver \
                broadcastsender \
                chat \
                fortuneclient \
                fortuneserver \
                ftp \
                http \
                loopback \
                threadedfortuneserver \
 		torrent

# install
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS network.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/network
INSTALLS += sources
