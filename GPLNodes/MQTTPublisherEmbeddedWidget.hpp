
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
 * @file MQTTPublisherEmbeddedWidget.hpp
 * @brief Embedded widget for MQTT publisher configuration and control.
 *
 * This file provides a UI widget for configuring MQTT broker connection
 * parameters and publishing messages. It offers controls for connection
 * management, QoS settings, retained messages, and manual message publishing.
 *
 * **Key Features:**
 * - Broker connection configuration (host, port, credentials)
 * - Quality of Service (QoS) level selection
 * - Retained message flag
 * - Topic and message input
 * - Connect/Disconnect button
 * - Manual publish button
 * - Connection status indication
 *
 * **MQTT Concepts:**
 * - **QoS 0:** At most once delivery (fire and forget)
 * - **QoS 1:** At least once delivery (acknowledged)
 * - **QoS 2:** Exactly once delivery (assured)
 * - **Retained:** Broker stores last message for new subscribers
 *
 * @see MQTTPublisherModel
 * @see QMqttClient
 */

#pragma once

#include <QWidget>
#include <QSpinBox>
#include <QtGlobal>

namespace Ui {
class MQTTPublisherEmbeddedWidget;
}

/**
 * @struct MQTTPublisherParameters
 * @brief MQTT publisher configuration parameters.
 *
 * Encapsulates all settings needed for MQTT broker connection and publishing.
 *
 * **Parameter Details:**
 * - **msHost:** Broker hostname or IP (e.g., "mqtt.eclipse.org", "192.168.1.100")
 * - **miPort:** Broker port (default: 1883 for plain, 8883 for TLS)
 * - **msUsername:** Authentication username (optional, empty if not needed)
 * - **msPassword:** Authentication password (optional, empty if not needed)
 * - **miQoS:** Quality of Service level (0, 1, or 2)
 * - **mbRetain:** Retain flag (true = broker stores message for new subscribers)
 * - **msTopic:** MQTT topic to publish to (e.g., "sensors/temperature")
 * - **msMessage:** Message payload to publish
 */
typedef struct MQTTPublisherParameters{
    QString msHost;              ///< Broker hostname or IP address
    int miPort{1883};            ///< Broker port (1883=plain, 8883=TLS)
    QString msUsername;          ///< Authentication username (optional)
    QString msPassword;          ///< Authentication password (optional)
    int miQoS{0};                ///< Quality of Service (0, 1, or 2)
    bool mbRetain{false};        ///< Retain message flag
    QString msTopic;             ///< Topic to publish to
    QString msMessage;           ///< Message payload
} MQTTPublisherParameters;

/**
 * @class MQTTPublisherEmbeddedWidget
 * @brief Widget for MQTT publisher configuration and control.
 *
 * This widget provides a comprehensive interface for configuring MQTT
 * broker connections and publishing messages. It includes input fields
 * for all connection parameters, QoS settings, and manual publish control.
 *
 * **Widget Components:**
 * - **Host:** Text field for broker address
 * - **Port:** Spin box for port number (1-65535)
 * - **Username:** Text field for authentication
 * - **Password:** Secure text field (hidden characters)
 * - **QoS:** Spin box (0-2) for Quality of Service
 * - **Retain:** Checkbox for retained messages
 * - **Topic:** Text field for publish topic
 * - **Message:** Text field for message payload
 * - **Connect Button:** Toggle connection to broker
 * - **Publish Button:** Manually publish current message
 *
 * **Connection States:**
 * - **Disconnected:** Gray button, "Connect" label
 * - **Connecting:** Orange button, "Connecting..." label
 * - **Connected:** Green button, "Disconnect" label
 * - **Error:** Red button, "Error - Connect" label
 *
 * **Qt Version Compatibility:**
 * Handles API changes between Qt 6.6 and 6.7 for checkbox state signals.
 *
 * @see MQTTPublisherModel
 */
class MQTTPublisherEmbeddedWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an MQTTPublisherEmbeddedWidget.
     * @param parent Parent widget.
     *
     * Initializes UI components and sets default values.
     */
    explicit MQTTPublisherEmbeddedWidget( QWidget *parent = nullptr );
    
    /**
     * @brief Destructor.
     */
    ~MQTTPublisherEmbeddedWidget();

    /**
     * @brief Sets widget parameters from model.
     * @param params Publisher parameters to display.
     *
     * Updates all UI fields to reflect provided parameters.
     */
    void
    set_params(const MQTTPublisherParameters & params );

    /**
     * @brief Gets current widget parameters.
     * @param params Output parameter to receive current settings.
     *
     * Reads all UI fields and populates parameter structure.
     */
    void
    get_params( MQTTPublisherParameters & params );

    /**
     * @brief Updates connection state visual indication.
     * @param connected true if connected to broker.
     *
     * Changes button color and label based on connection status.
     */
    void
    set_mqtt_connection_state( bool connected );

Q_SIGNALS:
    /**
     * @brief Signal emitted when a button is clicked.
     * @param button Button identifier (0=Connect, 1=Publish).
     *
     * Notifies model of user button actions.
     */
    void
    button_clicked_signal( int button );

    /**
     * @brief Signal emitted when any parameter changes.
     * @param params Updated parameters.
     *
     * Allows model to respond to configuration changes.
     */
    void
    params_changed_signal( const MQTTPublisherParameters & params );

    /**
     * @brief Signal emitted when widget is resized.
     *
     * Notifies node view to update geometry.
     */
    void
    widget_resized_signal();

public Q_SLOTS:
    /**
     * @brief Slot for Connect/Disconnect button clicks.
     * @param checked Button toggle state (true=connect, false=disconnect).
     *
     * Emits button_clicked_signal(0) to trigger connection toggle.
     */
    void
    connect_button_clicked(bool checked);

    /**
     * @brief Slot for Publish button clicks.
     *
     * Emits button_clicked_signal(1) to trigger message publish.
     */
    void
    publish_button_clicked();

    /**
     * @brief Slot for host field changes.
     * @param text New hostname or IP address.
     */
    void
    host_line_edit_text_changed(const QString & text );

    /**
     * @brief Slot for port field changes.
     * @param port New port number.
     */
    void
    port_spin_box_value_changed(int port);

    /**
     * @brief Slot for username field changes.
     * @param text New username.
     */
    void
    username_line_edit_text_changed(const QString & text);

    /**
     * @brief Slot for password field changes.
     * @param text New password.
     */
    void
    password_line_edit_text_changed(const QString & text);

    /**
     * @brief Slot for QoS field changes.
     * @param qos New Quality of Service level (0-2).
     */
    void
    qos_spin_box_value_changed(int qos);

    /**
     * @brief Slot for retain checkbox changes.
     * @param state New checkbox state.
     *
     * Signature differs between Qt versions for compatibility.
     */
    void
#if (QT_VERSION < QT_VERSION_CHECK(6, 7, 0) )
    retain_checkbox_state_changed(int state);
#else
    retain_checkbox_check_state_changed(Qt::CheckState state);
#endif

    /**
     * @brief Slot for topic field changes.
     * @param text New topic string.
     */
    void
    topic_line_edit_text_changed(const QString & text);

    /**
     * @brief Slot for message field changes.
     * @param text New message payload.
     */
    void
    message_line_edit_text_changed(const QString & text);

protected:
    /**
     * @brief Handles widget resize events.
     * @param event Resize event.
     *
     * Emits widget_resized_signal() to update node view.
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MQTTPublisherEmbeddedWidget *ui;  ///< UI form instance
    MQTTPublisherParameters mParameters;   ///< Current parameter values
};
