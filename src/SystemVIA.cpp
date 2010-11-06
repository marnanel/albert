#include "SystemVIA.h"
#include <QDebug>

SystemVIA::SystemVIA(Memory *inside):
  MemoryMap(inside) {

  inside->hookMemoryMap(0xFE40, 0xFE5F, this);

  mPinLevelA = 0xFF;
  mPinLevelB = 0xFF;

  // Set (almost) everything to zero.

  mIRA = 0;
  mIRB = 0xF0;  // The top bit is set hig
  mORA = 0;
  mORB = 0;
  mDDRA = 0;
  mDDRB = 0;
  mACR = 0;
  mTimer1 = 0;
  mTimer2 = 0;
  mT1LL = 0;
  mT1LH = 0;
  mT2LL = 0;
  mT2LH = 0;
  mSR = 0;

}

int SystemVIA::read(int address) {
  qDebug() << QString("System VIA read from %1").arg(address, 4, 16);

  switch (address) {
  case 0xfe40:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe41:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe42:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe43:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe44:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe45:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe46:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe47:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe48:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe49:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe4a:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe4b:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe4c:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe4d:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe4e:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  case 0xfe4f:
    qDebug() << QString("System VIA read from %1").arg(address, 4, 16);
    break;

  }
  return 0;
}

void SystemVIA::write(int address, int value) {
  switch (address)
    {
    case 0xfe40:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe41:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe42:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe43:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe44:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe45:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe46:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe47:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe48:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe49:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe4a:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe4b:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe4c:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe4d:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe4e:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;

    case 0xfe4f:
      qDebug() << QString("System VIA write to %1").arg(address, 4, 16);
      break;
    }
}

// Stubs for now:

void SystemVIA::keyDown(int row, int col) {
  Q_UNUSED(row);
  Q_UNUSED(col);
  // ...
}

void SystemVIA::keyUp(int row, int col) {
  Q_UNUSED(row);
  Q_UNUSED(col);
  // ...
}

void SystemVIA::allKeysUp() {
  // ...
}
