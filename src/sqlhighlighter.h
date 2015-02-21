/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_SQLHIGHLIGHTER_H_
#define _SEQUELJOE_SQLHIGHLIGHTER_H_

#include <QSyntaxHighlighter>

class SqlHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit SqlHighlighter(QTextDocument* parent);

    enum {
        STATE_NONE      = 0x00,
        STATE_DBLSTR    = 0x01,
        STATE_SNGSTR    = 0x02,
        STATE_COMMENT   = 0x04,
        STATE_CURRENT   = 0x08
    };

    union State {
        struct {
            char mode;
            char idx;
            char col;
        } s;
        int opaque;
    };

protected:
    virtual void highlightBlock(const QString &text) override;

private:

    struct Rule {
        QRegExp pattern;
        QColor fg;
    };
    QVector<Rule> rules;
    QColor defaultFg;
};

#endif // _SEQUELJOE_SQLHIGHLIGHTER_H_
