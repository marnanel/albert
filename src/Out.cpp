#include "Processor.h"
#include <QDebug>

#include "golden-trail.c"

#define OpcodeDetails(opcode, mode, cycles) /* nothing */ ;

typedef enum _Mode {
  MODE_IMPLIED,
  MODE_ZERO_PAGE_Y,
  MODE_INDIRECT,
  MODE_IMMEDIATE,
  MODE_ZERO_PAGE_X,
  MODE_ABSOLUTE_X,
  MODE_INDIRECT_Y,
  MODE_ZERO_PAGE,
  MODE_ABSOLUTE,
  MODE_ABSOLUTE_Y,
  MODE_INDIRECT_X,
  MODE_ACCUMULATOR,
  MODE_BRANCH // not an official mode, but useful to model things
} Mode;

typedef enum _Op {
  OP_EOR,
  OP_RTI,
  OP_SBC,
  OP_ORA,
  OP_DEC,
  OP_ASL,
  OP_NOP,
  OP_LSR,
  OP_BIT,
  OP_ROL,
  OP_LDX,
  OP_CMP,
  OP_JMP,
  OP_STA,
  OP_JSR,
  OP_STY,
  OP_ADC,
  OP_INC,
  OP_BRK,
  OP_CPX,
  OP_AND,
  OP_LDA,
  OP_LDY,
  OP_STX,
  OP_RTS,
  OP_ROR,
  OP_CPY,
  OP_BPL,
  OP_BMI,
  OP_BVC,
  OP_BVS,
  OP_BCC,
  OP_BCS,
  OP_BNE,
  OP_BEQ,

  OP_CLC,
  OP_SEC,
  OP_CLI,
  OP_SEI,
  OP_CLV,
  OP_CLD,
  OP_SED,

  OP_TSX,
  OP_TXS,
  OP_PHA,
  OP_PLA,

  // Fallback: will remove.
  OP_XXX
} Op;

QString opnames =
  "EOR RTI SBC ORA DEC ASL NOP LSR BIT ROL LDX CMP "
  "JMP STA JSR STY ADC INC BRK CPX AND LDA LDY STX "
  "RTS ROR CPY BPL BMI BVC BVS BCC BCS BNE BEQ CLC "
  "SEC CLI SEI CLV CLD SED TSX TXS PHA PLA XXX";

struct Opcode {
  Op operation;
  Mode mode;
  int cycles;
};

