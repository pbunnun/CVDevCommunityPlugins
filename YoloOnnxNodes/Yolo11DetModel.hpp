/**
 * @file Yolo11DetModel.hpp
 * @brief YOLOv11 object detection node with threaded ONNX inference.
 *
 * Implements YOLOv11 object detection using ONNX Runtime in a dedicated thread,
 * with improved accuracy and performance over YOLOv8.
 *
 * **Key Features:**
 * - YOLOv11 ONNX model inference (latest architecture)
 * - Enhanced detection accuracy vs YOLOv8
 * - Threaded processing with bounded queue
 * - Configurable confidence and IOU thresholds
 * - Automatic bounding box drawing
 * - GPU acceleration support
 * - Class label loading from file
 *
 * **YOLOv11 Improvements:**
 * - Better small object detection
 * - Improved feature extraction
 * - Enhanced multi-scale detection
 * - Optimized inference speed
 *
 * **Architecture:**
 * @code
 * Input Image → Bounded Queue → Detection Thread → YOLO11 Detector
 *                                      ↓
 *                              Draw Bounding Boxes
 *                                      ↓
 *                               Output Image + Sync
 * @endcode
 *
 * **Configuration File Format (YAML/XML):**
 * @code
 * %YAML:1.0
 * Model Filename: "/path/to/yolo11n.onnx"
 * Labels Filename: "/path/to/coco.names"
 * Confidence Threadhold: 0.5
 * IOU Threadhold: 0.45
 * Using GPU: 1
 * @endcode
 *
 * **Typical Usage:**
 * @code
 * // 1. Create node
 * auto detNode = std::make_unique<Yolo11DetModel>();
 * 
 * // 2. Set config file property
 * QString configPath = "/path/to/yolo11_config.yaml";
 * detNode->setModelProperty("config_filename", QVariant(configPath));
 * 
 * // 3. Connect input image
 * auto imageData = std::make_shared<CVImageData>(frame);
 * detNode->setInData(imageData, 0);
 * 
 * // 4. Get detection result
 * auto resultData = detNode->outData(0);  // Image with bounding boxes
 * auto syncData = detNode->outData(1);    // Sync signal
 * @endcode
 *
 * **Version Comparison:**
 * - YOLOv8: Balanced speed/accuracy
 * - YOLOv11: Higher accuracy, similar speed, better small objects
 *
 * @see YOLO11Detector for detection implementation
 * @see Yolo8DetModel for YOLOv8 version
 * @see https://github.com/ultralytics/ultralytics for YOLOv11
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


#pragma once

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QSemaphore>
#include <QtCore/QMutex>

#include "PBNodeDelegateModel.hpp"

#include "CVImageData.hpp"
#include "SyncData.hpp"
#include "tools/BoundedThreadSafeQueue.hpp"

class YOLO11Detector;

using QtNodes::PortType;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

/**
 * @class Yolo11DetThread
 * @brief Worker thread for YOLOv11 object detection inference.
 *
 * Runs YOLOv11 detection in a separate thread, consuming images from a
 * bounded queue and emitting results via Qt signals.
 *
 * **Thread Lifecycle:**
 * @code
 * 1. Thread started → wait for model ready
 * 2. Model loaded → begin dequeuing images
 * 3. For each image:
 *    - Detect objects with YOLOv11
 *    - Draw bounding boxes
 *    - Emit result_ready signal
 * 4. Abort signal → exit thread
 * @endcode
 *
 * **YOLOv11 Features:**
 * - Enhanced feature pyramid network
 * - Improved anchor-free detection
 * - Better handling of occlusion
 *
 * @see YOLO11Detector for detection algorithm
 */
class Yolo11DetThread : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief Constructs YOLOv11 detection thread with image queue.
     *
     * @param parent Optional parent QObject (default: nullptr)
     * @param pCVMatQueue Shared bounded queue for input images
     */
    explicit
    Yolo11DetThread( QObject *parent = nullptr, std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > pCVMatQueue = nullptr );

    /**
     * @brief Destructor - stops thread and cleans up detector.
     */
    ~Yolo11DetThread() override;

    /**
     * @brief Loads YOLOv11 model and configuration from file.
     *
     * @param config Path to configuration file
     * @return bool True if model loaded successfully
     *
     * **Config File Example:**
     * @code
     * %YAML:1.0
     * Model Filename: "yolo11n.onnx"
     * Labels Filename: "coco.names"
     * Confidence Threadhold: 0.5
     * IOU Threadhold: 0.45
     * Using GPU: 1
     * @endcode
     */
    bool
    readConfig( QString & );

