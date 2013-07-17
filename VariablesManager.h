#ifndef VARIABLESMANAGER_H
#define VARIABLESMANAGER_H

/*
* This is a part of EnvironmentExplorer program
* which is licensed under LGPLv2.
*
* Github: https://github.com/PeterBocan/EnvironmentExplorer
* Author: https://twitter.com/PeterBocan
*/

//
// VariablesManager class handles a variable management.
//

#include <QSettings>
#include <QVariant>
#include <QObject>
#include <QList>
#include <QHash>

class QSettings;
namespace EnvironmentExplorer
{
    struct Variable
    {
        // Represents the name of env. var.
        QString name;
        // Represents the current value.
        QVariant value;
        // Represents the default value
        // (only if it from the environment)
        QVariant defaultValue;

        enum Type { Global, User };
        Type type;
    };

    class VariablesManager : public QObject
    {
        Q_OBJECT

    public:

          VariablesManager(QObject* parent = 0);

          void addUserVariable(const QString &name,
                               const QVariant &val);

          void addGlobalVariable(const QString &name,
                                 const QVariant &val);

          void loadVariables();
          void saveVariables();

          bool contains(const QString &name) const;

          bool replaceVariable(const QString &name,
                               const Variable &var);

          void removeVariable(const QString &name);

          void removeVariable(const QString &name,
                              Variable::Type type);

          QList<Variable> userEnvironment() const;
          QList<Variable> systemEnvironment() const;

          const Variable& variable(const QString& name) const;

          void dumpVariables(Variable::Type t);

    protected:

          void addVariable(const QString &name,
                           const QVariant &val,
                           Variable::Type type = Variable::User);


    private:
          QHash<QString, Variable> parseEnvironment(const QSettings &set,
                                                    Variable::Type t);

          QSettings* machineSettings,
                   * userSettings;

          QList<QString> variables;
          QHash<QString, Variable> globals, locals;

    };

}
#endif // VARIABLESMANAGER_H