struct Opcode opcodes[256] = {
   { OP_BRK, MODE_IMPLIED, 7 }, // 0x00
   { OP_ORA, MODE_INDIRECT_X, 6 }, // 0x01
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x02
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x03
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x04
   { OP_ORA, MODE_ZERO_PAGE, 2 }, // 0x05
   { OP_ASL, MODE_ZERO_PAGE, 5 }, // 0x06
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x07
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x08
   { OP_ORA, MODE_IMMEDIATE, 2 }, // 0x09
   { OP_ASL, MODE_ACCUMULATOR, 2 }, // 0x0a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x0b
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x0c
   { OP_ORA, MODE_ABSOLUTE, 4 }, // 0x0d
   { OP_ASL, MODE_ABSOLUTE, 6 }, // 0x0e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x0f
   { OP_BPL, MODE_BRANCH, 2 }, // 0x10
   { OP_ORA, MODE_INDIRECT_Y, 5 }, // 0x11
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x12
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x13
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x14
   { OP_ORA, MODE_ZERO_PAGE_X, 3 }, // 0x15
   { OP_ASL, MODE_ZERO_PAGE_X, 6 }, // 0x16
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x17
   { OP_CLC, MODE_IMPLIED, 2 }, // 0x18
   { OP_ORA, MODE_ABSOLUTE_Y, 4 }, // 0x19
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x1a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x1b
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x1c
   { OP_ORA, MODE_ABSOLUTE_X, 4 }, // 0x1d
   { OP_ASL, MODE_ABSOLUTE_X, 7 }, // 0x1e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x1f
   { OP_JSR, MODE_ABSOLUTE, 6 }, // 0x20
   { OP_AND, MODE_INDIRECT_X, 6 }, // 0x21
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x22
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x23
   { OP_BIT, MODE_ZERO_PAGE, 3 }, // 0x24
   { OP_AND, MODE_ZERO_PAGE, 2 }, // 0x25
   { OP_ROL, MODE_ZERO_PAGE, 5 }, // 0x26
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x27
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x28
   { OP_AND, MODE_IMMEDIATE, 2 }, // 0x29
   { OP_ROL, MODE_ACCUMULATOR, 2 }, // 0x2a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x2b
   { OP_BIT, MODE_ABSOLUTE, 4 }, // 0x2c
   { OP_AND, MODE_ABSOLUTE, 4 }, // 0x2d
   { OP_ROL, MODE_ABSOLUTE, 6 }, // 0x2e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x2f
   { OP_BMI, MODE_BRANCH, 2 }, // 0x30
   { OP_AND, MODE_INDIRECT_Y, 5 }, // 0x31
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x32
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x33
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x34
   { OP_AND, MODE_ZERO_PAGE_X, 3 }, // 0x35
   { OP_ROL, MODE_ZERO_PAGE_X, 6 }, // 0x36
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x37
   { OP_SEC, MODE_IMPLIED, 2 }, // 0x38
   { OP_AND, MODE_ABSOLUTE_Y, 4 }, // 0x39
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x3a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x3b
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x3c
   { OP_AND, MODE_ABSOLUTE_X, 4 }, // 0x3d
   { OP_ROL, MODE_ABSOLUTE_X, 7 }, // 0x3e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x3f
   { OP_RTI, MODE_IMPLIED, 6 }, // 0x40
   { OP_EOR, MODE_INDIRECT_X, 6 }, // 0x41
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x42
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x43
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x44
   { OP_EOR, MODE_ZERO_PAGE, 3 }, // 0x45
   { OP_LSR, MODE_ZERO_PAGE, 5 }, // 0x46
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x47
   { OP_PHA, MODE_IMPLIED, 3 }, // 0x48
   { OP_EOR, MODE_IMMEDIATE, 2 }, // 0x49
   { OP_LSR, MODE_ACCUMULATOR, 2 }, // 0x4a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x4b
   { OP_JMP, MODE_ABSOLUTE, 3 }, // 0x4c
   { OP_EOR, MODE_ABSOLUTE, 4 }, // 0x4d
   { OP_LSR, MODE_ABSOLUTE, 6 }, // 0x4e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x4f
   { OP_BVC, MODE_BRANCH, 2 }, // 0x50
   { OP_EOR, MODE_INDIRECT_Y, 5 }, // 0x51
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x52
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x53
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x54
   { OP_EOR, MODE_ZERO_PAGE_X, 4 }, // 0x55
   { OP_LSR, MODE_ZERO_PAGE_X, 6 }, // 0x56
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x57
   { OP_CLI, MODE_IMPLIED, 2 }, // 0x58
   { OP_EOR, MODE_ABSOLUTE_Y, 4 }, // 0x59
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x5a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x5b
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x5c
   { OP_EOR, MODE_ABSOLUTE_X, 4 }, // 0x5d
   { OP_LSR, MODE_ABSOLUTE_X, 7 }, // 0x5e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x5f
   { OP_RTS, MODE_IMPLIED, 6 }, // 0x60
   { OP_ADC, MODE_INDIRECT_X, 6 }, // 0x61
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x62
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x63
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x64
   { OP_ADC, MODE_ZERO_PAGE, 3 }, // 0x65
   { OP_ROR, MODE_ZERO_PAGE, 5 }, // 0x66
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x67
   { OP_PLA, MODE_IMPLIED, 4 }, // 0x68
   { OP_ADC, MODE_IMMEDIATE, 2 }, // 0x69
   { OP_ROR, MODE_ACCUMULATOR, 2 }, // 0x6a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x6b
   { OP_JMP, MODE_INDIRECT, 5 }, // 0x6c
   { OP_ADC, MODE_ABSOLUTE, 4 }, // 0x6d
   { OP_ROR, MODE_ABSOLUTE, 6 }, // 0x6e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x6f
   { OP_BVS, MODE_BRANCH, 2 }, // 0x70
   { OP_ADC, MODE_INDIRECT_Y, 5 }, // 0x71
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x72
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x73
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x74
   { OP_ADC, MODE_ZERO_PAGE_X, 4 }, // 0x75
   { OP_ROR, MODE_ZERO_PAGE_X, 6 }, // 0x76
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x77
   { OP_SEI, MODE_IMPLIED, 2 }, // 0x78
   { OP_ADC, MODE_ABSOLUTE_Y, 4 }, // 0x79
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x7a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x7b
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x7c
   { OP_ADC, MODE_ABSOLUTE_X, 4 }, // 0x7d
   { OP_ROR, MODE_ABSOLUTE_X, 7 }, // 0x7e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x7f
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x80
   { OP_STA, MODE_INDIRECT_X, 6 }, // 0x81
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x82
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x83
   { OP_STY, MODE_ZERO_PAGE, 3 }, // 0x84
   { OP_STA, MODE_ZERO_PAGE, 3 }, // 0x85
   { OP_STX, MODE_ZERO_PAGE, 3 }, // 0x86
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x87
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x88
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x89
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x8a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x8b
   { OP_STY, MODE_ABSOLUTE, 4 }, // 0x8c
   { OP_STA, MODE_ABSOLUTE, 4 }, // 0x8d
   { OP_STX, MODE_ABSOLUTE, 4 }, // 0x8e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x8f
   { OP_BCC, MODE_BRANCH, 2 }, // 0x90
   { OP_STA, MODE_INDIRECT_Y, 6 }, // 0x91
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x92
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x93
   { OP_STY, MODE_ZERO_PAGE_X, 4 }, // 0x94
   { OP_STA, MODE_ZERO_PAGE_X, 4 }, // 0x95
   { OP_STX, MODE_ZERO_PAGE_Y, 4 }, // 0x96
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x97
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x98
   { OP_STA, MODE_ABSOLUTE_Y, 5 }, // 0x99
   { OP_TXS, MODE_IMPLIED, 2 }, // 0x9a
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x9b
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x9c
   { OP_STA, MODE_ABSOLUTE_X, 5 }, // 0x9d
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x9e
   { OP_XXX, MODE_IMPLIED, 0 }, // 0x9f
   { OP_LDY, MODE_IMMEDIATE, 2 }, // 0xa0
   { OP_LDA, MODE_INDIRECT_X, 6 }, // 0xa1
   { OP_LDX, MODE_IMMEDIATE, 2 }, // 0xa2
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xa3
   { OP_LDY, MODE_ZERO_PAGE, 3 }, // 0xa4
   { OP_LDA, MODE_ZERO_PAGE, 3 }, // 0xa5
   { OP_LDX, MODE_ZERO_PAGE, 3 }, // 0xa6
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xa7
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xa8
   { OP_LDA, MODE_IMMEDIATE, 2 }, // 0xa9
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xaa
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xab
   { OP_LDY, MODE_ABSOLUTE, 4 }, // 0xac
   { OP_LDA, MODE_ABSOLUTE, 4 }, // 0xad
   { OP_LDX, MODE_ABSOLUTE, 4 }, // 0xae
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xaf
   { OP_BCS, MODE_BRANCH, 2 }, // 0xb0
   { OP_LDA, MODE_INDIRECT_Y, 5 }, // 0xb1
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xb2
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xb3
   { OP_LDY, MODE_ZERO_PAGE_X, 4 }, // 0xb4
   { OP_LDA, MODE_ZERO_PAGE_X, 4 }, // 0xb5
   { OP_LDX, MODE_ZERO_PAGE_Y, 4 }, // 0xb6
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xb7
   { OP_CLV, MODE_IMPLIED, 2 }, // 0xb8
   { OP_LDA, MODE_ABSOLUTE_Y, 4 }, // 0xb9
   { OP_TSX, MODE_IMPLIED, 2 }, // 0xba
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xbb
   { OP_LDY, MODE_ABSOLUTE_X, 4 }, // 0xbc
   { OP_LDA, MODE_ABSOLUTE_X, 4 }, // 0xbd
   { OP_LDX, MODE_ABSOLUTE_Y, 4 }, // 0xbe
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xbf
   { OP_CPY, MODE_IMMEDIATE, 2 }, // 0xc0
   { OP_CMP, MODE_INDIRECT_X, 6 }, // 0xc1
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xc2
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xc3
   { OP_CPY, MODE_ZERO_PAGE, 3 }, // 0xc4
   { OP_CMP, MODE_ZERO_PAGE, 3 }, // 0xc5
   { OP_DEC, MODE_ZERO_PAGE, 5 }, // 0xc6
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xc7
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xc8
   { OP_CMP, MODE_IMMEDIATE, 2 }, // 0xc9
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xca
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xcb
   { OP_CPY, MODE_ABSOLUTE, 4 }, // 0xcc
   { OP_CMP, MODE_ABSOLUTE, 4 }, // 0xcd
   { OP_DEC, MODE_ABSOLUTE, 6 }, // 0xce
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xcf
   { OP_BNE, MODE_BRANCH, 2 }, // 0xd0
   { OP_CMP, MODE_INDIRECT_Y, 5 }, // 0xd1
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xd2
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xd3
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xd4
   { OP_CMP, MODE_ZERO_PAGE_X, 4 }, // 0xd5
   { OP_DEC, MODE_ZERO_PAGE_X, 6 }, // 0xd6
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xd7
   { OP_CLD, MODE_IMPLIED, 2 }, // 0xd8
   { OP_CMP, MODE_ABSOLUTE_Y, 4 }, // 0xd9
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xda
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xdb
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xdc
   { OP_CMP, MODE_ABSOLUTE_X, 4 }, // 0xdd
   { OP_DEC, MODE_ABSOLUTE_X, 7 }, // 0xde
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xdf
   { OP_CPX, MODE_IMMEDIATE, 2 }, // 0xe0
   { OP_SBC, MODE_INDIRECT_X, 6 }, // 0xe1
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xe2
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xe3
   { OP_CPX, MODE_ZERO_PAGE, 3 }, // 0xe4
   { OP_SBC, MODE_ZERO_PAGE, 3 }, // 0xe5
   { OP_INC, MODE_ZERO_PAGE, 5 }, // 0xe6
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xe7
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xe8
   { OP_SBC, MODE_IMMEDIATE, 2 }, // 0xe9
   { OP_NOP, MODE_IMPLIED, 2 }, // 0xea
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xeb
   { OP_CPX, MODE_ABSOLUTE, 4 }, // 0xec
   { OP_SBC, MODE_ABSOLUTE, 4 }, // 0xed
   { OP_INC, MODE_ABSOLUTE, 6 }, // 0xee
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xef
   { OP_BEQ, MODE_BRANCH, 2 }, // 0xf0
   { OP_SBC, MODE_INDIRECT_Y, 5 }, // 0xf1
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xf2
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xf3
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xf4
   { OP_SBC, MODE_ZERO_PAGE_X, 4 }, // 0xf5
   { OP_INC, MODE_ZERO_PAGE_X, 6 }, // 0xf6
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xf7
   { OP_SED, MODE_IMPLIED, 2 }, // 0xf8
   { OP_SBC, MODE_ABSOLUTE_Y, 4 }, // 0xf9
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xfa
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xfb
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xfc
   { OP_SBC, MODE_ABSOLUTE_X, 4 }, // 0xfd
   { OP_INC, MODE_ABSOLUTE_X, 7 }, // 0xfe
   { OP_XXX, MODE_IMPLIED, 0 }, // 0xff
};

