#pragma once

#include <stdio.h>

#include "core/cc1101.h"
#include "core/utils.h"

template <typename Publish>
void publishMqttRadioDiagnostics(const tmeter_data &data, Publish publish)
{
    char value[24];
    const auto publishInteger = [&](const char *topic, int measurement, bool valid) {
        snprintf(value, sizeof(value), "%d", measurement);
        publish(topic, valid ? value : "None", true);
    };
    publishInteger("rssi_dbm", data.rssi_dbm, data.signal_valid);
    publishInteger("rssi_percentage", calculateMeterdBmToPercentage(data.rssi_dbm), data.signal_valid);
    publishInteger("lqi", data.lqi, data.lqi_valid);
    publishInteger("lqi_percentage", calculateLQIToPercentage(data.lqi), data.lqi_valid);
    snprintf(value, sizeof(value), "%.3f", static_cast<float>(data.freqest) * 1.587f);
    publish("frequency_estimate", data.signal_valid ? value : "None", true);
}
