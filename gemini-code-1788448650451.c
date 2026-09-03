#define VRAM ((volatile unsigned short*)0x06000000)
#define DISPLAY_CONTROL ((volatile unsigned short*)0x04000000)
#define KEYINPUT (*(volatile unsigned short*)0x04000130)

#define MODE_3 0x0003
#define BG2_ENABLE 0x0400

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

#define KEY_RIGHT  (1 << 4)
#define KEY_LEFT   (1 << 5)
#define KEY_UP     (1 << 6)
#define KEY_DOWN   (1 << 7)

// Цвета
#define COLOR_BLUE   0x7C00
#define COLOR_RED    0x001F
#define COLOR_YELLOW 0x03FF
#define COLOR_GREEN  0x03E0
#define COLOR_WHITE  0x7FFF

static unsigned int seed = 12345;
int rand_range(int min, int max) {
    seed = seed * 1103515245 + 12345;
    unsigned int val = (seed / 65536) % 32768;
    return min + (val % (max - min + 1));
}

void clear_screen(unsigned short color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        VRAM[i] = color;
    }
}

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

// Пиксельные матрицы 3x5 для цифр 0-9
const unsigned char digits[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b001, 0b002, 0b010, 0b010}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

void draw_digit(int x, int y, int num, unsigned short color) {
    if (num < 0 || num > 9) return;
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 3; c++) {
            if ((digits[num][r] >> (2 - c)) & 1) {
                draw_rect(x + c * 2, y + r * 2, 2, 2, color);
            }
        }
    }
}

void draw_score(int x, int y, int score, unsigned short color) {
    int tens = (score / 10) % 10;
    int units = score % 10;
    draw_digit(x, y, tens, color);
    draw_digit(x + 8, y, units, color);
}

int check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void wait_vblank() {
    volatile unsigned short* scanline = (volatile unsigned short*)0x04000006;
    while (*scanline >= 160);
    while (*scanline < 160);
}

int main() {
    *DISPLAY_CONTROL = MODE_3 | BG2_ENABLE;

    // Параметры игрока
    int player_x = 115, player_y = 75;
    int player_size = 10;
    int speed = 2;

    // Параметры монетки
    int coin_x = 50, coin_y = 50;
    int coin_size = 6;

    // Параметры врага
    int enemy_x = 20, enemy_y = 20;
    int enemy_size = 12;
    int enemy_dx = 2; // Скорость по X
    int enemy_dy = 2; // Скорость по Y

    int score = 0;

    clear_screen(COLOR_BLUE);

    while (1) {
        wait_vblank();

        // Стираем старые позиции объектов фоновым цветом
        draw_rect(player_x, player_y, player_size, player_size, COLOR_BLUE);
        draw_rect(enemy_x, enemy_y, enemy_size, enemy_size, COLOR_BLUE);
        draw_score(10, 10, score, COLOR_BLUE);

        // Управление игроком
        unsigned short keys = ~KEYINPUT;
        if ((keys & KEY_LEFT) && player_x > 0) player_x -= speed;
        if ((keys & KEY_RIGHT) && player_x < SCREEN_WIDTH - player_size) player_x += speed;
        if ((keys & KEY_UP) && player_y > 0) player_y -= speed;
        if ((keys & KEY_DOWN) && player_y < SCREEN_HEIGHT - player_size) player_y += speed;

        // Движение и отскок врага
        enemy_x += enemy_dx;
        enemy_y += enemy_dy;

        if (enemy_x <= 0 || enemy_x >= SCREEN_WIDTH - enemy_size) {
            enemy_dx = -enemy_dx;
        }
        if (enemy_y <= 0 || enemy_y >= SCREEN_HEIGHT - enemy_size) {
            enemy_dy = -enemy_dy;
        }

        // 1. Проверка столкновения с монеткой
        if (check_collision(player_x, player_y, player_size, player_size, coin_x, coin_y, coin_size, coin_size)) {
            draw_rect(coin_x, coin_y, coin_size, coin_size, COLOR_BLUE);
            score++;
            if (score > 99) score = 0;

            coin_x = rand_range(20, SCREEN_WIDTH - 20);
            coin_y = rand_range(20, SCREEN_HEIGHT - 20);
        }

        // 2. Проверка столкновения с врагом (Проигрыш)
        if (check_collision(player_x, player_y, player_size, player_size, enemy_x, enemy_y, enemy_size, enemy_size)) {
            // Стираем всё перед рестартом
            draw_rect(coin_x, coin_y, coin_size, coin_size, COLOR_BLUE);
            
            // Сброс параметров
            score = 0;
            player_x = 115;
            player_y = 75;
            enemy_x = 20;
            enemy_y = 20;
            coin_x = rand_range(20, SCREEN_WIDTH - 20);
            coin_y = rand_range(20, SCREEN_HEIGHT - 20);
        }

        // Отрисовка всех объектов на новых позициях
        draw_rect(coin_x, coin_y, coin_size, coin_size, COLOR_YELLOW);
        draw_rect(enemy_x, enemy_y, enemy_size, enemy_size, COLOR_GREEN);
        draw_rect(player_x, player_y, player_size, player_size, COLOR_RED);
        draw_score(10, 10, score, COLOR_WHITE);
    }

    return 0;
}