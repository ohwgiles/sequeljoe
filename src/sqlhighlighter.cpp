/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "sqlhighlighter.h"
#include <QTextDocument>
#define SQL_KEYWORDS {"ABORT","ACTION","ADD","AFTER","ALL","ALTER","ANALYZE","AND","AS","ASC","ATTACH","AUTO_INCREMENT","AUTOINCREMENT","BEFORE","BEGIN","BETWEEN","BY","CASCADE","CASE","CAST","CHECK","COLLATE","COLUMN","COMMIT","CONFLICT","CONSTRAINT","CREATE","CROSS","CURRENT_DATE","CURRENT_TIME","CURRENT_TIMESTAMP","DATABASE","DEFAULT","DEFERRABLE","DEFERRED","DELETE","DESC","DETACH","DISTINCT","DROP","EACH","ELSE","END","ESCAPE","EXCEPT","EXCLUSIVE","EXISTS","EXPLAIN","FAIL","FOR","FOREIGN","FROM","FULL","GLOB","GROUP","HAVING","IF","IGNORE","IMMEDIATE","IN","INDEX","INDEXED","INITIALLY","INNER","INSERT","INSTEAD","INTERSECT","INTO","IS","ISNULL","JOIN","KEY","LEFT","LIKE","LIMIT","MATCH","NATURAL","NO","NOT","NOTNULL","NULL","OF","OFFSET","ON","OR","ORDER","OUTER","PLAN","PRAGMA","PRIMARY","QUERY","RAISE","RECURSIVE","REFERENCES","REGEXP","REINDEX","RELEASE","RENAME","REPLACE","RESTRICT","RIGHT","ROLLBACK","ROW","SAVEPOINT","SELECT","SET","TABLE","TEMP","TEMPORARY","THEN","TO","TRANSACTION","TRIGGER","UNION","UNIQUE","UPDATE","USING","VACUUM","VALUES","VIEW","VIRTUAL","WHEN","WHERE","WITH","WITHOUT"}
#define SQL_TYPES {"TINYINT","SMALLINT","MEDIUMINT","INT","INTEGER","BIGINT","FLOAT","DOUBLE","DOUBLE PRECISION","REAL","DECIMAL","NUMERIC","DATE","DATETIME","TIMESTAMP","TIME","YEAR","CHAR","VARCHAR","TINYBLOB","TINYTEXT","BLOB","TEXT","MEDIUMBLOB","MEDIUMTEXT","LONGBLOB","LONGTEXT","ENUM","SET","UNSIGNED"}
#define SQL_FUNCS {"ABS","ACOS","ADDDATE","ADDTIME","AES_DECRYPT","AES_ENCRYPT","AND","Area","AsBinary","AsWKB","ASCII","ASIN","AsText","AsWKT","ATAN2","ATAN","ATAN","AVG","BENCHMARK","BETWEEN","BIN","BINARY","BIT_AND","BIT_COUNT","BIT_LENGTH","BIT_OR","BIT_XOR","CASE","CAST","CEIL","CEILING","Centroid","CHAR_LENGTH","CHAR","CHARACTER_LENGTH","CHARSET","COALESCE","COERCIBILITY","COLLATION","COMPRESS","CONCAT_WS","CONCAT","CONNECTION_ID","Contains","CONV","CONVERT_TZ","CONVERT","COS","COT","COUNT","COUNT","CRC32","Crosses","CURDATE","CURRENT_DATE","CURRENT_DATE","CURRENT_TIME","CURRENT_TIME","CURRENT_TIMESTAMP","CURRENT_TIMESTAMP","CURRENT_USER","CURRENT_USER","CURTIME","DATABASE","DATE_ADD","DATE_FORMAT","DATE_SUB","DATE","DATEDIFF","DAY","DAYNAME","DAYOFMONTH","DAYOFWEEK","DAYOFYEAR","DECODE","DEFAULT","DEGREES","DES_DECRYPT","DES_ENCRYPT","Dimension","Disjoint","DIV","ELT","ENCODE","ENCRYPT","EndPoint","Envelope","Equals","EXP","EXPORT_SET","ExteriorRing","EXTRACT","FIELD","FIND_IN_SET","FLOOR","FORMAT","FOUND_ROWS","FROM_DAYS","FROM_UNIXTIME","GeomCollFromText","GeometryCollectionFromText","GeomCollFromWKB","GeometryCollectionFromWKB","GeometryCollection","GeometryN","GeometryType","GeomFromText","GeometryFromText","GeomFromWKB","GET_FORMAT","GET_LOCK","GLength","GREATEST","GROUP_CONCAT","HEX","HOUR","IF","IFNULL","IN","INET_ATON","INET_NTOA","INSERT","INSTR","InteriorRingN","Intersects","INTERVAL","IS_FREE_LOCK","IS NOT NULL","IS NOT","IS NULL","IS_USED_LOCK","IS","IsClosed","IsEmpty","ISNULL","IsSimple","LAST_DAY","LAST_INSERT_ID","LCASE","LEAST","LEFT","LENGTH","LIKE","LineFromText","LineFromWKB","LineStringFromWKB","LineString","LN","LOAD_FILE","LOCALTIME","LOCALTIME","LOCALTIMESTAMP","LOCALTIMESTAMP","LOCATE","LOG10","LOG2","LOG","LOWER","LPAD","LTRIM","MAKE_SET","MAKEDATE","MAKETIME","MASTER_POS_WAIT","MATCH","MAX","MBRContains","MBRDisjoint","MBREqual","MBRIntersects","MBROverlaps","MBRTouches","MBRWithin","MD5","MICROSECOND","MID","MIN","MINUTE","MLineFromText","MultiLineStringFromText","MLineFromWKB","MultiLineStringFromWKB","MOD","MONTH","MONTHNAME","MPointFromText","MultiPointFromText","MPointFromWKB","MultiPointFromWKB","MPolyFromText","MultiPolygonFromText","MPolyFromWKB","MultiPolygonFromWKB","MultiLineString","MultiPoint","MultiPolygon","NAME_CONST","NOW","NULLIF","NumGeometries","NumInteriorRings","NumPoints","OCT","OCTET_LENGTH","OLD_PASSWORD","OR","ORD","Overlaps","PASSWORD","PERIOD_ADD","PERIOD_DIFF","PI","Point","PointFromText","PointFromWKB","PointN","PolyFromText","PolygonFromText","PolyFromWKB","PolygonFromWKB","Polygon","POSITION","POW","POWER","PROCEDURE ANALYSE","QUARTER","QUOTE","RADIANS","RAND","REGEXP","RELEASE_LOCK","REPEAT","REPLACE","REVERSE","RIGHT","RLIKE","ROUND","ROW_COUNT","RPAD","RTRIM","SCHEMA","SEC_TO_TIME","SECOND","SESSION_USER","SHA1","SHA","SIGN","SIN","SLEEP","SOUNDEX","SOUNDS LIKE","SPACE","SQRT","SRID","StartPoint","STD","STDDEV_POP","STDDEV_SAMP","STDDEV","STR_TO_DATE","STRCMP","SUBDATE","SUBSTR","SUBSTRING_INDEX","SUBSTRING","SUBTIME","SUM","SYSDATE","SYSTEM_USER","TAN","TIME_FORMAT","TIME_TO_SEC","TIME","TIMEDIFF","TIMESTAMP","TIMESTAMPADD","TIMESTAMPDIFF","TO_DAYS","Touches","TRIM","TRUNCATE","UCASE","UNCOMPRESS","UNCOMPRESSED_LENGTH","UNHEX","UNIX_TIMESTAMP","UPPER","USER","UTC_DATE","UTC_TIME","UTC_TIMESTAMP","UUID","VALUES","VAR_POP","VAR_SAMP","VARIANCE","VERSION","WEEK","WEEKDAY","WEEKOFYEAR","Within","X","XOR","Y","YEAR","YEARWEEK"}

