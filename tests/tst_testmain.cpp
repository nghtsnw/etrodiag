#include <QtTest>

#include "testapppaths.h"
#include "testdataprofiler.h"
#include "testframecheck.h"
#include "testmaskmath.h"
#include "testprofiledata.h"
#include "testprotocolsettings.h"
#include "testwordvalue.h"

static int runTest(QObject *test, int argc, char *argv[])
{
    const int status = QTest::qExec(test, argc, argv);
    delete test;
    return status;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv); //нужен модулям, которые считают пути приложения

    int status = 0;
    status |= runTest(new TestAppPaths(), argc, argv);
    status |= runTest(new TestDataProfiler(), argc, argv);
    status |= runTest(new TestFrameCheck(), argc, argv);
    status |= runTest(new TestWordValue(), argc, argv);
    status |= runTest(new TestMaskMath(), argc, argv);
    status |= runTest(new TestProfileData(), argc, argv);
    status |= runTest(new TestProtocolSettings(), argc, argv);
    return status;
}
