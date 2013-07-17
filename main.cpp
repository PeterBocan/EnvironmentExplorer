/*
* This is a part of EnvironmentExplorer program
* which is licensed under LGPLv2.
*
* Github: https://github.com/PeterBocan/EnvironmentExplorer
* Author: https://twitter.com/PeterBocan
*/

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
