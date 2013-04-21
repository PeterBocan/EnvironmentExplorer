#include "MainDialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication ExplorerRuntime(argc, argv);

    EnvironmentExplorer::MainDialog mainDialog(0);
    mainDialog.show();
    
    return ExplorerRuntime.exec();
}