SqlHighlighter::SqlHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent),
    defaultFg(Qt::black)
{
    Rule r;
    // keywords
    r.fg = Qt::blue;
    for(const char* keyword : SQL_KEYWORDS) {
        r.pattern = QRegExp(QString("\\b") + keyword + "\\b", Qt::CaseInsensitive);
        rules.append(r);
    }
    // types
    r.fg = Qt::darkCyan;
    for(const char* type : SQL_TYPES) {
        r.pattern = QRegExp(QString("\\b") + type + "\\b", Qt::CaseInsensitive);
        rules.append(r);
    }
    // functions
    r.fg = Qt::red;
    for(const char* type : SQL_FUNCS) {
        r.pattern = QRegExp(QString("\\b") + type + "\\b", Qt::CaseInsensitive);
        rules.append(r);
    }

}

void SqlHighlighter::highlightBlock(const QString &text) {
    State pbs;
    pbs.opaque = previousBlockState();
    if(pbs.opaque == -1) {
        pbs.opaque = 0;
        pbs.s.col = -1;
    }

    if(text.length() == 0) {
        setCurrentBlockState(pbs.opaque);
        return;
    }

    auto bgColor = [](int i) -> QColor {
        return QColor::fromHsv((75*i+20)%255,20,250);
    };

    State st = pbs;

    QTextCharFormat f;
    f.setBackground(bgColor(st.s.idx));
    setFormat(0, currentBlock().length(), f);

    int startIndex = 0;
    int len = 0;

    auto hlWords = [&](int from){
        foreach (const Rule &rule, rules) {
            QRegExp expression(rule.pattern);
            int index = expression.indexIn(text, from);
            while(index >= 0) {
                int length = expression.matchedLength();
                f.setForeground(rule.fg);

                setFormat(index, length, f);
                index = expression.indexIn(text, index + length);
            };
        }
    };

    auto findBegin = [&](){
        int s = QRegExp("(?:/\\*|[\"']|--|;)").indexIn(text, startIndex+len);
        if(s >=0) {
            QChar c = text.at(s);
            if(c == '/' || c == '-')
                st.s.mode = STATE_COMMENT;
            else if(c == '\'')
                st.s.mode = STATE_SNGSTR;
            else if(c == '"')
                st.s.mode = STATE_DBLSTR;
            else { // end of statement
                st.s.idx++;
                st.s.col = s;
                hlWords(startIndex + len);
                f.setForeground(defaultFg);
                f.setBackground(bgColor(st.s.idx));
                setFormat(s+1,currentBlock().length() - s-1, f);
                hlWords(s);
            }
        }
        return s;
    };

    setCurrentBlockState(st.opaque);
    hlWords(0);

    if(!(pbs.opaque&(STATE_COMMENT|STATE_DBLSTR|STATE_SNGSTR))) {
        startIndex = findBegin();
    }

    while(startIndex >= 0) {
        // special case for strings
        char md = st.s.mode;
        if(startIndex == 0 && (((pbs.opaque&STATE_DBLSTR) && text.at(0) == '"') ||
                ((pbs.opaque&STATE_SNGSTR) && text.at(0) == '\''))) {
            len = 1;
            st.s.mode = STATE_NONE;
        } else if(text.at(startIndex) == '-') {
            len = currentBlock().length() - startIndex;
            st.s.mode = STATE_NONE;
        } else if(st.s.mode == STATE_NONE) {
            len = 1;
            }else{
            int end = st.s.mode == STATE_COMMENT ?
                        QRegExp("\\*/").indexIn(text, startIndex) :
                        st.s.mode == STATE_DBLSTR ?
                        QRegExp("[^\\\\]\"").indexIn(text, startIndex) :
                        QRegExp("[^\\\\]'").indexIn(text, startIndex);

            if(end == -1) {

                len = currentBlock().length() - startIndex;
            } else {
                // len(*/)==len([^\\]['"])==2
                len = end + 2 - startIndex;
                st.s.mode = STATE_NONE;
            }
        }
        if((md & STATE_COMMENT))
            f.setForeground(Qt::gray);
        else if(md & (STATE_DBLSTR|STATE_SNGSTR))
            f.setForeground(Qt::darkGreen);
        setFormat(startIndex, len, f);
        startIndex = findBegin();
    }
    setCurrentBlockState(st.opaque);

}

