#ifndef MEMORY_H
#define MEMORY_H 1

#include <QObject>
#include <QHash>

class MemoryMap;

class Memory : public QObject {

  Q_OBJECT

 public:
  Memory();

  int readByteFrom(int position);
  int readWordFrom(int position);

  void writeByteTo(int position, int byte);

  /**
   * Hooks a given series of addresses.
   * When these addresses are
   * read to or written from, the given MemoryMap
   * will be notified.  This only works for addresses
   * within FRED, JIM, and SHEILA (FC00 to FEFF).
   */
  void hookMemoryMap(int startAddress,
		     int endAddress,
		     MemoryMap *map);

 private:
  QByteArray m_ram;
  QByteArray m_languageRom;
  QByteArray m_mos;
  QHash<int, MemoryMap*> mMemoryMap;
};

#endif
