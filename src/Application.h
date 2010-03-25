#ifndef APPLICATION_H
#define APPLICATION_H 1

#include "Memory.h"
#include "Processor.h"
#include <QApplication>

class Application: public QApplication {

  Q_OBJECT

 public:
  Application(int argc, char **argv);

 protected:
  void timerEvent(QTimerEvent *event);

 private:
  Memory m_memory;
  Processor m_processor;
};

#endif
