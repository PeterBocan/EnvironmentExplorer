
/*
* This is a part of EnvironmentExplorer program
* which is licensed under LGPLv2.
*
* Github: https://github.com/PeterBocan/EnvironmentExplorer
* Author: https://twitter.com/PeterBocan
*/

#include "MainDialog.h"
#include "MainDialogUi.h"
#include "VariablesManager.h"

#include <QApplication>
#include <QTime>
#include <QVariant>
#include <QFile>
#include <QString>
#include <QtDebug>

#if defined(Q_OS_WIN32)
#include <qt_windows.h>
#endif

namespace EnvironmentExplorer
{
    static QColor globalsVariablesColor = QColor(255,247,193);
    static QColor localsVariablesColor = QColor(255,255,255);

    bool isInvokerAdmin()
    {
        BOOL result;
        SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
        PSID AdministratorsGroup;

        result = AllocateAndInitializeSid(&NtAuthority, 2,
                 SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                 0, 0, 0, 0, 0, 0, &AdministratorsGroup);

        if(result)
        {
            if (!CheckTokenMembership(NULL, AdministratorsGroup, &result))
                 result = FALSE;

            FreeSid(AdministratorsGroup);
        }

        return (result ? true : false);
    }

    MainDialog::MainDialog(QWidget *parent)
        : QWidget(parent), ui(new UserInterface()),
          variableManager(new VariablesManager()),
          variableDialog(new VariableDialog(this))
    {
        setWindowTitle(tr("Environment explorer"));
        setLayout(ui->layout);

        resize(850, 600);

        initConnections();

        if (!isInvokerAdmin())
            ui->saveButton->setDisabled(true);



        variableManager->loadVariables();
        fillTable();
    }

    MainDialog::~MainDialog()
    { delete ui; }

    void MainDialog::initConnections()
    {
        // panel...
        connect(ui->addButton, &QPushButton::pressed, this, &MainDialog::addVariable);
        connect(ui->closeButton, &QPushButton::pressed, this, &MainDialog::close);
        connect(ui->exportButton, &QPushButton::pressed, this, &MainDialog::exportEnvironment);
        connect(ui->saveButton, &QPushButton::pressed, this, &MainDialog::saveEnvironment);
        connect(ui->resetButton, &QPushButton::pressed, this, &MainDialog::resetTable);

        // table...
        connect(ui->mainTable, &QTableWidget::itemDoubleClicked, this, &MainDialog::editVariable);
        connect(ui->mainTable, &QTableWidget::customContextMenuRequested, this, &MainDialog::contextMenu);
    }

    void MainDialog::fillTable()
    {
        QList<Variable> systems = variableManager->systemEnvironment();
        QList<Variable> locals = variableManager->userEnvironment();

            count_systems = systems.count();
        int count_locals = locals.count();
        ui->mainTable->setRowCount(count_locals + count_systems);

        for(int i = 0; i < count_systems; i++)
        {
            Variable var = systems.at(i);
            QTableWidgetItem* nameItem = new QTableWidgetItem(var.name);
            nameItem->setBackground(QBrush(globalsVariablesColor));

            QString result = var.defaultValue.toString();
            if (var.defaultValue.type() == QMetaType::QStringList ||
                var.defaultValue.type() == QVariant::StringList)
                result = var.defaultValue.toStringList().join("\n");

            QTableWidgetItem* valueItem = new QTableWidgetItem(result);
            valueItem->setBackground(QBrush(globalsVariablesColor));

            ui->mainTable->setItem(i, 0, nameItem);
            ui->mainTable->setItem(i, 1, valueItem);
        }

        for (int i = 0; i < count_locals; ++i)
        {
            Variable var = locals.at(i);
            QTableWidgetItem* nameItem = new QTableWidgetItem(var.name);
            nameItem->setBackground(QBrush(localsVariablesColor));

            QString result = var.defaultValue.toString();
            if (var.defaultValue.type() == QMetaType::QStringList ||
                var.defaultValue.type() == QVariant::StringList)
                result = var.defaultValue.toStringList().join("\n");

            QTableWidgetItem* valueItem = new QTableWidgetItem(result);
            valueItem->setBackground(QBrush(localsVariablesColor));

            ui->mainTable->setItem(i + count_systems, 0, nameItem);
            ui->mainTable->setItem(i + count_systems, 1, valueItem);
        }

        ui->mainTable->resizeColumnsToContents();
        ui->mainTable->resizeRowsToContents();
    }

