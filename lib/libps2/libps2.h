#pragma once
#include <cstdint>
#include <functional>

namespace libps2 {

class PS2 {
 public:
	using recv_callback_t = std::function<void(uint8_t)>;

	PS2();

	/**
	 * @brief Start protocol handling
	 *
	 * @param data_pin PIN number for data line.
	 * @param clock_pin PIN number for clock line. Must work with FALLING pin change interrupt.
	 * @return true Success
	 * @return false Failure
	 */
	bool begin(uint8_t data_pin, uint8_t clock_pin);

	void end();

	/**
	 * @brief Set the recv callback object
	 *
	 * @param callback
	 * @note Callback will be called from ISR (interrupt service routine). Callback should return as quick as possible.
	 */
	void set_recv_callback(recv_callback_t callback) { this->callback = callback; }

	/**
	 * @brief Send data to device
	 *
	 * @param data
	 */
	void send(uint8_t data);

 private:
	enum class isr_state_t : uint8_t { idle, r_data, r_parity, r_stop, s_wait, s_parity, s_stop, s_wait_ack };
	static constexpr size_t rx_size = 10;

	recv_callback_t callback = nullptr;
	uint32_t last_interrupted = 0;
	isr_state_t isr_state = isr_state_t::idle;
	uint8_t clock_pin;
	uint8_t data_pin;
	int8_t bit_count;
	uint8_t data;
	bool parity;

	void clock_isr();
	void reset_for_recv();
	void reset_for_send(uint8_t data);
	void start_interrupt();
	void stop_interrupt();

	static void clock_isr_func(void* param) { ((PS2*)param)->clock_isr(); }
};

}  // namespace libps2
