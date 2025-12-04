
// Copyright © 2024-2025, NECTEC, all rights reserved
//
// This file is distributed under the terms of the GNU General Public License v3 (GPLv3) only.
// It is intended for use as a GPL plugin/submodule for CVDev.
//
// CVDev itself is distributed under the Apache License, Version 2.0.
//
// This file: GPL v3. See <https://www.gnu.org/licenses/gpl-3.0.html>
// CVDev: Apache 2.0. See <https://www.apache.org/licenses/LICENSE-2.0>


#include "MQTTPublisherModel.hpp"
#include <QDebug>
#include <QEvent>
#include <QDir>
#include <QVariant>
#include <SyncData.hpp>

const QString MQTTPublisherModel::_category = QString( "GPL" );

const QString MQTTPublisherModel::_model_name = QString( "MQTT Publisher" );

MQTTPublisherModel::
MQTTPublisherModel()
    : PBNodeDelegateModel( _model_name ),
      // PBNodeDelegateModel( model's name, is it source data, is it enable at start? )
      mpEmbeddedWidget( new MQTTPublisherEmbeddedWidget( qobject_cast<QWidget *>(this) ) ),
      mpMQTTClient( new QMqttClient( qobject_cast<QWidget *>(this) ) )
{
    connect( mpEmbeddedWidget, &MQTTPublisherEmbeddedWidget::button_clicked_signal, this, &MQTTPublisherModel::em_button_clicked );
    connect( mpEmbeddedWidget, &MQTTPublisherEmbeddedWidget::params_changed_signal, this, &MQTTPublisherModel::params_signal_changed );
    connect( mpEmbeddedWidget, &MQTTPublisherEmbeddedWidget::widget_resized_signal, this, &MQTTPublisherModel::embeddedWidgetSizeUpdated );

    connect( mpMQTTClient, &QMqttClient::stateChanged, this, &MQTTPublisherModel::mqtt_state_changed );
    connect( mpMQTTClient, &QMqttClient::disconnected, this, &MQTTPublisherModel::mqtt_broker_disconnected );

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

    propId = "retain";
    auto propRetain = std::make_shared< TypedProperty < bool > >("Retain", propId, QMetaType::Bool, mParameters.mbRetain);
    mvProperty.push_back( propRetain );
    mMapIdToProperty[ propId ] = propRetain;

    propId = "message";
    auto propMessage = std::make_shared< TypedProperty< QString > > ("Message", propId, QMetaType::QString, mParameters.msMessage);
    mvProperty.push_back( propMessage );
    mMapIdToProperty[ propId ] = propMessage;
}

unsigned int
MQTTPublisherModel::
nPorts( PortType portType ) const
{
    switch( portType )
    {
    case PortType::In:
        return( 2 );
    case PortType::Out:
        return( 0 );
    default:
        return( 0 );
    }
}

NodeDataType
MQTTPublisherModel::
dataType(PortType portType, PortIndex portIndex) const
{
    if( portType == PortType::In )
    {
        if( portIndex == 0 )
            return SyncData().type();
        else if( portIndex == 1 )
            return InformationData().type();
        else
            return NodeDataType();
    }
    else
        return NodeDataType();
}

std::shared_ptr<NodeData>
MQTTPublisherModel::
outData(PortIndex)
{
    return nullptr;
}

QJsonObject
MQTTPublisherModel::
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
    cParams[ "retain" ] = mParameters.mbRetain;
    cParams[ "topic" ] = mParameters.msTopic;
    cParams[ "message" ] = mParameters.msMessage;

    modelJson[ "cParams" ] = cParams;

    return modelJson;
}

void
MQTTPublisherModel::
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

        v = paramsObj["retain"];
        if( !v.isNull() )
        {
            auto prop = mMapIdToProperty[ "retain" ];
            auto typedProp = std::static_pointer_cast< TypedProperty< bool > >( prop );
            typedProp->getData() = v.toBool();
            mParameters.mbRetain = v.toBool();
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

        v = paramsObj[ "message" ];
        if( !v.isNull() )
        {
            auto prop = mMapIdToProperty[ "message" ];
            /* Restore internal property */
            auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
            typedProp->getData() = v.toString();
            mParameters.msMessage = v.toString();
        }
        mpEmbeddedWidget->set_params( mParameters );
    }
    mpEmbeddedWidget->setEnabled( isEnable() );
}

void
MQTTPublisherModel::
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
    else if( id == "retain" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< bool > >( prop );
        typedProp->getData() = value.toBool();
        mParameters.mbRetain = value.toBool();
    }
    else if( id == "topic" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
        typedProp->getData() = value.toString();
        mParameters.msTopic = value.toString();
    }
    else if( id == "message" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
        typedProp->getData() = value.toString();
        mParameters.msMessage = value.toString();
    }
    mpEmbeddedWidget->set_params( mParameters );
}

void
MQTTPublisherModel::
enable_changed(bool enable)
{
    PBNodeDelegateModel::enable_changed( enable );

    mpEmbeddedWidget->setEnabled(enable);
}

void
MQTTPublisherModel::
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
    else if( button == 2 ) //Publish
    {
        mpMQTTClient->publish( mParameters.msTopic, mParameters.msMessage.toUtf8(), static_cast<quint8>(mParameters.miQoS), mParameters.mbRetain );
    }
}

void
MQTTPublisherModel::
params_signal_changed( const MQTTPublisherParameters & params )
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
    if( mParameters.mbRetain != params.mbRetain )
    {
        auto prop = mMapIdToProperty["retain"];
        auto typedProp = std::static_pointer_cast< TypedProperty< bool > >(prop);
        typedProp->getData() = params.mbRetain;
        mParameters.mbRetain = params.mbRetain;

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
    if( mParameters.msMessage != params.msMessage )
    {
        auto prop = mMapIdToProperty["message"];
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >(prop);
        typedProp->getData() = params.msMessage;
        mParameters.msMessage = params.msMessage;

        Q_EMIT property_changed_signal( prop );
    }
}

void
MQTTPublisherModel::
mqtt_state_changed()
{
    qDebug() << "MQTT State : " << mpMQTTClient->state();
}

void
MQTTPublisherModel::
mqtt_broker_disconnected()
{
    mpEmbeddedWidget->set_mqtt_connection_state(false);
}

void
MQTTPublisherModel::
setInData( std::shared_ptr< NodeData > nodeData, PortIndex )
{
    if( !isEnable() )
        return;
    if( nodeData )
    {
        auto d = std::dynamic_pointer_cast< InformationData >( nodeData );
        mParameters.msMessage = d->info();
        mpEmbeddedWidget->set_params( mParameters );
        em_button_clicked(2);
    }
}


