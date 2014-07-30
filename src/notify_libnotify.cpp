#include "notify.h"
#include <libnotify/notify.h>

Notifier::Notifier()
{
	notify_init("SequelJoe");
}

void Notifier::send(const char* title, const char* msg) {
	NotifyNotification* n = notify_notification_new(title, msg, 0);
	notify_notification_show(n, 0);
}

Notifier* notify;
