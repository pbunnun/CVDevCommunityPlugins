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

#include "YoloOnnxNodePlugin.hpp"
#include "Yolo8SegModel.hpp"
#include "Yolo11SegModel.hpp"
#include "Yolo8DetModel.hpp"
#include "Yolo11DetModel.hpp"

QStringList YoloNodePlugin::registerDataModel( std::shared_ptr< NodeDelegateModelRegistry > model_regs )
{
    QStringList duplicate_model_names;

    registerModel< Yolo8SegModel >( model_regs, duplicate_model_names );
    registerModel< Yolo11SegModel >( model_regs, duplicate_model_names );
    registerModel< Yolo8DetModel >( model_regs, duplicate_model_names );
    registerModel< Yolo11DetModel >( model_regs, duplicate_model_names );
    return duplicate_model_names;
}
