#ifndef MEMORYMAP_H
#define MEMORYMAP_H

#include <QObject>
#include "Memory.h"

class MemoryMap: public QObject {

  Q_OBJECT

 public:
  MemoryMap(Memory *inside);

  virtual int read(int address);
  virtual void write(int address, int value);

};

#endif
