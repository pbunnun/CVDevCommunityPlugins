
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
 * @file MQTTSubscriberEmbeddedWidget.hpp
 * @brief Embedded widget for MQTT subscriber configuration and control.
 *
 * This file provides a UI widget for configuring MQTT broker connection
 * and subscribing to topics. It offers controls for connection management,
 * QoS settings, and topic filter configuration.
 *
 * **Key Features:**
 * - Broker connection configuration (host, port, credentials)
 * - Quality of Service (QoS) level selection
 * - Topic filter with wildcard support
 * - Connect/Disconnect button
 * - Connection status indication
 *
 * **Topic Wildcards:**
 * - **+** Single-level wildcard (e.g., "sensors/+/temperature")
 * - **#** Multi-level wildcard (e.g., "sensors/#" matches all descendants)
 *
 * @see MQTTSubscriberModel
 * @see QMqttClient
 */

#pragma once

#include <QWidget>
#include <QSpinBox>
#include <QtGlobal>

namespace Ui {
class MQTTSubscriberEmbeddedWidget;
}

/**
 * @struct MQTTSubscriberParameters
 * @brief MQTT subscriber configuration parameters.
 *
 * Encapsulates all settings needed for MQTT broker connection and subscription.
 *
 * **Parameter Details:**
 * - **msHost:** Broker hostname or IP
 * - **miPort:** Broker port (1883=plain, 8883=TLS)
 * - **msUsername:** Authentication username (optional)
 * - **msPassword:** Authentication password (optional)
 * - **miQoS:** Maximum Quality of Service for subscriptions (0, 1, or 2)
 * - **msTopic:** Topic filter to subscribe to (supports wildcards)
 *
 * **Topic Filter Examples:**
 * @code
 * "sensors/temperature"        // Exact topic
 * "sensors/+/temperature"      // Any sensor ID
 * "building/floor1/#"          // All topics under floor1
 * "#"                          // All topics (use cautiously)
 * @endcode
 */
typedef struct MQTTSubscriberParameters{
    QString msHost;              ///< Broker hostname or IP address
    int miPort{1883};            ///< Broker port
    QString msUsername;          ///< Authentication username (optional)
    QString msPassword;          ///< Authentication password (optional)
    int miQoS{0};                ///< Maximum QoS for subscription (0, 1, or 2)
    QString msTopic;             ///< Topic filter (supports + and # wildcards)
} MQTTSubscriberParameters;

/**
 * @class MQTTSubscriberEmbeddedWidget
 * @brief Widget for MQTT subscriber configuration and control.
 *
 * This widget provides an interface for configuring MQTT broker connections
 * and subscribing to topics. It displays connection status and allows
 * runtime configuration changes.
 *
 * **Widget Components:**
 * - **Host:** Text field for broker address
 * - **Port:** Spin box for port number
 * - **Username:** Text field for authentication
 * - **Password:** Secure text field
 * - **QoS:** Spin box (0-2) for subscription QoS
 * - **Topic:** Text field for topic filter (with wildcard support)
 * - **Connect Button:** Toggle connection to broker
 *
 * **Connection States:**
 * - **Disconnected:** Gray button, "Connect" label
 * - **Connecting:** Orange button, "Connecting..." label
 * - **Connected:** Green button, "Disconnect" label
 * - **Subscribed:** Green button with checkmark icon
 * - **Error:** Red button, "Error - Connect" label
 *
 * @see MQTTSubscriberModel
 */
class MQTTSubscriberEmbeddedWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an MQTTSubscriberEmbeddedWidget.
     * @param parent Parent widget.
     */
    explicit MQTTSubscriberEmbeddedWidget( QWidget *parent = nullptr );
    
    /**
     * @brief Destructor.
     */
    ~MQTTSubscriberEmbeddedWidget();

    /**
     * @brief Sets widget parameters from model.
     * @param params Subscriber parameters to display.
     */
    void
    set_params(const MQTTSubscriberParameters & params );

    /**
     * @brief Gets current widget parameters.
     * @param params Output parameter to receive current settings.
     */
    void
    get_params( MQTTSubscriberParameters & params );

    /**
     * @brief Updates connection state visual indication.
     * @param connected true if connected and subscribed.
     */
    void
    set_mqtt_connection_state( bool connected );

Q_SIGNALS:
    /**
     * @brief Signal emitted when Connect button is clicked.
     * @param button Button identifier (0=Connect).
     */
    void
    button_clicked_signal( int button );

    /**
     * @brief Signal emitted when any parameter changes.
     * @param params Updated parameters.
     */
    void
    params_changed_signal( const MQTTSubscriberParameters & params );

    /**
     * @brief Signal emitted when widget is resized.
     */
    void
    widget_resized_signal();

public Q_SLOTS:
    /**
     * @brief Slot for Connect/Disconnect button clicks.
     * @param checked Button toggle state.
     */
    void
    connect_button_clicked(bool checked);

    /**
     * @brief Slot for host field changes.
     * @param text New hostname or IP.
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
     * @param qos New maximum QoS level.
     */
    void
    qos_spin_box_value_changed(int qos);

    /**
     * @brief Slot for topic filter field changes.
     * @param text New topic filter (may include wildcards).
     */
    void
    topic_line_edit_text_changed(const QString & text);

protected:
    /**
     * @brief Handles widget resize events.
     * @param event Resize event.
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MQTTSubscriberEmbeddedWidget *ui;  ///< UI form instance
    MQTTSubscriberParameters mParameters;   ///< Current parameter values
};

