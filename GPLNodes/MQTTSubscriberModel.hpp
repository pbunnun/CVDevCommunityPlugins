
// Copyright © 2024-2025, NECTEC, all rights reserved
//
// This file is distributed under the terms of the GNU General Public License v3 (GPLv3) only.
// It is intended for use as a GPL plugin/submodule for CVDev.
//
// CVDev itself is distributed under the Apache License, Version 2.0.
//
// This file: GPL v3. See <https://www.gnu.org/licenses/gpl-3.0.html>
// CVDev: Apache 2.0. See <https://www.apache.org/licenses/LICENSE-2.0>


/**
 * @file MQTTSubscriberModel.hpp
 * @brief Node model for subscribing to MQTT broker topics.
 *
 * This file implements an MQTT subscriber node that connects to an MQTT broker
 * and receives messages from subscribed topics. It outputs received messages
 * as InformationData for downstream processing.
 *
 * **Key Features:**
 * - Connect to any MQTT broker
 * - Topic wildcard subscriptions (+ and #)
 * - Configurable QoS levels
 * - Username/password authentication
 * - Real-time message output
 * - Connection state monitoring
 * - Automatic resubscription on reconnect
 *
 * **Use Cases:**
 * - IoT data ingestion
 * - Remote monitoring
 * - Event-driven processing
 * - System integration via messaging
 * - Multi-source data aggregation
 *
 * **Note:** Uses GPL-licensed QtMqtt library.
 *
 * @see MQTTSubscriberEmbeddedWidget
 * @see MQTTPublisherModel
 * @see QMqttClient
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QSemaphore>
#include "MQTTSubscriberEmbeddedWidget.hpp"
#include "PBNodeDelegateModel.hpp"
#include <QtMqtt/QMqttClient>
#include "InformationData.hpp"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

/**
 * @class MQTTSubscriberModel
 * @brief Node model for subscribing to MQTT broker topics.
 *
 * This model integrates MQTT subscription into the dataflow graph, allowing
 * real-time message reception from MQTT brokers. Received messages are output
 * as InformationData for downstream processing.
 *
 * **Input Ports:**
 * None (source node)
 *
 * **Output Ports:**
 * 1. **InformationData** - Received message payload
 *
 * **Key Features:**
 * - **Topic Wildcards:**
 *   - **+:** Single-level wildcard (e.g., "sensors/+/temp" matches any sensor ID)
 *   - **#:** Multi-level wildcard (e.g., "building/#" matches all descendants)
 * - **QoS Levels:** Maximum requested QoS (broker may downgrade)
 * - **Authentication:** Username/password support
 * - **Auto-reconnect:** Maintains connection and resubscribes
 * - **Message Queueing:** Buffers messages during processing
 *
 * **Properties (Configurable):**
 * - **host:** Broker hostname or IP address
 * - **port:** Broker port (1883=plain, 8883=TLS)
 * - **username:** Authentication username (optional)
 * - **password:** Authentication password (optional)
 * - **qos:** Maximum Quality of Service for subscription (0, 1, or 2)
 * - **topic:** Topic filter to subscribe to (supports wildcards)
 *
 * **Topic Filter Examples:**
 * @code
 * // Exact topic
 * "sensors/temperature"
 * 
 * // Single-level wildcard (+ matches one level)
 * "sensors/+/temperature"       // Matches: sensors/living_room/temperature
 *                                //          sensors/kitchen/temperature
 * 
 * // Multi-level wildcard (# matches all remaining levels)
 * "building/floor1/#"           // Matches: building/floor1/room1/temp
 *                                //          building/floor1/room2/humidity
 *                                //          building/floor1/room3/light/level
 * 
 * // Combined wildcards
 * "devices/+/sensors/#"         // Matches: devices/device1/sensors/temp
 *                                //          devices/device2/sensors/humidity/value
 * 
 * // All topics (use with caution!)
 * "#"
 * @endcode
 *
 * **Example Workflows:**
 * @code
 * // Process sensor data
 * [MQTTSubscriber: "sensors/+/temperature"] -> [Parser] -> [Display]
 * 
 * // Alert on messages
 * [MQTTSubscriber: "alerts/#"] -> [Filter] -> [Notification]
 * 
 * // Multi-source aggregation
 * [MQTTSubscriber: "data/#"] -> [Aggregator] -> [Database]
 * @endcode
 *
 * **QoS Behavior:**
 * - **Requested QoS:** Maximum desired (set in subscription)
 * - **Granted QoS:** Actual QoS (may be lower, broker decides)
 * - **Message QoS:** Individual message QoS (publisher sets)
 * - **Effective QoS:** Minimum of (granted, message) QoS
 *
 * **Subscription Process:**
 * 1. Set broker parameters and topic filter
 * 2. Click Connect button
 * 3. Client connects to broker
 * 4. Automatically subscribes to topic filter
 * 5. Receives matching messages
 * 6. Outputs as InformationData
 *
 * **Wildcard Matching Rules:**
 * - **+:** Matches exactly one level (non-empty)
 * - **#:** Matches zero or more levels (must be last)
 * - Wildcards only match level boundaries (/)
 * - Topics starting with $ (system topics) not matched by #
 *
 * **Message Handling:**
 * - **Arrival:** mqtt_message_received() slot called
 * - **Processing:** Message converted to InformationData
 * - **Output:** dataUpdated() signal triggers downstream
 * - **Queueing:** Messages buffered if processing slow
 *
 * **Connection Management:**
 * - **Connect:** Widget button or enable node
 * - **Disconnect:** Widget button, disable node, or error
 * - **Reconnect:** Automatic on connection loss (with backoff)
 * - **Resubscribe:** Automatic after reconnection
 *
 * **Error Handling:**
 * - Connection failures shown in widget (red button)
 * - Subscription errors logged
 * - Auto-reconnect on disconnect
 * - Invalid topic filters rejected
 *
 * **Performance Considerations:**
 * - High-frequency messages: Use QoS 0 for throughput
 * - Wildcard efficiency: More specific = better performance
 * - Message buffering: Monitor queue size
 * - Processing speed: Ensure downstream can keep up
 *
 * **Security:**
 * - Use TLS port (8883) for encrypted communication
 * - Implement ACLs on broker (restrict topic access)
 * - Validate received data (untrusted sources)
 * - Monitor for message injection attacks
 * - Store credentials securely
 *
 * **Common Patterns:**
 *
 * **Device Monitoring:**
 * @code
 * Topic: "devices/+/status"
 * Receives: devices/device1/status, devices/device2/status, ...
 * Use: Monitor all device statuses
 * @endcode
 *
 * **Hierarchical Data:**
 * @code
 * Topic: "building/floor2/#"
 * Receives: All topics under floor2
 * Use: Floor-specific monitoring
 * @endcode
 *
 * **Event Streams:**
 * @code
 * Topic: "events/critical"
 * Receives: Critical events only
 * Use: Alert processing
 * @endcode
 *
 * **Best Practices:**
 * 1. Use specific topic filters (avoid "#" when possible)
 * 2. Choose appropriate QoS for use case
 * 3. Implement message validation
 * 4. Monitor connection state
 * 5. Handle reconnection gracefully
 * 6. Test topic filters thoroughly
 * 7. Consider message volume with wildcards
 * 8. Use retained messages for state initialization
 *
 * **Troubleshooting:**
 *
 * **No Messages Received:**
 * - Verify broker connection (check widget status)
 * - Confirm topic filter matches published topics
 * - Check topic case sensitivity
 * - Verify publisher is sending messages
 * - Test with MQTT client tool (mosquitto_sub)
 * - Check broker ACLs (authorization)
 *
 * **Connection Issues:**
 * - Verify broker is running and accessible
 * - Check host/port configuration
 * - Validate credentials
 * - Check firewall rules
 * - Test network connectivity
 *
 * **Slow Message Delivery:**
 * - Check network latency
 * - Monitor broker load
 * - Verify downstream processing speed
 * - Consider reducing QoS if acceptable
 * - Check message queue buildup
 *
 * **Missing Some Messages:**
 * - QoS 0: Expected behavior (no guarantee)
 * - Check network reliability
 * - Monitor for connection drops
 * - Increase QoS level if critical
 *
 * @see MQTTSubscriberEmbeddedWidget
 * @see MQTTPublisherModel
 * @see QMqttClient
 */
