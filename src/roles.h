#ifndef ROLES_H
#define ROLES_H

#include <qnamespace.h>

enum SequelJoeCustomRoles {
    DatabaseIsFileRole = Qt::UserRole,
    FilterColumnRole,
    FilterOperationRole,
    FilterValueRole,
    ForeignKeyTableRole,
    ForeignKeyColumnRole,
    ExpandedColumnIndexRole,
    SqlTypeRole
};


#endif // ROLES_H
