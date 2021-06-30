#include <iostream>
#include <cstring>
#include <bitset>

#include "Emulator.h"

void Emulator::load(uint8_t *romData, long size) {
    memcpy(&rom, romData, size);
}

void Emulator::run() {
    uint8_t instruction = ingestUint8();

//    std::cout << std::hex << "PC: " << pc - 1 << ", INSTR: " << std::bitset<8>(instruction) << ", A: " << (uint16_t)regA << ", B: " << (uint16_t)regB << ", HL: " << regHL.hl << ", F: " << (uint16_t)status << std::endl;

    switch ((instruction & 0xFC) >> 2) {
        default:
        case 0b000000: // NOP
            break;
        case 0b000001: // LDR imm8,reg
            getReg(instruction) = ingestUint8();
            setFlag(FLAG_Z, !getReg(instruction));
            break;
        case 0b000010: // LDR [imm16],reg
            getReg(instruction) = readUint8(ingestUint16());
            setFlag(FLAG_Z, !getReg(instruction));
            break;
        case 0b000011: // LDR [HL],reg
            getReg(instruction) = readUint8(regHL.hl);
            setFlag(FLAG_Z, !getReg(instruction));
            break;
        case 0b000100: // STR [imm16],reg
            writeUint8(ingestUint16(), getReg(instruction));
            break;
        case 0b000101: // STR [HL],reg
            writeUint8(regHL.hl, getReg(instruction));
            break;
        case 0b000110: // LDA imm16
            regHL.hl = ingestUint16();
            break;
        case 0b000111: // IN imm8,reg
            getReg(instruction) = readPort(ingestUint8());
            setFlag(FLAG_Z, !getReg(instruction));
            break;
        case 0b001000: // OUT imm8,reg
            writePort(ingestUint8(), getReg(instruction));
            break;
        case 0b001001: // INC reg
            getReg(instruction)++;
            setFlag(FLAG_Z, !getReg(instruction));
            setFlag(FLAG_C, !getReg(instruction));
            break;
        case 0b001010: // DEC reg
            setFlag(FLAG_C, !getReg(instruction));
            getReg(instruction)--;
            setFlag(FLAG_Z, !getReg(instruction));
            break;
        case 0b001011: // INA
            regHL.hl++;
            break;
        case 0b001100: // DEA
            regHL.hl--;
            break;
        case 0b001101: // ADD imm8
            instrADD(ingestUint8());
            break;
        case 0b001110: // ADD reg
            instrADD(getReg(instruction));
            break;
        case 0b001111: // ADC imm8
            instrADC(ingestUint8());
            break;
        case 0b010000: // ADC reg
            instrADC(getReg(instruction));
            break;
        case 0b010001: // SUB imm8
            instrSUB(ingestUint8());
            break;
        case 0b010010: // SUB reg
            instrSUB(getReg(instruction));
            break;
        case 0b010011: // SBC imm8
            instrSBC(ingestUint8());
            break;
        case 0b010100: // SBC reg
            instrSBC(getReg(instruction));
            break;
        case 0b010101: // AND imm8
            instrAND(ingestUint8());
            break;
        case 0b010110: // AND reg
            instrAND(getReg(instruction));
            break;
        case 0b010111: // OR imm8
            instrOR(ingestUint8());
            break;
        case 0b011000: // OR reg
            instrOR(getReg(instruction));
            break;
        case 0b011001: // XOR imm8
            instrXOR(ingestUint8());
            break;
        case 0b011010: // XOR reg
            instrXOR(getReg(instruction));
            break;
        case 0b011011: // CMP imm8
            instrCMP(ingestUint8());
            break;
        case 0b011100: // CMP reg
            instrCMP(getReg(instruction));
            break;
        case 0b011101: // JMP imm16
            pc = ingestUint16();
            break;
        case 0b011110: // JMP HL
            pc = regHL.hl;
            break;
        case 0b011111: // JR imm8
            pc += ingestInt8();
            break;
        case 0b100000: // JR imm8,cc
            if (checkCondition(instruction))
                pc += ingestInt8();
            else
                pc += 1;
            break;
        case 0b100001: // JR imm8,nn
            if (!checkCondition(instruction))
                pc += ingestInt8();
            else
                pc += 1;
            break;
        case 0b100010: // IN reg
            getReg(instruction) = readPort(regA);
            setFlag(FLAG_Z, !getReg(instruction));
            break;
        case 0b100011: // OUT reg
            writePort(regA, getReg(instruction));
            break;
        case 0b111111: // HALT
            throw HaltException();
    }
}

