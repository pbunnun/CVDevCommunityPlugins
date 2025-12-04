
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
 * @file MQTTPublisherModel.hpp
 * @brief Node model for publishing messages to MQTT broker.
 *
 * This file implements an MQTT publisher node that connects to an MQTT broker
 * and publishes messages triggered by input data or manual button clicks. It
 * supports configurable QoS levels, retained messages, and authentication.
 *
 * **MQTT Protocol Overview:**
 * MQTT (Message Queuing Telemetry Transport) is a lightweight publish/subscribe
 * messaging protocol designed for IoT and M2M communication.
 *
 * **Key Features:**
 * - Connect to any MQTT broker (Mosquitto, HiveMQ, AWS IoT, etc.)
 * - Configurable QoS (0, 1, 2)
 * - Retained messages for last known state
 * - Username/password authentication
 * - Manual or data-driven publishing
 * - Connection state monitoring
 *
 * **Use Cases:**
 * - IoT sensor data publishing
 * - Remote monitoring alerts
 * - Command and control systems
 * - Event notification
 * - System integration via messaging
 *
 * **Note:** Uses GPL-licensed QtMqtt library.
 *
 * @see MQTTPublisherEmbeddedWidget
 * @see MQTTSubscriberModel
 * @see QMqttClient
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QSemaphore>
#include "MQTTPublisherEmbeddedWidget.hpp"
#include "PBNodeDelegateModel.hpp"
#include <QtMqtt/QMqttClient>

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

/**
 * @class MQTTPublisherModel
 * @brief Node model for publishing messages to MQTT broker.
 *
 * This model integrates MQTT publishing into the dataflow graph, allowing
 * data-driven or manual message publishing to MQTT brokers. It provides
 * flexible configuration and connection management.
 *
 * **Input Ports:**
 * 1. **InformationData** - Message payload to publish (triggers auto-publish)
 *
 * **Output Ports:**
 * None (sink node)
 *
 * **Key Features:**
 * - **Auto-publish:** Publishes when input data arrives
 * - **Manual publish:** Publish button in widget
 * - **QoS Levels:**
 *   - **0:** At most once (fire and forget, no acknowledgment)
 *   - **1:** At least once (acknowledged, may duplicate)
 *   - **2:** Exactly once (assured delivery, highest overhead)
 * - **Retained Messages:** Broker stores last message for new subscribers
 * - **Authentication:** Username/password support
 * - **Connection Management:** Connect/Disconnect controls
 *
 * **Properties (Configurable):**
 * - **host:** Broker hostname or IP address
 * - **port:** Broker port (1883=plain, 8883=TLS)
 * - **username:** Authentication username (optional)
 * - **password:** Authentication password (optional)
 * - **qos:** Quality of Service level (0, 1, or 2)
 * - **retain:** Retain message flag
 * - **topic:** MQTT topic to publish to
 * - **message:** Default message payload
 *
 * **MQTT Topics:**
 * Topics are hierarchical paths separated by slashes:
 * @code
 * sensors/temperature          // Single level
 * building/floor1/room3/temp   // Multi-level
 * device/+/status              // Wildcard (+ = single level)
 * sensors/#                    // Wildcard (# = all descendants)
 * @endcode
 *
 * **Example Workflows:**
 * @code
 * // Publish sensor data
 * [Camera] -> [ObjectDetection] -> [Counter] -> [MQTTPublisher: "counts/people"]
 * 
 * // Alert on threshold
 * [Sensor] -> [Threshold] -> [MQTTPublisher: "alerts/temperature"]
 * 
 * // Status updates
 * [ProcessingNode] -> [StatusText] -> [MQTTPublisher: "system/status"]
 * @endcode
 *
 * **QoS Selection Guide:**
 * - **QoS 0:** Sensor data, high-frequency updates, loss acceptable
 * - **QoS 1:** Important events, occasional duplicates OK
 * - **QoS 2:** Critical commands, financial data, no loss/duplication
 *
 * **Retained Messages:**
 * When retain=true:
 * - Broker stores last message
 * - New subscribers immediately receive it
 * - Useful for status/configuration
 * - Example: "device/status" → "online"
 *
 * **Common MQTT Brokers:**
 * - **Mosquitto:** Open-source, lightweight (mosquitto.org)
 * - **HiveMQ:** Enterprise-grade, clustering
 * - **AWS IoT Core:** Cloud MQTT service
 * - **Azure IoT Hub:** Microsoft cloud platform
 * - **CloudMQTT:** Hosted MQTT service
 * - **EMQX:** Scalable, distributed
 *
 * **Connection Process:**
 * 1. Set broker parameters (host, port, credentials)
 * 2. Click Connect button
 * 3. Widget shows connection state (Connecting → Connected)
 * 4. Publish messages manually or via input data
 * 5. Monitor connection status
 * 6. Auto-reconnect on disconnect (optional)
 *
 * **Publishing Modes:**
 *
 * **Manual Publishing:**
 * 1. Enter message in widget text field
 * 2. Click Publish button
 * 3. Message sent immediately
 *
 * **Data-Driven Publishing:**
 * 1. Connect InformationData input
 * 2. Each new data arrival triggers publish
 * 3. Input data becomes message payload
 *
 * **Error Handling:**
 * - Connection failures shown in widget (red button)
 * - Publish failures logged but not blocking
 * - Auto-reconnect attempts on disconnect
 * - Invalid QoS/topic handled gracefully
 *
 * **Security Considerations:**
 * - Use TLS port (8883) for encrypted communication
 * - Store passwords securely (consider environment variables)
 * - Implement access control on broker
 * - Validate topic names to prevent injection
 * - Monitor for unauthorized publishes
 *
 * **Performance:**
 * - Non-blocking async operations
 * - Handles high-frequency publishing (100+ msg/sec)
 * - Queuing for offline periods (QoS 1/2)
 * - Minimal overhead for QoS 0
 *
 * **Best Practices:**
 * 1. Use descriptive topic hierarchies
 * 2. Choose appropriate QoS for use case
 * 3. Set retain flag for status messages
 * 4. Implement Last Will for disconnect detection
 * 5. Monitor broker load and quotas
 * 6. Use TLS in production
 * 7. Test with local broker before cloud deployment
 * 8. Handle connection state changes gracefully
 *
 * **Troubleshooting:**
 *
 * **Connection Failed:**
 * - Verify broker is running and accessible
 * - Check host/port configuration
 * - Validate username/password
 * - Check firewall rules
 * - Test with MQTT client tool (mosquitto_pub)
 *
 * **Messages Not Received:**
 * - Verify topic name (case-sensitive)
 * - Check subscriber topic filters
 * - Ensure QoS compatible
 * - Monitor broker logs
 *
 * **Slow Publishing:**
 * - Reduce QoS level if acceptable
 * - Check network latency
 * - Monitor broker performance
 * - Consider batching messages
 *
 * @see MQTTPublisherEmbeddedWidget
 * @see MQTTSubscriberModel
 * @see QMqttClient
 */