class MQTTSubscriberModel : public PBNodeDelegateModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an MQTTSubscriberModel.
     *
     * Initializes MQTT client and embedded widget.
     */
    MQTTSubscriberModel();

    /**
     * @brief Destructor.
     *
     * Unsubscribes, disconnects from broker, and cleans up MQTT client.
     */
    virtual
        ~MQTTSubscriberModel() override
        {
            if( mpMQTTClient )
                delete mpMQTTClient;
        }

    /**
     * @brief Saves model state to JSON.
     * @return QJsonObject containing connection parameters.
     *
     * Saves all configuration except password.
     */
    QJsonObject
    save() const override;

    /**
     * @brief Loads model state from JSON.
     * @param p QJsonObject with saved configuration.
     */
    void
    load(QJsonObject const &p) override;

    /**
     * @brief Returns the number of ports.
     * @param portType Input or Output.
     * @return 0 for input (source), 1 for output (message).
     */
    unsigned int
    nPorts(PortType portType) const override;

    /**
     * @brief Returns the data type for a specific port.
     * @param portType Input or Output.
     * @param portIndex Port index.
     * @return InformationData for output.
     */
    NodeDataType
    dataType(PortType portType, PortIndex portIndex) const override;

    /**
     * @brief Returns output data.
     * @param port Output port index (0).
     * @return Shared pointer to last received message data.
     */
    std::shared_ptr<NodeData>
    outData(PortIndex port) override;

    /**
     * @brief Sets input data (no inputs for source node).
     * @param nodeData Input data (unused).
     * @param Port index.
     */
    void
    setInData(std::shared_ptr<NodeData>, PortIndex) override { }

    /**
     * @brief Returns the embedded configuration widget.
     * @return Pointer to MQTTSubscriberEmbeddedWidget.
     */
    QWidget *
    embeddedWidget() override { return mpEmbeddedWidget; }
    
    /**
     * @brief Sets a model property.
     * @param Property name.
     * @param QVariant value.
     */
    void
    setModelProperty( QString &, const QVariant & ) override;

    static const QString _category;   ///< Node category
    static const QString _model_name; ///< Node display name

