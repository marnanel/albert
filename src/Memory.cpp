#include <QFile>
#include <QDebug>
#include <stdio.h>

#include "Memory.h"
#include "MemoryMap.h"

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

  } else if (position>=0xFC00 && position<=0xFEFF) {

    // FRED/JIM/SHEILA read.

    if (mMemoryMap.contains(position)) {
      return mMemoryMap[position]->read(position);
    } else {
      qDebug() << QString("Unknown FRED/JIM/SHEILA read %1").arg(position, 0, 16);
      return 0;
    }

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

    if (position>=0x7C00 && position<=0x7CFF) {
      // Mode 7 memory write.  Just dump it for now.
      printf("%c", byte);
    }

  } else if (position>=0xFC00 && position<=0xFEFF) {

    // FRED/JIM/SHEILA write.

    // === FRED ===
    //
    // FC10 to FC13 - Teletext
    // FC40 to FC43 - SCSI

    // === JIM ===
    //
    // FDF0 to FDF3 - SASI

    // === SHEILA ===
    //
    // FE00 to FE07 - CRTC
    // FE08 - ACIA Status
    // FE09 - ACIA Data
    // FE10 - SERPROC
    // FE20 to FE23 - Video ULA
    // FE30 to FE33 - Paged ROM selection
    // FE4x - System VIA
    // FE5x - System VIA
    // FE6x - User VIA
    // FE7x - User VIA
    // FE8x - Disc 8271
    // FE9x - Disc 8271
    // FEAx - Econet (always return 0xFE because we don't support it)
    // FEBx - Econet (always return 0xFE because we don't support it)
    // FECx - ADVAL
    // FEDx - ADVAL
    // FEEx - Tube
    // FEFx - Tube
    

    if (mMemoryMap.contains(position)) {
      mMemoryMap[position]->write(position, byte);
    } else {
      qDebug() << QString("Unknown FRED/JIM/SHEILA write %1 %2").arg(position, 0, 16).arg(byte, 0, 16);
    }

  } else {

    // Just ignore it.

  }

}

void Memory::hookMemoryMap(int startAddress,
			   int endAddress,
			   MemoryMap *map)
{
  for (int i=startAddress; i<=endAddress; i++) {
    mMemoryMap[i] = map;
  }
}
