
// Copyright © 2024-2025, NECTEC, all rights reserved
//
// This file is distributed under the terms of the GNU General Public License v3 (GPLv3) only.
// It is intended for use as a GPL plugin/submodule for CVDev.
//
// CVDev itself is distributed under the Apache License, Version 2.0.
//
// This file: GPL v3. See <https://www.gnu.org/licenses/gpl-3.0.html>
// CVDev: Apache 2.0. See <https://www.apache.org/licenses/LICENSE-2.0>


#include "GPLNodePlugin.hpp"
#include "MQTTPublisherModel.hpp"
#include "MQTTSubscriberModel.hpp"

QStringList GPLNodePlugin::registerDataModel( std::shared_ptr< NodeDelegateModelRegistry > model_regs )
{
    QStringList duplicate_model_names;

    registerModel< MQTTPublisherModel >( model_regs, duplicate_model_names );
    registerModel< MQTTSubscriberModel >( model_regs, duplicate_model_names );

    return duplicate_model_names;
}
