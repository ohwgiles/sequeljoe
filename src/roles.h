#ifndef ROLES_H
#define ROLES_H

#include <qnamespace.h>

enum SequelJoeCellEditors {
    SJCellEditDefault = 0,
    SJCellEditLongText,
    SJCellEditForeignKey
};

enum SequelJoeCustomRoles {
    DatabaseIsFileRole = Qt::UserRole,
    FilterColumnRole,
    FilterOperationRole,
    FilterValueRole,
    ForeignKeyRole,
    ExpandedColumnIndexRole,
    EditorTypeRole
};


#endif // ROLES_H
