/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "querypanel.h"

#include "tableview.h"
#include "sqlhighlighter.h"
#include "sqlmodel.h"

#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSqlQueryModel>
#include <QPlainTextEdit>
#include <QSqlError>
#include <QSqlQuery>
#include <QLabel>
#include <QAction>


QueryPanel::QueryPanel(QWidget* parent) :
    QWidget(parent),
    model(nullptr)
{
    QBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    QSplitter* splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);

    { // top half: query editor
        QWidget* top = new QWidget(splitter);
        QBoxLayout* editorLayout = new QVBoxLayout(top);
        editorLayout->setContentsMargins(0,0,0,0);

        editor = new QPlainTextEdit(this);
        QFont f;
        f.setStyleHint(QFont::Monospace);
        new SqlHighlighter(editor->document());
        editor->setFont(f);
        editorLayout->addWidget(editor);

        error = new QLabel(this);
        error->setWordWrap(true);
        error->hide();
        error->setStyleSheet("QLabel{color:red;}");
        error->setContentsMargins(8,8,8,8);
        editorLayout->addWidget(error);

        QAction* runQueryAction = new QAction(this);
        QKeySequence ctrlEnter(Qt::CTRL + Qt::Key_Return);
        runQueryAction->setShortcut(ctrlEnter);
        addAction(runQueryAction);

        QAction* runAllAction = new QAction(this);
        QKeySequence ctrlShiftEnter(Qt::CTRL + Qt::SHIFT + Qt::Key_Return);
        runAllAction->setShortcut(ctrlShiftEnter);
        addAction(runAllAction);


        QBoxLayout* toolbar = new QHBoxLayout();
        toolbar->addStretch();

        QPushButton* run = new QPushButton("Execute statement under cursor (" + ctrlEnter.toString(QKeySequence::NativeText) + ")", this);
        toolbar->addWidget(run);

        QPushButton* runall = new QPushButton("Execute all (" + ctrlShiftEnter.toString(QKeySequence::NativeText) + ")", this);
        toolbar->addWidget(runall);

        editorLayout->addLayout(toolbar);

        splitter->addWidget(top);

        connect(editor, SIGNAL(textChanged()), error, SLOT(hide()));
        connect(runQueryAction, SIGNAL(triggered()), this, SLOT(executeQuery()));
        connect(run, SIGNAL(clicked()), runQueryAction, SIGNAL(triggered()));
        connect(runAllAction, SIGNAL(triggered()), this, SLOT(executeAll()));
        connect(runall, SIGNAL(clicked()), runAllAction, SIGNAL(triggered()));

    }

    { // bottom half: results table
        QWidget* bottom = new QWidget(splitter);
        QBoxLayout* v = new QVBoxLayout(bottom);
        v->setContentsMargins(0,0,0,0);
        v->setSpacing(0);

        results = new TableView(this);
        results->setModel(model);
        v->addWidget(results);

        status = new QLabel(bottom);
        status->hide();
        v->addWidget(status);

        bottom->setLayout(v);
        splitter->addWidget(bottom);
    }
    layout->addWidget(splitter);
}

void QueryPanel::setModel(SqlModel *m) {
    if(model)
        delete model;
    model = m;
    results->setModel(m);
}
union State {
    struct {
        char mode;
        char idx;
        char col;
    } s;
    int opaque;
};
QString QueryPanel::getActiveStatement(int block, int col) {

    int from = 0, to = -1;
    State st;

    QTextBlock b = editor->document()->findBlockByNumber(block);
    int scol = col;
    while(b.isValid()) {
        st.opaque = b.userState();
        if(st.s.col != -1 && st.s.col < scol) {
            from = b.position() + st.s.col + 1;
            break;
        }
        scol = INT_MAX;
        b = b.previous();
    }

    b = editor->document()->findBlockByNumber(block);
    scol = col;
    while(b.isValid()) {
        st.opaque = b.userState();
        if(st.s.col != -1 && st.s.col >= scol) {
            to = b.position() + st.s.col;
            break;
        }
        scol = 0;
        b = b.next();
    }

    QString all = editor->document()->toPlainText();
    if(to < 0)
        to = all.length();
    return all.mid(from,to);;
}

void QueryPanel::executeQuery() {
    QTextCursor c = editor->textCursor();
    QString stmt = getActiveStatement(c.blockNumber(), c.columnNumber());
    error->hide();
    status->hide();
    model->setQuery(stmt);
    model->select();
}

void QueryPanel::executeAll() {
    error->hide();
    status->hide();
    model->setQuery(editor->toPlainText());
    model->select();
}
