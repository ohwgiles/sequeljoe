/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#include "connectionwidget.h"

#include "dbconnection.h"
#include "savedconfig.h"
#include "favourites.h"
#include "passkeywidget.h"
#include "dbfilewidget.h"
#include "driver.h"
#include "roles.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>
#include <QListWidget>
#include <QSqlDatabase>
#include <QHBoxLayout>
#include <QFormLayout>

ConnectionWidget::ConnectionWidget(QWidget *parent) :
    QWidget(parent)
{
    QBoxLayout* layout = new QHBoxLayout(this);

    { // widget for selecting saved database connections
        favourites = new Favourites(this);

        connect(favourites, SIGNAL(favouriteSelected(QString)), this, SLOT(loadSettings(QString)));
        connect(this, SIGNAL(nameChanged(QString)), favourites, SLOT(updateName(QString)));

        layout->addWidget(favourites,1);
    }

    { // widget for configuring a connection
        QWidget* cfgWidget = new QWidget(this);
        QBoxLayout* cfgLayout = new QVBoxLayout(cfgWidget);
        boxSetup = new QGroupBox("Connection Setup", cfgWidget);

        // hacky, but seems to look good on most platforms. without this, the forms look too loose
        boxSetup->setStyleSheet("QGroupBox{border:1px solid #bbb;border-radius:2px;margin-top:3ex;}QGroupBox::title{subcontrol-origin:margin;}");

        form = nullptr;

        name = new QLineEdit(boxSetup);
        //name->hide();

        sqlType = new QComboBox(boxSetup);
        sqlType->setModel(Driver::driverListModel(sqlType));
        sqlType->hide();

        host = new QLineEdit(boxSetup);
        //host->hide();

        port = new QLineEdit(boxSetup);
        port->setPlaceholderText(QString::number(SavedConfig::DEFAULT_SQL_PORT));
        port->setValidator(new QIntValidator(port));
        //port->hide();

        dbName = new DbFileWidget(boxSetup);
        //dbName->hide();

        username = new QLineEdit(boxSetup);
        //username->hide();

        password = new QLineEdit(boxSetup);
        password->setEchoMode(QLineEdit::Password);
        //password->hide();

        chkUseSsh = new QCheckBox("SSH Tunnel", boxSetup);
        //chkUseSsh->hide();

        { // widget for configuring the SSH tunnel
            boxSsh = new QGroupBox("SSH", boxSetup);
            boxSsh->setEnabled(false);
            connect(chkUseSsh, SIGNAL(toggled(bool)), boxSsh, SLOT(setEnabled(bool)));

            QFormLayout* sshForm = new QFormLayout(boxSsh);

            sshHost = new QLineEdit(boxSsh);
            sshForm->addRow("Host", sshHost);

            sshPort = new QLineEdit(boxSsh);
            sshPort->setPlaceholderText(QString::number(SavedConfig::DEFAULT_SSH_PORT));
            sshPort->setValidator(new QIntValidator(sshPort));
            sshForm->addRow("Port", sshPort);

            sshUsername = new QLineEdit(boxSsh);
            sshForm->addRow("Username", sshUsername);

            sshPassKey = new PassKeyWidget(boxSsh);
            sshForm->addRow("Password", sshPassKey);

            //boxSsh->hide();
        }

        connectButton = new QPushButton("Connect", boxSetup);
        connectButton->hide();
        //form->addWidget(pushButton);
        connect(connectButton, &QPushButton::clicked, this, &ConnectionWidget::connectButtonClicked);

        cfgLayout->addWidget(boxSetup);
        cfgLayout->setAlignment(boxSetup, Qt::AlignCenter);

        layout->addWidget(cfgWidget,4);
    }

    connect(name, SIGNAL(textEdited(QString)), this, SLOT(setupNameChanged(QString)));
    connect(host, SIGNAL(textEdited(QString)), this, SLOT(setupHostChanged(QString)));
    connect(port, SIGNAL(textEdited(QString)), this, SLOT(setupPortChanged(QString)));
    connect(sqlType, SIGNAL(currentIndexChanged(int)), this, SLOT(setupSqlTypeChanged(int)));
    connect(dbName, SIGNAL(changed(QString)), this, SLOT(setupDbChanged(QString)));
    connect(username, SIGNAL(textEdited(QString)), this, SLOT(setupUserChanged(QString)));
    connect(password, SIGNAL(textEdited(QString)), this, SLOT(setupPassChanged(QString)));
    connect(chkUseSsh, SIGNAL(toggled(bool)), this, SLOT(setupUseSshChanged(bool)));
    connect(sshHost, SIGNAL(textEdited(QString)), this, SLOT(setupSshHostChanged(QString)));
    connect(sshPort, SIGNAL(textEdited(QString)), this, SLOT(setupSshPortChanged(QString)));
    connect(sshUsername, SIGNAL(textEdited(QString)), this, SLOT(setupSshUserChanged(QString)));
    connect(sshPassKey, SIGNAL(changed(bool,QString)), this, SLOT(setupSshPassKeyChanged(bool,QString)));

    favourites->populateFromConfig();
}

void ConnectionWidget::connectButtonClicked() {
    emit doConnect(group);
}

void ConnectionWidget::setupNameChanged(QString name) {
    QSettings s;
    s.beginGroup(group);
    s.setValue("Name", name);
    s.endGroup();
    emit nameChanged(name);
}

