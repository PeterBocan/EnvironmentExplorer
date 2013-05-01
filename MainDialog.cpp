#include "MainDialog.h"

namespace EnvironmentExplorer
{

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
                   * addButton,
                   * closeButton;

        // Add Variable Dialog
        QDialog* variableDialog;
        QGridLayout* dialogLayout;
        QDialogButtonBox* variableDialogButtonBox;

        QPushButton* addVariableButton,
                   * cancelDialogButton;

        QLabel* nameEditLabel,
              * valueEditLabel;

        QLineEdit* nameEdit,
                 * valueEdit;


        // Handle key events of the QTableWidget
        KeyEventFilter* tableKeyEvents,
                      * listKeyEvents;

        Ui::Ui() : tableKeyEvents(new KeyEventFilter),
                    listKeyEvents(new KeyEventFilter)
        {
            table = new QTableWidget();
            table->setColumnCount(2);
            table->installEventFilter(tableKeyEvents); // install event filter
            table->setVerticalScrollMode(QTableWidget::ScrollPerPixel);
            table->setHorizontalScrollMode(QTableWidget::ScrollPerPixel);
            table->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
            table->itemDelegate()->setObjectName("vars_table_item_delegate");

            buttonBox = new QDialogButtonBox(Qt::Horizontal);
            addButton = buttonBox->addButton(QString("Add"), QDialogButtonBox::ActionRole);
            saveButton = buttonBox->addButton(QDialogButtonBox::Save);
            resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
            closeButton = buttonBox->addButton(QDialogButtonBox::Close);

            layout = new QVBoxLayout();
            layout->setContentsMargins(5,5,5,5);
            layout->addWidget(table);
            layout->addWidget(buttonBox);

            // Add Variable Dialog...

            variableDialog = new QDialog();
            variableDialog->setMinimumSize(300, 100);
            dialogLayout = new QGridLayout(variableDialog);

            nameEdit = new QLineEdit();
            nameEditLabel = new QLabel("Name:");
            nameEditLabel->setBuddy(nameEdit);

            valueEdit = new QLineEdit();
            valueEditLabel = new QLabel("Value:");
            valueEditLabel->setBuddy(valueEdit);

            variableDialogButtonBox = new QDialogButtonBox();
            addVariableButton = variableDialogButtonBox->addButton("Add", QDialogButtonBox::AcceptRole);
            cancelDialogButton = variableDialogButtonBox->addButton("Cancel", QDialogButtonBox::RejectRole);
            connect(variableDialogButtonBox, &QDialogButtonBox::rejected, variableDialog, &QDialog::close);

            dialogLayout->addWidget(nameEditLabel, 0, 0);
            dialogLayout->addWidget(nameEdit, 0, 1);
            dialogLayout->addWidget(valueEditLabel, 1, 0);
            dialogLayout->addWidget(valueEdit, 1, 1);
            dialogLayout->addWidget(variableDialogButtonBox, 2, 0, 2, 0);
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

        // connect signals to slots
        connect(dialogUi->table, &QTableWidget::doubleClicked, this, &MainDialog::operateLineEditor);
        connect(dialogUi->tableKeyEvents, &KeyEventFilter::returnPressed, this, &MainDialog::operateLineEditor);
        connect(dialogUi->table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainDialog::checkEditorState);
        connect(dialogUi->table->itemDelegate(), &QAbstractItemDelegate::commitData, this, &MainDialog::commitEditorData);

        // Add Variable dialog
        connect(dialogUi->variableDialogButtonBox, &QDialogButtonBox::accepted, this, &MainDialog::addVariable);
        connect(dialogUi->nameEdit, &QLineEdit::textChanged, this, &MainDialog::validateVariableName);
        connect(dialogUi->listKeyEvents, &KeyEventFilter::returnPressed, this, &MainDialog::operateListEditor);


        // set up layout
        setLayout(dialogUi->layout);

        // parse system variables
        defaults = parseSystemEnvironment();

        // load environment
        loadEnvironmentVariables();

        // select the first one
        QModelIndex index = dialogUi->table->model()->index(0,0);
        dialogUi->table->selectionModel()->select(index, QItemSelectionModel::Select);
    }

    MainDialog::~MainDialog()
    { delete dialogUi; }

