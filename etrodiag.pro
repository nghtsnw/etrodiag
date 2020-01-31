QT += widgets serialport
requires(qtConfig(combobox))

TARGET = etrodiag
TEMPLATE = app

SOURCES += \
    bitmaskobj.cpp \
    bitsetform.cpp \
    bytebutton.cpp \
    bytedefinition.cpp \
    bytesettingsform.cpp \
    dataprofiler.cpp \
    device.cpp \
    devsettingsform.cpp \
    getstream.cpp \
    main.cpp \
    mainwindow.cpp \
    masksettingsdialog.cpp \
    newconnect.cpp \
    settingsdialog.cpp \
    console.cpp \
    txtmaskobj.cpp

HEADERS += \
    bitmaskobj.h \
    bitsetform.h \
    bytebutton.h \
    bytedefinition.h \
    bytesettingsform.h \
    dataprofiler.h \
    device.h \
    devsettingsform.h \
    getstream.h \
    mainwindow.h \
    masksettingsdialog.h \
    newconnect.h \
    settingsdialog.h \
    console.h \
    txtmaskobj.h

FORMS += \
    bitsetform.ui \
    bytesettingsform.ui \
    devsettingsform.ui \
    mainwindow.ui \
    masksettingsdialog.ui \
    newconnect.ui \
    settingsdialog.ui

RESOURCES +=

INSTALLS += target
