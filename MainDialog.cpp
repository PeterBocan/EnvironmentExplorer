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
                case Qt::Key_Up: { arrowUpPressed(); return true; }
                case Qt::Key_Down: { arrowDownPressed(); return true; }
                case Qt::Key_Left: { arrowLeftPressed(); return true; }
                case Qt::Key_Right: { arrowRightPressed(); return true; }

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

        Ui::Ui()
        {
            table = new QTableWidget();
            table->setColumnCount(2);
            table->installEventFilter(new KeyEventFilter); // install event filter
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
        : QWidget(parent), dialogUi(new MainDialog::Ui)
    {
        setWindowTitle(tr("Environment explorer"));
        setMinimumSize(450, 600);

        // connect buttons to actions
        connect(dialogUi->closeButton, &QPushButton::pressed, this, &QDialog::close);


        // set up layout
        setLayout(dialogUi->layout);

        // load environment
        loadEnvironmentVariables();
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
}