    /*
    * Load environment variables from the OS.
    */
    void MainDialog::loadEnvironmentVariables()
    {
        QMultiHash<QString, QVariant> vars = defaults;
        int varsCount = defaults.keys().count();

        // set count of rows
        dialogUi->table->setRowCount(varsCount);

        // push lines into the table...
        QList<QString> keys = vars.uniqueKeys();
        int n = 0;
        foreach(QString key, keys)
        {
            QTableWidgetItem* variableName = new QTableWidgetItem(key);
            QTableWidgetItem* variableValues = new QTableWidgetItem();

            // set text
            QVariant value = vars.value(key);
            switch (value.type())
            {
                case QVariant::String: variableValues->setText(value.toString()); break;
                case QVariant::StringList: variableValues->setText(value.toStringList().join("\n")); break;
                default: qDebug() << "" << endl;
            }

            dialogUi->table->setItem(n, 0, variableName);
            dialogUi->table->setItem(n, 1, variableValues);
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
            qDebug() << "[operateLineEditor] close persistent";
            dialogUi->table->closePersistentEditor(dialogUi->table->selectedItems().at(0));
            openedEditor = false;
        } else {
            qDebug() << "[operateLineEditor] open persistent ";
            QTableWidgetItem *currentItem = dialogUi->table->selectedItems().at(0);

            // if it has multiple lines ...
            if (currentItem->text().contains("\n"))
            {
                qDebug() << "[operateLineEditor] add list widget";
                QWidget* currentWidget = dialogUi->table->cellWidget(currentItem->row(), currentItem->column());

                // if it is not set ...
                if (!currentWidget) {
                    QListWidget* listEditor = new QListWidget();
                    listEditor->setObjectName(QString("listedit_r%1").arg(currentItem->row()));
                    listEditor->installEventFilter(dialogUi->listKeyEvents);
                    listEditor->addItems(currentItem->text().split("\n"));
                    listEditor->setEditTriggers(QListWidget::EditKeyPressed);
                    listEditor->itemDelegate()->setObjectName(QString("list_itemdelegate_%1").arg(currentItem->row()));

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
                } else {
                    dialogUi->table->closePersistentEditor(dialogUi->table->selectedItems().at(0));
                    openedEditor = false;
                }
            }
            else { // if it has only a single value
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
                retype<QListWidget*>(dialogUi->table->cellWidget(currentListEditorIndex.row(), currentListEditorIndex.column())); // get list widget

        if (openedEditor) {
            list->closePersistentEditor(list->currentItem());
            openedEditor = false;
            qDebug() << "\tClosing editor." << endl;
        } else {
            list->editItem(list->currentItem());
            openedEditor = true;
            qDebug() << "\tOpening editor." << endl;
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
    { dialogUi->variableDialog->show(); }

    void MainDialog::addVariable()
    {
        QTableWidgetItem* varNameItem = new QTableWidgetItem(dialogUi->nameEdit->text());
        QTableWidgetItem* varValueItem = new QTableWidgetItem(dialogUi->valueEdit->text());

        dialogUi->table->insertRow(dialogUi->table->rowCount());

        dialogUi->table->setItem(dialogUi->table->rowCount() - 1, 0, varNameItem);
        dialogUi->table->setItem(dialogUi->table->rowCount() - 1, 1, varValueItem);

        dialogUi->variableDialog->hide();

        // reset
        dialogUi->nameEdit->setText("");
        dialogUi->valueEdit->setText("");
    }

    void MainDialog::validateVariableName(QString currentText)
    {
        bool disabled = true;

        // if already contains that variable name, disallow adding of a new variable
        if (variables.keys().contains(currentText)) disabled = true;
        else disabled = false;

        dialogUi->addVariableButton->setDisabled(disabled);
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

    QMultiHash<QString, QVariant> MainDialog::parseSystemEnvironment()
    {
        QMultiHash<QString, QVariant> result;
        QStringList systemVariables = QProcessEnvironment::systemEnvironment().toStringList();

        foreach (QString variable, systemVariables)
        {
            QStringList keyValueRelation = variable.split("=");
            if (keyValueRelation.at(1).contains(";")) {
                result.insert(keyValueRelation.at(0), QVariant(keyValueRelation.at(1).split(";")));
                continue;
            } else {
                result.insert(keyValueRelation.at(0), QVariant(keyValueRelation.at(1)));
                continue;
            }
        }

        return result;
    }

    void MainDialog::updateVariable(QModelIndex cell, const QString &find, const QString &replace)
    {

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

            currentListWidget->item(currentListWidget->currentRow())->setText(lineEdit->text());
            currentListWidget->model()->setData(currentListWidget->currentIndex(), lineEdit->text());
        }

        updateVariable(dialogUi->table->currentIndex(), currentItem->text(), lineEdit->text());
    }

}

