
#ifndef __MCP23009_H__INCLUDE__
#define __MCP23009_H__INCLUDE__

#include "IoAbstractionWire.h"

//
// MCP23017 support.
//

// definitions of register addresses.

#define IODIR_A         0x00
#define IPOL_A          0x01
#define GPINTEN_A       0x02
#define DEFVAL_A        0x03
#define INTCON_A        0x04
#define IOCON_A         0x05
#define GPPU_A          0x06
#define INTF_A          0x07
#define INTCAP_A        0x08
#define GPIO_A          0x09
#define OUTLAT_A        0x0A


// #define CHANGE_PORTA_BIT 0
// #define CHANGE_PORTB_BIT 1
// #define READER_PORTA_BIT 2
// #define READER_PORTB_BIT 3

/**
 * This abstaction supports most of the available features on the 23x09 range of IOExpanders. It supports most
 * of the GPIO functions and nearly all of the interrupt modes, and is therefore very close to Arduino pins in
 * terms of functionality.
 */
class MCP23009IoAbstraction : public BasicIoAbstraction {
private:
	WireType wireImpl;
	uint8_t  address;
	pinid_t  intPin;
	uint8_t  intMode;
	uint8_t  portFlags;
	bool     needsInit;
	uint8_t lastRead;
	uint8_t toWrite;
public:
	/**
	 * Normally, it's easier to use the helper functions to create an instance of this class rather than create yourself.
	 * @see iofrom23017
	 * @see iofrom23017IntPerPort
	 */
	MCP23009IoAbstraction(uint8_t address, Mcp23xInterruptMode intMode,  pinid_t intPin, WireType wireImpl);
	virtual ~MCP23009IoAbstraction() {;}

	/**
	 * Sets the pin direction similar to pinMode, pin direction on this device supports INPUT_PULLUP, INPUT and OUTPUT.
	 * @param pin the pin to set direction for on this device
	 * @param mode the mode such as INPUT, INPUT_PULLUP, OUTPUT
	 */
	void pinDirection(pinid_t pin, uint8_t mode) override;

	void writeValue(pinid_t pin, uint8_t value) override;
	uint8_t readValue(pinid_t pin) override;

	/**
	 * Attaches an interrupt to the device and links it to the arduino pin. On the MCP23009 nearly all interrupt modes
	 * are supported, including CHANGE, RISING, FALLING and are selective both per port and by pin.
	 */
	void attachInterrupt(pinid_t pin, RawIntHandler intHandler, uint8_t mode) override;
	
	/** 
	 * updates settings on the board after changes 
	 */
	bool runLoop() override;
	
	/**
	 * Writes a complete 8 bit port value, that is updated to the device each sync
	 * Any pin between 0-7 refers to the port.
	 */
	void writePort(pinid_t pin, uint8_t port) override;

	/**
	 * Reads the complete 8 bit byte from the last cached state, that is updated each sync.
	 */ 
	uint8_t readPort(pinid_t pin) override;

    /**
     * This MCP23009 only function inverts the meaning of a given input pin. The pins for this
     * are 0..7 and true will invert the meaning, whereas false will leave as is. regardless if
     * you are using any other IO expanders, using this function requires that you have an actual
     * MCP23009IoAbstraction reference. If you want to use this feature, instead of the variable
     * being of type IoAbstractionRef, it should be of type MCP23009IoAbstraction*
     * 
     * @param pin the input pin between 0..7
     * @param shouldInvert true to invert the given pin, otherwise false.
     */
    void setInvertInputPin(pinid_t pin, bool shouldInvert);

private:
	void toggleBitInRegister(uint8_t regAddr, uint8_t theBit, bool value);
	void initDevice();
	// bool writeToDevice(uint8_t reg, uint16_t command);
	// uint16_t readFromDevice(uint8_t reg);
	bool writeToDevice8(uint8_t reg, uint8_t command);
	uint8_t readFromDevice8(uint8_t reg);
};

/**
 * Perform digital read and write functions using 23017 expanders. These expanders are the closest in
 * terms of functionality to regular Arduino pins, supporting most interrupt modes and very similar GPIO
 * capabilities. See the other helper methods if you want interrupts.
 * @param addr the i2c address of the device
 * @param wireImpl (defaults to using Wire) can be overriden to any pointer to another Wire/I2C
 * @return an IoAbstactionRef for the device
 */
IoAbstractionRef ioFrom23009(pinid_t addr, WireType wireImpl);

/**
 * Perform digital read and write functions using 23017 expanders. These expanders are the closest in
 * terms of functionality to regular Arduino pins, supporting most interrupt modes and very similar GPIO
 * capabilities. This uses one Arduino interrupt pin for BOTH ports on the device.
 * @param addr the i2c address of the device
 * @param intMode the interrupt mode the device will operate in
 * @param interruptPin the pin on the Arduino that will be used to detect the interrupt condition.
 * @param wireImpl (defaults to using Wire) can be overriden to any pointer to another Wire/I2C
 * @return an IoAbstactionRef for the device
 */
IoAbstractionRef ioFrom23009(uint8_t addr, Mcp23xInterruptMode intMode, pinid_t interruptPin, WireType wireImpl);

/**
 * Perform digital read and write functions using 23017 expanders. These expanders are the closest include
 * terms of functionality to regular Arduino pins, supporting most interrupt modes and very similar GPIO
 * capabilities. If interrupts are needed, this uses one Arduino pin for EACH port on the device.
 * @param addr the i2c address of the device
 * @param intMode the interrupt mode the device will operate in
 * @param interruptPinA the pin on the Arduino that will be used to detect the PORTA interrupt condition.
 * @param interruptPinB the pin on the Arduino that will be used to detect the PORTB interrupt condition.
 * @param wireImpl (defaults to using Wire) can be overriden to any pointer to another Wire/I2C
 * @return an IoAbstactionRef for the device
 */
IoAbstractionRef ioFrom23009IntPerPort(pinid_t addr, Mcp23xInterruptMode intMode, pinid_t interruptPin, WireType wireImpl);

inline IoAbstractionRef ioFrom23009IntPerPort(uint8_t addr, Mcp23xInterruptMode intMode, pinid_t interruptPin) {
    return ioFrom23009IntPerPort(addr, intMode, interruptPin, defaultWireTypePtr);
}

inline IoAbstractionRef ioFrom23009(uint8_t addr, Mcp23xInterruptMode intMode, pinid_t interruptPin) {
    return ioFrom23009(addr, intMode, interruptPin, defaultWireTypePtr);
}

inline IoAbstractionRef ioFrom23009(pinid_t addr) {
    return ioFrom23009(addr, defaultWireTypePtr);
}


#endif
