#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QtCore/QEvent>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QItemSelectionModel>

#include <QtGui/QKeyEvent>

#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QTableWidgetItem>

#if defined(QT_DEBUG)
#include <QtDebug>
#endif

namespace EnvironmentExplorer
{
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
         void arrowUpPressed();
         void arrowDownPressed();
         void arrowLeftPressed();
         void arrowRightPressed();
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
            void operateEditor();

            /**
            * Checks editors when the selection has changed.
            */
            void checkEditorState(QItemSelection, QItemSelection);


    private:
          // Using PIMPL for UI
          struct Ui;
          Ui* dialogUi;

          // Store all variabless
          QMap<QString, QVariant> variables;

          //
          bool openedEditor;

    };
}

#endif // MAINDIALOG_H
