#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <windows.h>

class Shape {
private:
    int x;
    int y;
    char type;

public:
    Shape(int x = 0, int y = 0, char type = ' ');

    int get_x() const { return x; }
    int get_y() const { return y; }
    char get_type() const { return type; }
    void set_type(char c) { type = c; }

    Shape shift_copy(int x_shift, int y_shift) const;
    void move(int x_move, int y_move);
    void set_coordinate(int x_new, int y_new);

    bool operator==(const Shape& rhs) const;
};

Shape::Shape(int x, int y, char type)
    : x{ x }, y{ y }, type{ type } {}

bool Shape::operator==(const Shape& rhs) const {
    return (x == rhs.get_x() && y == rhs.get_y());
}

Shape Shape::shift_copy(int x_shift, int y_shift) const {
    return Shape(x + x_shift, y + y_shift, type);
}

void Shape::move(int x_move, int y_move) {
    x += x_move;
    y += y_move;
}

void Shape::set_coordinate(int x_new, int y_new) {
    x = x_new;
    y = y_new;
}

enum piece_type {
    t_piece,
    i_piece,
    o_piece,
    l_piece,
    j_piece,
    s_piece,
    z_piece
};

enum move_direction {
    right = 1,
    left = -1
};

class Piece {
private:
    piece_type type;
    std::vector<Shape> body;
    Shape pos;

public:
    Piece() = default;
    Piece(Shape pos);
    void fall_down();
    void move(move_direction dir);
    void rotate();

    const std::vector<Shape>& get_body() const { return body; }
};

Piece::Piece(Shape pos)
    : pos{ pos } {
    srand(static_cast<unsigned>(time(0)));
    type = static_cast<piece_type>(rand() % 7);

    switch (type) {
    case t_piece:
        body = { pos, pos.shift_copy(1, 0), pos.shift_copy(-1, 0), pos.shift_copy(0, 1) };
        break;
    case i_piece:
        body = { pos, pos.shift_copy(0, -1), pos.shift_copy(0, 1), pos.shift_copy(0, 2) };
        break;
    case o_piece:
        body = { pos, pos.shift_copy(0, 1), pos.shift_copy(1, 0), pos.shift_copy(1, 1) };
        break;
    case l_piece:
        body = { pos, pos.shift_copy(0, 1), pos.shift_copy(0, -1), pos.shift_copy(1, -1) };
        break;
    case j_piece:
        body = { pos, pos.shift_copy(-1, 0), pos.shift_copy(0, 1), pos.shift_copy(0, 2) };
        break;
    case s_piece:
        body = { pos, pos.shift_copy(-1, 0), pos.shift_copy(0, 1), pos.shift_copy(1, 1) };
        break;
    case z_piece:
        body = { pos, pos.shift_copy(1, 0), pos.shift_copy(0, 1), pos.shift_copy(-1, 1) };
        break;
    }
}

void Piece::rotate() {
    for (auto& point : body) {
        int point_x = point.get_x() - pos.get_x();
        int point_y = point.get_y() - pos.get_y();
        point.set_coordinate((-1 * point_y) + pos.get_x(), point_x + pos.get_y());
    }
}

void Piece::fall_down() {
    pos.move(0, -1);
    for (auto& point : body)
        point.move(0, -1);
}

void Piece::move(move_direction dir) {
    pos.move(dir, 0);
    for (auto& point : body)
        point.move(dir, 0);
}

class Board {
private:
    int width;
    int height;
    std::vector<Shape> all_points;
    std::vector<Shape> built_points;

public:
    Board(int width = 30, int height = 30);

    int get_width() const { return width; }
    int get_height() const { return height; }
    const std::vector<Shape>& get_all_points() const { return all_points; }
    const std::vector<Shape>& get_built_points() const { return built_points; }

    void set_built_points(const std::vector<Shape>& built_points);
    void insert_to_built_points(const std::vector<Shape>& insert_points);
    int remove_row();
    void refresh();
};

Board::Board(int width, int height)
    : width{ width }, height{ height } {
    for (int i{ 0 }; i < width; i++)
        for (int j{ 0 }; j < height; j++)
            all_points.push_back(Shape(i, j));
}

void Board::refresh() {
    for (auto& all_pnt : all_points) {
        all_pnt.set_type(' ');
        for (auto& built_pnt : built_points) {
            if (all_pnt == built_pnt) {
                all_pnt = built_pnt;
                break;
            }
        }
    }
}

void Board::insert_to_built_points(const std::vector<Shape>& insert_points) {
    built_points.insert(built_points.end(), insert_points.begin(), insert_points.end());
}

int Board::remove_row() {
    int removed_rows{ 0 };
    int i{ 1 };
    while (i < height) {
        int built_points_count = std::count_if(built_points.begin(), built_points.end(), [i](const Shape& point) {
            return (point.get_y() == i);
            });

        if (built_points_count == (width - 2)) {
            removed_rows++;

            // erase-remove idiom
            auto it = std::remove_if(built_points.begin(), built_points.end(), [i](Shape point) {
                return (point.get_y() == i);
                });
            built_points.erase(it, built_points.end());

            std::for_each(built_points.begin(), built_points.end(), [i](Shape& point) {
                if (point.get_y() > i)
                    point.move(0, -1);
                });
        }
        else
            i++;
    }

    return removed_rows;
}

