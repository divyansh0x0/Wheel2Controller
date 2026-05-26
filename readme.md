# Wheel2Controller

PID controller for `stm32f103c8t6` microcontroller board.

Specs:
1. 20KB sram - Starts at 0x2000 0000 
2. 64KB flash - Starts at 0x0800 0000

## Documentations
Go through the following links to learn more
1. [GNU Linker Script](https://sourceware.org/binutils/docs/ld/Simple-Example.html)

## GPIO Modes
All gpio pins can be configured in 8 possible configuration:
1. The Input Modes (Listening)
When a pin is an input, it is just listening to the voltage on the wire.
   - Input Floating: The pin is completely disconnected from any internal power or ground. If nothing is connected to the outside of the pin, it "floats" and will randomly read HIGH or LOW based on ambient electrical noise in the room (like a loose antenna).
   - Input Pull-up: The chip connects a weak internal resistor between the pin and VDD (3.3V). If nothing is connected outside, the pin defaults to reading HIGH. If you connect a physical button to the pin and ground, pressing the button easily overpowers the weak resistor, and the pin reads LOW.
   - Input Pull-down: The exact opposite. A weak internal resistor connects the pin to Ground. It defaults to reading LOW until an external voltage strongly pulls it HIGH.
   - Analog: This completely bypasses the digital ON/OFF circuitry. The pin acts as a direct pipe to the chip's Analog-to-Digital Converter (ADC), allowing it to read varying voltages (e.g., 1.2V, 2.5V) from something like a battery voltage monitor.
     The Output Modes (Talking)

2. When a pin is an output, it is actively pushing voltage out or pulling it to ground. Microcontrollers do this using two internal microscopic switches (transistors): a top switch connected to 3.3V, and a bottom switch connected to Ground.
   - Output Push-Pull: Both internal switches are used. If you output a 1, the top switch closes, pushing 3.3V to the pin. If you output a 0, the bottom switch closes, pulling the pin to Ground. It actively drives the signal in both directions. (This is what we used for your LED!)
   - Output Open-Drain: Only the bottom (Ground) switch is active. The top switch is physically disconnected.
     - If you output a 0, it pulls the pin to Ground.
     - If you output a 1, it doesn't push 3.3V; it just disconnects everything (acting like Input Floating). To actually get a HIGH signal, you must add a physical resistor (a pull-up) to the outside of the chip.
3. Alternate Function (AF) Modes (Hardware Auto-Pilot)
Normally, your C++ code turns pins ON and OFF by writing 1s and 0s to an Output Data Register (ODR). But sometimes, your code isn't fast enough.
   - Alternate Function Push-Pull: The pin acts as a Push-Pull output, but your C++ code no longer controls it. Instead, an internal hardware peripheral (like a hardware Timer) takes over the pin. (You will use this to generate high-speed PWM signals for your motors).
   - Alternate Function Open-Drain: The pin acts as an Open-Drain output, but a hardware peripheral controls it. (You will use this for the I2C bus to talk to your MPU6050 Gyroscope, because I2C requires open-drain circuits so multiple devices can share the same wire without short-circuiting).