Q_SIGNALS:
    /**
     * @brief Emitted when YOLOv11 detection completes.
     *
     * @param image Processed image with bounding boxes drawn
     */
    void
    result_ready( cv::Mat & image );

protected:
    /**
     * @brief Main thread loop - processes images from queue.
     */
    void
    run() override;

private:
    YOLO11Detector * mpDetector {nullptr};
    std::vector<std::string> mvStrClasses;
    std::atomic<bool> mbModelReady {false};
    std::atomic<bool> mbAbort {false};
    std::vector<cv::String> mvStrOutNames;
    double mdConfidenceThreadhold{0};
    double mdIOUThreadhold{0};
    std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > mpCVMatQueue;
};

/**
 * @class Yolo11DetModel
 * @brief Node model for YOLOv11 object detection.
 *
 * Provides a dataflow node for YOLOv11 detection with threaded processing,
 * leveraging the latest YOLO architecture improvements.
 *
 * **Ports:**
 * - Input[0]: CVImageData - Input image for detection
 * - Output[0]: CVImageData - Image with drawn bounding boxes
 * - Output[1]: SyncData - Synchronization signal
 *
 * **Properties:**
 * - config_filename: Path to YAML/XML configuration file
 *
 * **YOLOv11 Advantages:**
 * @code
 * - Better small object detection (birds, distant cars)
 * - Improved crowded scene handling (people in crowds)
 * - Enhanced edge case robustness (partial occlusion)
 * - Similar inference speed to YOLOv8
 * @endcode
 *
 * @see Yolo11DetThread for detection thread
 * @see YOLO11Detector for detection implementation
 */
/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class Yolo11DetModel : public PBNodeDelegateModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs YOLOv11 detection model.
     */
    Yolo11DetModel();

    /**
     * @brief Destructor - cleans up detection thread.
     */
    virtual
    ~Yolo11DetModel() override
    {
        if( mpYolo11DetThread )
            delete mpYolo11DetThread;
    }

    /**
     * @brief Saves node state to JSON.
     *
     * @return QJsonObject containing config_filename
     */
    QJsonObject
    save() const override;

    /**
     * @brief Loads node state from JSON.
     *
     * @param p JSON object with saved state
     */
    void
    load(QJsonObject const &p) override;

    /**
     * @brief Returns port count for given type.
     *
     * @param portType Input or Output
     * @return unsigned int 1 for input, 2 for output
     */
    unsigned int
    nPorts(PortType portType) const override;

    /**
     * @brief Returns data type for specified port.
     *
     * @param portType Input or Output
     * @param portIndex Port index
     * @return NodeDataType Image for ports 0, Sync for output port 1
     */
    NodeDataType
    dataType( PortType portType, PortIndex portIndex ) const override;

    /**
     * @brief Returns output data for specified port.
     *
     * @param port Port index (0=image, 1=sync)
     * @return std::shared_ptr<NodeData> Detection result or sync signal
     */
    std::shared_ptr< NodeData >
    outData( PortIndex port ) override;

    /**
     * @brief Sets input data for processing.
     *
     * @param nodeData Input image data
     * @param portIndex Port index (unused)
     */
    void
    setInData( std::shared_ptr< NodeData > nodeData, PortIndex ) override;

    /**
     * @brief Returns embedded widget (none for this node).
     *
     * @return QWidget* nullptr
     */
    QWidget *
    embeddedWidget() override { return nullptr; }

    /**
     * @brief Sets model property value.
     *
     * @param property Property name ("config_filename")
     * @param value Property value (QString path)
     */
    void
    setModelProperty( QString &, const QVariant & ) override;

    /**
     * @brief Late initialization - loads YOLOv11 model and starts thread.
     */
    void
    late_constructor() override;

    /**
     * @brief Node category for palette.
     */
    static const QString _category;

    /**
     * @brief Node display name.
     */
    static const QString _model_name;

private Q_SLOTS:
    /**
     * @brief Receives YOLOv11 detection results from thread.
     *
     * @param image Processed image with bounding boxes
     */
    void
    received_result( cv::Mat & );

private:
    std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > mpCVMatQueue;
    std::shared_ptr< CVImageData > mpCVImageData { nullptr };
    std::shared_ptr< SyncData > mpSyncData;
    Yolo11DetThread * mpYolo11DetThread { nullptr };
    QString msConfig_Filename;

    /**
     * @brief Loads YOLOv11 model from configuration file.
     */
    void load_model();
};
