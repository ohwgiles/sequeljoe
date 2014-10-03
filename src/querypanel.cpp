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

        QBoxLayout* toolbar = new QHBoxLayout();
        QPushButton* run = new QPushButton("Run Query (" + ctrlEnter.toString(QKeySequence::NativeText) + ")", this);
        toolbar->addWidget(run);
        editorLayout->addLayout(toolbar);

        splitter->addWidget(top);

        connect(editor, SIGNAL(textChanged()), error, SLOT(hide()));
        connect(runQueryAction, SIGNAL(triggered()), this, SLOT(executeQuery()));
        connect(run, SIGNAL(clicked()), runQueryAction, SIGNAL(triggered()));
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

void QueryPanel::executeQuery() {
    error->hide();
    status->hide();
    model->setQuery(editor->document()->toPlainText());
    model->select();
}
