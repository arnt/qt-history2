TEMPLATE      = subdirs
SUBDIRS       = blockingfortuneclient \
		broadcastreceiver \
                broadcastsender \
                fortuneclient \
                fortuneserver \
                ftp \
                http \
                loopback \
                threadedfortuneserver

# install
target.path = $$[QT_INSTALL_DATA]/examples/network
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS network.pro README
sources.path = $$[QT_INSTALL_DATA]/examples/network
INSTALLS += target sources
