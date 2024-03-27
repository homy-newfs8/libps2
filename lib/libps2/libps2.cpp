#include "libps2.h"
#include <Arduino.h>

namespace libps2 {

namespace {

constexpr uint8_t PS2_INPUT_MODE = INPUT;

}

PS2::PS2() {}

void
PS2::start_interrupt() {
	attachInterruptParam(digitalPinToInterrupt(clock_pin), PS2::clock_isr_func, PinStatus::FALLING, this);
}

bool
PS2::begin(uint8_t data_pin, uint8_t clock_pin) {
	this->clock_pin = clock_pin;
	this->data_pin = data_pin;
	pinMode(clock_pin, PS2_INPUT_MODE);
	pinMode(data_pin, PS2_INPUT_MODE);

	reset_for_recv();
	start_interrupt();

	return true;
}

void
PS2::end() {
	stop_interrupt();
	isr_state = isr_state_t::idle;
}

void
PS2::reset_for_recv() {
	isr_state = isr_state_t::idle;
	bit_mask = 1;
	data = 0;
	parity = false;
	pinMode(data_pin, PS2_INPUT_MODE);
}

void
PS2::reset_for_send(uint8_t data) {
	this->data = data;
	bit_mask = 1;
	parity = false;
	isr_state = isr_state_t::s_wait;
}

void
PS2::stop_interrupt() {
	detachInterrupt(digitalPinToInterrupt(clock_pin));
}

void
PS2::send(uint8_t data) {
	stop_interrupt();
	digitalWrite(data_pin, HIGH);
	pinMode(data_pin, OUTPUT);
	digitalWrite(clock_pin, HIGH);
	pinMode(clock_pin, OUTPUT);
	delayMicroseconds(10);
	digitalWrite(clock_pin, LOW);
	delayMicroseconds(100);
	digitalWrite(data_pin, LOW);  // Send request
	reset_for_send(data);
	start_interrupt();
	pinMode(clock_pin, PS2_INPUT_MODE);
}

void
PS2::clock_isr() {
	auto now = micros();
	if (isr_state != isr_state_t::idle && isr_state != isr_state_t::s_wait && now - last_interrupted > 150) {
		reset_for_recv();
	}
	last_interrupted = now;
	switch (isr_state) {
		case isr_state_t::s_wait: {
			int d = (data & bit_mask) ? HIGH : LOW;
			digitalWrite(data_pin, d);
			if (d)
				parity = !parity;
			bit_mask <<= 1;
			if (bit_mask == 0) {
				isr_state = isr_state_t::s_parity;
			}
			break;
		}
		case isr_state_t::s_parity:
			digitalWrite(data_pin, parity ? LOW : HIGH);
			isr_state = isr_state_t::s_stop;
			break;
		case isr_state_t::s_stop:
			pinMode(data_pin, PS2_INPUT_MODE);
			isr_state = isr_state_t::s_wait_ack;
			break;
		case isr_state_t::s_wait_ack: {
			if (digitalRead(data_pin) == LOW) {
				// ACK
			} else {
				// NACK
			}
			reset_for_recv();
			break;
		}
		case isr_state_t::idle:
			if (digitalRead(data_pin) == LOW) {
				isr_state = isr_state_t::r_data;
			}
			break;
		case isr_state_t::r_data: {
			int d = digitalRead(data_pin);
			if (d == HIGH) {
				parity = !parity;
			}
			data |= d == HIGH ? bit_mask : 0;
			bit_mask <<= 1;
			if (bit_mask == 0) {
				isr_state = isr_state_t::r_parity;
			}
			break;
		}
		case isr_state_t::r_parity: {
			int p = digitalRead(data_pin);
			if (p == parity) {
				parity = false;
			} else {
				parity = true;
			}
			isr_state = isr_state_t::r_stop;
			break;
		}
		case isr_state_t::r_stop:
			if (parity && digitalRead(data_pin) == HIGH) {
				if (callback) {
					callback(data);
				}
			}
			reset_for_recv();
			break;
		default:
			// NOOP
			break;
	}
}

}  // namespace libps2