Processor::Processor(Memory *memory):
  QObject()
{
  m_memory = memory;

  m_programCounter = memory->readWordFrom(0xFFFC);

  m_accumulator = 0;
  m_x = 0;
  m_y = 0;
  m_stack = 0x1FF;

  m_sign = false;
  m_zero = false;
  m_decimal = false;
  m_overflow = false;
  m_carry = false;
  m_interrupt = false;

  m_waiting = 0;

  m_goldenTrailPosition = 0;
}

void Processor::pushToStack(int value) {
  m_memory->writeByteTo( 100 + m_stack,
			 value);
  m_stack--;
}

int Processor::popFromStack() {
  m_stack++;
  return m_memory->readByteFrom( 100 + m_stack);
}

void Processor::oneShot() {

  int opcode;
  int address = 0;
  int param = 0;
  bool address_is_accumulator = false;
  int branchDisplacement = 0;

  if (m_goldenTrailPosition != -1) {
    qDebug() << m_programCounter << golden_trail[m_goldenTrailPosition] << this->disassemble(m_programCounter);
    if (golden_trail[m_goldenTrailPosition] == 0) {
      qDebug() << "Congratulations!  This is the end of the Golden Trail.";
      m_goldenTrailPosition = -1;
    } else if (golden_trail[m_goldenTrailPosition] != m_programCounter) {
      qDebug() << "Aborting after " << m_goldenTrailPosition << "for breach of Golden Trail";
      throw "Breach of golden trail";
      return;
    }
    m_goldenTrailPosition++;
  }

  emit programCounter(m_programCounter);
  opcode = m_memory->readByteFrom(m_programCounter++);
  m_waiting = opcodes[opcode].cycles;

  switch (opcodes[opcode].mode) {

  case MODE_IMPLIED:
    // nothing
    break;

  case MODE_ZERO_PAGE:
  case MODE_ZERO_PAGE_Y:
  case MODE_ZERO_PAGE_X:
    address = m_memory->readByteFrom(m_programCounter);
    param = m_memory->readByteFrom(address);
    m_programCounter++;
    break;

  case MODE_INDIRECT:
  case MODE_INDIRECT_X:
  case MODE_INDIRECT_Y:
    // not sure what to do here
    qDebug() << "Help! Indirect mode!";
    break;

  case MODE_IMMEDIATE:
    param = m_memory->readByteFrom(m_programCounter);
    m_programCounter++;
    break;
    
  case MODE_BRANCH:
    branchDisplacement = m_memory->readByteFrom(m_programCounter);
    if (branchDisplacement & 0x80) {
      branchDisplacement |= ~0xFF;
    }
    m_programCounter++;
    break;
    
  case MODE_ABSOLUTE:
  case MODE_ABSOLUTE_X:
  case MODE_ABSOLUTE_Y:
    address = m_memory->readWordFrom(m_programCounter);
    // we may want to add a flag saying don't fetch the param
    param = m_memory->readByteFrom(address);
    m_programCounter += 2;
    break;

  case MODE_ACCUMULATOR:
    address_is_accumulator = true;
    param = m_accumulator;
    break;

  default:
    qDebug() << "Warning: unknown mode.";
  }

  // Now, let's look up what we should be doing next.

  switch (opcodes[opcode].operation) {

  case OP_BPL:    // Branch on plus.
    OpcodeDetails(0x10, MODE_BRANCH,         2);
    if (!m_sign) {
      m_programCounter += branchDisplacement;
      m_waiting++;
    }
    break;

  case OP_BMI:    // Branch on minus.
    OpcodeDetails(0x30, MODE_BRANCH,         2);
    if (m_sign) {
      m_programCounter += branchDisplacement;
      m_waiting++;
    }
    break;

  case OP_BVC:    // Branch on overflow clear.
    OpcodeDetails(0x50, MODE_BRANCH,         2);
    if (!m_overflow) {
      m_programCounter += branchDisplacement;
      m_waiting++;
    }
    break;

  case OP_BVS:    // Branch on overflow set.
    OpcodeDetails(0x70, MODE_BRANCH,         2);
    if (m_overflow) {
      m_programCounter += branchDisplacement;
      m_waiting++;
    }
    break;

  case OP_BCC:    // Branch on carry clear
    OpcodeDetails(0x90, MODE_BRANCH,         2);
    if (!m_carry) {
      m_programCounter += branchDisplacement;
      m_waiting++;
    }
    break;

  case OP_BCS:    // Branch on carry set
    OpcodeDetails(0xb0, MODE_BRANCH,         2);
    if (m_carry) {
      m_programCounter += branchDisplacement;
      m_waiting++;
    }
    break;

  case OP_BNE:    // Branch on not equal
    OpcodeDetails(0xd0, MODE_BRANCH,         2);
    if (!m_zero) {
      m_programCounter += branchDisplacement;
      m_waiting++;
    }
    break;

  case OP_BEQ:    // Branch on equal
    OpcodeDetails(0xf0, MODE_BRANCH,         2);
    if (m_zero) {
      m_programCounter += branchDisplacement;
      m_waiting++;
    }
    break;

  case OP_LDA:    // Load accumulator.
    OpcodeDetails(0xa1, MODE_INDIRECT_X,     6);
    OpcodeDetails(0xa5, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xa9, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xad, MODE_ABSOLUTE,       4);
    OpcodeDetails(0xb1, MODE_INDIRECT_Y,     5);
    OpcodeDetails(0xb5, MODE_ZERO_PAGE_X,    4);
    OpcodeDetails(0xb9, MODE_ABSOLUTE_Y,     4);
    OpcodeDetails(0xbd, MODE_ABSOLUTE_X,     4);
    qDebug() << "Loading the accumulator with " << param;
    m_accumulator = param;
    m_sign = (param & 0x80) != 0;
    m_zero = param == 0;
    break;

  case OP_LDX:    // Load X.
    OpcodeDetails(0xa2, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xa6, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xae, MODE_ABSOLUTE,       4);
    OpcodeDetails(0xb6, MODE_ZERO_PAGE_Y,    4);
    OpcodeDetails(0xbe, MODE_ABSOLUTE_Y,     4);
    qDebug() << "Loading X with " << param;
    m_x = param;
    m_sign = (param & 0x80) != 0;
    m_zero = param == 0;
    break;

  case OP_LDY:    // Load Y.
    OpcodeDetails(0xa0, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xa4, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xac, MODE_ABSOLUTE,       4);
    OpcodeDetails(0xb4, MODE_ZERO_PAGE_X,    4);
    OpcodeDetails(0xbc, MODE_ABSOLUTE_X,     4);
    qDebug() << "Loading Y with " << param;
    m_y = param;
    m_sign = (param & 0x80) != 0;
    m_zero = param == 0;
    break;

  case OP_PHA:
    OpcodeDetails(0x48, MODE_IMPLIED,        3);
    // Push accumulator.
    this->pushToStack(m_accumulator);
    break;

  case OP_PLA:
    OpcodeDetails(0x68, MODE_IMPLIED,        4);
    // Pull accumulator.
    m_accumulator = this->popFromStack();
    break;

  case OP_STA:
    OpcodeDetails(0x81, MODE_INDIRECT_X,     6);
    OpcodeDetails(0x85, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0x8d, MODE_ABSOLUTE,       4);
    OpcodeDetails(0x91, MODE_INDIRECT_Y,     6);
    OpcodeDetails(0x95, MODE_ZERO_PAGE_X,    4);
    OpcodeDetails(0x99, MODE_ABSOLUTE_Y,     5);
    OpcodeDetails(0x9d, MODE_ABSOLUTE_X,     5);
    // Store accumulator.
    // Does not affect the flags.
    qDebug() << "Storing the accumulator to " << address;
    m_memory->writeByteTo(address, m_accumulator);
    break;

  case OP_STX:
    OpcodeDetails(0x86, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0x8e, MODE_ABSOLUTE,       4);
    OpcodeDetails(0x96, MODE_ZERO_PAGE_Y,    4);
    // Store X.
    // Does not affect the flags.
    qDebug() << "Storing X to " << address;
    m_memory->writeByteTo(address, m_x);
    break;

  case OP_STY:
    OpcodeDetails(0x84, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0x8c, MODE_ABSOLUTE,       4);
    OpcodeDetails(0x94, MODE_ZERO_PAGE_X,    4);
    // Store Y.
    // Does not affect the flags.
    qDebug() << "Storing Y to " << address;
    m_memory->writeByteTo(address, m_y);
    break;

  case OP_NOP:
    OpcodeDetails(0xea, MODE_IMPLIED,        2);
    // Ah, peace.
    break;

  case OP_CLC:    // Clear carry
    OpcodeDetails(0x18, MODE_IMPLIED,        2);
    m_carry = false;
    break;

  case OP_SEC:    // Set carry
    OpcodeDetails(0x38, MODE_IMPLIED,        2);
    m_carry = true;
    break;

  case OP_CLI:    // Clear interrupt
    OpcodeDetails(0x58, MODE_IMPLIED,        2);
    m_interrupt = false;
    break;

  case OP_SEI:    // Set interrupt
    OpcodeDetails(0x78, MODE_IMPLIED,        2);
    m_interrupt = true;
    break;

  case OP_CLV:    // Clear overflow
    OpcodeDetails(0xb8, MODE_IMPLIED,        2);
    m_overflow = false;
    break;

  case OP_CLD:
    OpcodeDetails(0xd8, MODE_IMPLIED,        2);
    // Clear decimal
    m_overflow = true;
    break;

  case OP_SED:
    OpcodeDetails(0xf8, MODE_IMPLIED,        2);
    // Set decimal
    m_decimal = true;
    qDebug() << "Warning: decimal flag set; we may not work properly";
    break;

  case OP_TSX:
    OpcodeDetails(0xba, MODE_IMPLIED,        2);
    // Transfer stack to X
    m_x = m_stack;
    break;

  case OP_TXS:
    OpcodeDetails(0x9a, MODE_IMPLIED,        2);
    // Transfer X to stack
    m_stack = m_x;
    break;

  case OP_ASL:
    OpcodeDetails(0x06, MODE_ZERO_PAGE,      5);
    OpcodeDetails(0x0a, MODE_ACCUMULATOR,    2);
    OpcodeDetails(0x0e, MODE_ABSOLUTE,       6);
    OpcodeDetails(0x16, MODE_ZERO_PAGE_X,    6);
    OpcodeDetails(0x1e, MODE_ABSOLUTE_X,     7);
    // Arithmetic shift left
    param = param << 1;
    m_carry = param & 0x100;
    param &= 0xFF;
    m_zero = param==0;
    m_sign = (param & 0x80) != 0;

    if (address_is_accumulator) {
      m_accumulator = param;
    } else {
      m_memory->writeByteTo(address, param);      
    }
    break;

  case OP_XXX:
    qDebug() << "Alert!  Unknown opcode: " << QString("%1").arg(opcode, 2, 16);
    throw "Unknown opcode.";
    break;

    // Not implemented:

  case OP_SBC:
    OpcodeDetails(0xe1, MODE_INDIRECT_X,     6);
    OpcodeDetails(0xe5, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xe9, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xed, MODE_ABSOLUTE,       4);
    OpcodeDetails(0xf1, MODE_INDIRECT_Y,     5);
    OpcodeDetails(0xf5, MODE_ZERO_PAGE_X,    4);
    OpcodeDetails(0xf9, MODE_ABSOLUTE_Y,     4);
    OpcodeDetails(0xfd, MODE_ABSOLUTE_X,     4);

  case OP_CMP:
    OpcodeDetails(0xc1, MODE_INDIRECT_X,     6);
    OpcodeDetails(0xc5, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xc9, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xcd, MODE_ABSOLUTE,       4);
    OpcodeDetails(0xd1, MODE_INDIRECT_Y,     5);
    OpcodeDetails(0xd5, MODE_ZERO_PAGE_X,    4);
    OpcodeDetails(0xd9, MODE_ABSOLUTE_Y,     4);
    OpcodeDetails(0xdd, MODE_ABSOLUTE_X,     4);

  case OP_JSR:
    OpcodeDetails(0x20, MODE_ABSOLUTE,       6);

  case OP_INC:
    OpcodeDetails(0xe6, MODE_ZERO_PAGE,      5);
    OpcodeDetails(0xee, MODE_ABSOLUTE,       6);
    OpcodeDetails(0xf6, MODE_ZERO_PAGE_X,    6);
    OpcodeDetails(0xfe, MODE_ABSOLUTE_X,     7);

  case OP_AND:
    OpcodeDetails(0x21, MODE_INDIRECT_X,     6);
    OpcodeDetails(0x25, MODE_ZERO_PAGE,      2);
    OpcodeDetails(0x29, MODE_IMMEDIATE,      2);
    OpcodeDetails(0x2d, MODE_ABSOLUTE,       4);
    OpcodeDetails(0x31, MODE_INDIRECT_Y,     5);
    OpcodeDetails(0x35, MODE_ZERO_PAGE_X,    3);
    OpcodeDetails(0x39, MODE_ABSOLUTE_Y,     4);
    OpcodeDetails(0x3d, MODE_ABSOLUTE_X,     4);

  case OP_RTS:
    OpcodeDetails(0x60, MODE_IMPLIED,        6);

  case OP_CPY:
    OpcodeDetails(0xc0, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xc4, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xcc, MODE_ABSOLUTE,       4);

  case OP_ROR:
    OpcodeDetails(0x66, MODE_ZERO_PAGE,      5);
    OpcodeDetails(0x6a, MODE_ACCUMULATOR,    2);
    OpcodeDetails(0x6e, MODE_ABSOLUTE,       6);
    OpcodeDetails(0x76, MODE_ZERO_PAGE_X,    6);
    OpcodeDetails(0x7e, MODE_ABSOLUTE_X,     7);

  case OP_EOR:
    OpcodeDetails(0x41, MODE_INDIRECT_X,     6);
    OpcodeDetails(0x45, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0x49, MODE_IMMEDIATE,      2);
    OpcodeDetails(0x4d, MODE_ABSOLUTE,       4);
    OpcodeDetails(0x51, MODE_INDIRECT_Y,     5);
    OpcodeDetails(0x55, MODE_ZERO_PAGE_X,    4);
    OpcodeDetails(0x59, MODE_ABSOLUTE_Y,     4);
    OpcodeDetails(0x5d, MODE_ABSOLUTE_X,     4);

  case OP_RTI:
    OpcodeDetails(0x40, MODE_IMPLIED,        6);

  case OP_DEC:
    OpcodeDetails(0xc6, MODE_ZERO_PAGE,      5);
    OpcodeDetails(0xce, MODE_ABSOLUTE,       6);
    OpcodeDetails(0xd6, MODE_ZERO_PAGE_X,    6);
    OpcodeDetails(0xde, MODE_ABSOLUTE_X,     7);

  case OP_ORA:
    OpcodeDetails(0x01, MODE_INDIRECT_X,     6);
    OpcodeDetails(0x05, MODE_ZERO_PAGE,      2);
    OpcodeDetails(0x09, MODE_IMMEDIATE,      2);
    OpcodeDetails(0x0d, MODE_ABSOLUTE,       4);
    OpcodeDetails(0x11, MODE_INDIRECT_Y,     5);
    OpcodeDetails(0x15, MODE_ZERO_PAGE_X,    3);
    OpcodeDetails(0x19, MODE_ABSOLUTE_Y,     4);
    OpcodeDetails(0x1d, MODE_ABSOLUTE_X,     4);

  case OP_BIT:
    OpcodeDetails(0x24, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0x2c, MODE_ABSOLUTE,       4);

  case OP_LSR:
    OpcodeDetails(0x46, MODE_ZERO_PAGE,      5);
    OpcodeDetails(0x4a, MODE_ACCUMULATOR,    2);
    OpcodeDetails(0x4e, MODE_ABSOLUTE,       6);
    OpcodeDetails(0x56, MODE_ZERO_PAGE_X,    6);
    OpcodeDetails(0x5e, MODE_ABSOLUTE_X,     7);

  case OP_ROL:
    OpcodeDetails(0x26, MODE_ZERO_PAGE,      5);
    OpcodeDetails(0x2a, MODE_ACCUMULATOR,    2);
    OpcodeDetails(0x2e, MODE_ABSOLUTE,       6);
    OpcodeDetails(0x36, MODE_ZERO_PAGE_X,    6);
    OpcodeDetails(0x3e, MODE_ABSOLUTE_X,     7);

  case OP_JMP:
    OpcodeDetails(0x4c, MODE_ABSOLUTE,       3);
    OpcodeDetails(0x6c, MODE_INDIRECT,       5);

  case OP_ADC:
    OpcodeDetails(0x61, MODE_INDIRECT_X,     6);
    OpcodeDetails(0x65, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0x69, MODE_IMMEDIATE,      2);
    OpcodeDetails(0x6d, MODE_ABSOLUTE,       4);
    OpcodeDetails(0x71, MODE_INDIRECT_Y,     5);
    OpcodeDetails(0x75, MODE_ZERO_PAGE_X,    4);
    OpcodeDetails(0x79, MODE_ABSOLUTE_Y,     4);
    OpcodeDetails(0x7d, MODE_ABSOLUTE_X,     4);

  case OP_BRK:
    OpcodeDetails(0x00, MODE_IMPLIED,        7);

  case OP_CPX:
    OpcodeDetails(0xe0, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xe4, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xec, MODE_ABSOLUTE,       4);

  default:
    qDebug() << "Alert!  Unimplemented opcode encountered: " << QString("%2 %1").arg(opcode, 2, 16).arg(opnames.mid(opcodes[opcode].operation*4, 3));
    throw "Unimplemented opcode.";
  }

}

