# code.py
import time
import random
import board
import displayio
from adafruit_display_shapes.rect import Rect
import digitalio

# ----- Конфигурация поля и графики -----
BOARD_WIDTH = 10
BOARD_HEIGHT = 20
BLOCK_SIZE = 12          # размер одного блока (в пикселях)
BOARD_OFFSET_X = 20      # смещение игрового поля по X
BOARD_OFFSET_Y = 20      # смещение игрового поля по Y

# Определение цветов для фигур (в формате 0xRRGGBB)
COLORS = [
    0x00FFFF,  # I – Циан
    0x0000FF,  # J – Синий
    0xFFA500,  # L – Оранжевый
    0xFFFF00,  # O – Желтый
    0x00FF00,  # S – Зеленый
    0xFF00FF,  # T – Магента
    0xFF0000,  # Z – Красный
]

BACKGROUND_COLOR = 0x000000
GRID_COLOR = 0x404040

# ----- Настройка кнопок ввода ----- 
# Предполагается, что на плате определены board.BUTTON_A, BUTTON_B, board.BUTTON_C 
button_a = digitalio.DigitalInOut(board.BUTTON_A)
button_a.direction = digitalio.Direction.INPUT
button_a.pull = digitalio.Pull.UP

button_b = digitalio.DigitalInOut(board.BUTTON_B)
button_b.direction = digitalio.Direction.INPUT
button_b.pull = digitalio.Pull.UP

button_c = digitalio.DigitalInOut(board.BUTTON_C)
button_c.direction = digitalio.Direction.INPUT
button_c.pull = digitalio.Pull.UP

# ----- Инициализация displayio -----
display = board.DISPLAY

# Создаем корневую группу для отрисовки
root_group = displayio.Group()
display.show(root_group)

# ----- Определение фигур тетромино -----
# Каждая фигура задается списком смещений относительно центра (pivot)
# Порядок: I, J, L, O, S, T, Z
TETROMINOES = [
    [(-2, 0), (-1, 0), (0, 0), (1, 0)],         # I
    [(-1, -1), (-1, 0), (0, 0), (1, 0)],          # J
    [(1, -1), (-1, 0), (0, 0), (1, 0)],           # L
    [(0, 0), (1, 0), (0, 1), (1, 1)],             # O
    [(0, 0), (1, 0), (-1, 1), (0, 1)],            # S
    [(-1, 0), (0, 0), (1, 0), (0, 1)],            # T
    [(-1, 0), (0, 0), (0, 1), (1, 1)],            # Z
]

# ----- Определение вспомогательных структур -----
# Класс TetrisPiece – описывает текущую падающую фигуру.
class TetrisPiece:
    def __init__(self, type_index: int, blocks: list[tuple[int,int]]):
        self.type = type_index  # тип фигуры (0..6)
        # Копия списка смещений (чтобы не менять шаблон)
        self.blocks = [ (x, y) for (x, y) in blocks ]
        # Начальные координаты фигуры на игровом поле
        self.x = BOARD_WIDTH // 2
        self.y = 0

    def rotate(self):
        # Фигура O (тип 3) не вращается
        if self.type == 3:
            return
        new_blocks = []
        for (x, y) in self.blocks:
            # Поворот на 90 градусов по часовой стрелке: (x, y) -> (y, -x)
            new_blocks.append( (y, -x) )
        self.blocks = new_blocks

    def rotated_copy(self):
        if self.type == 3:
            return self
        new_blocks = []
        for (x, y) in self.blocks:
            new_blocks.append( (y, -x) )
        new_piece = TetrisPiece(self.type, new_blocks)
        new_piece.x = self.x
        new_piece.y = self.y
        return new_piece

