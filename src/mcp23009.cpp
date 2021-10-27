
#include <Arduino.h>
#include "mcp23009.h"

//

MCP23009IoAbstraction::MCP23009IoAbstraction(uint8_t address, Mcp23xInterruptMode intMode, pinid_t intPin, WireType wireImpl) {
	this->wireImpl = wireImpl;
	this->address = address;
	this->intPin = intPin;
	this->intMode = intMode;
	this->toWrite = this->lastRead = 0;
	this->needsInit = true;
	this->portFlags = 0;
}

void MCP23009IoAbstraction::initDevice() {
	uint8_t controlReg = (readFromDevice8(IOCON_A) & 0xff);
	
	bitClear(controlReg, IOCON_SEQOP_BIT);

	writeToDevice8(IOCON_A, controlReg);

	portFlags = 0;
	needsInit = false;
}

void MCP23009IoAbstraction::toggleBitInRegister(uint8_t regAddr, uint8_t theBit, bool value) {
	uint16_t reg = readFromDevice8(regAddr);
	bitWrite(reg, theBit, value);

	// for debugging to see the commands being sent, uncomment below
	//serdebugF4("toggle(regAddr, bit, toggle): ", regAddr, theBit, value);
	//serdebugFHex("Value: ", reg);
	// end debugging code

	writeToDevice8(regAddr, reg);
}

void MCP23009IoAbstraction::pinDirection(pinid_t pin, uint8_t mode) {
	if(needsInit) initDevice();

	toggleBitInRegister(IODIR_A, pin, (mode == INPUT || mode == INPUT_PULLUP));
	toggleBitInRegister(GPPU_A, pin, mode == INPUT_PULLUP);

	bitSet(portFlags, READER_PORTA_BIT);
}

void MCP23009IoAbstraction::writeValue(pinid_t pin, uint8_t value) {
	if(needsInit) initDevice();

	bitWrite(toWrite, pin, value);
	bitSet(portFlags, CHANGE_PORTA_BIT);
}

uint8_t MCP23009IoAbstraction::readValue(pinid_t pin) {
	return bitRead(lastRead, pin);
}

uint8_t MCP23009IoAbstraction::readPort(pinid_t pin) {
	return lastRead;
}

void MCP23009IoAbstraction::writePort(pinid_t pin, uint8_t value) {
		toWrite = value;
		bitSet(portFlags, CHANGE_PORTA_BIT);
}

bool MCP23009IoAbstraction::runLoop() {
	if(needsInit) initDevice();

	bool writeOk = true;

	bool flagA = bitRead(portFlags, CHANGE_PORTA_BIT);
    if(flagA) 
		writeOk = writeToDevice8(OUTLAT_A, toWrite);

	bitClear(portFlags, CHANGE_PORTA_BIT);

	flagA = bitRead(portFlags, READER_PORTA_BIT);
    if(flagA)
		lastRead = readFromDevice8(GPIO_A);

	return writeOk;
}

// bool MCP23009IoAbstraction::writeToDevice(uint8_t reg, uint16_t command) {
// 	uint8_t data[3];
// 	data[0] = reg;
//     data[1] = (uint8_t)command;
//     data[2] = (uint8_t)(command>>8);
// 	return ioaWireWriteWithRetry(wireImpl, address, data, sizeof data);
// }

bool MCP23009IoAbstraction::writeToDevice8(uint8_t reg, uint8_t command) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = (uint8_t)command;
    return ioaWireWriteWithRetry(wireImpl, address, data, sizeof data);
}

// uint16_t MCP23009IoAbstraction::readFromDevice(uint8_t reg) {
// 	ioaWireWriteWithRetry(wireImpl, address, &reg, 1, 0, false);

// 	uint8_t data[2];
// 	ioaWireRead(wireImpl, address, data, 1);
// 	// read will get port.
// 	return data[0];
// }

uint8_t MCP23009IoAbstraction::readFromDevice8(uint8_t reg) {
    ioaWireWriteWithRetry(wireImpl, address, &reg, 1, 0);

	uint8_t buffer[2];
    if(ioaWireRead(wireImpl, address, buffer, 1)){
       return buffer[0];
    }
    return 0;
}

void MCP23009IoAbstraction::attachInterrupt(pinid_t pin, RawIntHandler intHandler, uint8_t mode) {
	// only if there's an interrupt pin set
	if(intPin == 0xff) return;

	auto  inbuiltIo = internalDigitalIo();
	uint8_t pm = (intMode == ACTIVE_HIGH_OPEN || intMode == ACTIVE_LOW_OPEN) ? INPUT_PULLUP : INPUT;
	uint8_t im = (intMode == ACTIVE_HIGH || intMode == ACTIVE_HIGH_OPEN) ? RISING : FALLING;
    inbuiltIo->pinDirection(intPin, pm);
    inbuiltIo->attachInterrupt(intPin, intHandler, im);

	toggleBitInRegister(GPINTEN_A, pin, true);
	toggleBitInRegister(INTCON_A, pin, mode != CHANGE);
	toggleBitInRegister(DEFVAL_A, pin, mode == FALLING);
}

void MCP23009IoAbstraction::setInvertInputPin(pinid_t pin, bool shouldInvert) {
    toggleBitInRegister(IPOL_A, pin, shouldInvert);
}

IoAbstractionRef ioFrom23009(pinid_t addr, WireType wireImpl) {
	return ioFrom23009IntPerPort(addr, NOT_ENABLED, 0xff, wireImpl);
}

IoAbstractionRef ioFrom23009(uint8_t addr, Mcp23xInterruptMode intMode, pinid_t interruptPin, WireType wireImpl) {
	return ioFrom23009IntPerPort(addr, intMode, interruptPin, wireImpl);
}

IoAbstractionRef ioFrom23009IntPerPort(pinid_t addr, Mcp23xInterruptMode intMode, pinid_t intPin, WireType wireImpl) {
	return new MCP23009IoAbstraction(addr, intMode, intPin, wireImpl);
}
