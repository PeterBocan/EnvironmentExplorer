#include "MainDialog.h"

namespace EnvironmentExplorer
{
    //
    // Installs event filter for
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
    //
    //
    struct MainDialog::Ui
    {
        QVBoxLayout* layout;
        QTableWidget* table;
        QDialogButtonBox* buttonBox;

        QPushButton* saveButton,
                   * resetButton,
                   * addButton,
                   * closeButton;

        // Handle key events of the QTableWidget
        KeyEventFilter* tableKeyEvents;

        Ui::Ui() : tableKeyEvents(new KeyEventFilter)
        {
            table = new QTableWidget();
            table->setColumnCount(2);
            table->installEventFilter(tableKeyEvents); // install event filter
            table->setVerticalScrollMode(QTableWidget::ScrollPerPixel);
            table->setHorizontalScrollMode(QTableWidget::ScrollPerPixel);
            table->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));

            buttonBox = new QDialogButtonBox(Qt::Horizontal);
            addButton = buttonBox->addButton(QString("Add"), QDialogButtonBox::ActionRole);
            saveButton = buttonBox->addButton(QDialogButtonBox::Save);
            resetButton = buttonBox->addButton(QDialogButtonBox::Reset);
            closeButton = buttonBox->addButton(QDialogButtonBox::Close);

            layout = new QVBoxLayout();
            layout->setContentsMargins(5,5,5,5);
            layout->addWidget(table);
            layout->addWidget(buttonBox);
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
        setMinimumSize(450, 600);

        // connect buttons to actions
        connect(dialogUi->closeButton, &QPushButton::pressed, this, &QDialog::close);

        // connect signals to slots
        connect(dialogUi->tableKeyEvents, &KeyEventFilter::returnPressed, this, &MainDialog::operateEditor);
        connect(dialogUi->table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainDialog::checkEditorState);

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

    /*
    * Load environment variables from the OS.
    */
    void MainDialog::loadEnvironmentVariables()
    {
        QStringList vars = QProcessEnvironment::systemEnvironment().toStringList();

        // set count of rows
        dialogUi->table->setRowCount(vars.count());

        // push lines...
        for (int i = 0; i < vars.count(); ++i)
        {
            // keyValueRelation[0] - key name
            // keyValueRelation[1] - value(s)
            QStringList keyValueRelation = vars[i].split("=");
            QTableWidgetItem* variableName = new QTableWidgetItem(keyValueRelation.at(0));
            QTableWidgetItem* variableValues = new QTableWidgetItem();

            // if value contains dir separator, split them...
            if (keyValueRelation.at(1).contains(";"))
            {
                QStringList values = keyValueRelation.at(1).split(";");
                variables.insert(keyValueRelation.at(0), values);

                QString textValues = keyValueRelation.at(1);
                variableValues->setText(textValues.replace(";", "\n")); // replace separator with new line
            }
            else // if not, add into the table.
            {
                variables.insert(keyValueRelation.at(0), keyValueRelation.at(1));
                variableValues->setText(keyValueRelation.at(1));
            }

            dialogUi->table->setItem(i, 0, variableName);
            dialogUi->table->setItem(i, 1, variableValues);
        }

        // resize table to its content
        dialogUi->table->resizeColumnsToContents();
        dialogUi->table->resizeRowsToContents();
    }

    // Will be executed on key pressed event
    void MainDialog::operateEditor()
    {
        if (openedEditor) {
            dialogUi->table->closePersistentEditor(dialogUi->table->selectedItems().at(0));
            openedEditor = false;
        } else {
            dialogUi->table->openPersistentEditor(dialogUi->table->selectedItems().at(0));
            openedEditor = true;
        }
    }

    void MainDialog::checkEditorState(QItemSelection /*selected*/, QItemSelection deselected)
    {
        if (openedEditor) // if it is already opened, close it!
        {
            // on initialization (empty list) this is going to crash.
            if (deselected.empty()) return;

            foreach (QModelIndex index, deselected.indexes())
            {
                QTableWidgetItem* cell = dialogUi->table->item(index.row(), index.column());
                dialogUi->table->closePersistentEditor(cell);
            }
            openedEditor = false;
            return;
        }
        else // if it is not, open it up.
        {


        }
    }
}

