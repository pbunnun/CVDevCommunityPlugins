
// Copyright © 2025, NECTEC, all rights reserved
//
// This file is distributed under the terms of the GNU General Public License v3 (GPLv3) only.
// It is intended for use as a GPL plugin/submodule for CVDev.
//
// CVDev itself is distributed under the Apache License, Version 2.0.
//
// This file: GPL v3. See <https://www.gnu.org/licenses/gpl-3.0.html>
// CVDev: Apache 2.0. See <https://www.apache.org/licenses/LICENSE-2.0>


#include "MQTTPublisherEmbeddedWidget.hpp"
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include "ui_MQTTPublisherEmbeddedWidget.h"
#include <QDebug>

MQTTPublisherEmbeddedWidget::MQTTPublisherEmbeddedWidget( QWidget *parent )
    : QWidget( parent ),
      ui( new Ui::MQTTPublisherEmbeddedWidget )
{
    ui->setupUi( this );
    connect(ui->mpConnectButton, QOverload<bool>::of(&QPushButton::clicked), this, &MQTTPublisherEmbeddedWidget::connect_button_clicked);
    connect(ui->mpPublishButton, &QPushButton::clicked, this, &MQTTPublisherEmbeddedWidget::publish_button_clicked);
    connect(ui->mpHostLineEdit, &QLineEdit::textChanged, this, &MQTTPublisherEmbeddedWidget::host_line_edit_text_changed);
    connect(ui->mpPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MQTTPublisherEmbeddedWidget::port_spin_box_value_changed);
    connect(ui->mpUsernameLineEdit, &QLineEdit::textChanged, this, &MQTTPublisherEmbeddedWidget::username_line_edit_text_changed);
    connect(ui->mpPasswordLineEdit, &QLineEdit::textChanged, this, &MQTTPublisherEmbeddedWidget::password_line_edit_text_changed);
    connect(ui->mpQoSSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MQTTPublisherEmbeddedWidget::qos_spin_box_value_changed);
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
    connect(ui->mpRetainCheckbox, QOverload<int>::of(&QCheckBox::stateChanged), this, &MQTTPublisherEmbeddedWidget::retain_checkbox_state_changed);
#else
    connect(ui->mpRetainCheckbox, &QCheckBox::checkStateChanged, this, &MQTTPublisherEmbeddedWidget::retain_checkbox_check_state_changed);
#endif
    connect(ui->mpTopicLineEdit, &QLineEdit::textChanged, this, &MQTTPublisherEmbeddedWidget::topic_line_edit_text_changed);
    connect(ui->mpMessageLineEdit, &QLineEdit::textChanged, this, &MQTTPublisherEmbeddedWidget::message_line_edit_text_changed);
}

MQTTPublisherEmbeddedWidget::~MQTTPublisherEmbeddedWidget()
{
    delete ui;
}

void
MQTTPublisherEmbeddedWidget::
connect_button_clicked(bool checked)
{
    if( checked )
    {
        ui->mpPublishButton->setEnabled( true );
        ui->mpHostLineEdit->setEnabled( false );
        ui->mpPortSpinBox->setEnabled( false );
        ui->mpUsernameLineEdit->setEnabled( false );
        ui->mpPasswordLineEdit->setEnabled( false );
        ui->mpQoSSpinBox->setEnabled( false );
        ui->mpRetainCheckbox->setEnabled( false );
        ui->mpTopicLineEdit->setEnabled( false );
        Q_EMIT button_clicked_signal( 0 );
    }
    else
    {
        ui->mpPublishButton->setEnabled( false );
        ui->mpHostLineEdit->setEnabled( true );
        ui->mpPortSpinBox->setEnabled( true );
        ui->mpUsernameLineEdit->setEnabled( true );
        ui->mpPasswordLineEdit->setEnabled( true );
        ui->mpQoSSpinBox->setEnabled( true );
        ui->mpRetainCheckbox->setEnabled( true );
        ui->mpTopicLineEdit->setEnabled( true );
        Q_EMIT button_clicked_signal( 1 );
    }
}

