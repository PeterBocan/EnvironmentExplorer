#ifndef MAINDIALOG_H
#define MAINDIALOG_H


#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QMultiHash>
#include <QtCore/QHashIterator>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QItemSelectionModel>

#include <QtGui/QKeyEvent>

#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialog>
#include <QtWidgets/QAction>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QTableWidgetItem>

#include <QtDebug>

namespace EnvironmentExplorer
{
    template <class NewType> NewType retype(QWidget* w)
    { return static_cast<NewType>(w); }

    /*
    * Event filter onto QTableWidget
    */
    class KeyEventFilter : public QObject
    {
        Q_OBJECT
    public:
         bool eventFilter(QObject *, QEvent *);

    signals:
         void returnPressed();
    };

    class MainDialog : public QWidget
    {
        Q_OBJECT

    public:
            MainDialog(QWidget *parent = 0);
            ~MainDialog();

    protected:
            /**
             * \fn Load variables from environment.
             */
            void loadEnvironmentVariables();

            /**
            * Enter hitted? Open/Close editor.
            */
            void operateLineEditor();

            /**
            * Checks editors when the selection has changed.
            */
            void checkEditorState(QItemSelection, QItemSelection);

            /**
            * Operate list widget.
            */
            void operateListEditor();

            /**
            *
            */
            void addVariableDialog();

            /**
            *
            */
            void addVariable();

            /**
            *
            */
            void validateVariableName(QString);

            /**
            *
            */
            void resetVariables();

            /**
            *
            */
            void editVariable();

            /**
            *
            */
            QMultiHash<QString, QVariant> parseSystemEnvironment();

            /**
            * Commit data from editor into a model
            */
            void commitEditorData(QWidget*);

            /**
            *
            */
            void updateVariable(QModelIndex cell,
                                const QString &find,
                                const QString &replace);

            /**
            *
            */
            void removeVariable();

            /**
            * Context menu
            */
            void contextMenu(QPoint pos);

            /**
            *
            */
            void saveVariables();

    private:
          // Using PIMPL for UI
          struct Ui;
          Ui* dialogUi;

          // Store all variables
          QMultiHash<QString, QVariant> variables;

          // Defaults
          QMultiHash<QString, QVariant> defaults;

          //
          bool openedEditor;
    };
}

#endif // MAINDIALOG_H
