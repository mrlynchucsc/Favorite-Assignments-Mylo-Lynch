// Copyright (c) 2024 Ethan Sifferman.
// All rights reserved. Distribution Prohibited.

#include "cpu.h"

#include <algorithm>
#include <bitset>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "console.h"

BananaCpu::BananaCpu(Console& console, std::vector<uint8_t>& RAM)
    : console_(console),
      RAM_(RAM),
      registers_(32, 0),
      PC_(0),
      op_table_(64, &BananaCpu::NOP),
      function_table_(64, &BananaCpu::NOP) {
  op_table_[kFUNC] = &BananaCpu::ExecuteRType;
  op_table_[kBEQ] = &BananaCpu::BEQ;
  op_table_[kSB] = &BananaCpu::SB;
  op_table_[kJAL] = &BananaCpu::JAL;
  op_table_[kLBU] = &BananaCpu::LBU;
  op_table_[kJ] = &BananaCpu::J;
  op_table_[kADDI] = &BananaCpu::ADDI;
  op_table_[kBNE] = &BananaCpu::BNE;
  op_table_[kLW] = &BananaCpu::LW;
  op_table_[kSW] = &BananaCpu::SW;

  function_table_[kSUB] = &BananaCpu::SUB;
  function_table_[kSRL] = &BananaCpu::SRL;
  function_table_[kAND] = &BananaCpu::AND;
  function_table_[kNOR] = &BananaCpu::NOR;
  function_table_[kSRA] = &BananaCpu::SRA;
  function_table_[kSLL] = &BananaCpu::SLL;
  function_table_[kJR] = &BananaCpu::JR;
  function_table_[kOR] = &BananaCpu::OR;
  function_table_[kSLT] = &BananaCpu::SLT;
  function_table_[kADD] = &BananaCpu::ADD;
}

void BananaCpu::NOP() {  // No operation, just increment the program counter
  PC_ += 4;
}

// I Type
void BananaCpu::BEQ() {  // Opcode: 2, Branch on Equal
  if (registers_[reg_a_] == registers_[reg_b_]) {
    PC_ += 4 * immediate_;
  }
  PC_ += 4;
}

void BananaCpu::SB() {  // Opcode: 11, Store Byte
  int addr = registers_[reg_a_] + immediate_;
  console_.write8(addr, registers_[reg_b_] & 0xFF);
  PC_ += 4;

  if (addr == console_.kDebugstdoutAddress) {  // print to stdout
    std::cout << (char)registers_[reg_b_];
  } else if (addr == console_.kDebugstderrAddress) {  // print to stderr
    std::cerr << (char)registers_[reg_b_];
  } else if (addr ==
             console_.kStopExecutionAddress) {  // terminate Banana execution
    exit(0);
  }
}

void BananaCpu::JAL() {      // Opcode: 24, Jump and Link
  registers_[31] = PC_ + 4;  // Link register
  PC_ = 4 * immediate_;  // Jump to immediate (make sure immediate is already
                         // the correct absolute address)
}

void BananaCpu::LBU() {  // Opcode: 25, Load Byte Unsigned
  int addr = (registers_[reg_a_] + immediate_);

  // Handle input from STDIN
  if (addr == console_.kDebugstdinAddress) {
    registers_[reg_b_] = static_cast<uint8_t>(std::cin.get());
  } else {
    registers_[reg_b_] = console_.read8(addr);
  }
  PC_ += 4;
}

void BananaCpu::J() {  // Opcode: 37, Jump
  PC_ = 4 * immediate_;
}

void BananaCpu::ADDI() {  // Opcode: 46, Add Immediate
  registers_[reg_b_] = registers_[reg_a_] + immediate_;
  PC_ += 4;
}

void BananaCpu::BNE() {  // Opcode: 51, Branch On Not Equal
  if (registers_[reg_a_] != registers_[reg_b_]) {
    PC_ = PC_ + 4 + (4 * immediate_);
  } else {
    PC_ += 4;
  }
}

void BananaCpu::LW() {  // Opcode: 53, Load Word (2 Bytes)
  registers_[reg_b_] = console_.read16(registers_[reg_a_] + immediate_);
  PC_ += 4;
}

void BananaCpu::SW() {  // Opcode: 58, Store Word (2 Bytes)
  console_.write16(registers_[reg_a_] + immediate_, registers_[reg_b_]);
  PC_ += 4;
}

// R type
void BananaCpu::SUB() {  // Function: 0, Subtract
  registers_[reg_c_] = registers_[reg_a_] - registers_[reg_b_];
  PC_ += 4;
}

void BananaCpu::SRL() {  // Function: 13, Shift Right Logical
  registers_[reg_c_] = (unsigned)registers_[reg_b_] >> shift_value_;
  PC_ += 4;
}