void Emulator::reset() {
    regA = 0;
    regB = 0;
    regHL.hl = 0;
    pc = 0;
    status = 0;
    fillScreen(RGB888{0, 0, 0});
    memset(ram, 0, 0x8000);
    memset(lights, 0, 10);
    memset(sevenSegmentDisplays, 0, 6);
    graphicsX = 0;
    graphicsY = 0;
}

uint8_t *Emulator::getDisplayBuffer() {
    return displayBuffer;
}

std::string &Emulator::getPrintBuffer() {
    return printBuffer;
}

void Emulator::setSwitch(int id, bool value) {
    switches[id] = value;
}

void Emulator::setButton(int id, bool value) {
    buttons[id] = value;
}

bool Emulator::getLight(int id) {
    return lights[id];
}

uint8_t Emulator::getSevenSegmentDisplay(int id) {
    return sevenSegmentDisplays[id];
}

uint8_t *Emulator::getMemory() {
    return ram;
}

uint8_t *Emulator::getROM() {
    return rom;
}

uint8_t Emulator::getRegA() const {
    return regA;
}

uint8_t Emulator::getRegB() const {
    return regB;
}

uint8_t Emulator::getRegH() const {
    return regHL.values.h;
}

uint8_t Emulator::getRegL() const {
    return regHL.values.l;
}

uint16_t Emulator::getPC() const {
    return pc;
}

uint8_t Emulator::getStatus() const {
    return status;
}

uint8_t Emulator::readUint8(uint16_t address) {
    if (address <= 0x7FFF)
        return rom[address];
    else
        return ram[address - 0x8000];
}

void Emulator::writeUint8(uint16_t address, uint8_t value) {
    if (address >= 0x8000)
        ram[address - 0x8000] = value;
}

uint16_t Emulator::readUint16(uint16_t address) {
    return readUint8(address) << 8 | readUint8(address + 1);
}

uint8_t Emulator::ingestUint8() {
    return readUint8(pc++);
}

int8_t Emulator::ingestInt8() {
    uint8_t value = ingestUint8();
    return *(int8_t*)&value; // Casting normally destroys the value
}

uint16_t Emulator::ingestUint16() {
    pc += 2;
    return readUint16(pc - 2);
}

uint8_t &Emulator::getReg(uint8_t instruction) {
    switch (instruction & 0x3) {
        default:
        case 0:
            return regA;
        case 1:
            return regB;
        case 2:
            return regHL.values.h;
        case 3:
            return regHL.values.l;
    }
}

void Emulator::setFlag(uint8_t flag, bool set) {
    if (set)
        status |= flag;
    else
        status &= ~flag;
}

bool Emulator::checkCondition(uint8_t instruction) const {
    return status & (1 << (3 - instruction & 0x3));
}

uint8_t Emulator::readPort(uint8_t port) {
    switch (port) {
        case 2 ... 7: // Seven Segment Displays
            return sevenSegmentDisplays[port - 2];
        case 8 ... 9: // I/O Panel Buttons
            return buttons[port - 8];
        case 10 ... 19: // LEDs
            return lights[port - 10];
        case 20 ... 29: // I/O Panel Switches
            return switches[port - 20];
        case 30: // Graphics X
            return graphicsX;
        case 31: // Graphics Y
            return graphicsY;
        default:
            return 0;
    }
}