class Game {
private:
    Board board;
    Piece curr_piece;
    std::vector<Shape> final_points;
    int score;
    int speed;
    std::string playerName;
    const int DOWN = VK_DOWN;
    const int UP = VK_UP;
    const int RIGHT = VK_RIGHT;
    const int LEFT = VK_LEFT;


    void controls();
    void running();
    bool hit_built_points_down();
    bool checked_collision(const Piece& piece);
    void draw();
    void refresh_final_points();
    void set_border();
    void ClearScreen();
    bool game_over();
    bool regame();

public:
    Game();
    void start_game();
};

Game::Game()
    : score{ 0 } {
    std::cout << "Enter your name: ";
    std::cin >> playerName;
}

void Game::start_game() {
    do {
        system("cls");
        board = Board(20, 20);
        running();
    } while (regame());
}

void Game::running() {
    while (!game_over()) {
        curr_piece = Piece(Shape((board.get_width() - 1) / 2, board.get_height(), 'O'));
        while (!hit_built_points_down()) {
            speed = 200;
            ClearScreen();
            curr_piece.fall_down();
            score += board.remove_row();
            board.refresh();
            refresh_final_points();
            draw();
            controls();
            Sleep(speed);
        }
    }
}

bool Game::game_over() {
    for (const auto& pnt : board.get_built_points())
        if (pnt.get_y() >= board.get_height() - 2) {
            return true;
        }
    return false;
}

bool Game::regame() {
    std::cout << "======Game over======" << std::endl;
    std::cout << "replay ?? (y/n) " << std::endl;
    char c{};
    bool invalid{ false };
    do {
        std::cin >> c;
        if (c == 'y')
            return true;
        else if (c == 'n')
            return false;
        else {
            std::cout << "invalid entry\n";
            invalid = true;
        }
    } while (invalid);

    return false;
}

bool Game::hit_built_points_down() {
    auto temp_piece = curr_piece;
    temp_piece.fall_down();

    for (const auto& next_piece_pnt : temp_piece.get_body()) {
        if (next_piece_pnt.get_y() == 0 || std::find(board.get_built_points().begin(), board.get_built_points().end(), next_piece_pnt) != board.get_built_points().end()) {
            board.insert_to_built_points(curr_piece.get_body());
            return true;
        }
    }

    return false;
}

bool Game::checked_collision(const Piece& piece) {
    for (const auto& piece_pnt : piece.get_body()) {
        if (piece_pnt.get_x() == 0 || piece_pnt.get_x() == (board.get_width() - 1) || std::find(board.get_built_points().begin(), board.get_built_points().end(), piece_pnt) != board.get_built_points().end()) {
            return false;
        }
    }

    return true;
}

void Game::refresh_final_points() {
    final_points = board.get_all_points();
    for (auto& final_pnt : final_points) {
        for (auto piece_pnt : curr_piece.get_body()) {
            if (final_pnt == piece_pnt) {
                final_pnt = piece_pnt;
                break;
            }
        }
    }

    set_border();
}

void Game::set_border() {
    for (auto& point : final_points) {
        if (point.get_x() == 0 || point.get_y() == 0 || point.get_x() == board.get_width() - 1 || point.get_y() == board.get_height() - 1) {
            point.set_type('-');
        }
    }
}

void Game::draw() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    for (int i{ board.get_height() - 1 }; i >= 0; i--) {
        for (int j{ 0 }; j < board.get_width(); j++) {
            auto t = std::find(final_points.begin(), final_points.end(), Shape(j, i));            
            if (t->get_type() == '-') {
                SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | BACKGROUND_RED);
            }
            else if (t->get_type() == 'O') {
                SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | FOREGROUND_BLUE);
            }
            else {
                SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN);
            }
            std::cout << t->get_type();
        }
        std::cout << std::endl;
    }
    std::cout << "\n " << playerName << "'s Score = " << score << std::endl;
}

void Game::controls() {
    if (GetAsyncKeyState(DOWN))
        speed = 10;
    else if (GetAsyncKeyState(UP)) {
        auto temp_peice = curr_piece;
        temp_peice.rotate();
        if (checked_collision(temp_peice))
            curr_piece.rotate();
    }
    else if (GetAsyncKeyState(RIGHT)) {
        auto temp_peice = curr_piece;
        temp_peice.move(right);
        if (checked_collision(temp_peice))
            curr_piece.move(right);
    }
    else if (GetAsyncKeyState(LEFT)) {
        auto temp_peice = curr_piece;
        temp_peice.move(left);
        if (checked_collision(temp_peice))
            curr_piece.move(left);
    }
}

void Game::ClearScreen() {
    COORD cursorPosition; cursorPosition.X = 0; cursorPosition.Y = 0; SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    Game new_game;
    new_game.start_game();

    return 0;
}