void Processor::oneCycle() {

  if (m_waiting > 0) {
    m_waiting--;
  } else {
    this->oneShot();
  }
}

void Processor::twoCycles() {
  for (int i=0; i<2; i++) {
    this->oneCycle();
  }
}

QString Processor::disassemble(int where) {
  QString result;

  int opcode = m_memory->readByteFrom(where);
  Opcode op = opcodes[opcode];

  result += opnames.mid(op.operation*4, 3);

  switch (op.mode) {
  case MODE_ACCUMULATOR:
    result += " A";
    break;
  case MODE_IMPLIED:
    // nothing
    break;
  case MODE_BRANCH:
    // nothing for now
    break;
  case MODE_IMMEDIATE:
    result += " #$%1";
    result = result.arg(m_memory->readByteFrom(where+1), 2, 16);
    break;
  case MODE_ZERO_PAGE:
    result += " $%1";
    result = result.arg(m_memory->readByteFrom(where+1), 2, 16);
    break;
  case MODE_ZERO_PAGE_X:
    result += " $%1,X";
    result = result.arg(m_memory->readByteFrom(where+1), 2, 16);
    break;
  case MODE_ZERO_PAGE_Y:
    result += " $%1,Y";
    result = result.arg(m_memory->readByteFrom(where+1), 2, 16);
    break;
  case MODE_ABSOLUTE:
    result += " $%1";
    result = result.arg(m_memory->readWordFrom(where+1), 4, 16);
    break;
  case MODE_ABSOLUTE_X:
    result += " $%1,X";
    result = result.arg(m_memory->readWordFrom(where+1), 4, 16);
    break;
  case MODE_ABSOLUTE_Y:
    result += " $%1,Y";
    result = result.arg(m_memory->readWordFrom(where+1), 4, 16);
    break;
  case MODE_INDIRECT:
    result += " ($%1)";
    result = result.arg(m_memory->readWordFrom(where+1), 4, 16);
    break;
  case MODE_INDIRECT_X:
    result += " ($%1,X)";
    result = result.arg(m_memory->readByteFrom(where+1), 2, 16);
    break;
  case MODE_INDIRECT_Y:
    result += " ($%1,Y)";
    result = result.arg(m_memory->readByteFrom(where+1), 2, 16);
    break;
  }

  return result;
}
