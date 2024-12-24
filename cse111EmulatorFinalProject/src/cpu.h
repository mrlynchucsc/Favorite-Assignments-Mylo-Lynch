// cpu.h
// Copyright (c) 2024 Ethan Sifferman.
// All rights reserved. Distribution Prohibited.

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Console;

class BananaCpu {
 protected:
 private:
  Console& console_;
  std::vector<uint8_t>& RAM_;

 public:
  typedef void (BananaCpu::*Instruction)();
  std::vector<Instruction> op_table_;
  std::vector<Instruction> function_table_;
  std::vector<int16_t> registers_;

  int16_t op_code_, reg_a_, reg_b_, reg_c_, shift_value_, function_, immediate_;
  uint16_t PC_;  // Program Counter

  // Constructor
  BananaCpu(Console& OS, std::vector<uint8_t>& RAM);

  // Save State
  void saveState(std::ofstream& out);
  void loadState(std::ifstream& in);

  // Decode / Execute
  void ExecuteInstruction(uint32_t);  // Decodes & Execute Instruction
  void ExecuteRType();                // Called if R-Type Instruction
  void DecodeInstruction(uint32_t);
  void PrintInstruction();

  enum OpCode {
    // I types
    kFUNC = 0x00,  // Opcode: for R-Type Instructions
    kBEQ = 0x02,   // Opcode:  2, Branch On Equal
    kSB = 0x0B,    // Opcode: 11, Store Byte
    kJAL = 0x18,   // Opcode: 24, Jump And Link
    kLBU = 0X19,   // Opcode: 25, Load Byte Unsigned
    kJ = 0X25,     // Opcode: 37, Jump
    kADDI = 0X2E,  // Opcode: 46, Add Immediate
    kBNE = 0X33,   // Opcode: 51, Branch On Not Equal
    kLW = 0X35,    // Opcode: 53, Load Word
    kSW = 0X3A,    // Opcode: 58, Store Word
  };

  enum FunctionCode {
    // R types (assuming common opcode 0)
    kSUB = 0x00,  // Function:  0, Subtract
    kSRL = 0x0D,  // Function: 13, Shift Right Logical
    kAND = 0x13,  // Function: 19, And
    kNOR = 0x15,  // Function: 21, Nor
    kSRA = 0x19,  // Function: 25, Shift Right Arithmetic
    kSLL = 0x1E,  // Function: 30, Shift Left Logical
    kJR = 0x28,   // Function: 40, Jump Register
    kOR = 0x32,   // Function: 50, Or
    kSLT = 0x39,  // Function: 57, Set Less Than
    kADD = 0x3C   // Function: 60, Add
  };

  // CPU Instructions
  void NOP();

  // I Type Instructions
  void BEQ();
  void SB();
  void JAL();
  void LBU();
  void J();
  void ADDI();
  void BNE();
  void LW();
  void SW();

  // R Type Instructions
  void SUB();
  void SRL();
  void AND();
  void NOR();
  void SRA();
  void SLL();
  void JR();
  void OR();
  void SLT();
  void ADD();
};