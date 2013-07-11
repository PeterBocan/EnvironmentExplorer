#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QtWidgets/QWidget>

class QTableWidgetItem;

namespace EnvironmentExplorer
{
    struct UserInterface;
    class VariablesManager;
    class VariableDialog;

    class MainDialog : public QWidget
    {
        Q_OBJECT

        // User Interface
        UserInterface* ui;

        // Manager
        VariablesManager* variableManager;

        // Dialog
        VariableDialog* variableDialog;

    public:
            MainDialog(QWidget *parent = 0);
            ~MainDialog();

    protected:
            void initConnections();
            void fillTable();

    protected slots:
            void contextMenu();

            void addVariable();
            void editVariable(QTableWidgetItem* item);
            void removeVariable();
            void saveEnvironment();
            void exportEnvironment();
            void resetTable();

            void exportPlainText(const QString &file);
            void exportHtml(const QString &file);

    };
}

#endif // MAINDIALOG_H
