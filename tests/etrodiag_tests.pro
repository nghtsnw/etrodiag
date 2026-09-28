QT += core testlib serialport widgets

TARGET = etrodiag_tests
CONFIG += console testcase c++17
CONFIG -= app_bundle

TEMPLATE = app

# Модули с чистой логикой лежат в корне проекта и компилируются прямо в тесты
INCLUDEPATH += $$PWD/..

SOURCES += \
    tst_testmain.cpp \
    testapppaths.cpp \
    testdataprofiler.cpp \
    testframecheck.cpp \
    testmaskmath.cpp \
    testwordvalue.cpp \
    testprofiledata.cpp \
    testprotocolsettings.cpp \
    $$PWD/../apppaths.cpp \
    $$PWD/../dataprofiler.cpp \
    $$PWD/../framecheck.cpp \
    $$PWD/../maskmath.cpp \
    $$PWD/../wordvalue.cpp \
    $$PWD/../profiledata.cpp \
    $$PWD/../protocolsettings.cpp

HEADERS += \
    testapppaths.h \
    testdataprofiler.h \
    testframecheck.h \
    testmaskmath.h \
    testwordvalue.h \
    testprofiledata.h \
    testprotocolsettings.h \
    $$PWD/../apppaths.h \
    $$PWD/../dataprofiler.h \
    $$PWD/../framecheck.h \
    $$PWD/../maskmath.h \
    $$PWD/../wordvalue.h \
    $$PWD/../profiledata.h \
    $$PWD/../protocolsettings.h \
    $$PWD/../global.h

message(=== Configuration of etrodiag_tests ===)
message(Qt version: $$[QT_VERSION])
message(etrodiag_tests binary will be created in folder: $$OUT_PWD)
