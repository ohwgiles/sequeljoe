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

#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSqlQueryModel>
#include <QPlainTextEdit>
#include <QSqlError>
#include <QSqlQuery>
#include <QLabel>

QueryPanel::QueryPanel(QWidget* parent) :
    QWidget(parent),
    model_(new QSqlQueryModel)
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

void QueryPanel::executeQuery() {
    error_->hide();
    status_->hide();
    QSqlQuery q(*db_);
    q.prepare(editor_->document()->toPlainText());
    db_->execQuery(q);
    model_->setQuery(q);
    QSqlError err = model_->lastError();
    if(err.isValid()) {
        error_->setText(err.text());
        error_->show();
    } else {
        status_->setText("Query OK: " + QString::number(model_->query().numRowsAffected()) + " rows affected");
        status_->show();
    }
    results_->update();
}
