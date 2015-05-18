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

class SchemaConstraintsProxyModel : public QAbstractListModel {
    Q_OBJECT
public:
    SchemaConstraintsProxyModel(QString currentTable,
            const ConstraintMap& constraints, QObject *parent = 0) :
        QAbstractListModel(parent),
        tableName(currentTable),
        constraints(constraints)
    {
    }
    int rowCount(const QModelIndex&) const override {
        return constraints.count();
    }
    QVariant data(const QModelIndex& idx, int role) const override {
        if(idx.isValid()) {
            // there isn't really a numerical index to the constraints,
            // and order doesn't matter. So just loop through the map
            // until the counter has had enough
            int i = idx.row();
            auto it = constraints.begin();
            while(i--) it++;
            if(role == Qt::DisplayRole)
                return QString(it->type == ConstraintDetail::CONSTRAINT_FOREIGNKEY ? "Foreign Key" : "Unique") + " : " + it.key();
            else if(role == Qt::TextColorRole)
                return QColor::fromHsv(255.0 * it->sequence / (constraints.size()+1),180,180);
            else if(role == Qt::EditRole)
                return QVariant::fromValue(ConstraintMap::constraint(it));
        } else if(role == TableNameRole)
            return tableName;
        return QVariant();
    }
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override {
        if(value.isNull())
            emit removeConstraint(index.data(Qt::EditRole).value<Constraint>());
        else {
            if(index.isValid()) {
                Constraint oldConstraint = index.data(Qt::EditRole).value<Constraint>();
                emit removeConstraint(oldConstraint);
            }
            Constraint newConstraint = value.value<Constraint>();
            emit saveConstraint(newConstraint);
        }
        return true;
    }
    void beginReset() { beginResetModel(); }
    void resetDone() { endResetModel(); emit layoutChanged(); }
signals:
    void saveConstraint(Constraint c);
    void removeConstraint(Constraint c);
private:
    QString tableName;
    const ConstraintMap& constraints;
};

QAbstractItemModel* SqlSchemaModel::constraintsModel() const { return constraintsProxy; }

SqlSchemaModel::SqlSchemaModel(DbConnection& db, QString tableName, QObject *parent) :
    SqlModel(db, parent),
    tableName(tableName),
    constraintsProxy(new SchemaConstraintsProxyModel(tableName, schema.constraints, this))
{
    res.clear();
    connect(constraintsProxy, &SchemaConstraintsProxyModel::saveConstraint, this, &SqlSchemaModel::saveConstraint);
    connect(constraintsProxy, &SchemaConstraintsProxyModel::removeConstraint, this, &SqlSchemaModel::removeConstraint);
}

void SqlSchemaModel::select() {
    beginResetModel();
    constraintsProxy->beginReset();
    QMetaObject::invokeMethod(&db, "queryTableColumns", Qt::QueuedConnection, Q_ARG(Schema*, &schema), Q_ARG(QString, tableName), Q_ARG(QObject*, this));
}

void SqlSchemaModel::selectComplete() {
    //data.columnNames.resize(columnCount());
    constraintsProxy->resetDone();
    SqlModel::selectComplete();
}

int SqlSchemaModel::columnCount(const QModelIndex &parent) const {
    return SCHEMA_NUM_FIELDS;
}

int SqlSchemaModel::rowCount(const QModelIndex &parent) const {
    if(!dataSafe)
        return 0;

    int c = schema.columns.size();
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
QString SqlSchemaModel::schemaQuery(const std::array<QVariant,SCHEMA_NUM_FIELDS>& def) {
    QString fkq = "";
//    ForeignKey fk = def[SCHEMA_FOREIGNKEY].value<ForeignKey>();
//    if(!fk.column.isNull() || !fk.constraint.isNull()) {
//        if(!fk.constraint.isNull())
//            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, "ALTER TABLE " + tableName + " DROP FOREIGN KEY \"" + fk.constraint + "\""), Q_ARG(QObject*, this));

//        fk.constraint = "FK_" + tableName.toUpper() + "_" + def[SCHEMA_NAME].toString().toUpper() + "_" + fk.table.toUpper() + "_" + fk.column.toUpper();
//        setData(index(updatingRow, SCHEMA_FOREIGNKEY), QVariant::fromValue<ForeignKey>(fk), Qt::EditRole);

//        if(!fk.column.isNull())
//            fkq += ", ADD CONSTRAINT \""+fk.constraint+"\" FOREIGN KEY (\"" + def[SCHEMA_NAME].toString() + "\") REFERENCES \"" + fk.table + "\" (\"" + fk.column + "\")";
//    }

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

void SqlSchemaModel::saveConstraint(Constraint c) {
    Q_ASSERT(!c.name.isEmpty());
    QString q;
    if(c.detail.type == ConstraintDetail::CONSTRAINT_FOREIGNKEY) {
        q = "ALTER TABLE \"" + tableName + "\" "
        "ADD CONSTRAINT \"" + c.name + "\" FOREIGN KEY (\"" + c.detail.fk.column + "\") "
        "REFERENCES \"" + c.detail.fk.refTable + "\" (\"" + c.detail.fk.refColumn + "\")";
    } else if(c.detail.type == ConstraintDetail::CONSTRAINT_UNIQUE) {
        // todo
    }
    QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, q), Q_ARG(QObject*, this));
}

