QT += widgets serialport xml
requires(qtConfig(combobox))

TARGET = etrodiag
TEMPLATE = app
TRANSLATIONS = etrodiag_ru.ts
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
    livegraph.cpp \
    logger.cpp \
    main.cpp \
    mainwindow.cpp \
    masksettingsdialog.cpp \
    newconnect.cpp \
    newgraph.cpp \
    settingsdialog.cpp \
    console.cpp \
    txtmaskobj.cpp

HEADERS += \
    bitmaskobj.h \
    bitmaskstruct.h \
    bitsetform.h \
    bytebutton.h \
    bytedefinition.h \
    bytesettingsform.h \
    dataprofiler.h \
    device.h \
    devsettingsform.h \
    getstream.h \
    livegraph.h \
    logger.h \
    mainwindow.h \
    masksettingsdialog.h \
    newconnect.h \
    newgraph.h \
    settingsdialog.h \
    console.h \
    txtmaskobj.h

FORMS += \
    bitsetform.ui \
    bytesettingsform.ui \
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
