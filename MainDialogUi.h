#ifndef MAINDIALOGUI_H
#define MAINDIALOGUI_H

#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialog>
#include <QtWidgets/QAction>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QTableWidgetItem>

namespace EnvironmentExplorer
{
    //
    // UI structure.
    //
    struct MainDialog::Ui
    {
        // Main Dialog
        QVBoxLayout* layout;
        QTableWidget* table;
        QDialogButtonBox* buttonBox;

        QPushButton* saveButton,
                   * resetButton,
                   * exportButton,
                   * addButton,
                   * closeButton;

        // Add Variable Dialog
        struct AddDialog
        {
            AddDialog()
            {
                dialog = new QDialog();
                dialog->setWindowTitle("Add variable");
                dialog->setMinimumSize(300, 100);
                layout = new QGridLayout(dialog);

                nameEdit = new QLineEdit();
                nameLabel = new QLabel("Name:");
                nameLabel->setBuddy(nameEdit);

                valueEdit = new QLineEdit();
                valueLabel = new QLabel("Value:");
                valueLabel->setBuddy(valueEdit);

                dialogButtonBox = new QDialogButtonBox();
                addButton = dialogButtonBox->addButton("Add", QDialogButtonBox::AcceptRole);
                cancelButton = dialogButtonBox->addButton("Cancel", QDialogButtonBox::RejectRole);
                QObject::connect(dialogButtonBox, &QDialogButtonBox::rejected, dialog, &QDialog::close);

                layout->addWidget(nameLabel, 0, 0);
                layout->addWidget(nameEdit, 0, 1);
                layout->addWidget(valueLabel, 1, 0);
                layout->addWidget(valueEdit, 1, 1);
                layout->addWidget(dialogButtonBox, 2, 0, 2, 0);
            }

            QDialog* dialog;
            QGridLayout* layout;
            QDialogButtonBox* dialogButtonBox;

            QPushButton* addButton,
                       * cancelButton;

            QLabel* nameLabel,
                  * valueLabel;

            QLineEdit* nameEdit,
                     * valueEdit;
        };

        // Edit Variable Dialog
        struct EditDialog
        {
            EditDialog()
            {
                dialog = new QDialog();
                dialog->setWindowTitle("Edit variable");
                dialog->setMinimumSize(300, 140);
                layout = new QGridLayout(dialog);

                nameEdit = new QLineEdit();
                nameLabel = new QLabel("Name:");
                nameLabel->setBuddy(nameEdit);

                valueEdit = new QLineEdit();
                valueLabel = new QLabel("Value:");
                valueLabel->setBuddy(valueEdit);

                multiValuesCheck = new QCheckBox("Multiple values");
                multiValuesCheck->setChecked(false);

                itemList = new QListWidget();
                itemList->setEditTriggers(QListWidget::DoubleClicked);

                addItemItem = new QListWidgetItem(itemList);
                addItemItem->setText("Add new value...");
                addItemItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);

                dialogButtonBox = new QDialogButtonBox();
                saveButton = dialogButtonBox->addButton(QDialogButtonBox::Save);
                cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
                QObject::connect(dialogButtonBox, &QDialogButtonBox::rejected, dialog, &QDialog::close);

                layout->addWidget(nameLabel, 0, 0);
                layout->addWidget(nameEdit, 0, 1);
                layout->addWidget(valueLabel, 1, 0, Qt::AlignTop|Qt::AlignRight);
                layout->addWidget(valueEdit, 1, 1);
                layout->addWidget(multiValuesCheck, 2, 1);
                layout->addWidget(dialogButtonBox, 3, 0, 2, 0);
            }

            QDialog* dialog;
            QGridLayout* layout;
            QDialogButtonBox* dialogButtonBox;

            QCheckBox* multiValuesCheck;

            QPushButton* saveButton,
                       * cancelButton;

            QLabel* nameLabel,
                  * valueLabel;

            QLineEdit* nameEdit,
                     * valueEdit;

            QListWidget* itemList;
            QListWidgetItem* addItemItem;
        };

        // Handle key events of the QTableWidget
        KeyEventFilter* tableKeyEvents,
                      * listKeyEvents;

        AddDialog* addVariableDialog;
        EditDialog* editVariableDialog;

        Ui() : tableKeyEvents(new KeyEventFilter),
               listKeyEvents(new KeyEventFilter)
        {
            table = new QTableWidget();
            table->setColumnCount(2);
            table->setContextMenuPolicy(Qt::CustomContextMenu);
            table->installEventFilter(tableKeyEvents); // install event filter
            table->horizontalHeader()->setStretchLastSection(true);
            table->setVerticalScrollMode(QTableWidget::ScrollPerPixel);
            table->setHorizontalScrollMode(QTableWidget::ScrollPerPixel);
            table->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
            table->itemDelegate()->setObjectName("vars_table_item_delegate");

            buttonBox = new QDialogButtonBox(Qt::Horizontal);
            addButton = buttonBox->addButton(QString("Add"), QDialogButtonBox::ActionRole);
            exportButton = buttonBox->addButton(QString("Export"), QDialogButtonBox::ActionRole);
            saveButton = buttonBox->addButton(QDialogButtonBox::Save);
            resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
            closeButton = buttonBox->addButton(QDialogButtonBox::Close);

            layout = new QVBoxLayout();
            layout->setContentsMargins(5,5,5,5);
            layout->addWidget(table);
            layout->addWidget(buttonBox);

            // Add Variable Dialog...
            addVariableDialog = new AddDialog();

            // Edit Variable Dialog...
            editVariableDialog = new EditDialog();
        }

    }; // End of MainDialog::Ui


}

#endif