private Q_SLOTS:
    /**
     * @brief Slot for widget button clicks.
     * @param button Button identifier (0=Connect).
     *
     * Handles Connect/Disconnect actions.
     */
    void
    em_button_clicked( int button );

    /**
     * @brief Slot for parameter changes from widget.
     * @param params Updated parameters.
     *
     * Updates model configuration and reconnects if needed.
     */
    void
    params_signal_changed( const MQTTSubscriberParameters & params );

    /**
     * @brief Slot for node enable/disable state changes.
     * @param Enabled state.
     *
     * Disconnects when node is disabled.
     */
    void
    enable_changed(bool) override;

    /**
     * @brief Slot for MQTT client state changes.
     *
     * Updates widget connection status, subscribes when connected.
     */
    void
    mqtt_state_changed();

    /**
     * @brief Slot for broker disconnection events.
     *
     * Handles unexpected disconnections and cleanup.
     */
    void
    mqtt_broker_disconnected();

    /**
     * @brief Slot for received MQTT messages.
     * @param message Message payload bytes.
     * @param topic Topic name the message was published to.
     *
     * Converts message to InformationData and triggers output update.
     */
    void
    mqtt_message_received(const QByteArray &message, const QMqttTopicName &topic);

private:
    std::shared_ptr< InformationData > mpInformationData { nullptr }; ///< Output message data
    MQTTSubscriberEmbeddedWidget * mpEmbeddedWidget { nullptr };      ///< Configuration widget
    MQTTSubscriberParameters mParameters;                              ///< Connection parameters

    QMqttClient * mpMQTTClient { nullptr };                            ///< MQTT client instance
};

