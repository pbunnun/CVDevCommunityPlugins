
// Copyright © 2024-2025, NECTEC, all rights reserved
//
// This file is distributed under the terms of the GNU General Public License v3 (GPLv3) only.
// It is intended for use as a GPL plugin/submodule for CVDev.
//
// CVDev itself is distributed under the Apache License, Version 2.0.
//
// This file: GPL v3. See <https://www.gnu.org/licenses/gpl-3.0.html>
// CVDev: Apache 2.0. See <https://www.apache.org/licenses/LICENSE-2.0>


#include "MQTTSubscriberModel.hpp"
#include <QDebug>
#include <QEvent>
#include <QDir>
#include <QVariant>
#include <SyncData.hpp>

const QString MQTTSubscriberModel::_category = QString( "GPL" );

const QString MQTTSubscriberModel::_model_name = QString( "MQTT Subscriber" );

MQTTSubscriberModel::
MQTTSubscriberModel()
    : PBNodeDelegateModel( _model_name ),
      // PBNodeDelegateModel( model's name, is it source data, is it enable at start? )
      mpEmbeddedWidget( new MQTTSubscriberEmbeddedWidget( qobject_cast<QWidget *>(this) ) ),
      mpMQTTClient( new QMqttClient( qobject_cast<QWidget *>(this) ) )
{
    mpInformationData = std::make_shared< InformationData >();

    connect( mpEmbeddedWidget, &MQTTSubscriberEmbeddedWidget::button_clicked_signal, this, &MQTTSubscriberModel::em_button_clicked );
    connect( mpEmbeddedWidget, &MQTTSubscriberEmbeddedWidget::params_changed_signal, this, &MQTTSubscriberModel::params_signal_changed );
    connect( mpEmbeddedWidget, &MQTTSubscriberEmbeddedWidget::widget_resized_signal, this, &MQTTSubscriberModel::embeddedWidgetSizeUpdated );

    connect( mpMQTTClient, &QMqttClient::stateChanged, this, &MQTTSubscriberModel::mqtt_state_changed );
    connect( mpMQTTClient, &QMqttClient::disconnected, this, &MQTTSubscriberModel::mqtt_broker_disconnected );
    connect( mpMQTTClient, &QMqttClient::messageReceived, this, &MQTTSubscriberModel::mqtt_message_received );

    QString propId = "host";
    auto propHost = std::make_shared< TypedProperty< QString > > ("Host", propId, QMetaType::QString, mParameters.msHost);
    mvProperty.push_back( propHost );
    mMapIdToProperty[ propId ] = propHost;

    IntPropertyType intPropertyType;
    intPropertyType.miMax = 8888;
    intPropertyType.miMin = 1880;
    intPropertyType.miValue = mParameters.miPort;
    propId = "port";
    auto propPort = std::make_shared< TypedProperty < IntPropertyType > > ("Port", propId, QMetaType::Int, intPropertyType );
    mvProperty.push_back( propPort );
    mMapIdToProperty[ propId ] = propPort;

    intPropertyType.miMax = 2;
    intPropertyType.miMin = 0;
    intPropertyType.miValue = mParameters.miQoS;
    propId = "qos";
    auto propQoS = std::make_shared< TypedProperty < IntPropertyType > > ("QoS", propId, QMetaType::Int, intPropertyType );
    mvProperty.push_back( propQoS );
    mMapIdToProperty[ propId ] = propQoS;

    propId = "username";
    auto propUsername = std::make_shared< TypedProperty< QString > > ("Username", propId, QMetaType::QString, mParameters.msUsername);
    mvProperty.push_back( propUsername );
    mMapIdToProperty[ propId ] = propUsername;

    propId = "password";
    auto propPassword = std::make_shared< TypedProperty< QString > > ("Password", propId, QMetaType::QString, mParameters.msPassword);
    mvProperty.push_back( propPassword );
    mMapIdToProperty[ propId ] = propPassword;

    propId = "topic";
    auto propTopic = std::make_shared< TypedProperty< QString > > ("Topic", propId, QMetaType::QString, mParameters.msTopic);
    mvProperty.push_back( propTopic );
    mMapIdToProperty[ propId ] = propTopic;
}

