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

#include "Yolo11SegModel.hpp"


#include "CVImageData.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

#include "qtvariantproperty_p.h"
#include <QFile>
#include <QMessageBox>
#include "seg/YOLO11Seg.hpp"

const QString Yolo11SegModel::_category = QString("Yolo Onnx");

const QString Yolo11SegModel::_model_name = QString( "Yolo11 Segmentation" );

Yolo11SegThread::Yolo11SegThread( QObject * parent, std::shared_ptr<BoundedThreadSafeQueue<cv::Mat> > pCVMatQueue)
    : QThread(parent)
{
    mpCVMatQueue = pCVMatQueue;
}


Yolo11SegThread::
~Yolo11SegThread()
{
    mbAbort.store(true);
    mpCVMatQueue->set_finished();
    if( mpSegmentator )
        delete mpSegmentator;
    wait();
}


void
Yolo11SegThread::
run()
{
    cv::Mat frame;
    while( !mbAbort.load() )
    {
        if( !mbModelReady.load() )
            continue;
        if( mpCVMatQueue->dequeue( frame ) )
        {
            std::vector<Segmentation> results = mpSegmentator->segment( frame, mdConfidenceThreadhold, mdIOUThreadhold );
            mpSegmentator->drawSegmentationsAndBoxes(frame, results);
            //mpSegmentator->drawSegmentations(frame, results);
            Q_EMIT result_ready( frame );
        }
    }
}


bool
Yolo11SegThread::
readConfig( QString & config )
{
    try {
        cv::FileStorage fs(config.toStdString(), cv::FileStorage::READ);

        std::string model_filename, labels_filename;
        bool bGPU;
        fs["Model Filename"] >> model_filename;
        fs["Labels Filename"] >> labels_filename;
        fs["Confidence Threadhold"] >> mdConfidenceThreadhold;
        fs["IOU Threadhold"] >> mdIOUThreadhold;
        fs["Using GPU"] >> bGPU;

        if( QFile::exists( QString::fromStdString(model_filename) ) &&
            QFile::exists( QString::fromStdString(labels_filename) ) )
        {
            if( mpSegmentator )
                delete mpSegmentator;
            mpSegmentator = new YOLOv11SegDetector(model_filename, labels_filename, bGPU);
            mbModelReady.store(true);
            qDebug() << "Model Ready!";
        }
    }  catch ( cv::Exception & e ) {
        mbModelReady.store(false);
    }
    return mbModelReady.load();
}

Yolo11SegModel::
Yolo11SegModel()
    : PBNodeDelegateModel( _model_name )
{
    mpCVMatQueue = std::make_shared< BoundedThreadSafeQueue< cv::Mat > >(10);
    mpCVImageData = std::make_shared< CVImageData >( cv::Mat() );

    mpSyncData = std::make_shared< SyncData >();
    mpSyncData->data() = true;

    FilePathPropertyType filePathPropertyType;
    filePathPropertyType.msFilename = msConfig_Filename;
    filePathPropertyType.msFilter = "*.json";
    filePathPropertyType.msMode = "open";
    QString propId = "config_filename";
    auto propFileName = std::make_shared< TypedProperty<FilePathPropertyType> >("Config Filename", propId, QtVariantPropertyManager::filePathTypeId(), filePathPropertyType);
    mvProperty.push_back( propFileName );
    mMapIdToProperty[ propId ] = propFileName;
}

unsigned int
Yolo11SegModel::
nPorts(PortType portType) const
{
    unsigned int result = 1;

    switch (portType)
    {
    case PortType::In:
        result = 1;
        break;

    case PortType::Out:
        result = 2;
        break;

    default:
        break;
    }

    return result;
}

NodeDataType
Yolo11SegModel::
dataType(PortType, PortIndex portIndex) const
{
    if(portIndex == 0)
    {
        return CVImageData().type();
    }
    else if(portIndex == 1)
    {
        return SyncData().type();
    }
    return NodeDataType();
}

std::shared_ptr<NodeData>
Yolo11SegModel::
outData(PortIndex port)
{
    if( isEnable() )
    {
        if( port == 0 )
        {
            return mpCVImageData;
        }
        else if( port == 1 )
        {
            return mpSyncData;
        }
    }
    return nullptr;
}

void
Yolo11SegModel::
setInData( std::shared_ptr< NodeData > nodeData, PortIndex )
{
    if( !isEnable() )
        return;
    if( nodeData && mpSyncData->data() == true )
    {
        mpSyncData->data() = false;
        Q_EMIT dataUpdated(1);
        auto d = std::dynamic_pointer_cast< CVImageData >( nodeData );
        if( d && !d->data().empty() )
            mpCVMatQueue->enqueue( d->data() );
    }
}


QJsonObject
Yolo11SegModel::
save() const
{
    QJsonObject modelJson = PBNodeDelegateModel::save();
    QJsonObject cParams;
    cParams["config_filename"] = msConfig_Filename;
    modelJson["cParams"] = cParams;
    return modelJson;
}


void
Yolo11SegModel::
load( QJsonObject const &p )
{
    PBNodeDelegateModel::load(p);
    late_constructor();

    QJsonObject paramsObj = p["cParams"].toObject();
    if( !paramsObj.isEmpty() )
    {
        QJsonValue v = paramsObj["config_filename"];
        if( !v.isNull() )
        {
            auto prop = mMapIdToProperty["config_filename"];
            auto typedProp = std::static_pointer_cast< TypedProperty<QString> >( prop );
            typedProp->getData() = v.toString();
            msConfig_Filename = v.toString();
        }

        load_model();
    }
}


void
Yolo11SegModel::
setModelProperty( QString & id, const QVariant & value )
{
    PBNodeDelegateModel::setModelProperty( id, value );
    if( !mMapIdToProperty.contains( id ) )
        return;

    auto prop = mMapIdToProperty[ id ];
    if( id == "config_filename" )
    {
        auto typedProp = std::static_pointer_cast< TypedProperty< QString > >( prop );
        typedProp->getData() = value.toString();
        msConfig_Filename = value.toString();

        load_model();
    }
}


void
Yolo11SegModel::
late_constructor()
{
    if( !mpYolo11SegThread )
    {
        mpYolo11SegThread = new Yolo11SegThread(this, mpCVMatQueue);
        connect( mpYolo11SegThread, &Yolo11SegThread::result_ready, this, &Yolo11SegModel::received_result );
        load_model();
        mpYolo11SegThread->start();
    }
}


void
Yolo11SegModel::
received_result( cv::Mat & result )
{
    mpCVImageData->set_image( result );
    mpSyncData->data() = true;

    updateAllOutputPorts();
}

void
Yolo11SegModel::
load_model()
{
    if( msConfig_Filename.isEmpty() )
        return;
    if( QFile::exists( msConfig_Filename) )
    {
        mpYolo11SegThread->readConfig( msConfig_Filename );
    }
    else
    {
        QMessageBox err;
        err.setWindowTitle("Yolo Error!");
        err.setText(caption() + " Config Files Error!");
        QString sInformativeText = "Cannot load the following files ... \n";
        if( !QFile::exists(msConfig_Filename) )
            sInformativeText += "  - Config File is missing!\n";
        err.setInformativeText(sInformativeText);
        err.exec();
    }
}


