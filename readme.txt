Author:   Alfredo Rius
License:  BSD(LICENSE)

Code is based on the Adafruit's NFC/RFID controller breakout board
(https://www.adafruit.com/products/364).

This program is based on the readMifare.pde from Adafruit. It takes the
first 4 bytes of a Mifare card uid and compares it to flash memory. Since
it is saved to the flash, it will remember all cards even if the arduino
is disconnected.

If the card is registered it will grant access to "something".

Buttons: (buttons are set to have a pullup resistor so they ar inverted)
  - SLOT_SELECT: Selects a memory slot. LED will blink n times depending
      on the slot that has been selected.
        
  - MEMORY_BUTTON: If it's pressed for 3 seconds while a card is read,
      it will either override the selected slot or delete the card if it
      was already registered.
          
