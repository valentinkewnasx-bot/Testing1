// Адрес видеопамяти GBA для Mode 3 (240x160, 16-битный цвет)
#define VRAM ((volatile unsigned short*)0x06000000)
#define DISPLAY_CONTROL ((volatile unsigned short*)0x04000000)

// Режим Mode 3 + включение BG2 (фон)
#define MODE_3 0x0003
#define BG2_ENABLE 0x0400

// RGB цвет (5 бит на канал)
#define COLOR_BLUE 0x7C00

int main() {
    // Настраиваем дисплей
    *DISPLAY_CONTROL = MODE_3 | BG2_ENABLE;

    // Закрашиваем все 240x160 пикселей синим
    for (int i = 0; i < 240 * 160; i++) {
        VRAM[i] = COLOR_BLUE;
    }

    while (1) {
        // Бесконечный цикл, чтобы игра не завершалась
    }

    return 0;
}