class MQTTPublisherModel : public PBNodeDelegateModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an MQTTPublisherModel.
     *
     * Initializes MQTT client and embedded widget.
     */
    MQTTPublisherModel();

    /**
     * @brief Destructor.
     *
     * Disconnects from broker and cleans up MQTT client.
     */
    virtual
        ~MQTTPublisherModel() override
        {
            if( mpMQTTClient )
                delete mpMQTTClient;
        }

    /**
     * @brief Saves model state to JSON.
     * @return QJsonObject containing connection parameters.
     *
     * Saves all configuration except password (security).
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
     * @return 1 for input (message), 0 for output (sink).
     */
    unsigned int
    nPorts(PortType portType) const override;

    /**
     * @brief Returns the data type for a specific port.
     * @param portType Input or Output.
     * @param portIndex Port index.
     * @return InformationData for input.
     */
    NodeDataType
    dataType(PortType portType, PortIndex portIndex) const override;

    /**
     * @brief Returns output data (none for sink node).
     * @param port Output port index.
     * @return nullptr (no outputs).
     */
    std::shared_ptr<NodeData>
    outData(PortIndex port) override;

    /**
     * @brief Sets input data and publishes to MQTT broker.
     * @param nodeData Input InformationData (message payload).
     * @param Port index (0).
     *
     * Triggers automatic publish when data arrives.
     */
    void
    setInData(std::shared_ptr<NodeData>, PortIndex) override;

    /**
     * @brief Returns the embedded configuration widget.
     * @return Pointer to MQTTPublisherEmbeddedWidget.
     */
    QWidget *
    embeddedWidget() override { return mpEmbeddedWidget; }

    /**
     * @brief Sets a model property.
     * @param Property name.
     * @param QVariant value.
     *
     * Handles dynamic property updates.
     */
    void
    setModelProperty( QString &, const QVariant & ) override;

    static const QString _category;   ///< Node category
    static const QString _model_name; ///< Node display name

private Q_SLOTS:
    /**
     * @brief Slot for widget button clicks.
     * @param button Button identifier (0=Connect, 1=Publish).
     *
     * Handles Connect/Disconnect and Publish actions.
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
    params_signal_changed( const MQTTPublisherParameters & params );

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
     * Updates widget connection status indicator.
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

private:
    MQTTPublisherEmbeddedWidget * mpEmbeddedWidget {nullptr}; ///< Configuration widget
    MQTTPublisherParameters mParameters;                       ///< Connection parameters

    QMqttClient * mpMQTTClient {nullptr};                      ///< MQTT client instance
};

