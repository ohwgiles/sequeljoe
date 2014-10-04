/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "notify.h"
#include <libnotify/notify.h>

class Libnotify : public Notifier {
public:
    Libnotify() {
        notify_init("SequelJoe");
    }

    ~Libnotify() {
        notify_uninit();
    }

    void send(const char *title, const char *msg) override {
        NotifyNotification* n = notify_notification_new(title, msg, 0);
        notify_notification_show(n, 0);
    }
};

Libnotify* _notifier;

Notifier* Notifier::instance() {
    if(!_notifier)
        _notifier = new Libnotify();
    return _notifier;
}

void Notifier::cleanup() {
    delete _notifier;
    _notifier = 0;
}
