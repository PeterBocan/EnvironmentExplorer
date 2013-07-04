#include "MainDialog.h"

#include <QVariant>
#include <QSettings>
#include <QFormLayout>
#include <QCheckBox>

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#endif

namespace EnvironmentExplorer
{

    bool expandEnvironment(const QMultiHash<QString, QVariant> &env);

    //
    // Installs event filter for Return key.
    //
    bool KeyEventFilter::eventFilter(QObject *o, QEvent *e)
    {
        if(e->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
            switch (keyEvent->key())
            {
                case Qt::Key_Return: { returnPressed(); return true; }
                default: return o->event(e);
            }
        }
        else return o->event(e);
    }

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
                connect(dialogButtonBox, &QDialogButtonBox::rejected, dialog, &QDialog::close);

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
                itemList->setEditTriggers(QListWidget::AnyKeyPressed);

                QListWidgetItem* addItemItem = new QListWidgetItem(itemList);
                addItemItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);

                dialogButtonBox = new QDialogButtonBox();
                saveButton = dialogButtonBox->addButton(QDialogButtonBox::Save);
                cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
                connect(dialogButtonBox, &QDialogButtonBox::rejected, dialog, &QDialog::close);

                layout->addWidget(nameLabel, 0, 0);
                layout->addWidget(nameEdit, 0, 1);
                layout->addWidget(valueLabel, 1, 0);
                layout->addWidget(valueEdit, 1, 1);
                layout->addWidget(multiValuesCheck, 2, 1 );
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

    //
    //
    //
    //
    //
    MainDialog::MainDialog(QWidget *parent)
        : QWidget(parent), dialogUi(new MainDialog::Ui), openedEditor(false)
    {
        setWindowTitle(tr("Environment explorer"));
        setMinimumSize(850, 600);

        // connect buttons to actions
        connect(dialogUi->closeButton, &QPushButton::pressed, this, &QWidget::close);
        connect(dialogUi->addButton, &QPushButton::pressed, this, &MainDialog::addVariableDialog);
        connect(dialogUi->resetButton, &QPushButton::pressed, this, &MainDialog::resetVariables);
        connect(dialogUi->saveButton, &QPushButton::pressed, this, &MainDialog::saveVariables);

        // connect signals to slots
        connect(dialogUi->table, &QTableWidget::doubleClicked, this, &MainDialog::operateLineEditor);
        connect(dialogUi->tableKeyEvents, &KeyEventFilter::returnPressed, this, &MainDialog::operateLineEditor);
        connect(dialogUi->table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainDialog::checkEditorState);
        connect(dialogUi->table->itemDelegate(), &QAbstractItemDelegate::commitData, this, &MainDialog::commitEditorData);
        connect(dialogUi->table, &QTableWidget::customContextMenuRequested, this, &MainDialog::contextMenu);

        // Add Variable dialog
        connect(dialogUi->addVariableDialog->dialogButtonBox, &QDialogButtonBox::accepted, this, &MainDialog::addVariable);
        connect(dialogUi->addVariableDialog->nameEdit, &QLineEdit::textChanged, this, &MainDialog::validateVariableName);
        connect(dialogUi->listKeyEvents, &KeyEventFilter::returnPressed, this, &MainDialog::operateListEditor);

        // Edit Variable dialog
        connect(dialogUi->editVariableDialog->dialogButtonBox, &QDialogButtonBox::accepted, this, &MainDialog::editVariable);
        connect(dialogUi->editVariableDialog->nameEdit, &QLineEdit::textChanged, this, &MainDialog::validateVariableName);
        connect(dialogUi->editVariableDialog->multiValuesCheck, &QCheckBox::stateChanged,
             [=](int state) {
                if (state == Qt::Checked) {
                    dialogUi->editVariableDialog->valueEdit->hide();
                    dialogUi->editVariableDialog->layout->removeWidget(dialogUi->editVariableDialog->valueEdit);
                    dialogUi->editVariableDialog->layout->addWidget(dialogUi->editVariableDialog->itemList, 1, 1);
                    dialogUi->editVariableDialog->itemList->show();
                } else {
                    dialogUi->editVariableDialog->itemList->hide();
                    dialogUi->editVariableDialog->layout->removeWidget(dialogUi->editVariableDialog->itemList);
                    dialogUi->editVariableDialog->layout->addWidget( dialogUi->editVariableDialog->valueEdit, 1, 1);
                    dialogUi->editVariableDialog->valueEdit->show();
                }
                dialogUi->editVariableDialog->dialog->resize(
                    dialogUi->editVariableDialog->dialog->sizeHint());
        });

        // set up layout
        setLayout(dialogUi->layout);

        // load environment
        loadEnvironmentVariables();

        // select the first one
        QModelIndex index = dialogUi->table->model()->index(0,0);
        dialogUi->table->selectionModel()->select(index, QItemSelectionModel::Select);
    }

    MainDialog::~MainDialog()
    { delete dialogUi; }

    QList<QPair<QString, QVariant> > MainDialog::parseSystemEnvironment(const QSettings &env)
    {
        QList<QPair<QString, QVariant> > result;

        foreach (QString key, env.allKeys())
        {
            QPair<QString, QVariant> pair;
            pair.first = key;

            QString valueString = env.value(key).toString();
            QVariant value = (valueString.contains(";")) ? QVariant(valueString.split(";")) : QVariant(valueString);
            pair.second = value;

            result.append(pair);
        }

        return result;
    }


    /*
    * Load environment variables from the OS.
    */
    void MainDialog::loadEnvironmentVariables()
    {
        userSettings = new QSettings("HKEY_CURRENT_USER\\Environment", QSettings::NativeFormat);
        globalSettings = new QSettings("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", QSettings::NativeFormat);

        QList<QPair<QString, QVariant> > userPairing = parseSystemEnvironment(*userSettings);
        QList<QPair<QString, QVariant> > globalPairing = parseSystemEnvironment(*globalSettings);

        dialogUi->table->setRowCount(userPairing.count() + globalPairing.count());

        int n = 0;
        QPair<QString, QVariant> pair;
        foreach (pair, globalPairing) // global ones.
        {
            QTableWidgetItem* name = new QTableWidgetItem(pair.first);
                              name->setBackground(QBrush(QColor(255, 174, 201)));
            QTableWidgetItem* value = new QTableWidgetItem();
                              value->setBackground(QBrush(QColor(255, 174, 201)));

            switch(pair.second.type())
            {
                case QMetaType::QStringList:
                   value->setText(pair.second.toStringList().join("\n"));
                break;
                default:
                   value->setText(pair.second.toString());
                break;
            }

            dialogUi->table->setItem(n, 0, name);
            dialogUi->table->setItem(n, 1, value);
            n++;
        }

        foreach (pair, userPairing)
        {
            QTableWidgetItem* name = new QTableWidgetItem(pair.first);
            name->setBackground(QBrush(QColor(188, 241, 202)));

            QTableWidgetItem* value = new QTableWidgetItem();
            value->setBackground(QBrush(QColor(188, 241, 202)));

            switch(pair.second.type())
            {
                case QMetaType::QStringList:
                     value->setText(pair.second.toStringList().join("\n"));
                break;
                default:
                     value->setText(pair.second.toString());
                break;
            }

            dialogUi->table->setItem(n, 0, name);
            dialogUi->table->setItem(n, 1, value);
            n++;
        }

        // resize table to its content
        dialogUi->table->resizeColumnsToContents();
        dialogUi->table->resizeRowsToContents();
    }

    // Will be executed on key pressed event
    void MainDialog::operateLineEditor()
    {
        if (openedEditor) {
            dialogUi->table->closePersistentEditor(dialogUi->table->selectedItems().at(0));
            openedEditor = false;
        } else {
            QTableWidgetItem *currentItem = dialogUi->table->selectedItems().at(0);

            // if it has multiple lines ...
            if (currentItem->text().contains("\n"))
            {
                QWidget* currentWidget = dialogUi->table->cellWidget(currentItem->row(), currentItem->column());

                // if it is not set ...
                if (!currentWidget)
                {
                    QListWidget* listEditor = new QListWidget();
                    listEditor->setObjectName(QString("listedit_r%1").arg(currentItem->row()));
                    listEditor->installEventFilter(dialogUi->listKeyEvents);
                    listEditor->addItems(currentItem->text().split("\n", QString::SkipEmptyParts));
                    listEditor->setEditTriggers(QListWidget::AnyKeyPressed);
                    listEditor->itemDelegate()->setObjectName(QString("list_itemdelegate_%1").arg(currentItem->row()));

                    connect(listEditor, &QListWidget::itemDoubleClicked, [=](QListWidgetItem* item) { listEditor->editItem(item); });
                    connect(listEditor->itemDelegate(), &QAbstractItemDelegate::commitData, this, &MainDialog::commitEditorData);

                    // set list widget as cell widget.
                    dialogUi->table->setCellWidget(currentItem->row(), currentItem->column(), listEditor);

                    // for each item set flags as "editable"
                    for (int i = 0; i < listEditor->count(); ++i) {
                        QListWidgetItem* item = listEditor->item(i);
                        item->setFlags(item->flags() | Qt::ItemIsEditable);
                    }

                    // set focus on it.
                    listEditor->setFocus();
                    listEditor->setCurrentRow(0);
                    listEditor->item(0)->setSelected(true);

                    qDebug() << "MainDialog" << endl;
                    openedEditor = false;
                }
                else
                {
                    dialogUi->table->closePersistentEditor(dialogUi->table->selectedItems().at(0));
                    openedEditor = false;
                }
            }
            else
            {   // if it has only a single value
                dialogUi->table->editItem(currentItem);
                openedEditor = true;
            }
        }
    }

    void MainDialog::operateListEditor()
    {
        qDebug() << "[operateListEditor]";

        QModelIndex currentListEditorIndex = dialogUi->table->selectionModel()->currentIndex(); // get position of listWidget in the table
        QListWidget* list =
                retype<QListWidget*>(dialogUi->table->cellWidget(currentListEditorIndex.row(),
                                                                 currentListEditorIndex.column())); // get list widget

        if (openedEditor) {
            list->closePersistentEditor(list->currentItem());
            openedEditor = false;
        } else {
            list->editItem(list->currentItem());
            openedEditor = true;
        }
    }

    void MainDialog::checkEditorState(QItemSelection /*selected*/, QItemSelection deselected)
    {
        // on initialization (empty list) this is going to crash.
        if (deselected.empty()) return;

        foreach (QModelIndex index, deselected.indexes()) {
            QTableWidgetItem* cell = dialogUi->table->item(index.row(), index.column());
            dialogUi->table->closePersistentEditor(cell);
        }
        openedEditor = false;

        dialogUi->table->setFocus(); // set focus from list widget to table
    }

    void MainDialog::addVariableDialog()
    { dialogUi->addVariableDialog->dialog->show(); }

    void MainDialog::addVariable()
    {
        QTableWidgetItem* varNameItem = new QTableWidgetItem(dialogUi->addVariableDialog->nameEdit->text());
        QTableWidgetItem* varValueItem = new QTableWidgetItem(dialogUi->addVariableDialog->valueEdit->text());

        dialogUi->table->insertRow(dialogUi->table->rowCount());

        dialogUi->table->setItem(dialogUi->table->rowCount() - 1, 0, varNameItem);
        dialogUi->table->setItem(dialogUi->table->rowCount() - 1, 1, varValueItem);

        dialogUi->addVariableDialog->dialog->hide();

        // reset
        dialogUi->addVariableDialog->nameEdit->setText("");
        dialogUi->addVariableDialog->valueEdit->setText("");
    }

    void MainDialog::validateVariableName(QString currentText)
    {
        bool disabled = true;

        // if already contains that variable name, disallow adding of a new variable
        if (variables.contains(currentText)) disabled = true;
        else disabled = false;

        QLineEdit* nameEdit = reinterpret_cast<QLineEdit*>(sender());
        QDialogButtonBox* buttonBox = nameEdit->parentWidget()->findChildren<QDialogButtonBox*>().at(0);

        if (buttonBox)
            foreach (QAbstractButton* button, buttonBox->buttons())
                 if (buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
                     button->setDisabled(disabled);
    }

    void MainDialog::resetVariables()
    {
         QMultiHash<QString, QVariant> env = defaults;

         // reset all rows.
         for (int row = 0; row < dialogUi->table->rowCount(); row++)
         {
             QString key = dialogUi->table->item(row, 0)->text();
             if (env.keys().contains(key))
             {
                  QVariant data = dialogUi->table->model()->data(dialogUi->table->model()->index(row, 1));

                  switch (data.type())
                  {
                       case QVariant::String:
                            dialogUi->table->item(row, 1)->setText(env.value(key).toString());
                       break;
                       case QVariant::StringList:
                            dialogUi->table->item(row, 1)->setText(env.value(key).toStringList().join("\n"));
                       break;

                       default: { qDebug() << "Unexpected situation occured."; } break;
                  }
             }
             else // remove the row
                 dialogUi->table->removeRow(row);
         }
    }

    void MainDialog::updateVariable(QModelIndex cell, const QString &find, const QString &replace)
    {
        qDebug() << cell << find << replace;
    }

    void MainDialog::contextMenu(QPoint)
    {
        QMenu menu;

        QAction* editAction = menu.addAction("Edit variable");
        connect(editAction, &QAction::triggered, this, &MainDialog::editVariable);

        QAction* removeAction = menu.addAction("Remove variable");
        connect(removeAction, &QAction::triggered, this, &MainDialog::removeVariable);

        menu.exec(QCursor::pos());
    }

    void MainDialog::commitEditorData(QWidget *editor)
    {
        QLineEdit* lineEdit = retype<QLineEdit*>(editor);
        QString senderName = sender()->objectName();
        QTableWidgetItem* currentItem = dialogUi->table->currentItem();

        if (senderName == "vars_table_item_delegate")
        {
            // set to model the data...
            dialogUi->table->model()->setData(
                 dialogUi->table->model()->index(currentItem->row(),currentItem->column()), lineEdit->text());
        }
        else
        {
            QListWidget* currentListWidget = retype<QListWidget*>
                                                (dialogUi->table->cellWidget(currentItem->row(),currentItem->column()));
            // if the new content is empty, remove row...
            if (lineEdit->text().trimmed().isEmpty())
            {
                currentListWidget->model()->removeRow(currentListWidget->currentRow());

                // ...and if it is only single row now, get last thing from StringList
                // and jump from StringList to string.
                if (currentListWidget->count() == 1)
                {
                    QString lastItem = currentListWidget->item(0)->text();
                    currentItem->setText(lastItem.trimmed());
                }
            }
            else
            {
                currentListWidget->model()->setData(currentListWidget->currentIndex(), lineEdit->text());
            }
        }

        updateVariable(dialogUi->table->currentIndex(), currentItem->text(), lineEdit->text());
    }


    void MainDialog::editVariable()
    {
        dialogUi->editVariableDialog->nameEdit->setText(
            dialogUi->table->model()->data(
                 dialogUi->table->model()->index(
                      dialogUi->table->currentItem()->row(), 0
        )).toString());

        QVariant data = dialogUi->table->model()->data(dialogUi->table->model()->index(dialogUi->table->currentItem()->row(), 1));

        QStringList dataList = data.toString().split("\n");
        int count = dialogUi->editVariableDialog->itemList->count();
        foreach (QString item, dataList)
            dialogUi->editVariableDialog->itemList->insertItem(count - 1, item);

        dialogUi->editVariableDialog->valueEdit->setText(data.toString().replace("\n", ";"));
        dialogUi->editVariableDialog->multiValuesCheck->setChecked(true);
        dialogUi->editVariableDialog->dialog->show();

    }

    void MainDialog::removeVariable()
    {
        QString variableName = dialogUi->table->model()->data(dialogUi->table->model()->index(dialogUi->table->currentIndex().row(), 0)).toString();

        qDebug() << "[removeVariable] " << variableName << endl;
        variables.insert(variableName, QVariant(QString("")));

        dialogUi->table->removeRow(dialogUi->table->currentItem()->row());
    }

    void MainDialog::saveVariables()
    {
      //  expandEnvironment(variables);
    }

}

