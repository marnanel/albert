#include "Application.h"

Application::Application(int argc, char**argv):
  QApplication(argc, argv),
  m_processor(&m_memory)
{
  this->startTimer(250);
}

void Application::timerEvent(QTimerEvent *) {
  // called every quarter second;
  // the BBC clock runs at 2MHz
  // i.e. 2 million cycles per second
  // therefore we need to run 500,000 cycles
  m_processor.runCycles(500000);
}