void ConnectionWidget::setupHostChanged(QString host) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_HOST, host);
    s.endGroup();
}

void ConnectionWidget::setupPortChanged(QString port) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_PORT, port);
    s.endGroup();
}

void ConnectionWidget::setupSqlTypeChanged(int type) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_TYPE, sqlType->model()->data(sqlType->model()->index(type,0),Qt::EditRole).toString());

    bool isFile = dbTypeIsFile(type);
    bool hasCipher = dbTypeHasCipher(type);

    // QTBUG-6864

    for(QObject* obj: boxSetup->children()) {
        if(QWidget* w = qobject_cast<QWidget*>(obj))
            w->hide();
    }

    if(form) {
        delete form->labelForField(name);
        delete form->labelForField(sqlType);
        delete form->labelForField(host);
        delete form->labelForField(port);
        delete form->labelForField(dbName);
        delete form->labelForField(username);
        delete form->labelForField(password);
        delete form;
        form = 0;
    }

    form = new QFormLayout(boxSetup);

    form->addRow("Name", name);
    name->show();
    form->addRow("Connection Type", sqlType);
    sqlType->show();
    if(!isFile) {
        form->addRow("Host", host);
        host->show();
        form->addRow("Port", port);
        port->show();
    }
    form->addRow("Database", dbName);
    dbName->show();
    if(hasCipher) {
        form->addRow("Cipher Key", password);
        password->show();
    }
    if(!isFile) {
        form->addRow("Username", username);
        username->show();
        form->addRow("Password", password);
        password->show();
        form->addWidget(chkUseSsh);
        chkUseSsh->show();
        form->setWidget(form->rowCount(), QFormLayout::SpanningRole, boxSsh);
        boxSsh->show();
    }
    form->addWidget(connectButton);
    connectButton->show();

    boxSetup->setLayout(form);
    dbName->setFile(isFile);
    // changing the type invalidates the database name
    dbName->setValue("");
    s.endGroup();
}

void ConnectionWidget::setupDbChanged(QString value) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_DBNM, value);
    s.endGroup();
}

void ConnectionWidget::setupUserChanged(QString user) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_USER, user);
    s.endGroup();
}

void ConnectionWidget::setupPassChanged(QString pass) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_PASS, pass);
    s.endGroup();
}

void ConnectionWidget::setupUseSshChanged(bool ssh) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_USE_SSH, ssh);
    s.endGroup();
}

void ConnectionWidget::setupSshHostChanged(QString host) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_SSH_HOST, host);
    s.endGroup();
}

void ConnectionWidget::setupSshPortChanged(QString port) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_SSH_PORT, port);
    s.endGroup();
}

void ConnectionWidget::setupSshUserChanged(QString user) {
    QSettings s;
    s.beginGroup(group);
    s.setValue(SavedConfig::KEY_SSH_USER, user);
    s.endGroup();
}

void ConnectionWidget::setupSshPassKeyChanged(bool key, QString value) {
    QSettings s;
    s.beginGroup(group);
    s.remove(key ? SavedConfig::KEY_SSH_PASS : SavedConfig::KEY_SSH_KEY);
    s.setValue(key ? SavedConfig::KEY_SSH_KEY : SavedConfig::KEY_SSH_PASS, value);
    s.endGroup();
}

bool ConnectionWidget::dbTypeIsFile(int type) const {
    return sqlType->model()->data(sqlType->model()->index(type,0), DatabaseIsFileRole).toBool();
}

bool ConnectionWidget::dbTypeHasCipher(int type) const {
    return sqlType->model()->data(sqlType->model()->index(type,0), DatabaseHasCipherRole).toBool();
}

void ConnectionWidget::loadSettings(QString groupName) {
    group = groupName;
    QSettings s;
    s.beginGroup(group);
    name->setText(s.value("Name").toString());
    host->setText(s.value(SavedConfig::KEY_HOST).toString());

    sqlType->model()->setData(QModelIndex(), s.value(SavedConfig::KEY_TYPE).toString());
    int sqlTypeRow = sqlType->model()->data(QModelIndex{},Qt::EditRole).toInt();
    sqlType->setCurrentIndex(sqlTypeRow);
    setupSqlTypeChanged(sqlTypeRow);

    port->setText(s.value(SavedConfig::KEY_PORT).toString());
    dbName->setValue(s.value(SavedConfig::KEY_DBNM).toString());
    username->setText(s.value(SavedConfig::KEY_USER).toString());
    password->setText(s.value(SavedConfig::KEY_PASS).toString());
    chkUseSsh->setChecked(s.value(SavedConfig::KEY_USE_SSH).toBool());
    sshHost->setText(s.value(SavedConfig::KEY_SSH_HOST).toString());
    sshPort->setText(s.value(SavedConfig::KEY_SSH_PORT).toString());
    sshUsername->setText(s.value(SavedConfig::KEY_SSH_USER).toString());
    if(s.contains(SavedConfig::KEY_SSH_KEY))
        sshPassKey->setValue(true, s.value(SavedConfig::KEY_SSH_KEY).toString());
    else
        sshPassKey->setValue(false, s.value(SavedConfig::KEY_SSH_PASS).toString());
    s.endGroup();
}
