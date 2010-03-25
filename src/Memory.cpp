#include "Memory.h"
#include <QFile>
#include <QDebug>

// 0000 to 7FFF - RAM
// 8000 to BFFF - one or more language ROMs
// C000 to FFFF - MOS ROM
//  EXCEPT
// FCxx - FRED
// FDxx - JIM
// FExx - SHEILA

// How big is the MOS image? -- 16384, i.e. 4000
// And the BASIC image? -- same
// And the DFS image? -- same
// Therefore only one language ROM plus the MOS is swapped in at once
// No idea how the DFS fits into things, though.

Memory::Memory(): QObject(),
		  m_ram(0x8000, 0)
{
  QFile mos("/home/tthurman/.config/albert/os12.rom");
  mos.open(QIODevice::ReadOnly);
  m_mos = mos.readAll();
  mos.close();

  QFile basic("/home/tthurman/.config/albert/basic2.rom");
  basic.open(QIODevice::ReadOnly);
  m_languageRom = basic.readAll();
  basic.close();
}

int Memory::readByteFrom(int position) {

  if (position>=0x0000 && position<=0x7FFF) {

    // Memory read.
    return m_ram[position];

  } else if (position>=0x8000 && position<=0xBFFF) {

    // Language ROM read.
    return m_languageRom[position-0x8000] & 0xFF;

  } else if (position>=0xFC00 && position<=0xFCFF) {

    // FRED read.
    qDebug() << "FRED read";
    return 0;

  } else if (position>=0xFD00 && position<=0xFDFF) {

    // JIM read.
    qDebug() << "JIM read";
    return 0;

  } else if (position>=0xFE00 && position<=0xFEFF) {

    // SHEILA read.
    qDebug() << "SHEILA read";
    return 0;

  } else if (position>=0xC000 && position<=0xFFFF) {

    // MOS ROM read.
    return m_mos[position-0xC000] & 0xFF;

  } else {

    // Unknown; return zero.
    qDebug() << "Unknown memory read!";
    return 0;

  }
}

int Memory::readWordFrom(int position) {
  return
    this->readByteFrom(position) |
    this->readByteFrom(position+1) << 8;
}

void Memory::writeByteTo(int position, int byte) {

  if (position>=0x0000 && position<=0x7FFF) {

    // Memory write.
    m_ram[position] = byte & 0xFF;

  } else if (position>=0xFC00 && position<=0xFCFF) {

    // FRED write.
    qDebug() << "FRED write";

  } else if (position>=0xFD00 && position<=0xFDFF) {

    // JIM write.
    qDebug() << "JIM write";

  } else if (position>=0xFE00 && position<=0xFEFF) {

    // SHEILA write.
    qDebug() << "SHEILA write";

  } else {

    // Just ignore it.

  }

}
