#include "Processor.h"
#include <QDebug>

#define OpcodeDetails(opcode, mode, cycles) /* nothing */ ;

const int accumulator_address = -1;

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

#include "opnames.cpp"

struct Opcode {
  Op operation;
  Mode mode;
  int cycles;
};

#include "opcodes.cpp"

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

int Processor::readParam(int address) {
  if (address==accumulator_address) {
    return m_accumulator;
  } else {
    return m_memory->readByteFrom(address);
  }
}

void Processor::writeParam(int address, int param) {
  if (address==accumulator_address) {
    m_accumulator = param;
  } else {
    m_memory->writeByteTo(address, param);
  }
}

void Processor::oneShot() {

  int opcode;
  int address = 0;
  int param = 0;
  int branchDisplacement = 0;

#if 0
  if (m_goldenTrailPosition != -1) {
    //qDebug() << m_programCounter << golden_trail[m_goldenTrailPosition] << this->disassemble(m_programCounter);

    if (m_goldenTrailPosition % 1000==0) {
      qDebug() << m_goldenTrailPosition;
    }

    if (golden_trail[m_goldenTrailPosition] == 0) {
      qDebug() << "Congratulations!  This is the end of the Golden Trail.";
      m_goldenTrailPosition = -1;
    } else if (golden_trail[m_goldenTrailPosition] != m_programCounter) {
      qDebug() << "Aborting after " << m_goldenTrailPosition << "for breach of Golden Trail";
      throw "Breach of golden trail";
      return;
    } else {
      m_goldenTrailPosition++;
    }
  }
#endif

  opcode = m_memory->readByteFrom(m_programCounter++);
  m_waiting = opcodes[opcode].cycles;

  switch (opcodes[opcode].mode) {

  case MODE_IMPLIED:
    // nothing
    break;

  case MODE_ZERO_PAGE:
    address = m_memory->readByteFrom(m_programCounter);
    m_programCounter++;
    break;

  case MODE_ZERO_PAGE_Y:
    address = m_memory->readByteFrom(m_programCounter);
    address += m_x;
    m_programCounter++;
    break;

  case MODE_ZERO_PAGE_X:
    address = m_memory->readByteFrom(m_programCounter);
    address += m_y;
    m_programCounter++;
    break;

  case MODE_INDIRECT:
    address = m_memory->readWordFrom(m_programCounter);
    // theoretically we should update the program counter
    // to bypass the two bytes we just read, but the only
    // instruction that uses this mode is JMP!
    address = m_memory->readWordFrom(address);
    break;

  case MODE_INDIRECT_X:
    address = m_memory->readByteFrom(m_programCounter);
    m_programCounter++;
    address += m_x;
    address = m_memory->readWordFrom(address);
    break;

  case MODE_INDIRECT_Y:
    // note that indirect X and indirect Y are
    // deliberately dissimilar.
    address = m_memory->readByteFrom(m_programCounter);
    m_programCounter++;
    address = m_memory->readWordFrom(address);
    address += m_y;
    break;

  case MODE_IMMEDIATE:
    address = m_programCounter;
    m_programCounter++;
    break;
    
  case MODE_BRANCH:
    branchDisplacement = m_memory->readByteFrom(m_programCounter);
    if (branchDisplacement & 0x80) {
      // It's negative; do the sign extension.
      branchDisplacement |= ~0xFF;
    }
    m_programCounter++;
    break;
    
  case MODE_ABSOLUTE:
    address = m_memory->readWordFrom(m_programCounter);
    m_programCounter += 2;
    break;

  case MODE_ABSOLUTE_X:
    address = m_memory->readWordFrom(m_programCounter);
    m_programCounter += 2;
    break;

  case MODE_ABSOLUTE_Y:
    address = m_memory->readWordFrom(m_programCounter);
    m_programCounter += 2;
    break;

  case MODE_ACCUMULATOR:
    address = accumulator_address;
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

    param = this->readParam(address);
    //qDebug() << "Loading the accumulator with " << param;
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

    param = this->readParam(address);
    //qDebug() << "Loading X with " << param;
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
    //qDebug() << "Loading Y with " << param;

    param = this->readParam(address);
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
    //qDebug() << "Storing the accumulator to " << address;
    m_memory->writeByteTo(address, m_accumulator);
    break;

  case OP_STX:
    OpcodeDetails(0x86, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0x8e, MODE_ABSOLUTE,       4);
    OpcodeDetails(0x96, MODE_ZERO_PAGE_Y,    4);
    // Store X.
    // Does not affect the flags.
    //qDebug() << "Storing X to " << address;
    m_memory->writeByteTo(address, m_x);
    break;

  case OP_STY:
    OpcodeDetails(0x84, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0x8c, MODE_ABSOLUTE,       4);
    OpcodeDetails(0x94, MODE_ZERO_PAGE_X,    4);
    // Store Y.
    // Does not affect the flags.
    //qDebug() << "Storing Y to " << address;
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

  case OP_CLD:    // Clear decimal
    OpcodeDetails(0xd8, MODE_IMPLIED,        2);
    m_overflow = true;
    break;

  case OP_SED:    // Set decimal
    OpcodeDetails(0xf8, MODE_IMPLIED,        2);
    m_decimal = true;
    qDebug() << "Warning: decimal flag set; we may not work properly";
    break;

  case OP_TSX:    // Transfer stack to X
    OpcodeDetails(0xba, MODE_IMPLIED,        2);
    m_x = m_stack;
    break;

  case OP_TXS:    // Transfer X to stack
    OpcodeDetails(0x9a, MODE_IMPLIED,        2);
    m_stack = m_x;
    break;

  case OP_ASL:    // Arithmetic shift left
    OpcodeDetails(0x06, MODE_ZERO_PAGE,      5);
    OpcodeDetails(0x0a, MODE_ACCUMULATOR,    2);
    OpcodeDetails(0x0e, MODE_ABSOLUTE,       6);
    OpcodeDetails(0x16, MODE_ZERO_PAGE_X,    6);
    OpcodeDetails(0x1e, MODE_ABSOLUTE_X,     7);

    param = this->readParam(address);
    param = param << 1;
    m_carry = param & 0x100;
    param &= 0xFF;
    m_zero = param==0;
    m_sign = (param & 0x80) != 0;
    this->writeParam(address, param);
    break;


  case OP_TAX:  // Transfer A to X
    OpcodeDetails(0xaa, MODE_IMPLIED,        2);
    m_x = m_accumulator;
    m_zero = m_accumulator == 0;
    m_sign = (m_accumulator & 0x80) != 0;
    break;

  case OP_TXA:  // Transfer X to A
    OpcodeDetails(0x8a, MODE_IMPLIED,        2);
    m_accumulator = m_x;
    m_zero = m_accumulator == 0;
    m_sign = (m_accumulator & 0x80) != 0;
    break;

  case OP_DEX:  // Decrement X
    OpcodeDetails(0xca, MODE_IMPLIED,        2);
    m_x = (m_x-1) & 0xFF;
    m_zero = m_x == 0;
    m_sign = (m_x & 0x80) != 0;
    break;

  case OP_INX:  // Increment X
    OpcodeDetails(0xe8, MODE_IMPLIED,        2);
    m_x = (m_x+1) & 0xFF;
    m_zero = m_x == 0;
    m_sign = (m_x & 0x80) != 0;
    break;

  case OP_TAY:  // Transfer A to Y
    OpcodeDetails(0xa8, MODE_IMPLIED,        2);
    m_y = m_accumulator;
    m_zero = m_accumulator == 0;
    m_sign = (m_accumulator & 0x80) != 0;
    break;

  case OP_TYA:  // Transfer Y to A
    OpcodeDetails(0x98, MODE_IMPLIED,        2);
    m_accumulator = m_y;
    m_zero = m_accumulator == 0;
    m_sign = (m_accumulator & 0x80) != 0;
    break;

  case OP_DEY:  // Decrement Y
    OpcodeDetails(0x88, MODE_IMPLIED,        2);
    m_y = (m_y-1) & 0xFF;
    m_zero = m_y == 0;
    m_sign = (m_y & 0x80) != 0;
    break;

  case OP_INY:  // Increment Y
    OpcodeDetails(0xc8, MODE_IMPLIED,        2);
    m_y = (m_y+1) & 0xFF;
    m_zero = m_y == 0;
    m_sign = (m_y & 0x80) != 0;
    break;

  case OP_CMP:  // Compare
    OpcodeDetails(0xc1, MODE_INDIRECT_X,     6);
    OpcodeDetails(0xc5, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xc9, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xcd, MODE_ABSOLUTE,       4);
    OpcodeDetails(0xd1, MODE_INDIRECT_Y,     5);
    OpcodeDetails(0xd5, MODE_ZERO_PAGE_X,    4);
    OpcodeDetails(0xd9, MODE_ABSOLUTE_Y,     4);
    OpcodeDetails(0xdd, MODE_ABSOLUTE_X,     4);

    param = this->readParam(address);
    param = m_accumulator - param;
    m_sign = (param & 0x80) != 0;
    m_zero = param==0;
    m_carry = (param & 0x100) != 0;
    break;

  case OP_CPX: // Compare X
    OpcodeDetails(0xe0, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xe4, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xec, MODE_ABSOLUTE,       4);

    param = this->readParam(address);
    param = m_x - param;
    m_sign = (param & 0x80) != 0;
    m_zero = param==0;
    m_carry = (param & 0x100) != 0;
    break;

  case OP_CPY: // Compare Y
    OpcodeDetails(0xc0, MODE_IMMEDIATE,      2);
    OpcodeDetails(0xc4, MODE_ZERO_PAGE,      3);
    OpcodeDetails(0xcc, MODE_ABSOLUTE,       4);

    param = this->readParam(address);
    param = m_y - param;
    m_sign = (param & 0x80) != 0;
    m_zero = param==0;
    m_carry = (param & 0x100) != 0;
    break;

  case OP_INC: // Increment
    OpcodeDetails(0xe6, MODE_ZERO_PAGE,      5);
    OpcodeDetails(0xee, MODE_ABSOLUTE,       6);
    OpcodeDetails(0xf6, MODE_ZERO_PAGE_X,    6);
    OpcodeDetails(0xfe, MODE_ABSOLUTE_X,     7);

    param = this->readParam(address);
    param = (param+1) & 0xFF;
    m_memory->writeByteTo(address, param);
    m_zero = param == 0;
    m_sign = (param & 0x80) != 0;
    break;

  case OP_DEC: // Decrement
    OpcodeDetails(0xc6, MODE_ZERO_PAGE,      5);
    OpcodeDetails(0xce, MODE_ABSOLUTE,       6);
    OpcodeDetails(0xd6, MODE_ZERO_PAGE_X,    6);
    OpcodeDetails(0xde, MODE_ABSOLUTE_X,     7);

    param = this->readParam(address);
    param = (param-1) & 0xFF;
    m_memory->writeByteTo(address, param);
    m_zero = param == 0;
    m_sign = (param & 0x80) != 0;
    break;

    ////////////////////////////////////////////////////////////////

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

  case OP_JSR:
    OpcodeDetails(0x20, MODE_ABSOLUTE,       6);

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

  default:
    qDebug() << "Alert!  Unimplemented opcode encountered: " << QString("%2 %1").arg(opcode, 2, 16).arg(opnames.mid(opcodes[opcode].operation*4, 3));
    throw "Unimplemented opcode.";
  }

}

void Processor::runCycles(int count) {

  if (m_waiting >= count) {
    m_waiting -= count;
    return;
  }

  int cyclesTaken = m_waiting;

  while (cyclesTaken < count) {
    m_waiting = 0;

    this->oneShot();

    cyclesTaken += m_waiting;
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
    result += " ($%1),Y";
    result = result.arg(m_memory->readByteFrom(where+1), 2, 16);
    break;
  }

  return result;
}
