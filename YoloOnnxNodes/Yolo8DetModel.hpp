/**
 * @file Yolo8DetModel.hpp
 * @brief YOLOv8 object detection node with threaded ONNX inference.
 *
 * Implements YOLOv8 object detection using ONNX Runtime in a dedicated thread,
 * with configurable confidence/IOU thresholds and bounding box visualization.
 *
 * **Key Features:**
 * - YOLOv8 ONNX model inference
 * - Threaded processing with bounded queue
 * - Configurable confidence and IOU thresholds
 * - Automatic bounding box drawing
 * - GPU acceleration support
 * - Class label loading from file
 *
 * **Architecture:**
 * @code
 * Input Image → Bounded Queue → Detection Thread → YOLOv8 Detector
 *                                      ↓
 *                              Draw Bounding Boxes
 *                                      ↓
 *                               Output Image + Sync
 * @endcode
 *
 * **Configuration File Format (YAML/XML):**
 * @code
 * %YAML:1.0
 * Model Filename: "/path/to/yolov8n.onnx"
 * Labels Filename: "/path/to/coco.names"
 * Confidence Threadhold: 0.5
 * IOU Threadhold: 0.45
 * Using GPU: 1
 * @endcode
 *
 * **Typical Usage:**
 * @code
 * // 1. Create node
 * auto detNode = std::make_unique<Yolo8DetModel>();
 * 
 * // 2. Set config file property
 * QString configPath = "/path/to/yolo8_config.yaml";
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
 * **Thread Safety:**
 * - Uses BoundedThreadSafeQueue for image passing
 * - Atomic flags for model state
 * - Qt signals for result delivery
 *
 * @see YOLO8Detector for detection implementation
 * @see BoundedThreadSafeQueue for thread-safe queue
 * @see https://github.com/ultralytics/ultralytics for YOLOv8
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

class YOLO8Detector;

using QtNodes::PortType;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

/**
 * @class Yolo8DetThread
 * @brief Worker thread for YOLOv8 object detection inference.
 *
 * Runs YOLOv8 detection in a separate thread, consuming images from a
 * bounded queue and emitting results via Qt signals.
 *
 * **Thread Lifecycle:**
 * @code
 * 1. Thread started → wait for model ready
 * 2. Model loaded → begin dequeuing images
 * 3. For each image:
 *    - Detect objects
 *    - Draw bounding boxes
 *    - Emit result_ready signal
 * 4. Abort signal → exit thread
 * @endcode
 *
 * **Configuration Loading:**
 * @code
 * QString configFile = "yolo8_config.yaml";
 * if (thread->readConfig(configFile)) {
 *     // Model loaded successfully
 *     thread->start();
 * } else {
 *     // Failed to load model
 * }
 * @endcode
 *
 * @see YOLO8Detector for detection algorithm
 */
class Yolo8DetThread : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief Constructs detection thread with image queue.
     *
     * @param parent Optional parent QObject (default: nullptr)
     * @param pCVMatQueue Shared bounded queue for input images
     *
     * **Example:**
     * @code
     * auto queue = std::make_shared<BoundedThreadSafeQueue<cv::Mat>>(10);
     * auto thread = new Yolo8DetThread(this, queue);
     * @endcode
     */
    explicit
    Yolo8DetThread( QObject *parent = nullptr, std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > pCVMatQueue = nullptr );

    /**
     * @brief Destructor - stops thread and cleans up detector.
     *
     * Sets abort flag, marks queue finished, deletes detector, and waits for thread exit.
     */
    ~Yolo8DetThread() override;

    /**
     * @brief Loads YOLOv8 model and configuration from file.
     *
     * Parses YAML/XML config to load model, labels, thresholds, and GPU settings.
     *
     * @param config Path to configuration file
     * @return bool True if model loaded successfully
     *
     * **Config File Example:**
     * @code
     * %YAML:1.0
     * Model Filename: "yolov8n.onnx"
     * Labels Filename: "coco.names"
     * Confidence Threadhold: 0.5
     * IOU Threadhold: 0.45
     * Using GPU: 1
     * @endcode
     *
     * **Usage:**
     * @code
     * QString configPath = "/models/yolo8_config.yaml";
     * if (thread->readConfig(configPath)) {
     *     thread->start();  // Begin detection
     * }
     * @endcode
     */
    bool
    readConfig( QString & );

Q_SIGNALS:
    /**
     * @brief Emitted when detection completes on an image.
     *
     * @param image Processed image with bounding boxes drawn
     *
     * **Connection Example:**
     * @code
     * connect(thread, &Yolo8DetThread::result_ready,
     *         this, &Yolo8DetModel::received_result);
     * @endcode
     */
    void
    result_ready( cv::Mat & image );

