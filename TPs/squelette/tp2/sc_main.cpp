#include "bus.h"
#include "generator.h"
#include "memory.h"
#include "ensitlm.h"
#include "LCDC.h"
#include "addressmap.h"
#include "ROM.h"

int sc_main(int argc, char **argv) {
	srand(42);

	Generator gen("Generator1");
	Memory mem("Memory", VIDEO_MEMORY_SIZE);
	Bus bus("Bus");
	LCDC lcd("LCD_Controller", sc_core::sc_time(1.0/25.0, sc_core::SC_SEC));
	ROM image_rom("ROM_IMAGE");
	sc_core::sc_signal<bool, sc_core::SC_MANY_WRITERS> irq_signal("IRQ");

	bus.map(mem.target, VIDEO_ADDR_START, VIDEO_MEMORY_SIZE);
	// Banc de registres du lcd, cf Q2
	bus.map(lcd.target_socket, LCDC_ADDR_START, LCDC_REGISTER_SIZE);
	bus.map(image_rom.socket, ROM_ADDR_START, ROM_SIZE);

	gen.interrupt_port.bind(irq_signal);
	lcd.display_int.bind(irq_signal);
	gen.initiator.bind(bus.target);
	mem.target.bind(bus.initiator);
	image_rom.socket.bind(bus.initiator);
	lcd.target_socket.bind(bus.initiator);
	lcd.initiator_socket.bind(bus.target);

	sc_core::sc_start();
	return EXIT_SUCCESS;
}
