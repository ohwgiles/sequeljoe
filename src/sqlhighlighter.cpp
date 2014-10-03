/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "sqlhighlighter.h"

#define SQL_KEYWORDS {"ABORT","ACTION","ADD","AFTER","ALL","ALTER","ANALYZE","AND","AS","ASC","ATTACH","AUTOINCREMENT","BEFORE","BEGIN","BETWEEN","BY","CASCADE","CASE","CAST","CHECK","COLLATE","COLUMN","COMMIT","CONFLICT","CONSTRAINT","CREATE","CROSS","CURRENT_DATE","CURRENT_TIME","CURRENT_TIMESTAMP","DATABASE","DEFAULT","DEFERRABLE","DEFERRED","DELETE","DESC","DETACH","DISTINCT","DROP","EACH","ELSE","END","ESCAPE","EXCEPT","EXCLUSIVE","EXISTS","EXPLAIN","FAIL","FOR","FOREIGN","FROM","FULL","GLOB","GROUP","HAVING","IF","IGNORE","IMMEDIATE","IN","INDEX","INDEXED","INITIALLY","INNER","INSERT","INSTEAD","INTERSECT","INTO","IS","ISNULL","JOIN","KEY","LEFT","LIKE","LIMIT","MATCH","NATURAL","NO","NOT","NOTNULL","NULL","OF","OFFSET","ON","OR","ORDER","OUTER","PLAN","PRAGMA","PRIMARY","QUERY","RAISE","RECURSIVE","REFERENCES","REGEXP","REINDEX","RELEASE","RENAME","REPLACE","RESTRICT","RIGHT","ROLLBACK","ROW","SAVEPOINT","SELECT","SET","TABLE","TEMP","TEMPORARY","THEN","TO","TRANSACTION","TRIGGER","UNION","UNIQUE","UPDATE","USING","VACUUM","VALUES","VIEW","VIRTUAL","WHEN","WHERE","WITH","WITHOUT"}
#define SQL_TYPES {"CHARACTER","VARCHAR","CHARACTER","BINARY","BOOLEAN","VARBINARY","BINARY","INTEGER","SMALLINT","INTEGER","BIGINT","DECIMAL","NUMERIC","FLOAT","REAL","FLOAT","DOUBLE","DATE","TIME","TIMESTAMP","INTERVAL","ARRAY","MULTISET","XML"}

SqlHighlighter::SqlHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    Rule r;
    // keywords
    r.format.setForeground(Qt::blue);
    for(const char* keyword : SQL_KEYWORDS) {
        r.pattern = QRegExp(QString("\\b") + keyword + "\\b", Qt::CaseInsensitive);
        rules.append(r);
    }
    // types
    r.format.setForeground(Qt::darkRed);
    for(const char* type : SQL_TYPES) {
        r.pattern = QRegExp(QString("\\b") + type + "\\b", Qt::CaseInsensitive);
        rules.append(r);
    }
    // strings
    r.format.setForeground(Qt::darkGreen);
    r.pattern = QRegExp("\"(?:[^\"\\\\]|\\\\.)*\"");
    rules.append(r);
    r.pattern = QRegExp("'(?:[^'\\\\]|\\\\.)*'");
    rules.append(r);

    // comments
    r.pattern = QRegExp("--.*");
    r.format.setForeground(Qt::gray);
    rules.append(r);
    commentStart = QRegExp("/\\*");
    commentEnd = QRegExp("\\*/");
}

void SqlHighlighter::highlightBlock(const QString &text)
{
    foreach (const Rule &rule, rules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStart.indexIn(text);
    while (startIndex >= 0) {
        int endIndex = commentEnd.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEnd.matchedLength();
        }
        setFormat(startIndex, commentLength, Qt::gray);
        startIndex = commentStart.indexIn(text, startIndex + commentLength);
    }
}