protected:
    /**
     * @brief Main thread loop - processes images from queue.
     *
     * Continuously dequeues images, runs detection, draws boxes, and emits results.
     *
     * **Processing Loop:**
     * @code
     * while (!abort) {
     *     if (model_ready && dequeue(image)) {
     *         detections = detector->detect(image, conf, iou);
     *         detector->drawBoundingBoxMask(image, detections);
     *         emit result_ready(image);
     *     }
     * }
     * @endcode
     */
    void
    run() override;

private:
    /**
     * @brief YOLOv8 detector instance.
     */
    YOLO8Detector * mpDetector {nullptr};
    
    /**
     * @brief Class labels (e.g., "person", "car", "dog").
     */
    std::vector<std::string> mvStrClasses;
    
    /**
     * @brief Flag indicating model is loaded and ready.
     */
    std::atomic<bool> mbModelReady {false};
    
    /**
     * @brief Flag to request thread termination.
     */
    std::atomic<bool> mbAbort {false};
    
    /**
     * @brief Output layer names from ONNX model.
     */
    std::vector<cv::String> mvStrOutNames;

    /**
     * @brief Minimum confidence score for detections (0.0-1.0).
     */
    double mdConfidenceThreadhold{0};
    
    /**
     * @brief IoU threshold for non-maximum suppression (0.0-1.0).
     */
    double mdIOUThreadhold{0};
    
    /**
     * @brief Bounded queue for input images.
     */
    std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > mpCVMatQueue;
};

/**
 * @class Yolo8DetModel
 * @brief Node model for YOLOv8 object detection.
 *
 * Provides a dataflow node for YOLOv8 detection with threaded processing,
 * configurable parameters, and synchronized output.
 *
 * **Ports:**
 * - Input[0]: CVImageData - Input image for detection
 * - Output[0]: CVImageData - Image with drawn bounding boxes
 * - Output[1]: SyncData - Synchronization signal
 *
 * **Properties:**
 * - config_filename: Path to YAML/XML configuration file
 *
 * **Workflow:**
 * @code
 * 1. Set config_filename property
 * 2. late_constructor() loads model
 * 3. Input images → bounded queue
 * 4. Detection thread processes queue
 * 5. Results emitted via signal
 * 6. Output ports updated
 * @endcode
 *
 * **Example Graph:**
 * @code
 * [Image Source] → [Yolo8DetModel] → [Display]
 *                        ↓
 *                   [Sync Consumer]
 * @endcode
 *
 * @see Yolo8DetThread for detection thread
 * @see YOLO8Detector for detection implementation
 */
/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class Yolo8DetModel : public PBNodeDelegateModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs YOLOv8 detection model.
     *
     * Initializes bounded queue, image data, and sync data.
     */
    Yolo8DetModel();

    /**
     * @brief Destructor - cleans up detection thread.
     */
    virtual
    ~Yolo8DetModel() override
    {
        if( mpYolo8DetThread )
            delete mpYolo8DetThread;
    }

    /**
     * @brief Saves node state to JSON.
     *
     * @return QJsonObject containing config_filename
     *
     * **Saved Data:**
     * @code
     * {
     *   "config_filename": "/path/to/yolo8_config.yaml"
     * }
     * @endcode
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
     * Enqueues input image to detection thread's queue.
     *
     * @param nodeData Input image data
     * @param portIndex Port index (unused, only one input)
     *
     * **Example:**
     * @code
     * auto imageData = std::make_shared<CVImageData>(frame);
     * model->setInData(imageData, 0);
     * // Image queued for detection
     * @endcode
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
     *
     * **Example:**
     * @code
     * QString configPath = "/models/yolo8_config.yaml";
     * model->setModelProperty("config_filename", QVariant(configPath));
     * @endcode
     */
    void
    setModelProperty( QString &, const QVariant & ) override;

    /**
     * @brief Late initialization - loads model and starts thread.
     *
     * Called after node construction to load YOLO model from config file.
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
     * @brief Receives detection results from thread.
     *
     * Updates output image data and triggers data propagation.
     *
     * @param image Processed image with bounding boxes
     */
    void
    received_result( cv::Mat & );

private:
    /**
     * @brief Bounded queue for input images (capacity: 10).
     */
    std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > mpCVMatQueue;
    
    /**
     * @brief Output image data with detections.
     */
    std::shared_ptr< CVImageData > mpCVImageData { nullptr };
    
    /**
     * @brief Synchronization signal for output.
     */
    std::shared_ptr< SyncData > mpSyncData;

    /**
     * @brief Detection worker thread.
     */
    Yolo8DetThread * mpYolo8DetThread { nullptr };

    /**
     * @brief Path to configuration file.
     */
    QString msConfig_Filename;

    /**
     * @brief Loads YOLO model from configuration file.
     *
     * Creates thread, reads config, and starts processing.
     */
    void load_model();
};
