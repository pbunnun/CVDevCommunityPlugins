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

#include "Yolo8DetModel.hpp"


#include "CVImageData.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

#include "qtvariantproperty_p.h"
#include <QFile>
#include <QMessageBox>
#include "det/YOLO8.hpp"

const QString Yolo8DetModel::_category = QString("Yolo Onnx");

const QString Yolo8DetModel::_model_name = QString( "Yolo8 Detector" );

Yolo8DetThread::Yolo8DetThread(QObject * parent, std::shared_ptr<BoundedThreadSafeQueue<cv::Mat> > pCVMatQueue)
    : QThread(parent)
{
    mpCVMatQueue = pCVMatQueue;
}


Yolo8DetThread::
~Yolo8DetThread()
{
    mbAbort.store(true);
    mpCVMatQueue->set_finished();
    if( mpDetector )
        delete mpDetector;
    wait();
}


void
Yolo8DetThread::
run()
{
    cv::Mat frame;
    while( !mbAbort.load() )
    {
        if( !mbModelReady.load() )
            continue;
        if( mpCVMatQueue->dequeue( frame ) )
        {
            std::vector<Detection> results = mpDetector->detect( frame, mdConfidenceThreadhold, mdIOUThreadhold );
            mpDetector->drawBoundingBoxMask(frame, results);
            Q_EMIT result_ready( frame );
        }
    }
}


bool
Yolo8DetThread::
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
            if( mpDetector )
                delete mpDetector;
            mpDetector = new YOLO8Detector(model_filename, labels_filename, bGPU);
            mbModelReady.store(true);
            qDebug() << "Model Ready!";
        }
    }  catch ( cv::Exception & e ) {
        mbModelReady.store(false);
    }
    return mbModelReady.load();
}

Yolo8DetModel::
Yolo8DetModel()
    : PBNodeDelegateModel( _model_name )
{
    mpCVMatQueue = std::make_shared< BoundedThreadSafeQueue< cv::Mat > >(10);
    mpCVImageData = std::make_shared< CVImageData >( cv::Mat() );
    mpSyncData = std::make_shared< SyncData >(true);

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
Yolo8DetModel::
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
Yolo8DetModel::
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
Yolo8DetModel::
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
Yolo8DetModel::
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
Yolo8DetModel::
save() const
{
    QJsonObject modelJson = PBNodeDelegateModel::save();
    QJsonObject cParams;
    cParams["config_filename"] = msConfig_Filename;
    modelJson["cParams"] = cParams;
    return modelJson;
}


void
Yolo8DetModel::
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
Yolo8DetModel::
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
Yolo8DetModel::
late_constructor()
{
    if( !mpYolo8DetThread )
    {
        mpYolo8DetThread = new Yolo8DetThread(this, mpCVMatQueue);
        connect( mpYolo8DetThread, &Yolo8DetThread::result_ready, this, &Yolo8DetModel::received_result );
        load_model();
        mpYolo8DetThread->start();
    }
}


void
Yolo8DetModel::
received_result( cv::Mat & result )
{
    mpCVImageData->set_image( result );
    mpSyncData->data() = true;

    updateAllOutputPorts();
}

void
Yolo8DetModel::
load_model()
{
    if( msConfig_Filename.isEmpty() )
        return;
    if( QFile::exists( msConfig_Filename) )
    {
        mpYolo8DetThread->readConfig( msConfig_Filename );
    }
    else
    {
        QMessageBox err;
        err.setWindowTitle("Yolo Error!");
        err.setText(caption() + " Config Files Error!");
        QString sInformativeText = "Cannot load the following files ... \n";
        sInformativeText += msConfig_Filename + " is missing or malformed!\n";
        err.setInformativeText(sInformativeText);
        err.exec();
    }
}


