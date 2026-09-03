#define VRAM ((volatile unsigned short*)0x06000000)
#define DISPLAY_CONTROL ((volatile unsigned short*)0x04000000)
#define KEYINPUT (*(volatile unsigned short*)0x04000130)

// Настройки дисплея
#define MODE_3 0x0003
#define BG2_ENABLE 0x0400

// Размеры экрана GBA
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

// Маски кнопок D-Pad (активный уровень — LOW)
#define KEY_A      (1 << 0)
#define KEY_B      (1 << 1)
#define KEY_RIGHT  (1 << 4)
#define KEY_LEFT   (1 << 5)
#define KEY_UP     (1 << 6)
#define KEY_DOWN   (1 << 7)

// Цвета (15-битный BGR: 5 бит на канал)
#define COLOR_BLUE  0x7C00 // B: 31, G: 0, R: 0
#define COLOR_RED   0x001F // B: 0, G: 0, R: 31

// Очистка экрана
void clear_screen(unsigned short color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        VRAM[i] = color;
    }
}

// Отрисовка закрашенного прямоугольника
void draw_rect(int x, int y, int width, int height, unsigned short color) {
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            int px = x + c;
            int py = y + r;
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                VRAM[py * SCREEN_WIDTH + px] = color;
            }
        }
    }
}

// Задержка ожидания VBlank (вертикального импульса гашения) для предотвращения мерцания
void wait_vblank() {
    volatile unsigned short* scanline = (volatile unsigned short*)0x04000006;
    while (*scanline >= 160);
    while (*scanline < 160);
}

int main() {
    *DISPLAY_CONTROL = MODE_3 | BG2_ENABLE;

    // Начальные координаты квадрата
    int player_x = 115;
    int player_y = 75;
    int player_size = 10;
    int speed = 2;

    clear_screen(COLOR_BLUE);

    while (1) {
        wait_vblank();

        // Стираем квадрат на старой позиции (закрашиваем фоном)
        draw_rect(player_x, player_y, player_size, player_size, COLOR_BLUE);

        // Чтение ввода D-Pad
        unsigned short keys = ~KEYINPUT; // Инвертируем, чтобы нажатая кнопка стала 1

        if (keys & KEY_LEFT) {
            if (player_x > 0) player_x -= speed;
        }
        if (keys & KEY_RIGHT) {
            if (player_x < SCREEN_WIDTH - player_size) player_x += speed;
        }
        if (keys & KEY_UP) {
            if (player_y > 0) player_y -= speed;
        }
        if (keys & KEY_DOWN) {
            if (player_y < SCREEN_HEIGHT - player_size) player_y += speed;
        }

        // Рисуем квадрат на новой позиции
        draw_rect(player_x, player_y, player_size, player_size, COLOR_RED);
    }

    return 0;
}