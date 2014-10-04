/*
 * Copyright 2014 Oliver Giles
 *
 * This file is part of SequelJoe. SequelJoe is licensed under the
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_NOTIFY_H_
#define _SEQUELJOE_NOTIFY_H_

class QMainWindow;

class Notifier {
public:
    static Notifier* instance();
    static void cleanup();

    virtual void send(const char* title, const char* msg) = 0;

protected:
    Notifier() {}
    virtual ~Notifier() {}
};

#endif
