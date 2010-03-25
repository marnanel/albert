#ifndef MEMORY_H
#define MEMORY_H 1

#include <QObject>

class Memory : public QObject {

  Q_OBJECT

 public:
  Memory();

  int readByteFrom(int position);
  int readWordFrom(int position);

  void writeByteTo(int position, int byte);

 private:
  QByteArray m_ram;
  QByteArray m_languageRom;
  QByteArray m_mos;
};

#endif