unsigned int
MQTTSubscriberModel::
nPorts( PortType portType ) const
{
    switch( portType )
    {
    case PortType::In:
        return( 0 );
    case PortType::Out:
        return( 1 );
    default:
        return( 0 );
    }
}

NodeDataType
MQTTSubscriberModel::
dataType(PortType portType, PortIndex portIndex) const
{
    if( portType == PortType::Out )
    {
        if( portIndex == 0 )
            return InformationData().type();
        else
            return NodeDataType();
    }
    else
        return NodeDataType();
}

std::shared_ptr<NodeData>
MQTTSubscriberModel::
outData(PortIndex idx)
{
    std::shared_ptr<NodeData> result;
    if( isEnable() )
    {
        if( idx == 0 )
            result = mpInformationData;
    }
    return result;
}

QJsonObject
MQTTSubscriberModel::
save() const
{
    /*
     * If save() was overrided, PBNodeDelegateModel::save() must be called explicitely.
     */
    QJsonObject modelJson = PBNodeDelegateModel::save();

    QJsonObject cParams;
    cParams[ "host" ] = mParameters.msHost;
    cParams[ "port" ] = mParameters.miPort;
    cParams[ "username" ] = mParameters.msUsername;
    cParams[ "password" ] = mParameters.msPassword;
    cParams[ "qos" ] = mParameters.miQoS;
    cParams[ "topic" ] = mParameters.msTopic;

    modelJson[ "cParams" ] = cParams;

    return modelJson;
}

void
MQTTSubscriberModel::
load(const QJsonObject &p)
{
    /*
     * If load() was overrided, PBNodeDelegateModel::load() must be called explicitely.
     */
    PBNodeDelegateModel::load(p);

    QJsonObject paramsObj = p[ "cParams" ].toObject();

    if( !paramsObj.isEmpty() )
    {
        QJsonValue v = paramsObj[ "host" ];
        if( !v.isNull() )
        {
            auto prop = mMapIdToProperty[ "host" ];
            /* Restore internal property */
            auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
            typedProp->getData() = v.toString();
            mParameters.msHost = v.toString();
        }

        v = paramsObj[ "port" ];
        if( !v.isNull() )
        {
            auto prop = mMapIdToProperty[ "port" ];
            auto typedProp = std::static_pointer_cast< TypedProperty< IntPropertyType > >( prop );
            typedProp->getData().miValue = v.toInt();
            mParameters.miPort = v.toInt();
        }

        v = paramsObj[ "username" ];
        if( !v.isNull() )
        {
            auto prop = mMapIdToProperty[ "username" ];
            /* Restore internal property */
            auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
            typedProp->getData() = v.toString();
            mParameters.msUsername = v.toString();
        }

        v = paramsObj[ "password" ];
        if( !v.isNull() )
        {
            auto prop = mMapIdToProperty[ "password" ];
            /* Restore internal property */
            auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
            typedProp->getData() = v.toString();
            mParameters.msPassword = v.toString();
        }

        v = paramsObj[ "qos" ];
        if( !v.isNull() )
        {
            auto prop = mMapIdToProperty[ "qos" ];
            auto typedProp = std::static_pointer_cast< TypedProperty< IntPropertyType > >( prop );
            typedProp->getData().miValue = v.toInt();
            mParameters.miQoS = v.toInt();
        }

        v = paramsObj[ "topic" ];
        if( !v.isNull() )
        {
            auto prop = mMapIdToProperty[ "topic" ];
            /* Restore internal property */
            auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
            typedProp->getData() = v.toString();
            mParameters.msTopic = v.toString();
        }

        mpEmbeddedWidget->set_params( mParameters );
    }
    mpEmbeddedWidget->setEnabled( isEnable() );
}

