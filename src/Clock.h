#ifndef CLOCK_H
#define CLOCK_H 1

#include <QObject>

class Clock: public QObject {
 public:
  Clock();

  /**
   * Returns the current timestamp in cycles.
   */
  unsigned long getTime();

  /**
   * Advances the clock by a given number of
   * cycles.
   */
  void advance(int cycles);

 private:
  unsigned long m_clock;
};

#endif
