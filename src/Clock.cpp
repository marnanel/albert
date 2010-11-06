#include "Clock.h"

Clock::Clock(): QObject() {
  m_clock = 0;
}

unsigned long Clock::getTime() {
  return m_clock;
}

void Clock::advance(int cycles) {
  m_clock += cycles;
}
