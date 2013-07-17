#ifndef MAINDIALOGUI_H
#define MAINDIALOGUI_H

/*
* This is a part of EnvironmentExplorer program
* which is licensed under LGPLv2.
*
* Github: https://github.com/PeterBocan/EnvironmentExplorer
* Author: https://twitter.com/PeterBocan
*/

//
// This is a UI implementation header file.
//

#include <QtCore/QStringList>

#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialog>
#include <QtWidgets/QAction>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QTableWidgetItem>

#include "VariablesManager.h"

namespace EnvironmentExplorer
{
    //
    // UI structure.
    //

    // VariableDialog
    class VariableDialog : public QDialog
    {
        Q_OBJECT

    public:
        VariableDialog(QWidget* parent = 0);

        enum DialogMode { AddVariable, EditVariable };
        DialogMode mode() const
        { return dialogMode; }
        void setDialogMode(DialogMode mode);

        void setVariableName(const QString &name)
        { nameEdit->setText(name); }

        void setVariableValue(const QVariant &value);

        void setVariableType(Variable::Type t);

        QString variableName() const
        { return nameEdit->text(); }

        QVariant variableValue() const;

        Variable::Type variableType();

    private:
        DialogMode dialogMode;

        QGridLayout* mainLayout;
        QDialogButtonBox* dialogButtonBox;

        QListWidget* itemsList;

        QPushButton* addButton,
                   * cancelButton;

        QLabel* nameLabel,
              * valueLabel,
              * scopeLabel;

        QLineEdit* nameEdit,
                 * valueEdit;

        QCheckBox* multipleValuesCheck;
        QComboBox* scopeBox;
    };

    struct UserInterface
    {
        QTableWidget* mainTable;
        QVBoxLayout* layout;

        QDialogButtonBox* buttonPanel;
        QPushButton* addButton,
                   * saveButton,
                   * resetButton,
                   * closeButton,
                   * cancelButton,
                   * exportButton;

        UserInterface()
        {
            layout = new QVBoxLayout();

            mainTable = new QTableWidget(0, 2);
            mainTable->setEditTriggers(QTableWidget::NoEditTriggers);
            mainTable->setContextMenuPolicy(Qt::CustomContextMenu);
            mainTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Value");
            mainTable->horizontalHeader()->setStretchLastSection(true);
            mainTable->setHorizontalScrollMode(QTableWidget::ScrollPerPixel);
            mainTable->setVerticalScrollMode(QTableWidget::ScrollPerPixel);
            mainTable->setSelectionBehavior(QAbstractItemView::SelectRows);
            layout->addWidget(mainTable);

            buttonPanel = new QDialogButtonBox();
            addButton = buttonPanel->addButton(QString("Add"), QDialogButtonBox::ActionRole);
            exportButton = buttonPanel->addButton(QString("Export"), QDialogButtonBox::ActionRole);
            saveButton = buttonPanel->addButton(QDialogButtonBox::Save);
            resetButton = buttonPanel->addButton(QDialogButtonBox::Reset);
            closeButton = buttonPanel->addButton(QDialogButtonBox::Close);
            layout->addWidget(buttonPanel);
        }
    };

}

#endif
