
// Copyright © 2025, NECTEC, all rights reserved
//
// This file is distributed under the terms of the GNU General Public License v3 (GPLv3) only.
// It is intended for use as a GPL plugin/submodule for CVDev.
//
// CVDev itself is distributed under the Apache License, Version 2.0.
//
// This file: GPL v3. See <https://www.gnu.org/licenses/gpl-3.0.html>
// CVDev: Apache 2.0. See <https://www.apache.org/licenses/LICENSE-2.0>


#include "MQTTSubscriberEmbeddedWidget.hpp"
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include "ui_MQTTSubscriberEmbeddedWidget.h"
#include <QDebug>

MQTTSubscriberEmbeddedWidget::MQTTSubscriberEmbeddedWidget( QWidget *parent )
    : QWidget( parent ),
      ui( new Ui::MQTTSubscriberEmbeddedWidget )
{
    ui->setupUi( this );
    connect(ui->mpConnectButton, QOverload<bool>::of(&QPushButton::clicked), this, &MQTTSubscriberEmbeddedWidget::connect_button_clicked);
    connect(ui->mpHostLineEdit, &QLineEdit::textChanged, this, &MQTTSubscriberEmbeddedWidget::host_line_edit_text_changed);
    connect(ui->mpPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MQTTSubscriberEmbeddedWidget::port_spin_box_value_changed);
    connect(ui->mpUsernameLineEdit, &QLineEdit::textChanged, this, &MQTTSubscriberEmbeddedWidget::username_line_edit_text_changed);
    connect(ui->mpPasswordLineEdit, &QLineEdit::textChanged, this, &MQTTSubscriberEmbeddedWidget::password_line_edit_text_changed);
    connect(ui->mpQoSSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MQTTSubscriberEmbeddedWidget::qos_spin_box_value_changed);
    connect(ui->mpTopicLineEdit, &QLineEdit::textChanged, this, &MQTTSubscriberEmbeddedWidget::topic_line_edit_text_changed);

    connect(ui->mpConnectButton, QOverload<bool>::of(&QPushButton::clicked), this, &MQTTSubscriberEmbeddedWidget::connect_button_clicked);
    connect(ui->mpHostLineEdit, &QLineEdit::textChanged, this, &MQTTSubscriberEmbeddedWidget::host_line_edit_text_changed);
    connect(ui->mpPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MQTTSubscriberEmbeddedWidget::port_spin_box_value_changed);
    connect(ui->mpUsernameLineEdit, &QLineEdit::textChanged, this, &MQTTSubscriberEmbeddedWidget::username_line_edit_text_changed);
    connect(ui->mpPasswordLineEdit, &QLineEdit::textChanged, this, &MQTTSubscriberEmbeddedWidget::password_line_edit_text_changed);
    connect(ui->mpQoSSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MQTTSubscriberEmbeddedWidget::qos_spin_box_value_changed);
    connect(ui->mpTopicLineEdit, &QLineEdit::textChanged, this, &MQTTSubscriberEmbeddedWidget::topic_line_edit_text_changed);

}

MQTTSubscriberEmbeddedWidget::~MQTTSubscriberEmbeddedWidget()
{
    delete ui;
}

void
MQTTSubscriberEmbeddedWidget::
connect_button_clicked(bool checked)
{
    if( checked )
    {
        ui->mpHostLineEdit->setEnabled( false );
        ui->mpPortSpinBox->setEnabled( false );
        ui->mpUsernameLineEdit->setEnabled( false );
        ui->mpPasswordLineEdit->setEnabled( false );
        ui->mpQoSSpinBox->setEnabled( false );
        ui->mpTopicLineEdit->setEnabled( false );
        Q_EMIT button_clicked_signal( 0 );
    }
    else
    {
        ui->mpHostLineEdit->setEnabled( true );
        ui->mpPortSpinBox->setEnabled( true );
        ui->mpUsernameLineEdit->setEnabled( true );
        ui->mpPasswordLineEdit->setEnabled( true );
        ui->mpQoSSpinBox->setEnabled( true );
        ui->mpTopicLineEdit->setEnabled( true );
        Q_EMIT button_clicked_signal( 1 );
    }
}