void SqlSchemaModel::removeConstraint(Constraint c) {
    QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, "ALTER TABLE \"" + tableName + "\" DROP FOREIGN KEY \"" + c.name + "\""), Q_ARG(QObject*, this));
}


bool SqlSchemaModel::submit() {
    if(updatingRow != -1 && currentRowModifications.count() > 0) {
        if(updatingRow == schema.columns.size()) {
            if(currentRowModifications[SCHEMA_NAME].isNull()) {
                qDebug() << "Cannot create unnamed column";
                return false;
            }

            if(currentRowModifications[SCHEMA_TYPE].toString().isEmpty())
                currentRowModifications[SCHEMA_TYPE] = "TEXT";

            std::array<QVariant,SCHEMA_NUM_FIELDS> newColumn;

            for(auto it = currentRowModifications.cbegin(); it != currentRowModifications.cend(); ++it)
                newColumn[it.key()] = it.value();

            QString updateQuery = "ALTER TABLE \"" + tableName + "\" ADD COLUMN " + schemaQuery(newColumn);
            QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, updateQuery), Q_ARG(QObject*, this));

        } else {
            std::array<QVariant, SCHEMA_NUM_FIELDS> newColumn = schema.columns[updatingRow];
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

    if(role == EditorTypeRole)
        return SJCellEditDefault;

//    if(index.column() == SCHEMA_FOREIGNKEY) {
//        if(role == EditorTypeRole)
//            return SJCellEditForeignKey;
//        if(role == Qt::DisplayRole) {
//            ForeignKey fk = columnData[index.row()][index.column()].value<ForeignKey>();
//            if(!fk.column.isNull())
//                return QString(fk.constraint + ":" +fk.table + "." + fk.column);
//            else
//                return QString();
//        }
//    }

    if(index.column() == SCHEMA_CONSTRAINTS) {
        if(role == Qt::TextColorRole && index.row() < schema.columns.length()) {
            QVariantList shades;
            for(QString name : schema.columns.at(index.row()).at(SCHEMA_CONSTRAINTS).toStringList())
                shades << (double) schema.constraints[name].sequence / (schema.constraints.count() + 1);
            return shades;
        }
        return QVariant();
    }

    if(columnIsBoolType(index.column())) {
        if(role == Qt::CheckStateRole) {
            if(index.row() == updatingRow && !currentRowModifications[index.column()].isNull())
                return currentRowModifications[index.column()].toBool() ? Qt::Checked : Qt::Unchecked;
            if(index.row() < schema.columns.size())
                return schema.columns[index.row()][index.column()].toBool() ? Qt::Checked : Qt::Unchecked;
        }
    } else {
        if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole) && index.row() < rowCount() && index.column() < columnCount()) {
            if(index.row() == updatingRow && currentRowModifications.contains(index.column()))
                return currentRowModifications[index.column()];
            else if(index.row() < schema.columns.size())
                return schema.columns[index.row()][index.column()];
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
        case SCHEMA_CONSTRAINTS: return "Constraints";
        default: break;
        }
    }
    return QVariant();
}

bool SqlSchemaModel::deleteRows(QSet<int> rows) {
    beginResetModel();
    for(int i : rows) {
        QString query("ALTER TABLE \"" + tableName + "\" DROP COLUMN \"" + schema.columns.at(i).at(SCHEMA_NAME).toString() + "\"");
        QMetaObject::invokeMethod(&db, "queryTableUpdate", Q_ARG(QString, query), Q_ARG(QObject*, this));
        schema.columns.remove(i);
    }
    select(); // todo rowsremoved instead?
    return true;
}
#include "schemamodel.moc"
