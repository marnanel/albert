#ifndef SYSTEMVIA_H
#define SYSTEMVIA_H 1

#include "Memory.h"
#include "MemoryMap.h"

class SystemVIA: public MemoryMap {

  Q_OBJECT

 public:
  SystemVIA(Memory *inside);

  int read(int address);
  void write(int address, int value);

  public slots:

  /**
   * Received when a key is pressed.
   *
   * \param key  The key.  Multiply the
   *             row number by 100 and add
   *             the column number.
   */
  void keyDown(int key);

  /**
   * Received when a key is released.
   *
   * \param key  The key.  Multiply the
   *             row number by 100 and add
   *             the column number.
   */
  void keyUp(int key);

  /**
   * Received to cause all keys to be
   * released.
   */
  void allKeysUp();

 signals:
  /**
   * Sent to turn the Caps Lock LED on or off.
   *
   * \param on    Whether it's on.
   */
  void CapsLED(bool caps, bool on);

  /**
   * Sent to turn the Shift Lock LED on or off.
   *
   * \param on    Whether it's on.
   */
  void ShiftLED(bool caps, bool on);

 protected:

  // These are the registers of the 6522 VIA.

  int mORB;
  int mORA;
  int mDDRB;
  int mDDRA;
  int mT1CL;
  int mT1CH; // maybe do these as one?
  int mT1LL;
  int mT1LH;
  int mT2CL;
  int mT2CH;
  int mSR;
  int mACR;
  int mPCR;
  int mIFR;
  int mIER;

};

#endif
