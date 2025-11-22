#include <vector>

#include <esphome/components/network/ip_address.h>

#include "TZSPSerial.h"

namespace esphome {
namespace tzspserial {

void TZSPSerial::setup() {
    constexpr uint32_t stack_depth = 4096; // Not optimized
    constexpr UBaseType_t task_priority = 12; // Not optimized

    int err = uart_set_rx_timeout(static_cast<uart_port_t>(this->idf_uart->get_hw_serial_number()), this->symbol_timeout_);
Use code with caution.

Modify Line 59:
Change the line from:
cpp
uart_get_buffered_data_len(static_cast<uart::IDFUARTComponent*>(this->parent_)->get_hw_serial_number(), &bufferLen);
Use code with caution.

to:
cpp
uart_get_buffered_data_len(static_cast<uart_port_t>(static_cast<uart::IDFUARTComponent*>(this->parent_)->get_hw_serial_number()), &bufferLen);
Use code with caution.

Alternatively, you can simplify the casting by first assigning the result of get_hw_serial_number() to a variable of type uart_port_t.
After making these changes, save the file and attempt to compile the project again. The explicit casts should resolve the type mismatch errors.
For more information on ESPHome development and potential compatibility issues, refer to the ESPHome documentation. If this component is from a custom repository or not yet merged into the main ESPHome codebase, you might also consider reporting the issue to the component's author. 






6 sites
v4.1 - Easier TFT transfer · Blackymas NSPanel_HA_Blueprint · Discussion #1255
2 Dec 2023 — 3. Support to esp-idf framework Although this project still using ESPHome default framework (currently arduino ), we sta...

GitHub

Error mismatched types · Issue #217 · esp-rs/esp-idf-template
17 June 2024 — When I build the esp-idf-template for esp-idf 5.1 with Esp-idf 5.1, I get this error: error[E0308]: mismatched types -

GitHub

error: invalid conversion from 'int' to 'emac_rmii_clock_gpio_t' (IDFGH-13352) · Issue #14265 · espressif/esp-idf
28 July 2024 — Activity added Type: Bug bugs in IDF bugs in IDF on Jul 29, 2024 changed the title [-] error: invalid conversion from ...

GitHub


    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART RX timeout: %s", esp_err_to_name(err));
        this->mark_failed();
        return;
    }

    if (xTaskCreate([](void* o){ static_cast<TZSPSerial*>(o)->uart_event_task(); }, "UART_Event", stack_depth, this, task_priority, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create event task");
        this->mark_failed();
        return;
    }
}

void TZSPSerial::dump_config() {
    char buf[INET_ADDRSTRLEN]; \
    inet_ntop(this->tzsp_sockaddr_in_.sin_family, &this->tzsp_sockaddr_in_.sin_addr, buf, INET_ADDRSTRLEN); \

    ESP_LOGCONFIG(TAG, "TZSPSerial");
    ESP_LOGCONFIG(TAG, "  Destination: %s:%u", buf, ntohs(this->tzsp_sockaddr_in_.sin_port));
    ESP_LOGCONFIG(TAG, "  Protocol: %u", ntohs(this->tzsp_protocol_));
    ESP_LOGCONFIG(TAG, "  Discard protocol: %u", ntohs(this->tzsp_discard_protocol_));
    ESP_LOGCONFIG(TAG, "  Frame size: %u", this->frame_size_);
    ESP_LOGCONFIG(TAG, "  Symbol timeout: %u", this->symbol_timeout_);
    ESP_LOGCONFIG(TAG, "  Inverted: %s", this->inverted_ ? "YES" : "NO");
}

void TZSPSerial::load_buffer(std::vector<uint8_t>& buffer) {
    this->read_array(buffer.data(), buffer.size());

    if (this->inverted_)
        for (auto &b : buffer)
            b ^= 0xFF;
}

void TZSPSerial::uart_event_task() {
    uart_event_t event;
    std::vector<uint8_t> buffer(this->frame_size_);

    for (;;) {
        if(xQueueReceive(*static_cast<uart::IDFUARTComponent*>(this->parent_)->get_uart_event_queue(), &event, portMAX_DELAY)) {
            switch(event.type) {
                [[likely]] case UART_DATA:
                    size_t bufferLen;

                    uart_get_buffered_data_len(static_cast<uart::IDFUARTComponent*>(this->parent_)->get_hw_serial_number(), &bufferLen);
                    if (auto discard = bufferLen % buffer.size()) {
                        std::vector<uint8_t> discard_buffer(discard);

                        this->load_buffer(discard_buffer);
                        this->tzsp_send(discard_buffer, this->tzsp_discard_protocol_);

                        ESP_LOGD(TAG, "Discarded %d bytes", discard);
                    }

                    for (auto i = 0; i < event.size / buffer.size(); i++) {
                        this->load_buffer(buffer);
                        this->tzsp_send(buffer);
                    }

                    break;
                default:
                    ESP_LOGI(TAG, "Unhandled UART event type: %d", event.type);
                    break;
            }
        }
    }
}

}
}
