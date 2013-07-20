
/*
* This is a part of EnvironmentExplorer program
* which is licensed under LGPLv2.
*
* Github: https://github.com/PeterBocan/EnvironmentExplorer
* Author: https://twitter.com/PeterBocan
*/

#include "MainDialogUi.h"

#include <QRegExpValidator>
#include <QKeyEvent>
#include <QRegExp>
#include <QDebug>

namespace EnvironmentExplorer
{
    bool DeleteKeyEventFilter::eventFilter(QObject *obj, QEvent *e)
    {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent* event = dynamic_cast<QKeyEvent*>(e);
            if (event->key() == Qt::Key_Delete) {
                emit deleteKeyPressed();
                return true;
            }
        }
        return obj->eventFilter(obj,e);
    }

    VariableDialog::VariableDialog(QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle("variable dialog");
        setMinimumSize(300, 100);

        mainLayout = new QGridLayout(this);

        QRegExp rx("^[A-Z][0-9A-Z]*$");
        QRegExpValidator* v = new QRegExpValidator(rx);
        nameEdit = new QLineEdit();
        nameEdit->setValidator(v);

        nameLabel = new QLabel("Name:");
        nameLabel->setBuddy(nameEdit);
        connect(nameEdit, &QLineEdit::textChanged, [&](const QString &text){
            if (text.trimmed().isEmpty() ||
                text.contains(" ") ||
                valueEdit->text().isEmpty())
                addButton->setDisabled(true);
            else
                addButton->setDisabled(false);
        });

        valueEdit = new QLineEdit();
        valueLabel = new QLabel("Value:");
        valueLabel->setBuddy(valueEdit);

        connect(valueEdit, &QLineEdit::textChanged, [&](const QString &text) {
            if (text.trimmed().isEmpty() ||
                text.contains(" ") ||
                nameEdit->text().isEmpty())
                addButton->setDisabled(true);
            else
                addButton->setDisabled(false);
        });

        scopeLabel = new QLabel("Scope:");
        scopeBox = new QComboBox();
        scopeBox->addItems(QStringList() << "Global (System)" << "Local (User)");

        multipleValuesCheck = new QCheckBox("Multiple values");
        multipleValuesCheck->setChecked(false);
        connect(multipleValuesCheck, &QCheckBox::stateChanged, [&](int state){
            if (state == Qt::Checked) {
                mainLayout->removeWidget(valueEdit);
                valueEdit->hide();
                mainLayout->addWidget(itemsList, 3, 1);
                itemsList->show();
                resize(sizeHint());
            } else {
                mainLayout->removeWidget(itemsList);
                itemsList->hide();
                mainLayout->addWidget(valueEdit, 3, 1);
                valueEdit->show();
                resize(sizeHint());
            }
        });

        itemsList = new QListWidget();
        DeleteKeyEventFilter* filter = new DeleteKeyEventFilter();
        itemsList->installEventFilter(filter);
        itemsList->setEditTriggers(QListWidget::DoubleClicked);

        QListWidgetItem* addValueItem = new QListWidgetItem(itemsList);
        addValueItem->setText("Add new value...");
        addValueItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);

        connect(filter, &DeleteKeyEventFilter::deleteKeyPressed, [&](){
            QList<QListWidgetItem*> selection = itemsList->selectedItems();

            foreach (QListWidgetItem* item, selection)
            {
               delete item;
            }

        });

        connect(itemsList, &QListWidget::itemDoubleClicked,
            [&](QListWidgetItem* item){
                if (item == itemsList->item(itemsList->count()-1)) {
                    QListWidgetItem* newItem = new QListWidgetItem();
                    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
                    itemsList->insertItem(itemsList->count() - 1, newItem);
                }
            });

        connect(itemsList->model(), &QAbstractItemModel::rowsInserted,
           [&](const QModelIndex &/*parent*/, int /*first*/, int /*last*/)
                {
                    int count = itemsList->count();
                    bool valsAreEmpty = true;

                    for (int i = 0; i < count-1; i++)
                    {
                        QString val = itemsList->item(i)->text();
                        qDebug() << val;
                        if (!val.trimmed().isEmpty())
                            { valsAreEmpty = false; break; }
                    }

                    if (count > 1 && !valsAreEmpty)
                        addButton->setDisabled(false);
                    else
                        addButton->setDisabled(true);

                });

        dialogButtonBox = new QDialogButtonBox();
        addButton = dialogButtonBox->addButton("Add", QDialogButtonBox::AcceptRole);
        cancelButton = dialogButtonBox->addButton(QDialogButtonBox::Cancel);
        connect(addButton, &QPushButton::pressed, this, &QDialog::accept);

        addButton->setDisabled(true);
        connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &QDialog::close);

        mainLayout->addWidget(nameLabel, 0, 0);
        mainLayout->addWidget(nameEdit, 0, 1);
        mainLayout->addWidget(scopeLabel, 1, 0);
        mainLayout->addWidget(scopeBox, 1, 1);
        mainLayout->addWidget(multipleValuesCheck, 2, 1);
        mainLayout->addWidget(valueLabel, 3, 0, Qt::AlignTop|Qt::AlignRight);
        mainLayout->addWidget(valueEdit, 3, 1);
        mainLayout->addWidget(dialogButtonBox, 4, 0, 2, 0);
    }

    void VariableDialog::setVariableValue(const QVariant &value)
    {
        if (value.type() == QMetaType::QString)
        {
            QString val = value.toString();
            if (val.contains("\n"))
            {
                QStringList list = val.split("\n");
                foreach(QString item, list) {
                    QListWidgetItem* listItem = new QListWidgetItem(item);
                    listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
                     itemsList->insertItem(itemsList->count() - 1, listItem);
                }

                multipleValuesCheck->setChecked(true);
                addButton->setDisabled(((val.isEmpty()) ? true : false));
            }
            else
            {
                valueEdit->setText(value.toString());
                multipleValuesCheck->setChecked(false);
                addButton->setDisabled(((val.isEmpty()) ? true : false));
            }
        }
    }

    void VariableDialog::setDialogMode(DialogMode mode)
    {
        dialogMode = mode;

        itemsList->clear();
        QListWidgetItem* item = new QListWidgetItem("Add new item...", itemsList);
        itemsList->addItem(item);

        if (mode == AddVariable)
        {
            setWindowTitle("Add variable...");
            addButton->setText("Add");
        }
        else
        {
            setWindowTitle("Edit variable...");
            addButton->setText("Save");
            addButton->setDisabled(false);
        }
    }

    QVariant VariableDialog::variableValue() const
    {
        if (!multipleValuesCheck->isChecked())
            return QVariant(valueEdit->text());
        else
        {
            QStringList list;
            for (int i = 0; i < itemsList->count(); ++i)
                list.append(itemsList->item(i)->text());

            return QVariant(list);
        }
    }

    Variable::Type VariableDialog::variableType()
    {
        if (scopeBox->currentText() == "Global (System)")
            return Variable::Global;
        else
            return Variable::User;
    }

    void VariableDialog::setVariableType(Variable::Type t)
    {
        if (t == Variable::Global)
            scopeBox->setCurrentIndex(0);
        else
            scopeBox->setCurrentIndex(1);
    }
}
