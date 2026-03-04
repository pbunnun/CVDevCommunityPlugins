/**
 * @file Yolo11SegModel.hpp
 * @brief YOLOv11 instance segmentation node with threaded ONNX inference.
 *
 * Implements YOLOv11 instance segmentation using ONNX Runtime in a dedicated thread,
 * providing enhanced pixel-level object masks with improved accuracy over YOLOv8.
 *
 * **Key Features:**
 * - YOLOv11 segmentation ONNX model inference
 * - Enhanced instance segmentation accuracy
 * - Improved mask quality and boundary precision
 * - Threaded processing with bounded queue
 * - Configurable confidence and IOU thresholds
 * - Automatic mask and bounding box visualization
 * - GPU acceleration support
 * - Class label loading from file
 *
 * **YOLOv11 Segmentation Improvements:**
 * - Better mask boundaries (smoother edges)
 * - Improved small object segmentation
 * - Enhanced overlapping object handling
 * - More accurate instance separation
 *
 * **Architecture:**
 * @code
 * Input Image → Bounded Queue → Segmentation Thread → YOLO11Seg Detector
 *                                         ↓
 *                              Draw Masks + Bounding Boxes
 *                                         ↓
 *                                Output Image + Sync
 * @endcode
 *
 * **Configuration File Format (YAML/XML):**
 * @code
 * %YAML:1.0
 * Model Filename: "/path/to/yolo11n-seg.onnx"
 * Labels Filename: "/path/to/coco.names"
 * Confidence Threadhold: 0.5
 * IOU Threadhold: 0.45
 * Using GPU: 1
 * @endcode
 *
 * **Typical Usage:**
 * @code
 * // 1. Create node
 * auto segNode = std::make_unique<Yolo11SegModel>();
 * 
 * // 2. Set config file property
 * QString configPath = "/path/to/yolo11seg_config.yaml";
 * segNode->setModelProperty("config_filename", QVariant(configPath));
 * 
 * // 3. Connect input image
 * auto imageData = std::make_shared<CVImageData>(frame);
 * segNode->setInData(imageData, 0);
 * 
 * // 4. Get segmentation result
 * auto resultData = segNode->outData(0);  // Image with masks
 * auto syncData = segNode->outData(1);    // Sync signal
 * @endcode
 *
 * **Version Comparison:**
 * - YOLOv8-Seg: Good segmentation quality
 * - YOLOv11-Seg: Better boundaries, improved small objects, enhanced accuracy
 *
 * @see YOLOv11SegDetector for segmentation implementation
 * @see Yolo8SegModel for YOLOv8 segmentation version
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

class YOLOv11SegDetector;

using QtNodes::PortType;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

/**
 * @class Yolo11SegThread
 * @brief Worker thread for YOLOv11 instance segmentation inference.
 *
 * Runs YOLOv11 segmentation in a separate thread, consuming images from a
 * bounded queue and emitting high-quality segmented results via Qt signals.
 *
 * **Thread Lifecycle:**
 * @code
 * 1. Thread started → wait for model ready
 * 2. Model loaded → begin dequeuing images
 * 3. For each image:
 *    - Detect objects and generate masks with YOLOv11
 *    - Draw refined segmentation masks
 *    - Draw bounding boxes
 *    - Emit result_ready signal
 * 4. Abort signal → exit thread
 * @endcode
 *
 * **YOLOv11 Segmentation Features:**
 * - Enhanced mask decoder
 * - Improved prototype generation
 * - Better edge preservation
 * - Superior instance separation
 *
 * @see YOLOv11SegDetector for segmentation algorithm
 */
class Yolo11SegThread : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief Constructs YOLOv11 segmentation thread with image queue.
     *
     * @param parent Optional parent QObject (default: nullptr)
     * @param pCVMatQueue Shared bounded queue for input images
     */
    explicit
    Yolo11SegThread( QObject *parent = nullptr, std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > pCVMatQueue = nullptr );

    /**
     * @brief Destructor - stops thread and cleans up segmentator.
     */
    ~Yolo11SegThread() override;

    /**
     * @brief Loads YOLOv11 segmentation model and configuration from file.
     *
     * @param config Path to configuration file
     * @return bool True if model loaded successfully
     *
     * **Config File Example:**
     * @code
     * %YAML:1.0
     * Model Filename: "yolo11n-seg.onnx"
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
     * @brief Emitted when YOLOv11 segmentation completes.
     *
     * @param image Processed image with segmentation masks and boxes drawn
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
    YOLOv11SegDetector * mpSegmentator {nullptr};
    std::vector<std::string> mvStrClasses;
    std::atomic<bool> mbModelReady {false};
    std::atomic<bool> mbAbort {false};
    std::vector<cv::String> mvStrOutNames;
    double mdConfidenceThreadhold{0};
    double mdIOUThreadhold{0};
    std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > mpCVMatQueue;
};

/**
 * @class Yolo11SegModel
 * @brief Node model for YOLOv11 instance segmentation.
 *
 * Provides a dataflow node for YOLOv11 segmentation with threaded processing,
 * leveraging the latest segmentation architecture improvements.
 *
 * **Ports:**
 * - Input[0]: CVImageData - Input image for segmentation
 * - Output[0]: CVImageData - Image with drawn segmentation masks
 * - Output[1]: SyncData - Synchronization signal
 *
 * **Properties:**
 * - config_filename: Path to YAML/XML configuration file
 *
 * **YOLOv11 Segmentation Advantages:**
 * @code
 * - Sharper mask boundaries (refined edges)
 * - Better small object masks (detailed features)
 * - Improved overlapping handling (clean separation)
 * - Enhanced edge preservation (accurate contours)
 * @endcode
 *
 * **Example Applications:**
 * @code
 * - Medical imaging (organ segmentation)
 * - Autonomous driving (lane/object segmentation)
 * - Industrial inspection (defect segmentation)
 * - Video editing (background removal)
 * @endcode
 *
 * @see Yolo11SegThread for segmentation thread
 * @see YOLOv11SegDetector for segmentation implementation
 */
/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class Yolo11SegModel : public PBNodeDelegateModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs YOLOv11 segmentation model.
     */
    Yolo11SegModel();

    /**
     * @brief Destructor - cleans up segmentation thread.
     */
    virtual
    ~Yolo11SegModel() override
    {
        if( mpYolo11SegThread )
            delete mpYolo11SegThread;
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
     * @return std::shared_ptr<NodeData> Segmentation result or sync signal
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
     * @brief Late initialization - loads YOLOv11 segmentation model and starts thread.
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
     * @brief Receives YOLOv11 segmentation results from thread.
     *
     * @param image Processed image with segmentation masks
     */
    void
    received_result( cv::Mat & );

private:
    std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > mpCVMatQueue;
    std::shared_ptr< CVImageData > mpCVImageData { nullptr };
    std::shared_ptr<SyncData> mpSyncData;
    Yolo11SegThread * mpYolo11SegThread { nullptr };
    QString msConfig_Filename;

    /**
     * @brief Loads YOLOv11 segmentation model from configuration file.
     */
    void load_model();
};
