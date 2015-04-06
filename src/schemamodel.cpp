/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "schemamodel.h"
#include "dbconnection.h"
#include "driver.h"
#include "notify.h"
#include "foreignkey.h"

#include <QColor>
#include <QSqlQuery>
#include <QVector>
#include <QDebug>
#include <QSqlError>

SqlSchemaModel::SqlSchemaModel(DbConnection& db, QString tableName, QObject *parent) :
    SqlModel(db, parent),
    tableName(tableName)
{
    res.clear();
}

void SqlSchemaModel::select() {
    beginResetModel();
    QMetaObject::invokeMethod(&db, "queryTableColumns", Qt::QueuedConnection, Q_ARG(TableData*, &columnData), Q_ARG(QString, tableName), Q_ARG(QObject*, this));
}

void SqlSchemaModel::selectComplete() {
    //data.columnNames.resize(columnCount());
    SqlModel::selectComplete();
}

int SqlSchemaModel::columnCount(const QModelIndex &parent) const {
    return SCHEMA_NUM_FIELDS;
}

int SqlSchemaModel::rowCount(const QModelIndex &parent) const {
    if(!dataSafe)
        return 0;

    int c = columnData.size();
    return c + (updatingRow == c ? 1 : 0);
}

Qt::ItemFlags SqlSchemaModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(index.isValid()) {
        switch(index.column()) {
        case SCHEMA_NAME:
        case SCHEMA_TYPE:
        case SCHEMA_LENGTH:
        case SCHEMA_DEFAULT:
        case SCHEMA_EXTRA:
        case SCHEMA_COMMENT:
        case SCHEMA_FOREIGNKEY:
            flags |= Qt::ItemIsEditable;
            break;
        case SCHEMA_UNSIGNED:
            if(data(this->index(index.row(), SCHEMA_TYPE)).toString().contains("int", Qt::CaseInsensitive))
                flags |= Qt::ItemIsUserCheckable;
            break;
        case SCHEMA_NULL:
            flags |= Qt::ItemIsUserCheckable;
            break;
        default: break;
        }
    }
    return flags;
}


bool SqlSchemaModel::columnIsBoolType(int col) const {
    return col == SCHEMA_UNSIGNED || col == SCHEMA_NULL;
}

// todo clean up
QString SqlSchemaModel::schemaQuery(const QVector<QVariant> def) {
    QString fkq = "";
    ForeignKey fk = def[SCHEMA_FOREIGNKEY].value<ForeignKey>();
    if(!fk.column.isNull() || !fk.constraint.isNull()) {
        if(!fk.constraint.isNull())
            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, "ALTER TABLE " + tableName + " DROP FOREIGN KEY \"" + fk.constraint + "\""), Q_ARG(QObject*, this));

        fk.constraint = "FK_" + tableName.toUpper() + "_" + def[SCHEMA_NAME].toString().toUpper() + "_" + fk.table.toUpper() + "_" + fk.column.toUpper();
        setData(index(updatingRow, SCHEMA_FOREIGNKEY), QVariant::fromValue<ForeignKey>(fk), Qt::EditRole);

        if(!fk.column.isNull())
            fkq += ", ADD CONSTRAINT \""+fk.constraint+"\" FOREIGN KEY (\"" + def[SCHEMA_NAME].toString() + "\") REFERENCES \"" + fk.table + "\" (\"" + fk.column + "\")";
    }

    return "\"" + def[SCHEMA_NAME].toString() + "\" " +
    (def[SCHEMA_TYPE].toString().isEmpty() ? "TEXT" : def[SCHEMA_TYPE].toString()) +
    (!def[SCHEMA_LENGTH].toString().isEmpty() ? "(" + def[SCHEMA_LENGTH].toString() + ")" : "") +
    (def[SCHEMA_UNSIGNED].toBool() ? " UNSIGNED" : "") +
    (!def[SCHEMA_NULL].toBool() ? " NOT NULL" : "") +
    (!def[SCHEMA_DEFAULT].toString().isEmpty() ? " DEFAULT " + def[SCHEMA_DEFAULT].toString() : "") +
    (!def[SCHEMA_EXTRA].toString().isEmpty() ? " " + def[SCHEMA_EXTRA].toString() : "") +
    (!def[SCHEMA_COMMENT].toString().isEmpty() ? " COMMENT '" + def[SCHEMA_COMMENT].toString() + "'" : "") +
    fkq;
}

