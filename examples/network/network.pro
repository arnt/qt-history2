TEMPLATE      = subdirs
unset(EXAMPLES_NETWORK_SUBDIRS)
EXAMPLES_NETWORK_SUBDIRS = examples_network_blockingfortuneclient \
                           examples_network_broadcastreceiver \
                           examples_network_broadcastsender \
                           examples_network_chat \
                           examples_network_fortuneclient \
                           examples_network_fortuneserver \
                           examples_network_ftp \
                           examples_network_http \
                           examples_network_loopback \
                           examples_network_threadedfortuneserver \
                           examples_network_torrent

# install
target.path = $$[QT_INSTALL_EXAMPLES]/network
EXAMPLES_NETWORK_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS network.pro README
EXAMPLES_NETWORK_install_sources.path = $$[QT_INSTALL_EXAMPLES]/network
INSTALLS += target EXAMPLES_NETWORK_install_sources

#subdirs
examples_network_blockingfortuneclient.subdir = $$QT_BUILD_TREE/examples/network/blockingfortuneclient
examples_network_blockingfortuneclient.depends =  src_corelib src_gui src_network
examples_network_broadcastreceiver.subdir = $$QT_BUILD_TREE/examples/network/broadcastreceiver
examples_network_broadcastreceiver.depends =  src_corelib src_gui src_network
examples_network_broadcastsender.subdir = $$QT_BUILD_TREE/examples/network/broadcastsender
examples_network_broadcastsender.depends =  src_corelib src_gui src_network
examples_network_chat.subdir = $$QT_BUILD_TREE/examples/network/broadcastsender
examples_network_chat.depends =  src_corelib src_gui src_network
examples_network_fortuneclient.subdir = $$QT_BUILD_TREE/examples/network/fortuneclient
examples_network_fortuneclient.depends =  src_corelib src_gui src_network
examples_network_fortuneserver.subdir = $$QT_BUILD_TREE/examples/network/fortuneserver
examples_network_fortuneserver.depends =  src_corelib src_gui src_network
examples_network_ftp.subdir = $$QT_BUILD_TREE/examples/network/ftp
examples_network_ftp.depends =  src_corelib src_gui src_network
examples_network_http.subdir = $$QT_BUILD_TREE/examples/network/http
examples_network_http.depends =  src_corelib src_gui src_network
examples_network_loopback.subdir = $$QT_BUILD_TREE/examples/network/loopback
examples_network_loopback.depends =  src_corelib src_gui src_network
examples_network_threadedfortuneserver.subdir = $$QT_BUILD_TREE/examples/network/threadedfortuneserver
examples_network_threadedfortuneserver.depends =  src_corelib src_gui src_network
examples_network_torrent.subdir = $$QT_BUILD_TREE/examples/network/torrent
examples_network_torrent.depends =  src_corelib src_gui src_network
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_NETWORK_SUBDIRS
SUBDIRS += $$EXAMPLES_NETWORK_SUBDIRS