void BananaCpu::AND() {  // Function: 19, And
  registers_[reg_c_] = registers_[reg_a_] & registers_[reg_b_];
  PC_ += 4;
}

void BananaCpu::NOR() {  // Function: 21, Nor
  registers_[reg_c_] = ~(registers_[reg_a_] | registers_[reg_b_]);
  PC_ += 4;
}

void BananaCpu::SRA() {  // Function: 25, Shift Right Arithmetic
  registers_[reg_c_] = (signed)registers_[reg_b_] >> shift_value_;
  PC_ += 4;
}

void BananaCpu::SLL() {  // Function: 30, Shift Left Logical
  registers_[reg_c_] = (registers_[reg_b_] << shift_value_);
  PC_ += 4;
}

void BananaCpu::JR() {  // Function: 40, Jump Register
  PC_ = registers_[reg_a_];
}

void BananaCpu::OR() {  // Function: 50, Or
  registers_[reg_c_] = registers_[reg_a_] | registers_[reg_b_];
  PC_ += 4;
}

void BananaCpu::SLT() {  // Function: 57, Set Less Than
  registers_[reg_c_] = (registers_[reg_a_] < registers_[reg_b_]) ? 1 : 0;
  PC_ += 4;
}

void BananaCpu::ADD() {  // Function: 60, Add
  registers_[reg_c_] = registers_[reg_a_] + registers_[reg_b_];
  PC_ += 4;
}

void BananaCpu::ExecuteInstruction(uint32_t instruction) {
  DecodeInstruction(instruction);

  if (op_code_ < op_table_.size() && PC_ >= console_.kSLUGFileAddress) {
    (this->*op_table_[op_code_])();
  } else {
    NOP();
  }
}

void BananaCpu::ExecuteRType() {
  if (function_ < function_table_.size()) {
    (this->*function_table_[function_])();
  } else {
    NOP();
  }
}

void BananaCpu::DecodeInstruction(uint32_t instruction) {
  // mask and decode 32 bit instruction
  op_code_ = (instruction >> 26) & 0x0000003F;     // get bits 26-31
  reg_a_ = (instruction & 0x03E00000) >> 21;       // bits 21-25
  reg_b_ = (instruction & 0x001F0000) >> 16;       // bits 16-20
  reg_c_ = (instruction & 0x0000F800) >> 11;       // bits 11-15
  shift_value_ = (instruction & 0x000007C0) >> 6;  // bits 6-10
  function_ = instruction & 0x0000003F;            // bits 0-5
  immediate_ = instruction & 0x0000FFFF;           // bits 0-15
}

// Save and Load
void BananaCpu::saveState(std::ofstream& out) {
  out.write(reinterpret_cast<char*>(&PC_), sizeof(PC_));
  out.write(reinterpret_cast<char*>(&op_code_), sizeof(op_code_));
  out.write(reinterpret_cast<char*>(&reg_a_), sizeof(reg_a_));
  out.write(reinterpret_cast<char*>(&reg_b_), sizeof(reg_b_));
  out.write(reinterpret_cast<char*>(&reg_c_), sizeof(reg_c_));
  out.write(reinterpret_cast<char*>(&shift_value_), sizeof(shift_value_));
  out.write(reinterpret_cast<char*>(&function_), sizeof(function_));
  out.write(reinterpret_cast<char*>(&immediate_), sizeof(immediate_));
  out.write(reinterpret_cast<char*>(registers_.data()),
            registers_.size() * sizeof(registers_[0]));
  // Save other necessary state variables...
}

void BananaCpu::loadState(std::ifstream& in) {
  in.read(reinterpret_cast<char*>(&PC_), sizeof(PC_));
  in.read(reinterpret_cast<char*>(&op_code_), sizeof(op_code_));
  in.read(reinterpret_cast<char*>(&reg_a_), sizeof(reg_a_));
  in.read(reinterpret_cast<char*>(&reg_b_), sizeof(reg_b_));
  in.read(reinterpret_cast<char*>(&reg_c_), sizeof(reg_c_));
  in.read(reinterpret_cast<char*>(&shift_value_), sizeof(shift_value_));
  in.read(reinterpret_cast<char*>(&function_), sizeof(function_));
  in.read(reinterpret_cast<char*>(&immediate_), sizeof(immediate_));
  in.read(reinterpret_cast<char*>(registers_.data()),
          registers_.size() * sizeof(registers_[0]));
  // Load other necessary state variables...
}