void
MQTTPublisherEmbeddedWidget::
host_line_edit_text_changed(const QString & text )
{
    mParameters.msHost = text;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTPublisherEmbeddedWidget::
port_spin_box_value_changed(int port)
{
    mParameters.miPort = port;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTPublisherEmbeddedWidget::
username_line_edit_text_changed(const QString & text)
{
    mParameters.msUsername = text;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTPublisherEmbeddedWidget::
password_line_edit_text_changed(const QString & text)
{
    mParameters.msPassword = text;
    Q_EMIT params_changed_signal( mParameters );

}

void
MQTTPublisherEmbeddedWidget::
qos_spin_box_value_changed(int qos)
{
    mParameters.miQoS = qos;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTPublisherEmbeddedWidget::
#if (QT_VERSION < QT_VERSION_CHECK(6, 7, 0) )
retain_checkbox_state_changed(int state)
#else
retain_checkbox_check_state_changed(Qt::CheckState state)
#endif
{
    mParameters.mbRetain = (state == Qt::Checked);
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTPublisherEmbeddedWidget::
topic_line_edit_text_changed(const QString & text)
{
    mParameters.msTopic = text;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTPublisherEmbeddedWidget::
message_line_edit_text_changed(const QString & text)
{
    mParameters.msMessage = text;
    Q_EMIT params_changed_signal( mParameters );
}

void
MQTTPublisherEmbeddedWidget::
publish_button_clicked()
{
    Q_EMIT button_clicked_signal( 2 );
}

void
MQTTPublisherEmbeddedWidget::
set_params(const MQTTPublisherParameters &params )
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
    if( mParameters.mbRetain != params.mbRetain )
    {
        ui->mpRetainCheckbox->blockSignals(true);
        ui->mpRetainCheckbox->setChecked( params.mbRetain );
        ui->mpRetainCheckbox->blockSignals(false);
        mParameters.mbRetain = params.mbRetain;
    }
    if( mParameters.msMessage != params.msMessage )
    {
        ui->mpMessageLineEdit->blockSignals(true);
        ui->mpMessageLineEdit->setText( params.msMessage );
        ui->mpMessageLineEdit->blockSignals(false);
        mParameters.msMessage = params.msMessage;
    }
}

void
MQTTPublisherEmbeddedWidget::
get_params( MQTTPublisherParameters &params )
{
    params.msHost = ui->mpHostLineEdit->text();
    params.miPort = ui->mpPortSpinBox->value();
    params.msUsername = ui->mpUsernameLineEdit->text();
    params.msPassword = ui->mpPasswordLineEdit->text();
    params.msTopic = ui->mpTopicLineEdit->text();
    params.miQoS = ui->mpQoSSpinBox->value();
    params.mbRetain = ui->mpRetainCheckbox->isChecked();
    params.msMessage = ui->mpMessageLineEdit->text();
}

void
MQTTPublisherEmbeddedWidget::
set_mqtt_connection_state( bool connected )
{
    if( connected )
    {
        ui->mpPublishButton->setEnabled( true );
        ui->mpHostLineEdit->setEnabled( false );
        ui->mpPortSpinBox->setEnabled( false );
        ui->mpUsernameLineEdit->setEnabled( false );
        ui->mpPasswordLineEdit->setEnabled( false );
        ui->mpQoSSpinBox->setEnabled( false );
        ui->mpRetainCheckbox->setEnabled( false );
        ui->mpTopicLineEdit->setEnabled( false );
    }
    else
    {
        ui->mpPublishButton->setEnabled( false );
        ui->mpHostLineEdit->setEnabled( true );
        ui->mpPortSpinBox->setEnabled( true );
        ui->mpUsernameLineEdit->setEnabled( true );
        ui->mpPasswordLineEdit->setEnabled( true );
        ui->mpQoSSpinBox->setEnabled( true );
        ui->mpRetainCheckbox->setEnabled( true );
        ui->mpTopicLineEdit->setEnabled( true );
    }
}


void
MQTTPublisherEmbeddedWidget::
resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    Q_EMIT widget_resized_signal();
}
