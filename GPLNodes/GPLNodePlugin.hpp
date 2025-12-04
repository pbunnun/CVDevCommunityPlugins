
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
 * @file GPLNodePlugin.hpp
 * @brief Plugin interface for GPL-licensed node models.
 *
 * This file defines the plugin that registers GPL-licensed node models with
 * the CVDev application. GPL nodes are kept separate due to their copyleft
 * licensing requirements, which differ from the Apache 2.0 licensed core.
 *
 * **GPL License Note:**
 * Nodes in this plugin use GPL-licensed libraries (e.g., QtMqtt) and are
 * therefore subject to GPL licensing terms. Applications using these nodes
 * may be required to comply with GPL license requirements.
 *
 * **Registered GPL Models:**
 * - MQTT Publisher (publish messages to MQTT broker)
 * - MQTT Subscriber (receive messages from MQTT broker)
 *
 * **Use Cases:**
 * - IoT device communication
 * - Remote monitoring and control
 * - Event-driven automation
 * - Distributed system integration
 *
 * @see PluginInterface
 * @see MQTTPublisherModel
 * @see MQTTSubscriberModel
 */

#pragma once

#include <PluginInterface.hpp>

#include <QObject>

/**
 * @class GPLNodePlugin
 * @brief Plugin for registering GPL-licensed node models.
 *
 * This plugin implements the Qt plugin interface to register GPL-licensed
 * nodes with the CVDev application. It enables MQTT communication nodes
 * that depend on GPL-licensed Qt MQTT library.
 *
 * **GPL License Implications:**
 * - Uses QtMqtt library (GPL/Commercial dual-licensed)
 * - Applications using these nodes may need GPL compliance
 * - Separate plugin allows optional GPL feature loading
 * - Commercial Qt license removes GPL requirement
 *
 * **Registered Models:**
 * - **MQTTPublisherModel:** Publish data to MQTT broker
 * - **MQTTSubscriberModel:** Subscribe to MQTT topics
 *
 * **MQTT Protocol:**
 * - Lightweight publish/subscribe messaging
 * - Ideal for IoT and M2M communication
 * - Quality of Service (QoS) levels 0, 1, 2
 * - Retained messages for last known state
 * - Last Will and Testament (LWT)
 *
 * @see PluginInterface
 * @see MQTTPublisherModel
 * @see MQTTSubscriberModel
 */
class GPLNodePlugin : public QObject,
                        public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "CVDev.PluginInterface" FILE "basicnodes.json" )
    Q_INTERFACES( PluginInterface )

public:
    /**
     * @brief Registers all GPL node models.
     * @param model_regs Model registry to register nodes with.
     * @return List of registered model names.
     *
     * This method is called during plugin initialization to register
     * MQTT communication nodes.
     */
    QStringList registerDataModel( std::shared_ptr< NodeDelegateModelRegistry > model_regs );
};