void BananaCpu::PrintInstruction() {
  bool isRType = false;
  switch (op_code_) {
    case kFUNC:
      isRType = true;
      break;
    case kBEQ:
      std::cout << "Opcode: Branch On Equal\t\t\t\t"
                << "if Register[" << reg_a_ << "] == Register[" << reg_b_
                << "]\t PC = PC+4 + 4*" << std::dec << immediate_ << std::endl;
      break;
    case kSB:
      std::cout << "Opcode: Store Byte\t\t\t\t"
                << "Ram[Reg[" << reg_a_ << "] + ";
      if (immediate_ == 0) {
        std::cout << "0x0000]";
      } else {
        std::cout << "0x" << std::hex << immediate_;
      }
      std::cout << "] = Reg[" << reg_b_ << "]\n";
      break;
    case kJAL:
      std::cout << "Opcode: Jump And Link\t\t\t\t"
                << "Reg[31] = PC+4;\tPC=4*Immediate\n";  // immediate value will
                                                         // be unknown without
                                                         // running setup
      break;
    case kLBU:
      std::cout << "Opcode: Load Byte Unsigned\t\t\t"
                << "Ram[" << reg_b_ << "] = Ram[Reg[" << reg_a_ << "] + "
                << std::dec << immediate_ << "]\n";
      break;
    case kJ:
      std::cout << "Opcode: Jump\t\t\t\tPC=4*" << immediate_ << std::endl;
      break;
    case kADDI:
      std::cout << "Opcode: Add Immediate:\t\t\t\t"
                << "Reg[" << reg_b_ << "] = Reg[" << reg_a_ << "] + "
                << std::dec << immediate_ << std::endl;
      break;
    case kBNE:
      std::cout << "Opcode: Branch On Not Equal\t\t\t"
                << "if Reg[" << reg_a_ << "] != Reg[" << reg_b_
                << "]\t PC = PC+4 + 4*" << std::dec << immediate_ << std::endl;
      break;
    case kLW:
      std::cout << "Opcode: Load Word\t\t\t\tReg[" << reg_b_ << "] = Ram[Reg["
                << reg_a_ << "] + " << std::dec << immediate_ << "]\n";
      ;
      break;
    case kSW:
      std::cout << "Opcode: Store Word\t\t\t\tRam[Reg[" << reg_a_ << "] + "
                << std::dec << immediate_ << "] = Reg[" << reg_b_ << "]\n";
      break;
    default:
      std::cout << "Unknown opcode\n";
      break;
  }

  if (isRType == true) {
    switch (op_code_) {
      case kSUB:
        std::cout << "Function: 0, Subtract\t\t\t\t"
                  << "Reg[" << reg_c_ << "] = Reg[" << reg_a_ << "] - Reg["
                  << reg_b_ << "]\n";
        break;
      case kSRL:
        std::cout << "Function: 13, Shift Right Logical\t\t"
                  << "Reg[" << reg_c_ << "] = (unsigned)Reg[" << reg_b_
                  << "] value >> shifted right " << shift_value_ << std::endl;
        break;
      case kAND:
        std::cout << "Function: 19, And\t\t\t\tReg[" << reg_c_ << "] = Reg["
                  << reg_a_ << "] & Reg[" << reg_b_ << "]\n";
        break;
      case kNOR:
        std::cout << "Function: 21, Nor\t\t\t\tReg[" << reg_c_ << "] = ~(Reg["
                  << reg_a_ << "] | Reg[" << reg_b_ << "])\n";
        break;
      case kSRA:
        std::cout << "Function: 25, Shift Right Arithmetic\t\tReg[" << reg_c_
                  << "] = Reg[" << reg_b_ << "] shifted right" << shift_value_
                  << std::endl;
        break;
      case kSLL:
        std::cout << "Function: 30, Shift Left Logical\t\tReg[" << reg_c_
                  << "] = Reg[" << reg_b_ << "] shifted left" << shift_value_
                  << std::endl;
        break;
      case kJR:
        std::cout << "Function: 40, Jump Register\t\tPC = Reg[" << reg_a_
                  << "]\n";
        break;
      case kOR:
        std::cout << "Function: 50, Or\t\t\t\tReg[" << reg_c_ << "] = Reg["
                  << reg_a_ << "] | Reg[" << reg_b_ << "]\n";
        break;
      case kSLT:
        std::cout << "Function: 57, Set Less Than\t\tReg[" << reg_c_
                  << "] = Reg[" << reg_a_ << "] + Reg[" << reg_b_ << "]\n";
        break;
      case kADD:
        std::cout << "Function: 60, Add\t\t\t\tReg[" << reg_c_ << "] = Reg["
                  << reg_a_ << "] + Reg[" << reg_b_ << "]\n";
        break;
      default:
        std::cout << "Unknown function code" << std::endl;
    }
  }
}