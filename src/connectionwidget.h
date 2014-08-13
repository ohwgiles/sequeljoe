/*
 * Copyright 2014 Oliver Giles
 * 
 * This file is part of SequelJoe. SequelJoe is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information
 */
#ifndef _SEQUELJOE_CONNECTION_WIDGET_H_
#define _SEQUELJOE_CONNECTION_WIDGET_H_

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
    void loadSettings(QString);

private slots:
    void setupNameChanged(QString);
    void setupHostChanged(QString);
    void setupPortChanged(QString);
    void setupSqlTypeChanged(QString);
    void setupDbChanged(bool, QString);
    void setupUserChanged(QString);
    void setupPassChanged(QString);
    void setupUseSshChanged(bool);
    void setupSshHostChanged(QString);
    void setupSshPortChanged(QString);
    void setupSshUserChanged(QString);
    void setupSshPassKeyChanged(bool, QString);
    void connectButtonClicked();

private:
    QString group_;
    QLineEdit* name_;
    QLineEdit* host_;
    QLineEdit* port_;
    QComboBox* sqlType_;
    DbFileWidget* dbName_;
    QLineEdit* username_;
    QLineEdit* password_;
    QCheckBox* chkUseSsh_;
    QLineEdit* sshHost_;
    QLineEdit* sshPort_;
    QLineEdit* sshUsername_;
    PassKeyWidget* sshPassKey_;
    Favourites* favourites_;
};

#endif // _SEQUELJOE_CONNECTION_WIDGET_H_
