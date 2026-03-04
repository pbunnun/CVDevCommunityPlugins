/**
 * @file YoloOnnxNodePlugin.hpp
 * @brief Plugin registration for YOLO ONNX-based detection and segmentation nodes.
 *
 * Provides plugin interface implementation for registering YOLOv8 and YOLOv11
 * detection and segmentation nodes with the CVDev dataflow system.
 *
 * **Registered Nodes:**
 * - Yolo8DetModel: YOLOv8 object detection
 * - Yolo8SegModel: YOLOv8 instance segmentation
 * - Yolo11DetModel: YOLOv11 object detection
 * - Yolo11SegModel: YOLOv11 instance segmentation
 *
 * **Plugin Architecture:**
 * @code
 * Qt Plugin System
 *       ↓
 * YoloNodePlugin (PluginInterface)
 *       ↓
 * registerDataModel() → NodeDelegateModelRegistry
 *       ↓
 * {Yolo8DetModel, Yolo8SegModel, Yolo11DetModel, Yolo11SegModel}
 * @endcode
 *
 * **Model Versions:**
 * | Model          | Type        | Architecture | Best For                |
 * |----------------|-------------|--------------|-------------------------|
 * | Yolo8DetModel  | Detection   | YOLOv8       | Balanced speed/accuracy |
 * | Yolo8SegModel  | Segmentation| YOLOv8       | Instance masks          |
 * | Yolo11DetModel | Detection   | YOLOv11      | High accuracy           |
 * | Yolo11SegModel | Segmentation| YOLOv11      | Precise masks           |
 *
 * **Plugin Loading:**
 * @code
 * // Automatic loading via Qt plugin system
 * QPluginLoader loader("libYoloOnnxNodes.so");
 * QObject* plugin = loader.instance();
 * 
 * if (auto* yoloPlugin = qobject_cast<YoloNodePlugin*>(plugin)) {
 *     QStringList duplicates = yoloPlugin->registerDataModel(registry);
 * }
 * @endcode
 *
 * **Usage in Dataflow Graphs:**
 * @code
 * // Detection workflow
 * [Camera] → [Yolo11DetModel] → [Display]
 *                  ↓
 *            [Object Tracker]
 * 
 * // Segmentation workflow
 * [Video File] → [Yolo11SegModel] → [Mask Overlay]
 *                      ↓
 *                [Background Removal]
 * @endcode
 *
 * **Configuration Requirements:**
 * Each YOLO node requires a YAML/XML config file with:
 * - Model Filename: Path to .onnx model
 * - Labels Filename: Path to class labels
 * - Confidence Threshold: Minimum detection confidence
 * - IOU Threshold: NMS threshold
 * - Using GPU: Enable GPU acceleration
 *
 * @see PluginInterface for plugin contract
 * @see Yolo8DetModel for YOLOv8 detection
 * @see Yolo8SegModel for YOLOv8 segmentation
 * @see Yolo11DetModel for YOLOv11 detection
 * @see Yolo11SegModel for YOLOv11 segmentation
 */

//Copyright © 2025, NECTEC, all rights reserved

//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at

//    http://www.apache.org/licenses/LICENSE-2.0

//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

#ifndef YOLOONNXNODEPLUGIN_H
#define YOLOONNXNODEPLUGIN_H

#include <PluginInterface.hpp>

#include <QObject>

/**
 * @class YoloNodePlugin
 * @brief Qt plugin for YOLO ONNX-based computer vision nodes.
 *
 * Implements the PluginInterface to register YOLOv8 and YOLOv11 detection
 * and segmentation nodes with the CVDev node registry.
 *
 * **Plugin Metadata:**
 * - Interface ID: "CVDev.PluginInterface"
 * - Metadata File: basicnodes.json
 * - Plugin Type: Computer Vision / Object Detection
 *
 * **Registered Models:**
 * @code
 * 1. Yolo8SegModel  - YOLOv8 instance segmentation
 * 2. Yolo11SegModel - YOLOv11 instance segmentation
 * 3. Yolo8DetModel  - YOLOv8 object detection
 * 4. Yolo11DetModel - YOLOv11 object detection
 * @endcode
 *
 * **Plugin Loading Process:**
 * @code
 * 1. Qt loads plugin from shared library
 * 2. Plugin system calls registerDataModel()
 * 3. Each model registered with registry
 * 4. Models appear in node palette
 * 5. Users can instantiate nodes in graphs
 * @endcode
 *
 * **Model Selection Guide:**
 * @code
 * Choose YOLOv8 when:
 * - Need balanced speed/accuracy
 * - Working with standard objects
 * - Real-time processing priority
 * 
 * Choose YOLOv11 when:
 * - Need highest accuracy
 * - Detecting small/occluded objects
 * - Quality over speed priority
 * 
 * Choose Detection when:
 * - Only need bounding boxes
 * - Faster processing required
 * 
 * Choose Segmentation when:
 * - Need pixel-accurate masks
 * - Instance separation required
 * @endcode
 *
 * **Example Plugin Usage:**
 * @code
 * // In main application
 * auto registry = std::make_shared<NodeDelegateModelRegistry>();
 * 
 * // Load YOLO plugin
 * QPluginLoader loader("plugins/libYoloOnnxNodes.so");
 * auto* plugin = qobject_cast<PluginInterface*>(loader.instance());
 * 
 * if (plugin) {
 *     QStringList duplicates = plugin->registerDataModel(registry);
 *     if (!duplicates.isEmpty()) {
 *         qWarning() << "Duplicate models:" << duplicates;
 *     }
 * }
 * 
 * // Models now available in dataflow editor
 * auto model = registry->create("Yolo11DetModel");
 * @endcode
 *
 * @see PluginInterface for plugin contract
 * @see NodeDelegateModelRegistry for model registration
 */
class YoloNodePlugin : public QObject,
                        public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "CVDev.PluginInterface" FILE "basicnodes.json" )
    Q_INTERFACES( PluginInterface )

public:
    /**
     * @brief Registers YOLO models with the node registry.
     *
     * Registers all YOLO detection and segmentation models for use
     * in the dataflow editor.
     *
     * @param model_regs Shared pointer to node delegate model registry
     * @return QStringList List of duplicate model names (should be empty)
     *
     * **Registration Order:**
     * @code
     * 1. Yolo8SegModel  - YOLOv8 segmentation
     * 2. Yolo11SegModel - YOLOv11 segmentation
     * 3. Yolo8DetModel  - YOLOv8 detection
     * 4. Yolo11DetModel - YOLOv11 detection
     * @endcode
     *
     * **Example:**
     * @code
     * auto registry = std::make_shared<NodeDelegateModelRegistry>();
     * YoloNodePlugin plugin;
     * 
     * QStringList duplicates = plugin.registerDataModel(registry);
     * if (duplicates.isEmpty()) {
     *     qDebug() << "All YOLO models registered successfully";
     * }
     * 
     * // Create YOLO detection node
     * auto detModel = registry->create("Yolo11DetModel");
     * @endcode
     *
     * @note Returns empty list if no duplicates found
     * @see registerModel() for individual model registration
     */
    QStringList registerDataModel( std::shared_ptr< NodeDelegateModelRegistry > model_regs );
};

#endif
