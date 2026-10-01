QT += widgets serialport
requires(qtConfig(combobox))

TARGET = etrodiag
TEMPLATE = app

# Иконка исполняемого файла Windows (используется только на Windows)
RC_ICONS = etrodiag.ico

INCLUDEPATH += ../qtcsv
DEPENDPATH += ../qtcsv
include(qtcsv/qtcsv.pri)

TRANSLATIONS = etrodiag_ru.ts
SOURCES += \
    aboutdialog.cpp \
    apppaths.cpp \
    bitmaskobj.cpp \
    bitsetform.cpp \
    bytebutton.cpp \
    bytedefinition.cpp \
    bytesettingsform.cpp \
    controlboard.cpp \
    dataprofiler.cpp \
    device.cpp \
    devsettingsform.cpp \
    framecheck.cpp \
    getstream.cpp \
    livegraph.cpp \
    logger.cpp \
    main.cpp \
    mainwindow.cpp \
    maskmath.cpp \
    masksettingsdialog.cpp \
    newconnect.cpp \
    newgraph.cpp \
    packetdiagram.cpp \
    profiledata.cpp \
    protocolsettings.cpp \
    settingsdialog.cpp \
    console.cpp \
    txtmaskobj.cpp \
    wordvalue.cpp

HEADERS += \
    aboutdialog.h \
    apppaths.h \
    bitmaskobj.h \
    bitsetform.h \
    bytebutton.h \
    bytedefinition.h \
    bytesettingsform.h \
    controlboard.h \
    dataprofiler.h \
    device.h \
    devsettingsform.h \
    framecheck.h \
    getstream.h \
    global.h \
    livegraph.h \
    logger.h \
    mainwindow.h \
    maskmath.h \
    masksettingsdialog.h \
    newconnect.h \
    newgraph.h \
    packetdiagram.h \
    profiledata.h \
    protocolsettings.h \
    settingsdialog.h \
    console.h \
    txtmaskobj.h \
    wordvalue.h

FORMS += \
    aboutdialog.ui \
    bitsetform.ui \
    bytesettingsform.ui \
    controlboard.ui \
    devsettingsform.ui \
    livegraph.ui \
    mainwindow.ui \
    masksettingsdialog.ui \
    newconnect.ui \
    settingsdialog.ui

RESOURCES += \
    resources.qrc

INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

ANDROID_ABIS = arm64-v8a x86_64
