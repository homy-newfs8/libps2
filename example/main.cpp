#include <Arduino.h>
#include <libps2.h>
#include <mutex>
#include "mutex.hpp"
#include "sq.hpp"

using namespace libps2;

namespace {

constexpr uint8_t clock_pin = D10;
constexpr uint8_t data_pin = D9;

}  // namespace

SQ<uint8_t, 10> rx_buffer{};
Mutex r_mux;

int
available() {
	std::lock_guard<Mutex> lock(r_mux);
	return rx_buffer.count();
}

int
read() {
	std::lock_guard<Mutex> lock(r_mux);
	uint8_t v;
	if (rx_buffer.get(v)) {
		return v;
	}
	return -1;
}

PS2 ps2;

void
setup() {
	ps2.set_recv_callback([](auto code) { rx_buffer.put(code); });
	ps2.begin(data_pin, clock_pin);
}

uint32_t last_echo_sent = 0;

void
loop() {
	if (available() > 0) {
		int c;
		while ((c = read()) >= 0) {
			Serial.printf("%02x\n", c);
		}
	} else if (millis() - last_echo_sent > 5'000) {
		ps2.send(0xee);  // echo
	}
	delay(100);
}
