#pragma once
#ifdef ARDUINO_ARCH_RP2040
#include <pico/critical_section.h>

class Mutex {
 public:
	Mutex() { critical_section_init(&_lck); }
	virtual ~Mutex() { critical_section_deinit(&_lck); }
	void lock() { critical_section_enter_blocking(&_lck); }
	void unlock() { critical_section_exit(&_lck); }

 private:
	critical_section_t _lck;
};
#endif