void Emulator::writePort(uint8_t port, uint8_t value) {
    switch (port) {
        case 0: // Print Char
            std::cout << value << std::flush;
            printBuffer.push_back(*(char*)&value);
            if (printBuffer.length() > PRINT_BUFFER)
                printBuffer.erase(0, 1);
            break;
        case 1: // Print String
            std::cout << (char*)&rom[value] << std::flush;
            printBuffer.append((char*)&rom[value]);
            if (printBuffer.length() > PRINT_BUFFER)
                printBuffer.erase(0, printBuffer.length() - PRINT_BUFFER);
            break;
        case 2 ... 7: // Seven Segment Displays
            sevenSegmentDisplays[port - 2] = value;
            break;
        case 10 ... 19: // LEDs
            lights[port - 10] = value;
            break;
        case 30: // Graphics X
            graphicsX = value;
            break;
        case 31: // Graphics Y
            graphicsY = value;
            break;
        case 32: // Draw Pixel
            drawPixel(graphicsX, graphicsY, rgb332To888(value));
            break;
        case 33: // Draw Sprite
            drawSprite(value);
            break;
        case 34: // Clear Screen
            fillScreen(rgb332To888(value));
            break;
        default:
            break;
    }
}

void Emulator::instrADD(uint8_t value) {
    uint8_t newValue = regA + value;
    setFlag(FLAG_Z, !newValue);
    setFlag(FLAG_C, value + regA > 0xFF);
    setFlag(FLAG_N, newValue & 0x80);
    setFlag(FLAG_V, (regA & 0x80) == (value & 0x80) and (regA & 0x80) != (newValue & 0x80));
    regA = newValue;
}

void Emulator::instrADC(uint8_t value) {
    instrADD(value + ((status & FLAG_C) != 0));
}

void Emulator::instrSUB(uint8_t value) {
    regA = setSubFlags(value);
}

void Emulator::instrSBC(uint8_t value) {
    instrSUB(value + ((status & FLAG_C) != 0));
}

void Emulator::instrAND(uint8_t value) {
    regA &= value;
    setFlag(FLAG_Z, !regA);
}

void Emulator::instrOR(uint8_t value) {
    regA |= value;
    setFlag(FLAG_Z, !regA);
}

void Emulator::instrXOR(uint8_t value) {
    regA ^= value;
    setFlag(FLAG_Z, !regA);
}

void Emulator::instrCMP(uint8_t value) {
    setSubFlags(value);
}

uint8_t Emulator::setSubFlags(uint8_t value) {
    uint8_t newValue = regA - value;
    setFlag(FLAG_Z, !newValue);
    setFlag(FLAG_C, value > regA);
    setFlag(FLAG_N, newValue & 0x80);
    setFlag(FLAG_V, (regA & 0x80) == (value & 0x80) and (regA & 0x80) != (newValue & 0x80));
    return newValue;
}

RGB888 Emulator::rgb332To888(uint8_t color) {
    return RGB888{
        static_cast<uint8_t>(((color & 0xE0) >> 5) / 0x7 * 0xFF),
        static_cast<uint8_t>(((color & 0x1C) >> 2) / 0x7 * 0xFF),
        static_cast<uint8_t>((color & 0x03) / 0x3 * 0xFF)
    };
}

void Emulator::drawPixel(uint8_t x, uint8_t y, RGB888 color) {
    memcpy(displayBuffer + (DISPLAY_WIDTH * (graphicsY + y) + graphicsX + x) * 3, &color, 3);
}

void Emulator::drawSprite(uint8_t id) {
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            drawPixel(x, y, rgb332To888(readUint8(64 * id + 8 * y + x)));
}

void Emulator::fillScreen(RGB888 color) {
    for (int y = 0; y < DISPLAY_HEIGHT; y++)
        for (int x = 0; x < DISPLAY_WIDTH; x++)
            drawPixel(x, y, color);
}
