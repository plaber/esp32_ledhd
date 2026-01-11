/* hd poi firmware */

/*
accessing flash while an ISR is running WILL cause issues IF the ISR is executing in IRAM
remove ESP_INTR_FLAG_IRAM from https://github.com/Makuna/NeoPixelBus/blob/master/src/internal/NeoEsp32RmtMethod.h#L557

Pins 1 and 3 are REPL UART TX and RX respectively
Pins 6, 7, 8, 11, 16, and 17 are used for connecting the embedded flash, and are not recommended for other uses
Pins 34-39 are input only, and also do not have internal pull-up resistors
The pull value of some pins can be set to Pin.PULL_HOLD to reduce power consumption during deepsleep.

Important: Highly recomended to compile this code with ESP32 v2.0.17 board library, any of 3x version may cause erros
*/