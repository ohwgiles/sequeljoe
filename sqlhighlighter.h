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

class SqlHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    SqlHighlighter(QTextDocument* parent);
protected:
     void highlightBlock(const QString &text);

 private:
     struct Rule {
         QRegExp pattern;
         QTextCharFormat format;
     };
     QVector<Rule> rules_;
     QTextCharFormat comment_;

     QRegExp commentStart_;
     QRegExp commentEnd_;
};

#endif // _SEQUELJOE_SQLHIGHLIGHTER_H_
