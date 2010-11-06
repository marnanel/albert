#include "MemoryMap.h"
#include <QDebug>

MemoryMap::MemoryMap(Memory *inside):
  QObject() {

  Q_UNUSED(inside);

}

int MemoryMap::read(int address) {
  qDebug() << "Memory map read from" << address << "- returning 0";
  return 0;
}

void MemoryMap::write(int address, int value) {
  qDebug() << "Memory map write to" << address << "with" << value;
}