bool SqlSchemaModel::submit() {
    if(updatingRow != -1 && currentRowModifications.count() > 0) {
        if(updatingRow == columnData.size()) {
            if(currentRowModifications[SCHEMA_NAME].isNull()) {
                qDebug() << "Cannot create unnamed column";
                return false;
            }

            if(currentRowModifications[SCHEMA_TYPE].toString().isEmpty())
                currentRowModifications[SCHEMA_TYPE] = "TEXT";

            QVector<QVariant> newColumn;
            newColumn.resize(columnCount());

            for(auto it = currentRowModifications.cbegin(); it != currentRowModifications.cend(); ++it)
                newColumn[it.key()] = it.value();

            QString updateQuery = "ALTER TABLE \"" + tableName + "\" ADD COLUMN " + schemaQuery(newColumn);
            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, updateQuery), Q_ARG(QObject*, this));

        } else {
            QVector<QVariant> newColumn = columnData[updatingRow];
            newColumn[SCHEMA_FOREIGNKEY] = QVariant();
            for(auto it = currentRowModifications.cbegin(); it != currentRowModifications.cend(); ++it)
                newColumn[it.key()] = it.value();

            QString oldColumnName = originalColumnName.isEmpty() ? newColumn[SCHEMA_NAME].toString() : originalColumnName;
            QString updateQuery = QString("ALTER TABLE ") + "\"" + tableName + "\" CHANGE " + "\"" + oldColumnName + "\" " + schemaQuery(newColumn);

            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, updateQuery), Q_ARG(QObject*, this));
            originalColumnName.clear(); //< actually should probably be done after this was successful
        }

        emit schemaModified(tableName);
        return true;
    }

    return false;
}

bool SqlSchemaModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.isValid() && role == Qt::EditRole && index.column() == SCHEMA_NAME) {
        originalColumnName = data(this->index(index.row(), SCHEMA_NAME)).toString();
    }

    return SqlModel::setData(index, value, role);
}

QVariant SqlSchemaModel::data(const QModelIndex &index, int role) const {

    if(!dataSafe)
        return QVariant();

    if(!index.isValid())
        return QVariant{};

    if(index.column() == SCHEMA_FOREIGNKEY) {
        if(role == EditorTypeRole)
            return SJCellEditForeignKey;
        if(role == Qt::DisplayRole) {
            ForeignKey fk = columnData[index.row()][index.column()].value<ForeignKey>();
            if(!fk.column.isNull())
                return QString(fk.constraint + ":" +fk.table + "." + fk.column);
            else
                return QString();
        }
    }

    if(columnIsBoolType(index.column())) {
        if(role == Qt::CheckStateRole) {
            if(index.row() == updatingRow && !currentRowModifications[index.column()].isNull())
                return currentRowModifications[index.column()].toBool() ? Qt::Checked : Qt::Unchecked;
            if(index.row() < columnData.size())
                return columnData[index.row()][index.column()].toBool() ? Qt::Checked : Qt::Unchecked;
        }
    } else {
        if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole) && index.row() < rowCount() && index.column() < columnCount()) {
            if(index.row() == updatingRow && currentRowModifications.contains(index.column()))
                return currentRowModifications[index.column()];
            else if(index.row() < columnData.size())
                return columnData[index.row()][index.column()];
        }
    }
    return QVariant();
}

QVariant SqlSchemaModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole) {
        switch(section) {
        case SCHEMA_NAME: return "Name";
        case SCHEMA_TYPE: return "Type";
        case SCHEMA_LENGTH: return "Length";
        case SCHEMA_UNSIGNED: return "Unsigned";
        case SCHEMA_NULL: return "Allow Null";
        case SCHEMA_KEY: return "Key";
        case SCHEMA_DEFAULT: return "Default";
        case SCHEMA_EXTRA: return "Extra";
        case SCHEMA_COMMENT: return "Comment";
        case SCHEMA_FOREIGNKEY: return "Foreign Key";
        default: break;
        }
    }
    return QVariant();
}

bool SqlSchemaModel::deleteRows(QSet<int> rows) {
    beginResetModel();
    for(int i : rows) {
        QString query("ALTER TABLE \"" + tableName + "\" DROP COLUMN \"" + columnData.at(i).at(SCHEMA_NAME).toString() + "\"");
        QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this));
        columnData.remove(i);
    }
    select(); // todo rowsremoved instead?
    return true;
}