    void MainDialog::resetTable()
    {
        for (int row = 0; row < ui->mainTable->rowCount(); ++row)
        {
            QString variableName = ui->mainTable->item(row, 0)->text();
            Variable var = variableManager->variable(variableName);

            qDebug() << var.defaultName << var.name << var.defaultValue << var.value;

            if (var.defaultName.isEmpty())
            {   // non-existing variable, remove.
                ui->mainTable->removeRow(row);
                continue;
            }

            if (var.defaultName != var.name)
            {   // restore default name
                ui->mainTable->item(row, 0)->setText(var.defaultName);
            }

            if (var.defaultValue != var.value)
            {   // restore default values.
                QString result = ((var.defaultValue.type() == QVariant::StringList) ?
                                      var.defaultValue.toStringList().join("\n") :
                                      var.defaultValue.toString());
                ui->mainTable->item(row, 1)->setText(result);
            }

        }
    }

    void MainDialog::contextMenu()
    {
        QMenu menu;

        QAction* removeAction = menu.addAction("Remove variable");
        removeAction->setShortcut(QKeySequence(Qt::Key_Delete)); 
        connect(removeAction, &QAction::triggered, this, &MainDialog::removeVariable);

        menu.exec(QCursor::pos());
    }

    void MainDialog::addVariable()
    {
         variableDialog->setDialogMode(VariableDialog::AddVariable);
         int result = variableDialog->exec();

         if (result == QDialog::Accepted)
         {
             QString name = variableDialog->variableName();
             QVariant val = variableDialog->variableValue();
             Variable::Type type = variableDialog->variableType();

             QString str;
             if (val.type() == QMetaType::QString)
                 str = val.toString();
             if (val.type() == QMetaType::QStringList)
                 str = val.toStringList().join("\n");

             QColor background = (type == Variable::Global) ? globalsVariablesColor : localsVariablesColor;

             QTableWidgetItem* nameItem = new QTableWidgetItem(name);
             nameItem->setBackground(QBrush(background));
             QTableWidgetItem* valueItem = new QTableWidgetItem(str);
             valueItem->setBackground(QBrush(background));

             ui->mainTable->setRowCount(ui->mainTable->rowCount()+1);

             int row = (type == Variable::Global) ? count_systems : ui->mainTable->rowCount();

             ui->mainTable->insertRow(row);
             ui->mainTable->setItem(row, 0, nameItem);
             ui->mainTable->setItem(row, 1, valueItem);

             if (type == Variable::Global)
                 variableManager->addGlobalVariable(name, val);
             else
                 variableManager->addUserVariable(name, val);
         }
    }

    void MainDialog::editVariable(QTableWidgetItem* item)
    {
        QString oldName = ui->mainTable->item(item->row(), 0)->text();
        variableDialog->setDialogMode(VariableDialog::EditVariable);
        variableDialog->setVariableName(oldName);
        variableDialog->setVariableValue(ui->mainTable->item(item->row(), 1)->text());

        int result = variableDialog->exec();
        if (result == QDialog::Accepted)
        {
            QString name = variableDialog->variableName();
            QVariant val = variableDialog->variableValue();

            QString str;
            if (val.type() == QMetaType::QString)
                str = val.toString();
            if (val.type() == QMetaType::QStringList)
                str = val.toStringList().join("\n");

            ui->mainTable->item(item->row(), 0)->setText(name);
            ui->mainTable->item(item->row(), 1)->setText(str);
            ui->mainTable->resizeRowToContents(item->row());
            ui->mainTable->resizeColumnToContents(0);
            ui->mainTable->resizeColumnToContents(1);

            // reset variable
            Variable var;
            var.name = name;
            var.value = val;

            Variable oldVariable = variableManager->variable(oldName);
            variableManager->removeVariable(oldName); // We can not have a duplicate.
            var.defaultName = oldVariable.defaultName;
            var.defaultValue = oldVariable.defaultValue;

            variableManager->addVariable(var);
        }
    }

