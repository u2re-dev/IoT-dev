/*
  Тетрис для ESP32 S3 M5Stack Cardputer
  Язык: C++20
  
  Для компиляции необходимо добавить библиотеку M5Stack.
  
  Подключите устройство в Arduino IDE (или PlatformIO) и используйте стандартный цикл setup()/loop().
*/

#include <M5Stack.h>
#include <vector>
#include <array>
#include <cstdlib>
#include <ctime>

// Размеры игрового поля (см. классическую систему)
constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 20;

// Размер одного блока в пикселях (подберите под разрешение экрана)
constexpr int BLOCK_SIZE = 12;
// Смещение игрового поля на экране (можно изменить под ваше устройство)
constexpr int BOARD_OFFSET_X = 20;
constexpr int BOARD_OFFSET_Y = 20;

// Тип игрового поля – 0: пусто, >0: заполнено (можно использовать цвета)
using Board = std::array<std::array<int, BOARD_WIDTH>, BOARD_HEIGHT>;

// Структура описывает падающую фигуру (тетромино)
struct TetrisPiece {
  // Каждый блок задаётся смещением относительно центра (pivot)
  std::vector<std::pair<int,int>> blocks;
  // Координата центра (относительно поля)
  int x{0}, y{0};
  // id фигуры (можно использовать для выбора цвета и отключения поворота для "O")
  int type{0};

  // Поворачивает фигуру на 90 градусов по часовой стрелке.
  // Используем простую матричную операцию: (x, y) -> (y, -x)
  // Фигура O (type==3) не вращается.
  void rotate() {
    if(type == 3) return; // Фигура O не вращается
    for(auto& block : blocks) {
      int oldX = block.first;
      int oldY = block.second;
      block.first = oldY;
      block.second = -oldX;
    }
  }
  
  // Возвращает копию фигуры, повернутую на 90 градусов, не изменяя исходную.
  TetrisPiece rotatedCopy() const {
    if(type == 3) return *this;
    TetrisPiece p = *this;
    for(auto& block : p.blocks) {
      int oldX = block.first;
      int oldY = block.second;
      block.first = oldY;
      block.second = -oldX;
    }
    return p;
  }
};

// Класс игры Тетрис
class TetrisGame {
public:
  TetrisGame() {
    resetBoard();
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    spawnNewPiece();
    lastDropTime = millis();
  }

  // Основной игровой цикл: обновление логики и отрисовка
  void update() {
    handleInput();
    unsigned long now = millis();
    if(now - lastDropTime > dropInterval) {
      movePiece(0, 1);
      lastDropTime = now;
    }
    draw();
  }

private:
  Board board{};
  TetrisPiece currentPiece;
  unsigned long lastDropTime = 0;
  const unsigned long dropInterval = 500; // интервал падения в мс (можно уменьшать по мере игры)
  
  // Иконки для 7-ти тетромино. Координаты задаются относительно центра (pivot)
  // Расположение задаётся так, чтобы центр фигуры оказался примерно в (0,0)
  // Порядок: I, J, L, O, S, T, Z
  const std::vector<std::vector<std::pair<int,int>>> tetrominoes {
    // I
    { {-2,0}, {-1,0}, {0,0}, {1,0} },
    // J
    { {-1,-1}, {-1,0}, {0,0}, {1,0} },
    // L
    { {1,-1}, {-1,0}, {0,0}, {1,0} },
    // O
    { {0,0}, {1,0}, {0,1}, {1,1} },
    // S
    { {0,0}, {1,0}, {-1,1}, {0,1} },
    // T
    { {-1,0}, {0,0}, {1,0}, {0,1} },
    // Z
    { {-1,0}, {0,0}, {0,1}, {1,1} }
  };

  // Очищаем игровое поле
  void resetBoard() {
    for(auto& row : board) {
      row.fill(0);
    }
  }

  // Проверка, что позиция фигуры допустима (на поле и без пересечения с зафиксированными блоками)
  bool isValidPosition(const TetrisPiece& piece) {
    for(const auto& offset : piece.blocks) {
      int newX = piece.x + offset.first;
      int newY = piece.y + offset.second;
      // Проверяем границы поля
      if(newX < 0 || newX >= BOARD_WIDTH || newY < 0 || newY >= BOARD_HEIGHT)
        return false;
      // Проверяем наличие уже установленного блока
      if(board[newY][newX] != 0)
        return false;
    }
    return true;
  }
  
  // Фиксируем фигуру на игровом поле
  void fixPiece() {
    // Для простоты в board будем записывать id фигуры (от 1 до 7)
    for(const auto& offset : currentPiece.blocks) {
      int newX = currentPiece.x + offset.first;
      int newY = currentPiece.y + offset.second;
      if(newY >= 0 && newY < BOARD_HEIGHT &&
         newX >= 0 && newX < BOARD_WIDTH)
      {
        board[newY][newX] = currentPiece.type + 1; // +1 чтобы не было 0
      }
    }
    clearLines();
    spawnNewPiece();
  }

