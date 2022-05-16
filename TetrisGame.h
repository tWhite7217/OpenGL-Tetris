#pragma once

#include <queue>
#include <unordered_map>
#include <array>
#include <functional>

typedef enum
{
    I,
    J,
    L,
    O,
    S,
    Z,
    T
} PieceType;

typedef enum
{
    LIGHT_BLUE,
    DARK_BLUE,
    ORANGE,
    YELLOW,
    GREEN,
    RED,
    MAGENTA,
    EMPTY
} BoardSquareColor;

const int board_height = 22;
const int board_width = 10;

using PiecePositions = std::vector<std::pair<int, int>>;
using TetrisBoard = std::array<std::array<BoardSquareColor, board_width>, board_height>;

class TetrisGame
{
public:
    TetrisGame();
    void iterate_time();
    BoardSquareColor get_square(const int, const int);
    void hard_drop();
    void soft_drop();
    void handle_left_input();
    void handle_right_input();

private:
    typedef enum
    {
        LEFT,
        RIGHT,
        DOWN
    } MovementDirection;

    const std::unordered_map<PieceType, BoardSquareColor> piece_colors = {
        {I, LIGHT_BLUE},
        {J, DARK_BLUE},
        {L, ORANGE},
        {O, YELLOW},
        {S, GREEN},
        {Z, RED},
        {T, MAGENTA},
    };

    struct FallingPiece
    {
        PieceType type;
        PiecePositions positions;
    } falling_piece;

    // int score;
    int time;
    std::queue<PieceType> upcoming_pieces;
    std::array<PieceType, 7> seven_bag = {I, J, L, O, S, Z, T};

    TetrisBoard board = {{
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}},
    }};

    void initialize_game();
    void add_seven_pieces_to_queue();
    bool falling_piece_can_move(const MovementDirection);
    std::function<bool(const int, const int)> get_checker_function(const MovementDirection);
    void remove_falling_piece_from_board();
    void add_falling_piece_to_board();
    void add_next_piece_to_board();
    void initialize_falling_piece_positions(const PieceType);
    void move_falling_piece_down();
    void move_falling_piece_left();
    void move_falling_piece_right();
    void set_positions_to_color(const PiecePositions, const BoardSquareColor);
    void set_falling_piece_positions_to_one_lower();
};