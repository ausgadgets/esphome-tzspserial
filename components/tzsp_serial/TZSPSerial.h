#pragma once

#include <esphome/core/component.h>
#include <esphome/components/uart/uart.h>
#include <esphome/components/uart/uart_component_esp_idf.h>

#include <esphome/components/tzsp/tzsp.h>

namespace esphome {
namespace tzspserial {

static const auto TAG = "tzsp_serial";

class TZSPSerial : public Component, public uart::UARTDevice, public tzsp::TZSPSender {
  public:
    TZSPSerial(uart::IDFUARTComponent *parent) : uart::UARTDevice(parent) {}

    void setup() override;
    void dump_config() override;

    void set_frame_size(size_t frame_size) { this->frame_size_ = frame_size; }
    void set_symbol_timeout(uint8_t symbol_timeout) { this->symbol_timeout_ = symbol_timeout; }
    void set_inverted(bool inverted) { this->inverted_ = inverted; }

  protected:
    size_t frame_size_{};
    uint8_t symbol_timeout_{};
    bool inverted_{};

  private:
    uart::IDFUARTComponent* idf_uart;

    [[noreturn]] void uart_event_task();
};

}
}