void
MQTTSubscriberModel::
setModelProperty( QString & id, const QVariant & value )
{
    PBNodeDelegateModel::setModelProperty( id, value );
    if( !mMapIdToProperty.contains( id ) )
        return;

    auto prop = mMapIdToProperty[ id ];

    if( id == "host" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
        typedProp->getData() = value.toString();
        mParameters.msHost = value.toString();
    }
    else if( id == "port" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< IntPropertyType > >( prop );
        typedProp->getData().miValue = value.toInt();
        mParameters.miPort = value.toInt();
    }
    else if( id == "username" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
        typedProp->getData() = value.toString();
        mParameters.msUsername = value.toString();
    }
    else if( id == "password" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
        typedProp->getData() = value.toString();
        mParameters.msPassword = value.toString();
    }
    else if( id == "qos" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< IntPropertyType > >( prop );
        typedProp->getData().miValue = value.toInt();
        mParameters.miQoS = value.toInt();
    }
    else if( id == "topic" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
        typedProp->getData() = value.toString();
        mParameters.msTopic = value.toString();
    }
    mpEmbeddedWidget->set_params( mParameters );
}

void
MQTTSubscriberModel::
enable_changed(bool enable)
{
    PBNodeDelegateModel::enable_changed( enable );

    mpEmbeddedWidget->setEnabled(enable);
}

void
MQTTSubscriberModel::
em_button_clicked(int button)
{
    if( button == 0 ) //Connect
    {
        mpMQTTClient->setHostname( mParameters.msHost );
        mpMQTTClient->setPort( static_cast<quint16>(mParameters.miPort) );
        mpMQTTClient->setUsername( mParameters.msUsername );
        mpMQTTClient->setPassword( mParameters.msPassword );
        mpMQTTClient->connectToHost();
        mpEmbeddedWidget->set_mqtt_connection_state(true);
    }
    else if( button == 1 ) //Disconnect
    {
        mpMQTTClient->disconnectFromHost();
        mpEmbeddedWidget->set_mqtt_connection_state(false);
    }
}

void
MQTTSubscriberModel::
params_signal_changed( const MQTTSubscriberParameters & params )
{
    if( mParameters.msHost != params.msHost )
    {
        auto prop = mMapIdToProperty["host"];
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >(prop);
        typedProp->getData() = params.msHost;
        mParameters.msHost = params.msHost;

        Q_EMIT property_changed_signal( prop );
    }
    if( mParameters.miPort != params.miPort )
    {
        auto prop = mMapIdToProperty["port"];
        auto typedProp = std::static_pointer_cast< TypedProperty< IntPropertyType > >(prop);
        typedProp->getData().miValue = params.miPort;
        mParameters.miPort = params.miPort;

        Q_EMIT property_changed_signal( prop );
    }
    if( mParameters.msUsername != params.msUsername )
    {
        auto prop = mMapIdToProperty["username"];
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >(prop);
        typedProp->getData() = params.msUsername;
        mParameters.msUsername = params.msUsername;

        Q_EMIT property_changed_signal( prop );
    }
    if( mParameters.msPassword != params.msPassword )
    {
        auto prop = mMapIdToProperty["password"];
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >(prop);
        typedProp->getData() = params.msPassword;
        mParameters.msPassword = params.msPassword;

        Q_EMIT property_changed_signal( prop );
    }
    if( mParameters.miQoS != params.miQoS )
    {
        auto prop = mMapIdToProperty["qos"];
        auto typedProp = std::static_pointer_cast< TypedProperty< IntPropertyType > >(prop);
        typedProp->getData().miValue = params.miQoS;
        mParameters.miQoS = params.miQoS;

        Q_EMIT property_changed_signal( prop );
    }
    if( mParameters.msTopic != params.msTopic )
    {
        auto prop = mMapIdToProperty["topic"];
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >(prop);
        typedProp->getData() = params.msTopic;
        mParameters.msTopic = params.msTopic;

        Q_EMIT property_changed_signal( prop );
    }
}

void
MQTTSubscriberModel::
mqtt_state_changed()
{
    if( mpMQTTClient->state() == QMqttClient::Connected )
    {
        auto subscription = mpMQTTClient->subscribe( mParameters.msTopic, static_cast<quint8>( mParameters.miQoS) );
        if( !subscription )
            mpMQTTClient->disconnectFromHost();
    }
}

void
MQTTSubscriberModel::
mqtt_broker_disconnected()
{
    mpEmbeddedWidget->set_mqtt_connection_state(false);
}

void
MQTTSubscriberModel::
mqtt_message_received(const QByteArray &message, const QMqttTopicName &)
{
    mpInformationData->set_information(message);
    updateAllOutputPorts();
}


