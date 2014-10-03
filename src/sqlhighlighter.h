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

protected:
    virtual void highlightBlock(const QString &text) override;

private:
     struct Rule {
         QRegExp pattern;
         QTextCharFormat format;
     };
     QVector<Rule> rules;
     QTextCharFormat comment;

     QRegExp commentStart;
     QRegExp commentEnd;
};

#endif // _SEQUELJOE_SQLHIGHLIGHTER_H_
