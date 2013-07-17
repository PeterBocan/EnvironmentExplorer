#include "VariablesManager.h"

#include <QSettings>
#include <QStringList>
#include <QDebug>


namespace EnvironmentExplorer
{

    VariablesManager::VariablesManager(QObject *parent)
        : QObject(parent)
    {
        machineSettings = new QSettings("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
                                        QSettings::NativeFormat);
        userSettings = new QSettings("HKEY_CURRENT_USER\\Environment",
                                     QSettings::NativeFormat);
    }

    void VariablesManager::loadVariables()
    {
        QStringList systemSettings = machineSettings->allKeys();
        QStringList localSettings = userSettings->allKeys();

        variables = systemSettings + localSettings;

        globals = parseEnvironment(*machineSettings, Variable::Global);
        locals = parseEnvironment(*userSettings, Variable::User);
    }

    QList<Variable> VariablesManager::userEnvironment() const
    { return locals.values(); }

    QList<Variable> VariablesManager::systemEnvironment() const
    { return globals.values(); }

    void VariablesManager::saveVariables()
    {
        QHash<QString, Variable>::iterator it = globals.begin();

        for (; it != globals.end(); ++it)
        {
            Variable var = it.value();

            // Nothing to do, if there is no change.
            if (var.value == var.defaultValue)
                continue;

            if (var.value.type() == QVariant::String)
            {
                QString value = var.value.toString();
                if (value.isEmpty())
                    machineSettings->remove(it.key());
                else
                    machineSettings->setValue(it.key(), value);
            }
            else
            {
                QStringList list = var.value.toStringList();
                bool itemsAreEmpty = true;

                foreach (QString item, list)
                    if (!item.isEmpty()) { itemsAreEmpty = false; break; }

                if (list.isEmpty() || itemsAreEmpty)
                    machineSettings->remove(it.key());
                else
                {
                    QString value = list.join(";");
                    machineSettings->setValue(it.key(), value);
                }
            }
        }

        it = locals.begin();
        for (; it != locals.end(); ++it)
        {
            Variable var = it.value();

            // Nothing to do, if there is no change.
            if (var.value == var.defaultValue)
                continue;

            if (var.value.type() == QVariant::String)
            {
                QString value = var.value.toString();
                if (value.isEmpty()) // empty strings are not needed
                    machineSettings->remove(it.key());
                else
                    machineSettings->setValue(it.key(), value);
            }
            else
            {
                QStringList list = var.value.toStringList();
                bool itemsAreEmpty = true;

                foreach (QString item, list)
                    if (!item.isEmpty()) { itemsAreEmpty = false; break; }

                if (list.isEmpty() || itemsAreEmpty)
                    machineSettings->remove(it.key());
                else
                {
                    QString value = list.join(";");
                    machineSettings->setValue(it.key(), value);
                }
            }
        }
    }

    void VariablesManager::dumpVariables(Variable::Type t)
    {
        if (t == Variable::Global)
            foreach (Variable v, globals)
                qDebug() << QString("%1=%2").arg(v.name, v.value.toString());
        else
            foreach (Variable v, locals)
                qDebug() << QString("%1=%2").arg(v.name, v.value.toString());
    }


    void VariablesManager::addGlobalVariable(const QString &name, const QVariant &val)
    { addVariable(name, val, Variable::Global); }

    void VariablesManager::addUserVariable(const QString &name, const QVariant &val)
    { addVariable(name, val); }

    void VariablesManager::addVariable(const QString &name, const QVariant &val, Variable::Type var)
    {
        if (var == Variable::Global)
        {
            if (globals.contains(name)) {
                globals[name].name = name;
                globals[name].value = val;

                if (var != globals[name].type) {
                    Variable v = globals[name];
                    v.type = var;
                    globals.remove(name);
                    locals.insert(name, v);
                }

            } else {
                Variable v;
                v.name = name;
                v.value = val;
                v.type = var;
                globals.insert(name, v);
            }
        }
        else
        {
            if (locals.contains(name)) {
                locals[name].name = name;
                locals[name].value = val;

                if (var != locals[name].type) {
                    Variable v = locals[name];
                    v.type = var;
                    locals.remove(name);
                    globals.insert(name, v);
                }
            } else {
                Variable v;
                v.name = name;
                v.value = val;
                v.type = var;
                locals.insert(name, v);
            }
        }
    }

    bool VariablesManager::contains(const QString &name) const
    { return variables.contains(name); }

    void VariablesManager::removeVariable(const QString &name)
    {
        if (contains(name))
        {
            if (globals.contains(name))
                globals[name].value = QVariant();
            else
                locals[name].value = QVariant();
        }
    }

    void VariablesManager::removeVariable(const QString &name, Variable::Type type)
    {
        if (type == Variable::Global) {
            if (globals.contains(name))
                globals[name].value = QVariant();
        } else {
            if (locals.contains(name))
                locals[name].value = QVariant();
        }
    }

    QHash<QString, Variable> VariablesManager::parseEnvironment(const QSettings &set,
                                                                Variable::Type t)
    {
        QHash<QString, Variable> result;
        foreach (QString key, set.allKeys())
        {
            Variable var;
            var.name = key;
            var.type = t;
            QString val = set.value(key).toString();

            if (val.contains(";"))
                var.defaultValue = var.value = val.split(";");
            else
                var.defaultValue = var.value = val;

            result.insert(key, var);
        }

        return result;
    }

    const Variable& VariablesManager::variable(const QString& name) const
    {
        if (locals.contains(name))
            return locals[name];

        if (globals.contains(name))
            return globals[name];

        Q_ASSERT(false);
        return Variable();
    }

    bool VariablesManager::replaceVariable(const QString &name, const Variable &var)
    {
        if (globals.contains(name))
        {
            globals.remove(name);
            globals.insert(name, var);
            return true;
        }

        if (locals.contains(name))
        {
            locals.remove(name);
            locals.insert(name, var);
            return true;
        }

        return false;
    }

}
