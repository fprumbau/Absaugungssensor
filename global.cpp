#include "global.h"

// Debug-Level (Bitmasken)
// 1 (2^0): Loop Start/Ende Meldungen
// 2 (2^1): LoRa-Statusmeldungen (Prüfe auf Pakete)
// 4 (2^2): LoRa-Nachrichten
// 8 (2^3): ADXL-Ausgaben
const uint8_t debug = 12;  