# Класс TetrisGame – логика игры и отрисовка.
class TetrisGame:
    def __init__(self):
        # Игровое поле представлено как список списков
        self.board = [ [0]*BOARD_WIDTH for _ in range(BOARD_HEIGHT) ]
        self.current_piece = None
        self.drop_interval = 0.5  # интервал падения в секундах
        self.last_drop_time = time.monotonic()
        self.spawn_new_piece()

    def reset_board(self):
        for y in range(BOARD_HEIGHT):
            for x in range(BOARD_WIDTH):
                self.board[y][x] = 0

    def is_valid_position(self, piece: TetrisPiece) -> bool:
        # Проверка, что каждый блок фигуры находится в игровом поле и не пересекается
        for (dx, dy) in piece.blocks:
            new_x = piece.x + dx
            new_y = piece.y + dy
            if new_x < 0 or new_x >= BOARD_WIDTH or new_y < 0 or new_y >= BOARD_HEIGHT:
                return False
            if self.board[new_y][new_x] != 0:
                return False
        return True

    def fix_piece(self):
        # Фиксируем фигуру на игровом поле. Значение ячейки – тип фигуры (от 1 до 7)
        for (dx, dy) in self.current_piece.blocks:
            new_x = self.current_piece.x + dx
            new_y = self.current_piece.y + dy
            if 0 <= new_x < BOARD_WIDTH and 0 <= new_y < BOARD_HEIGHT:
                self.board[new_y][new_x] = self.current_piece.type + 1
        self.clear_lines()
        self.spawn_new_piece()

    def clear_lines(self):
        # Удаление полностью заполненных строк
        y = BOARD_HEIGHT - 1
        while y >= 0:
            if all(self.board[y][x] != 0 for x in range(BOARD_WIDTH)):
                # Удаляем строку и добавляем пустую вверху
                del self.board[y]
                self.board.insert(0, [0]*BOARD_WIDTH)
            else:
                y -= 1

    def spawn_new_piece(self):
        type_index = random.randrange(len(TETROMINOES))
        self.current_piece = TetrisPiece(type_index, TETROMINOES[type_index])
        self.current_piece.x = BOARD_WIDTH // 2
        self.current_piece.y = 0
        # Если новая фигура не может быть размещена – игра окончена, сброс (здесь можно добавить логику Game Over)
        if not self.is_valid_position(self.current_piece):
            self.reset_board()

    def move_piece(self, dx: int, dy: int):
        new_piece = TetrisPiece(self.current_piece.type, self.current_piece.blocks)
        new_piece.x = self.current_piece.x + dx
        new_piece.y = self.current_piece.y + dy
        if self.is_valid_position(new_piece):
            self.current_piece = new_piece
        else:
            # Если двигаться вниз не получается – зафиксировать фигуру
            if dy == 1:
                self.fix_piece()

    def rotate_piece(self):
        new_piece = self.current_piece.rotated_copy()
        if self.is_valid_position(new_piece):
            self.current_piece = new_piece

    def handle_input(self):
        # Если кнопка нажата (при pull-up значение LOW -> False)
        if not button_a.value:
            self.rotate_piece()
            time.sleep(0.2)  # задержка для предотвращения «дребезга»
        if not button_b.value:
            self.move_piece(-1, 0)
            time.sleep(0.15)
        if not button_c.value:
            self.move_piece(1, 0)
            time.sleep(0.15)

    def update(self):
        # Обработка ввода
        self.handle_input()

        # Автоматическое падение фигуры
        now = time.monotonic()
        if now - self.last_drop_time > self.drop_interval:
            self.move_piece(0, 1)
            self.last_drop_time = now

        self.draw()

    def draw(self):
        # Очищаем корневую группу
        while len(root_group) > 0:
            root_group.pop()

        # Фон экрана – прямоугольник, покрывающий весь экран
        bg = Rect(0, 0, display.width, display.height, fill=BACKGROUND_COLOR)
        root_group.append(bg)

        # Рисуем фиксированные блоки игрового поля (каждый блок – прямоугольник)
        for y in range(BOARD_HEIGHT):
            for x in range(BOARD_WIDTH):
                if self.board[y][x] != 0:
                    color = COLORS[self.board[y][x]-1]
                    block = Rect(BOARD_OFFSET_X + x * BLOCK_SIZE,
                                 BOARD_OFFSET_Y + y * BLOCK_SIZE,
                                 BLOCK_SIZE - 1, BLOCK_SIZE - 1,
                                 fill=color)
                    root_group.append(block)
                else:
                    # Можно нарисовать сетку: небольшой прямоугольник с обводкой
                    grid_block = Rect(BOARD_OFFSET_X + x * BLOCK_SIZE,
                                      BOARD_OFFSET_Y + y * BLOCK_SIZE,
                                      BLOCK_SIZE, BLOCK_SIZE,
                                      outline=GRID_COLOR)
                    root_group.append(grid_block)

        # Рисуем текущую фигуру
        for (dx, dy) in self.current_piece.blocks:
            draw_x = self.current_piece.x + dx
            draw_y = self.current_piece.y + dy
            # Если блок частично за пределами видимой области – пропускаем отрисовку
            if 0 <= draw_x < BOARD_WIDTH and 0 <= draw_y < BOARD_HEIGHT:
                color = COLORS[self.current_piece.type]
                piece_block = Rect(BOARD_OFFSET_X + draw_x * BLOCK_SIZE,
                                   BOARD_OFFSET_Y + draw_y * BLOCK_SIZE,
                                   BLOCK_SIZE - 1, BLOCK_SIZE - 1,
                                   fill=color)
                root_group.append(piece_block)

# ----- Создаем экземпляр игры -----
game = TetrisGame()

# ----- Основной цикл игры -----
while True:
    game.update()
    time.sleep(0.016)  # Задержка ~ 60 кадров в секунду
