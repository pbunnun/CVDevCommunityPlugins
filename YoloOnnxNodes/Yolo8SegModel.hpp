/**
 * @file Yolo8SegModel.hpp
 * @brief YOLOv8 instance segmentation node with threaded ONNX inference.
 *
 * Implements YOLOv8 instance segmentation using ONNX Runtime in a dedicated thread,
 * providing pixel-level object masks with configurable confidence/IOU thresholds.
 *
 * **Key Features:**
 * - YOLOv8 segmentation ONNX model inference
 * - Instance segmentation with pixel masks
 * - Threaded processing with bounded queue
 * - Configurable confidence and IOU thresholds
 * - Automatic mask and bounding box visualization
 * - GPU acceleration support
 * - Class label loading from file
 *
 * **Architecture:**
 * @code
 * Input Image → Bounded Queue → Segmentation Thread → YOLOv8Seg Detector
 *                                         ↓
 *                              Draw Masks + Bounding Boxes
 *                                         ↓
 *                                Output Image + Sync
 * @endcode
 *
 * **Configuration File Format (YAML/XML):**
 * @code
 * %YAML:1.0
 * Model Filename: "/path/to/yolov8n-seg.onnx"
 * Labels Filename: "/path/to/coco.names"
 * Confidence Threadhold: 0.5
 * IOU Threadhold: 0.45
 * Using GPU: 1
 * @endcode
 *
 * **Typical Usage:**
 * @code
 * // 1. Create node
 * auto segNode = std::make_unique<Yolo8SegModel>();
 * 
 * // 2. Set config file property
 * QString configPath = "/path/to/yolo8seg_config.yaml";
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
 * **Segmentation vs Detection:**
 * - Detection (Yolo8DetModel): Bounding boxes only
 * - Segmentation (Yolo8SegModel): Pixel-accurate masks + boxes
 *
 * @see YOLOv8SegDetector for segmentation implementation
 * @see Yolo8DetModel for detection-only version
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

class YOLOv8SegDetector;

using QtNodes::PortType;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeValidationState;

/**
 * @class Yolo8SegThread
 * @brief Worker thread for YOLOv8 instance segmentation inference.
 *
 * Runs YOLOv8 segmentation in a separate thread, consuming images from a
 * bounded queue and emitting segmented results via Qt signals.
 *
 * **Thread Lifecycle:**
 * @code
 * 1. Thread started → wait for model ready
 * 2. Model loaded → begin dequeuing images
 * 3. For each image:
 *    - Detect objects and generate masks
 *    - Draw segmentation masks
 *    - Draw bounding boxes
 *    - Emit result_ready signal
 * 4. Abort signal → exit thread
 * @endcode
 *
 * **Segmentation Output:**
 * - Colored masks overlaid on image
 * - Bounding boxes around segmented objects
 * - Class labels and confidence scores
 *
 * @see YOLOv8SegDetector for segmentation algorithm
 */
class Yolo8SegThread : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief Constructs segmentation thread with image queue.
     *
     * @param parent Optional parent QObject (default: nullptr)
     * @param pCVMatQueue Shared bounded queue for input images
     *
     * **Example:**
     * @code
     * auto queue = std::make_shared<BoundedThreadSafeQueue<cv::Mat>>(10);
     * auto thread = new Yolo8SegThread(this, queue);
     * @endcode
     */
    explicit
    Yolo8SegThread( QObject *parent = nullptr, std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > pCVMatQueue = nullptr );

    /**
     * @brief Destructor - stops thread and cleans up segmentator.
     *
     * Sets abort flag, marks queue finished, deletes detector, and waits for thread exit.
     */
    ~Yolo8SegThread() override;

    /**
     * @brief Loads YOLOv8 segmentation model and configuration from file.
     *
     * Parses YAML/XML config to load model, labels, thresholds, and GPU settings.
     *
     * @param config Path to configuration file
     * @return bool True if model loaded successfully
     *
     * **Config File Example:**
     * @code
     * %YAML:1.0
     * Model Filename: "yolov8n-seg.onnx"
     * Labels Filename: "coco.names"
     * Confidence Threadhold: 0.5
     * IOU Threadhold: 0.45
     * Using GPU: 1
     * @endcode
     *
     * **Usage:**
     * @code
     * QString configPath = "/models/yolo8seg_config.yaml";
     * if (thread->readConfig(configPath)) {
     *     thread->start();  // Begin segmentation
     * }
     * @endcode
     */
    bool
    readConfig( QString & );

