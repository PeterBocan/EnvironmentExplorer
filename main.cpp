#include <QApplication>
#include <qt_windows.h>

#include "MainDialog.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication ExplorerRuntime(argc, argv);

    EnvironmentExplorer::MainDialog mainDialog(0);
    mainDialog.show();
    
    return ExplorerRuntime.exec();
}
