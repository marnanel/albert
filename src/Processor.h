#ifndef PROCESSOR_H
#define PROCESSOR_H 1

#include "Memory.h"
#include "Clock.h"

class Processor: public QObject {

  Q_OBJECT

 public:
  Processor(Memory *memory,
	    Clock *clock);

  void oneShot();

  void runCycles(int count);

  QString disassemble(int where);

 signals:
  void programCounter(int pc);

 protected:
  void pushToStack(int value);
  int popFromStack();

 private:

  int readParam(int address);
  void writeParam(int address, int param);

  Memory *m_memory;
  Clock *m_clock;

  int m_programCounter;

  int m_accumulator;
  int m_x;
  int m_y;
  int m_stack;

  bool m_sign;
  bool m_zero;
  bool m_decimal;
  bool m_overflow;
  bool m_carry;
  bool m_interrupt;
  bool m_break;

  int m_goldenTrailPosition;
};

#endif
