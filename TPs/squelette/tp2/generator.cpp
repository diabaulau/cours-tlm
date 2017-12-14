#include "ensitlm.h"
#include "generator.h"
#include "addressmap.h"
#include "LCDC_registermap.h"
#include "ROM.h"

#define MASK4(_i_) (0xf << (4*(_i_)))

#define RAND_BASE_ACCEL 4
#define SPEED_LIMIT     9
#define ANIM_HYS_OFFSET 42

Generator::Generator(sc_core::sc_module_name name)
         : sc_core::sc_module(name) {
	this->vspeed = 0;
	this->current_offset = 0;
	this->hysteresis_offset = 0;

	SC_THREAD(gen);
	SC_METHOD(wait_interrupt);
}

void Generator::wait_interrupt() {
	next_trigger(interrupt_port.posedge_event());
	std::cout << "Generator : Received display interrupt\n";
	interrupt_event.notify();
}

void Generator::gen() {
	uint8_t pixels[8];
	ensitlm::data_t rom_data;

	initiator.write(LCDC_ADDR_START + LCDC_ADDR_REG, VIDEO_ADDR_START);
	initiator.write(LCDC_ADDR_START + LCDC_START_REG, 1);

	while(1) {
		apply_animation();
		if((rand() & 0x1f) == 0) {
			update_animation_params();
		}
		for(int i = 0; i < VIDEO_MEMORY_SIZE / 2; i+=4) {
			initiator.read(ROM_ADDR_START + i, rom_data);

			for(int j = 0; j < 8; j++) {
				pixels[j] = (rom_data & MASK4(j)) >> (4 * j);
			}

			initiator.write(animated_line_addr((2 * i) + 4),
					pixels[0] <<  4 |
					pixels[1] << 12 |
					pixels[2] << 20 |
					pixels[3] << 28);
			initiator.write(animated_line_addr(2 * i),
					pixels[4] <<  4 |
					pixels[5] << 12 |
					pixels[6] << 20 |
					pixels[7] << 28);
		}
		initiator.write(LCDC_ADDR_START + LCDC_INT_REG, 0);
		wait(interrupt_event);
	}
}

int Generator::animated_line_addr(int original_index) {
	int result;

	result = original_index + (VIDEO_W * (this->current_offset + this->hysteresis_offset));
	while (result >= VIDEO_MEMORY_SIZE) {
		result -= VIDEO_MEMORY_SIZE;
	}
	while (result < 0) {
		result += VIDEO_MEMORY_SIZE;
	}

	return VIDEO_ADDR_START + result;
}

void Generator::update_animation_params() {
	this->vspeed += ((rand() % (RAND_BASE_ACCEL * 2 + 1)) - RAND_BASE_ACCEL);
	if(this->vspeed > SPEED_LIMIT) {
		this->vspeed = SPEED_LIMIT;
	} else if (this->vspeed < -SPEED_LIMIT) {
		this->vspeed = -SPEED_LIMIT;
	}
	this->hysteresis_offset = ANIM_HYS_OFFSET * (rand() & 1);
}

void Generator::apply_animation() {
	this->current_offset = (this->current_offset + this->vspeed) % VIDEO_H;
}
