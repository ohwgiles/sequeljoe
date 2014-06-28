/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSqlQueryModel>
#include <QPlainTextEdit>
#include "querypanel.h"
#include "dbconnection.h"
#include "tableview.h"
#include "sqlhighlighter.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QLabel>
QueryPanel::QueryPanel(QWidget* parent) : QWidget(parent), model_(new QSqlQueryModel)
{
    QBoxLayout* b = new QVBoxLayout(this);
    b->setContentsMargins(0,0,0,0);
    b->setSpacing(0);

    QSplitter* splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    {
        QWidget* top = new QWidget(splitter);
        QBoxLayout* v = new QVBoxLayout(top);
        v->setContentsMargins(0,0,0,0);
        v->setSpacing(0);

        editor_ = new QPlainTextEdit(this);
        QFont f;
        f.setStyleHint(QFont::Monospace);
        new SqlHighlighter(editor_->document());
        editor_->setFont(f);

        v->addWidget(editor_);
        error_ = new QLabel(this);
        error_->setWordWrap(true);
        error_->hide();
        error_->setStyleSheet("QLabel{color:red;}");
        error_->setContentsMargins(8,8,8,8);
        v->addWidget(error_);
        connect(editor_, SIGNAL(textChanged()), error_, SLOT(hide()));
        QBoxLayout* h = new QHBoxLayout(this);

        //h->addWidget(new QSpacerItem());
        QPushButton* run = new QPushButton("Run Query");
        connect(run, SIGNAL(clicked()), this, SLOT(executeQuery()));
        run->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        h->addWidget(run);


        v->addLayout(h);

        top->setLayout(v);
        splitter->addWidget(top);
    }
    {
        QWidget* top = new QWidget(splitter);
        QBoxLayout* v = new QVBoxLayout(top);
        v->setContentsMargins(0,0,0,0);
        v->setSpacing(0);

        results_ = new TableView(this);
        results_->setModel(model_);
        v->addWidget(results_);

        status_ = new QLabel(top);
        status_->hide();
        v->addWidget(status_);

        top->setLayout(v);
        splitter->addWidget(top);
    }
    b->addWidget(splitter);
    //this->setLayout(b);
}
#include <QDebug>
void QueryPanel::executeQuery()
{
    error_->hide();
    status_->hide();
    QString q = editor_->document()->toPlainText();
    qDebug() << "query: " + q;
    db_->query(q, model_);
    QSqlError err = model_->lastError();
    if(err.isValid()) {
        error_->setText(err.text());
        error_->show();
    } else {
        status_->setText("Query OK: " + QString::number(model_->query().numRowsAffected()) + " rows affected");
        status_->show();
    }
    //model_->setQuery(editor_->document()->toPlainText(), *db_);
    results_->setModel(model_);
    results_->update();
    qDebug() << model_->query().numRowsAffected();

}
