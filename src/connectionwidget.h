/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_CONNECTIONWIDGET_H_
#define _SEQUELJOE_CONNECTIONWIDGET_H_

#include <QWidget>

class QGroupBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QPushButton;
class QListWidget;
class QListWidgetItem;
class Favourites;
class PassKeyWidget;
class DbFileWidget;

class ConnectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectionWidget(QWidget *parent = 0);

signals:
    void doConnect(QString);
    void nameChanged(QString);

public slots:
    void loadSettings(QString groupName);

private slots:
    void setupNameChanged(QString);
    void setupHostChanged(QString);
    void setupPortChanged(QString);
    void setupSqlTypeChanged(int);
    void setupDbChanged(QString);
    void setupUserChanged(QString);
    void setupPassChanged(QString);
    void setupUseSshChanged(bool);
    void setupSshHostChanged(QString);
    void setupSshPortChanged(QString);
    void setupSshUserChanged(QString);
    void setupSshPassKeyChanged(bool, QString);
    void connectButtonClicked();

private:
    bool dbTypeIsFile(int type) const;
    bool dbTypeHasCipher(int type) const;

    QString group;
    QGroupBox* boxSetup;
    QFormLayout* form;
    QLineEdit* name;
    QLineEdit* host;
    QLineEdit* port;
    QComboBox* sqlType;
    DbFileWidget* dbName;
    QLineEdit* username;
    QLineEdit* password;
    QCheckBox* chkUseSsh;
    QGroupBox* boxSsh;
    QLineEdit* sshHost;
    QLineEdit* sshPort;
    QLineEdit* sshUsername;
    PassKeyWidget* sshPassKey;
    QPushButton* connectButton;
    Favourites* favourites;
};

#endif // _SEQUELJOE_CONNECTIONWIDGET_H_
