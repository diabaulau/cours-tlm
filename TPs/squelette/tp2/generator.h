#ifndef GENERATOR_H
#define GENERATOR_H

#include "ensitlm.h"

SC_MODULE(Generator) {
	ensitlm::initiator_socket<Generator, false> initiator;
	sc_core::sc_in<bool> interrupt_port;
	sc_core::sc_event interrupt_event;

	int vspeed;
	int current_offset;
	int hysteresis_offset;

	SC_CTOR(Generator);

	void gen();
	void wait_interrupt();
	int animated_line_addr(int original_index);
	void update_animation_params();
	void apply_animation();
};

#endif
