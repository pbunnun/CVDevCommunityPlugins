# YoloOnnxNodes Plugin

These nodes are built on top of the YOLOs-CPP project ([YOLOs-CPP GitHub](https://github.com/Geekgineer/YOLOs-CPP)).

## Overview

Provides ONNX-based YOLO inference nodes for object detection inside CVDev. Nodes wrap model loading, preprocessing, inference, and postprocessing (NMS, class filtering) and emit detection results as structured data.

## Features

- ONNX model loading (YOLO family)
- Image preprocessing (resize, letterbox, normalization)
- Batch or single-frame inference
- Non-Maximum Suppression configurable via properties
- Output ports for annotated image and detection metadata

## Planned Enhancements

- Support multiple model versions (v5/v7/v8)
- GPU acceleration selection property
- Asynchronous worker integration (PBAsyncDataModel pattern)
- Optional Zenoh publishing of detections

## License

Follows upstream YOLOs-CPP licensing; integration code © 2025 NECTEC.
These nodes are built on top of the YOLOs-CPP project (https://github.com/Geekgineer/YOLOs-CPP). 