    void MainDialog::removeVariable()
    {
        QList<QTableWidgetItem*> selection = ui->mainTable->selectedItems();
        QTableWidgetItem* nameItem = selection.at(0);

        variableManager->removeVariable(nameItem->text());
        ui->mainTable->removeRow(ui->mainTable->row(nameItem));
    }

    void MainDialog::saveEnvironment()
    { variableManager->saveVariables(); }

    void MainDialog::exportEnvironment()
    {
        QString filterType;
        QString fileName = QFileDialog::getSaveFileName(0, "Save to file...", QString(),
                                     QString("HTML (*.html);;Text file (*.log)"),
                                     &filterType);

        if (fileName.isEmpty() && filterType.isEmpty())
            return;
        else
        {
            if (filterType == "Text file (*.log)")
               exportPlainText(fileName);
            else
               exportHtml(fileName);
        }
    }

    void MainDialog::exportHtml(const QString &file)
    {
        QString escapedName;
        if (!file.endsWith(".html") && file.contains("."))
        {
            QString nakedName = file.left(file.lastIndexOf(".") - 1); // excluding dot
            escapedName = nakedName.append(".html");
        }

        QFile fileHandle(escapedName);

        if (!fileHandle.open(QFile::WriteOnly|QFile::Text))
            QMessageBox::critical(0, QString("Error"),
                                  QString("Error occured:").append(fileHandle.errorString())
                                  .append("Canceling export."));
        else
        {
            QFile f("://template.html");
            f.open(QFile::ReadOnly);

            QString templateFile = f.readAll();
            QString timestamp = QTime::currentTime().toString();

            wchar_t ch_user[128];
            DWORD d = 128;
            GetComputerNameW(ch_user, &d); // WinAPI

            QString compName = QString::fromWCharArray(ch_user, d);

            QString tmpStr("");
            for (int row = 0; row < ui->mainTable->rowCount(); ++row)
            {
                QString name = ui->mainTable->item(row, 0)->text();
                QString val = ui->mainTable->item(row, 1)->text();
                tmpStr.append("           <tr>\r\n                <td>");
                tmpStr.append(name).append("</td>\r\n                <td>\r\n");

                QStringList list = (val.contains("\n")) ? val.split("\n") : QStringList(val);
                foreach(QString value, list) tmpStr.append("             " + value + "<br>\r\n");

                tmpStr.append("           </tr>\r\n");
            }

            fileHandle.write(templateFile.arg(compName, timestamp, tmpStr).toStdString().c_str());
            fileHandle.close();
        }
    }

    void MainDialog::exportPlainText(const QString &file)
    {
        QString escapedName;
        if (!file.endsWith(".log") && file.contains("."))
        {
            QString nakedName = file.left(file.lastIndexOf(".") - 1); // excluding dot
            escapedName = nakedName.append(".log");
        }

        QFile fileHandle(escapedName);

        if (!fileHandle.open(QFile::WriteOnly|QFile::Text))
            QMessageBox::critical(0, QString("Error"),
                                  QString("Error occured:").append(fileHandle.errorString())
                                  .append("Canceling export."));
        else
        {
            for (int row = 0; row < ui->mainTable->rowCount(); ++row)
            {
                QString name = ui->mainTable->item(row, 0)->text();
                QString val = ui->mainTable->item(row, 1)->text();

                QStringList list = (val.contains("\n")) ? val.split("\n") : QStringList(val);
                fileHandle.write(QString("Name: %1 \r\n").arg(name).toStdString().c_str());
                fileHandle.write("Value(s):\r\n");

                foreach(QString value, list)
                    fileHandle.write(QString("       %1\r\n").arg(value).toStdString().c_str());

                fileHandle.write("-----------------------------------------------\r\n");
            }
            fileHandle.close();
        }
    }
}

