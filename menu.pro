SOURCES = main.cpp carousel.cpp pixmap.cpp
HEADERS = carousel.h pixmap.h
RESOURCES = menu.qrc

# install
target.path = /root
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS menu.pro
INSTALLS += target