Q_SIGNALS:
    /**
     * @brief Emitted when segmentation completes on an image.
     *
     * @param image Processed image with segmentation masks and boxes drawn
     *
     * **Connection Example:**
     * @code
     * connect(thread, &Yolo8SegThread::result_ready,
     *         this, &Yolo8SegModel::received_result);
     * @endcode
     */
    void
    result_ready( cv::Mat & image );

protected:
    /**
     * @brief Main thread loop - processes images from queue.
     *
     * Continuously dequeues images, runs segmentation, draws masks/boxes, and emits results.
     *
     * **Processing Loop:**
     * @code
     * while (!abort) {
     *     if (model_ready && dequeue(image)) {
     *         segmentations = detector->segment(image, conf, iou);
     *         detector->drawMasks(image, segmentations);
     *         detector->drawBoundingBoxMask(image, segmentations);
     *         emit result_ready(image);
     *     }
     * }
     * @endcode
     */
    void
    run() override;

private:
    /**
     * @brief YOLOv8 segmentation detector instance.
     */
    YOLOv8SegDetector * mpSegmentator {nullptr};
    
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
 * @class Yolo8SegModel
 * @brief Node model for YOLOv8 instance segmentation.
 *
 * Provides a dataflow node for YOLOv8 segmentation with threaded processing,
 * configurable parameters, and synchronized output.
 *
 * **Ports:**
 * - Input[0]: CVImageData - Input image for segmentation
 * - Output[0]: CVImageData - Image with drawn segmentation masks
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
 * 4. Segmentation thread processes queue
 * 5. Results emitted via signal
 * 6. Output ports updated
 * @endcode
 *
 * **Example Graph:**
 * @code
 * [Image Source] → [Yolo8SegModel] → [Display]
 *                        ↓
 *                   [Mask Overlay]
 * @endcode
 *
 * **Segmentation Output Example:**
 * - Person: Red mask overlay
 * - Car: Blue mask overlay
 * - Dog: Green mask overlay
 * Each with bounding box and label
 *
 * @see Yolo8SegThread for segmentation thread
 * @see YOLOv8SegDetector for segmentation implementation
 */
/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class Yolo8SegModel : public PBNodeDelegateModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs YOLOv8 segmentation model.
     *
     * Initializes bounded queue, image data, and sync data.
     */
    Yolo8SegModel();

    /**
     * @brief Destructor - cleans up segmentation thread.
     */
    virtual
    ~Yolo8SegModel() override
    {
        if( mpYolo8SegThread )
            delete mpYolo8SegThread;
    }

    /**
     * @brief Saves node state to JSON.
     *
     * @return QJsonObject containing config_filename
     *
     * **Saved Data:**
     * @code
     * {
     *   "config_filename": "/path/to/yolo8seg_config.yaml"
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
     * @return std::shared_ptr<NodeData> Segmentation result or sync signal
     */
    std::shared_ptr< NodeData >
    outData( PortIndex port ) override;

    /**
     * @brief Sets input data for processing.
     *
     * Enqueues input image to segmentation thread's queue.
     *
     * @param nodeData Input image data
     * @param portIndex Port index (unused, only one input)
     *
     * **Example:**
     * @code
     * auto imageData = std::make_shared<CVImageData>(frame);
     * model->setInData(imageData, 0);
     * // Image queued for segmentation
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
     * QString configPath = "/models/yolo8seg_config.yaml";
     * model->setModelProperty("config_filename", QVariant(configPath));
     * @endcode
     */
    void
    setModelProperty( QString &, const QVariant & ) override;

    /**
     * @brief Late initialization - loads model and starts thread.
     *
     * Called after node construction to load YOLO segmentation model from config file.
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
     * @brief Receives segmentation results from thread.
     *
     * Updates output image data and triggers data propagation.
     *
     * @param image Processed image with segmentation masks
     */
    void
    received_result( cv::Mat & );

private:
    /**
     * @brief Bounded queue for input images (capacity: 10).
     */
    std::shared_ptr< BoundedThreadSafeQueue< cv::Mat > > mpCVMatQueue;
    
    /**
     * @brief Output image data with segmentation masks.
     */
    std::shared_ptr< CVImageData > mpCVImageData { nullptr };
    
    /**
     * @brief Synchronization signal for output.
     */
    std::shared_ptr<SyncData> mpSyncData;

    /**
     * @brief Segmentation worker thread.
     */
    Yolo8SegThread * mpYolo8SegThread { nullptr };

    /**
     * @brief Path to configuration file.
     */
    QString msConfig_Filename;

    /**
     * @brief Loads YOLO segmentation model from configuration file.
     *
     * Creates thread, reads config, and starts processing.
     */
    void load_model();
};
