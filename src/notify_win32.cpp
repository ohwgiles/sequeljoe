/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "notify.h"
#include <windows.h>
#include <QApplication>
#include <QMainWindow>
#include <string.h>
#include <QPixmap>
#include <QLabel>
class Win32Notifier : public Notifier {
public:
    Win32Notifier() {
        memset(&nd, 0, sizeof(NOTIFYICONDATA));
        nd.uFlags = NIF_INFO;
        nd.uTimeout= 5;
        nd.dwInfoFlags = NIIF_INFO;

        nd.cbSize=NOTIFYICONDATA_V2_SIZE;
        nd.uVersion=NOTIFYICON_VERSION;
        Shell_NotifyIcon(NIM_SETVERSION, &nd);
        Shell_NotifyIcon(NIM_ADD, &nd);
    }

    void send(const char *title, const char *msg) override {
        strcpy(nd.szInfo, msg);
        strcpy(nd.szInfoTitle, title);
        Shell_NotifyIcon(NIM_MODIFY, &nd);
    }

    ~Win32Notifier() {
        Shell_NotifyIcon(NIM_DELETE, &nd);
    }

private:
    NOTIFYICONDATA nd;
};

static Win32Notifier* _notifier = 0;

Notifier* Notifier::instance() {
    if(!_notifier)
        _notifier = new Win32Notifier();
    return _notifier;
}

void Notifier::cleanup() {
    delete _notifier;
    _notifier = 0;
}