  // Проверка и удаление заполненных линий
  void clearLines() {
    for(int y = 0; y < BOARD_HEIGHT; ++y) {
      bool full = true;
      for(int x = 0; x < BOARD_WIDTH; ++x) {
        if(board[y][x] == 0) {
          full = false;
          break;
        }
      }
      if(full) {
        // Перемещаем все строки выше вниз
        for(int ty = y; ty > 0; --ty) {
          board[ty] = board[ty-1];
        }
        board[0].fill(0);
      }
    }
  }

  // Создать новую фигуру в верхней части поля
  void spawnNewPiece() {
    int type = std::rand() % tetrominoes.size();
    currentPiece.type = type;
    currentPiece.blocks = tetrominoes[type];
    currentPiece.x = BOARD_WIDTH / 2;
    currentPiece.y = 0;
    // Если сразу невозможно разместить фигуру – конец игры. Здесь можно добавить логику "Game Over"
    if(!isValidPosition(currentPiece)) {
      resetBoard();
    }
  }

  // Перемещает фигуру на (dx, dy)
  void movePiece(int dx, int dy) {
    TetrisPiece moved = currentPiece;
    moved.x += dx;
    moved.y += dy;
    if(isValidPosition(moved)) {
      currentPiece = moved;
    } else if(dy != 0) {
      // Если мы не смогли опустить фигуру вниз, значит она достигла основания
      if(dy == 1) fixPiece();
    }
  }

  // Пытается повернуть фигуру
  void rotatePiece() {
    TetrisPiece rotated = currentPiece.rotatedCopy();
    if(isValidPosition(rotated)) {
      currentPiece = rotated;
    }
  }
  
  // Обрабатывает ввод с кнопок M5Stack (настройте под ваше устройство)
  void handleInput() {
    M5.update(); // Обновляем состояние кнопок
    // Кнопка A – поворот
    if(M5.BtnA.wasPressed()) {
      rotatePiece();
    }
    // Кнопка LEFT – перемещение влево (если у вас физические кнопки, можно использовать M5.BtnLeft)
    if(M5.BtnB.isPressed()) { // Здесь BtnB используем для передвижения влево
      movePiece(-1, 0);
      delay(150); // антидребезг (не обязательно)
    }
    // Кнопка RIGHT – перемещение вправо
    if(M5.BtnC.isPressed()) { // Здесь BtnC – вправо
      movePiece(1, 0);
      delay(150);
    }
    // Если нужна возможность ускоренного падения, можно добавить проверку другой кнопки
    if(M5.BtnA.pressedFor(300)) { // Ускоренное падение при долгом нажатии
      movePiece(0, 1);
      delay(50);
    }
  }

  // Отрисовка игрового поля и текущей фигуры
  void draw() {
    M5.Lcd.fillScreen(BLACK);
    
    // Рисуем поле: фиксированные блоки
    for(int y = 0; y < BOARD_HEIGHT; ++y) {
      for(int x = 0; x < BOARD_WIDTH; ++x) {
        if(board[y][x] != 0) {
          // Определяем цвет в зависимости от типа фигуры (вы можете расширить выбор)
          uint16_t col = getColor(board[y][x]-1);
          M5.Lcd.fillRect(BOARD_OFFSET_X + x * BLOCK_SIZE,
                          BOARD_OFFSET_Y + y * BLOCK_SIZE,
                          BLOCK_SIZE-1, BLOCK_SIZE-1, col);
        } else {
          // Можно нарисовать сетку
          M5.Lcd.drawRect(BOARD_OFFSET_X + x * BLOCK_SIZE,
                          BOARD_OFFSET_Y + y * BLOCK_SIZE,
                          BLOCK_SIZE, BLOCK_SIZE, DARKGREY);
        }
      }
    }
    // Рисуем текущую движущуюся фигуру
    for(const auto& offset : currentPiece.blocks) {
      int drawX = currentPiece.x + offset.first;
      int drawY = currentPiece.y + offset.second;
      // Если фигурка частично за пределами поля, пропускаем отрисовку
      if(drawX >= 0 && drawX < BOARD_WIDTH && drawY >= 0 && drawY < BOARD_HEIGHT) {
        uint16_t col = getColor(currentPiece.type);
        M5.Lcd.fillRect(BOARD_OFFSET_X + drawX * BLOCK_SIZE,
                        BOARD_OFFSET_Y + drawY * BLOCK_SIZE,
                        BLOCK_SIZE-1, BLOCK_SIZE-1, col);
      }
    }
  }
  
  // Простейшая функция для выбора цвета блоков по типу фигуры
  uint16_t getColor(int type) {
    // тип: 0..6
    switch(type) {
      case 0: return CYAN;        // I
      case 1: return BLUE;        // J
      case 2: return ORANGE;      // L
      case 3: return YELLOW;      // O
      case 4: return GREEN;       // S
      case 5: return MAGENTA;     // T
      case 6: return RED;         // Z
      default: return WHITE;
    }
  }
};

TetrisGame game;  // Глобальный экземпляр игры

void setup() {
  // Инициализация M5Stack
  M5.begin();
  M5.Lcd.setRotation(1);
  // Задаём фон экрана
  M5.Lcd.fillScreen(BLACK);
}

void loop() {
  // Вызываем обновление логики и отрисовку
  game.update();
  // Небольшая задержка для снижения нагрузки (рекомендуется настроить)
  delay(16);
}
