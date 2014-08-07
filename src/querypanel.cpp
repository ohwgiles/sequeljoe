/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "querypanel.h"

#include "dbconnection.h"
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
#include <QDebug>

QueryPanel::QueryPanel(QWidget* parent) :
    QWidget(parent),
    model_(0)
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
        editorLayout->setSpacing(0);

        editor_ = new QPlainTextEdit(this);
        QFont f;
        f.setStyleHint(QFont::Monospace);
        new SqlHighlighter(editor_->document());
        editor_->setFont(f);
        editorLayout->addWidget(editor_);

        error_ = new QLabel(this);
        error_->setWordWrap(true);
        error_->hide();
        error_->setStyleSheet("QLabel{color:red;}");
        error_->setContentsMargins(8,8,8,8);
        editorLayout->addWidget(error_);

        QBoxLayout* toolbar = new QHBoxLayout();
        QPushButton* run = new QPushButton("Run Query");
        run->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        toolbar->addWidget(run);
        editorLayout->addLayout(toolbar);

        splitter->addWidget(top);

        connect(editor_, SIGNAL(textChanged()), error_, SLOT(hide()));
        connect(run, SIGNAL(clicked()), this, SLOT(executeQuery()));
    }

    { // bottom half: results table
        QWidget* bottom = new QWidget(splitter);
        QBoxLayout* v = new QVBoxLayout(bottom);
        v->setContentsMargins(0,0,0,0);
        v->setSpacing(0);

        results_ = new TableView(this);
        results_->setModel(model_);
        v->addWidget(results_);

        status_ = new QLabel(bottom);
        status_->hide();
        v->addWidget(status_);

        bottom->setLayout(v);
        splitter->addWidget(bottom);
    }
    layout->addWidget(splitter);
}
void QueryPanel::setDb(DbConnection *db) {
    model_ = new SqlModel(*db);
    results_->setModel(model_);
}

void QueryPanel::executeQuery() {
    error_->hide();
    status_->hide();
    model_->setQuery(editor_->document()->toPlainText());
model_->refresh();
return;
#if 0
//begin
//    results_->showLoadingOverlay(true);
    query_->prepare(editor_->document()->toPlainText());
    QMetaObject::invokeMethod(db_, "execQuery", Q_ARG(QSqlQuery*, query_),
                              Q_ARG(QObject*, this), Q_ARG(const char*, "queryFinished"));
#endif
    //db_->execQuery(q);
}