void
MQTTSubscriberEmbeddedWidget::
host_line_edit_text_changed(const QString & text )
{
    mParameters.msHost = text;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTSubscriberEmbeddedWidget::
port_spin_box_value_changed(int port)
{
    mParameters.miPort = port;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTSubscriberEmbeddedWidget::
username_line_edit_text_changed(const QString & text)
{
    mParameters.msUsername = text;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTSubscriberEmbeddedWidget::
password_line_edit_text_changed(const QString & text)
{
    mParameters.msPassword = text;
    Q_EMIT params_changed_signal( mParameters );

}

void
MQTTSubscriberEmbeddedWidget::
qos_spin_box_value_changed(int qos)
{
    mParameters.miQoS = qos;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTSubscriberEmbeddedWidget::
topic_line_edit_text_changed(const QString & text)
{
    mParameters.msTopic = text;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTSubscriberEmbeddedWidget::
set_params(const MQTTSubscriberParameters &params )
{
    if( mParameters.msHost != params.msHost )
    {
        ui->mpHostLineEdit->blockSignals(true);
        ui->mpHostLineEdit->setText( params.msHost );
        ui->mpHostLineEdit->blockSignals(false);
        mParameters.msHost = params.msHost;
    }
    if( mParameters.miPort != params.miPort )
    {
        ui->mpPortSpinBox->blockSignals(true);
        ui->mpPortSpinBox->setValue( params.miPort );
        ui->mpPortSpinBox->blockSignals(false);
        mParameters.miPort = params.miPort;
    }
    if( mParameters.msUsername != params.msUsername )
    {
        ui->mpUsernameLineEdit->blockSignals(true);
        ui->mpUsernameLineEdit->setText( params.msUsername );
        ui->mpUsernameLineEdit->blockSignals(false);
        mParameters.msUsername = params.msUsername;
    }
    if( mParameters.msPassword != params.msPassword )
    {
        ui->mpPasswordLineEdit->blockSignals(true);
        ui->mpPasswordLineEdit->setText( params.msPassword );
        ui->mpPasswordLineEdit->blockSignals(false);
        mParameters.msPassword = params.msPassword;
    }
    if( mParameters.msTopic != params.msTopic )
    {
        ui->mpTopicLineEdit->blockSignals(true);
        ui->mpTopicLineEdit->setText( params.msTopic );
        ui->mpTopicLineEdit->blockSignals(false);
        mParameters.msTopic = params.msTopic;
    }
    if( mParameters.miQoS != params.miQoS )
    {
        ui->mpQoSSpinBox->blockSignals(true);
        ui->mpQoSSpinBox->setValue( params.miQoS );
        ui->mpQoSSpinBox->blockSignals(false);
        mParameters.miQoS = params.miQoS;
    }
}

void
MQTTSubscriberEmbeddedWidget::
get_params( MQTTSubscriberParameters &params )
{
    params.msHost = ui->mpHostLineEdit->text();
    params.miPort = ui->mpPortSpinBox->value();
    params.msUsername = ui->mpUsernameLineEdit->text();
    params.msPassword = ui->mpPasswordLineEdit->text();
    params.msTopic = ui->mpTopicLineEdit->text();
    params.miQoS = ui->mpQoSSpinBox->value();
}

void
MQTTSubscriberEmbeddedWidget::
set_mqtt_connection_state( bool connected )
{
    if( connected )
    {
        ui->mpHostLineEdit->setEnabled( false );
        ui->mpPortSpinBox->setEnabled( false );
        ui->mpUsernameLineEdit->setEnabled( false );
        ui->mpPasswordLineEdit->setEnabled( false );
        ui->mpQoSSpinBox->setEnabled( false );
        ui->mpTopicLineEdit->setEnabled( false );
    }
    else
    {
        ui->mpHostLineEdit->setEnabled( true );
        ui->mpPortSpinBox->setEnabled( true );
        ui->mpUsernameLineEdit->setEnabled( true );
        ui->mpPasswordLineEdit->setEnabled( true );
        ui->mpQoSSpinBox->setEnabled( true );
        ui->mpTopicLineEdit->setEnabled( true );
    }
}


void
MQTTSubscriberEmbeddedWidget::
resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    Q_EMIT widget_resized_signal();
}
