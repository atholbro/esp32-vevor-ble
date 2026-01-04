#pragma once
#include "esphome.h"

static void select_set_index(esphome::select::Select *select, uint8_t index) {
    const auto &options = select->traits.get_options();
    if (index < options.size()) {
        select->publish_state(options[index]);
    } else {
        ESP_LOGW("BLE", "Received out-of-bounds %s index: %d", select->get_name(), index);
    }
}