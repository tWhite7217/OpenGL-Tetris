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

const int upcoming_board_width = 4;
const int upcoming_board_lines_per_piece = 3;
const int num_upcoming_pieces_shown = 5;

using PiecePositions = std::array<std::pair<int, int>, 4>;
using BoardLine = std::array<BoardSquareColor, board_width>;
using TetrisBoard = std::array<BoardLine, board_height>;
using UpcomingPiece = std::array<std::array<BoardSquareColor, upcoming_board_width>, upcoming_board_lines_per_piece>;
using UpcomingBoard = std::array<UpcomingPiece, num_upcoming_pieces_shown>;

class TetrisGame
{
public:
    TetrisGame();
    void iterate_time();
    BoardSquareColor get_square(const int, const int);
    BoardSquareColor get_upcoming_square(const int, const int, const int);
    void hard_drop();
    void soft_drop();
    void hold_piece();
    void handle_left_input();
    void handle_right_input();
    void rotate_left();
    void rotate_right();
    int get_score();
    PieceType get_held_piece();
    bool get_if_a_piece_is_held();
    // std::queue<PieceType> get_upcoming_pieces();

private:
    enum class MovementDirection
    {
        LEFT,
        RIGHT,
        DOWN
    };

    enum class RotationDirection
    {
        LEFT,
        RIGHT
    };

    enum class RotationState
    {
        _0,
        _R,
        _L,
        _2
    };

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
        RotationState rotation_state;
    } falling_piece;

    int score = 0;
    bool a_piece_is_held = false;
    bool a_piece_was_held_this_turn = false;
    PieceType held_piece;
    std::queue<PieceType> upcoming_pieces;
    std::array<PieceType, 7> seven_bag = {I, J, L, O, S, Z, T};

    class RotationStatePairHashFunction
    {
    public:
        size_t operator()(const std::pair<RotationState, RotationState> &key) const
        {
            int first_state_as_int = static_cast<int>(key.first);
            int second_state_as_int = static_cast<int>(key.second);
            int hash = first_state_as_int * 4 + second_state_as_int;
            return size_t(hash);
        }
    };

    typedef std::unordered_map<std::pair<RotationState, RotationState>, std::array<std::pair<int, int>, 4>, RotationStatePairHashFunction> OffsetsMap;

    const OffsetsMap I_kick_offsets =
        {
            {{RotationState::_0, RotationState::_R}, {{{-2, 0}, {1, 0}, {-2, -1}, {1, 2}}}},
            {{RotationState::_R, RotationState::_0}, {{{-2, 0}, {-1, 0}, {2, 1}, {-1, -2}}}},
            {{RotationState::_R, RotationState::_2}, {{{-1, 0}, {2, 0}, {-1, 2}, {2, -1}}}},
            {{RotationState::_2, RotationState::_R}, {{{1, 0}, {-2, 0}, {1, -2}, {-2, 1}}}},
            {{RotationState::_2, RotationState::_L}, {{{2, 0}, {-1, 0}, {2, 1}, {-1, -2}}}},
            {{RotationState::_L, RotationState::_2}, {{{-2, 0}, {1, 0}, {-2, -1}, {1, 2}}}},
            {{RotationState::_L, RotationState::_0}, {{{1, 0}, {-2, 0}, {1, -2}, {-2, 1}}}},
            {{RotationState::_0, RotationState::_L}, {{{-1, 0}, {2, 0}, {-1, 2}, {2, -1}}}},
    };

    const OffsetsMap standard_kick_offsets =
        {
            {{RotationState::_0, RotationState::_R}, {{{-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}}},
            {{RotationState::_R, RotationState::_0}, {{{1, 0}, {1, -1}, {0, 2}, {1, 2}}}},
            {{RotationState::_R, RotationState::_2}, {{{1, 0}, {1, -1}, {0, 2}, {1, 2}}}},
            {{RotationState::_2, RotationState::_R}, {{{-1, 0}, {-1, 1}, {0, 2}, {-1, -2}}}},
            {{RotationState::_2, RotationState::_L}, {{{1, 0}, {1, 1}, {0, -2}, {1, -2}}}},
            {{RotationState::_L, RotationState::_2}, {{{-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}}},
            {{RotationState::_L, RotationState::_0}, {{{-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}}},
            {{RotationState::_0, RotationState::_L}, {{{1, 0}, {1, 1}, {0, -2}, {1, -2}}}},
    };

    const BoardLine empty_line = {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}};

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

    const UpcomingPiece upcoming_I = {{
        {{EMPTY, EMPTY, EMPTY, EMPTY}},
        {{piece_colors.at(I), piece_colors.at(I), piece_colors.at(I), piece_colors.at(I)}},
        {{EMPTY, EMPTY, EMPTY, EMPTY}},
    }};

    const UpcomingPiece upcoming_O = {{
        {{EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, piece_colors.at(O), piece_colors.at(O), EMPTY}},
        {{EMPTY, piece_colors.at(O), piece_colors.at(O), EMPTY}},
    }};

    const UpcomingPiece upcoming_J = {{
        {{EMPTY, EMPTY, EMPTY, EMPTY}},
        {{piece_colors.at(J), piece_colors.at(J), piece_colors.at(J), EMPTY}},
        {{piece_colors.at(J), EMPTY, EMPTY, EMPTY}},
    }};

    const UpcomingPiece upcoming_L = {{
        {{EMPTY, EMPTY, EMPTY, EMPTY}},
        {{piece_colors.at(L), piece_colors.at(L), piece_colors.at(L), EMPTY}},
        {{EMPTY, EMPTY, piece_colors.at(L), EMPTY}},
    }};

    const UpcomingPiece upcoming_S = {{
        {{EMPTY, EMPTY, EMPTY, EMPTY}},
        {{piece_colors.at(S), piece_colors.at(S), EMPTY, EMPTY}},
        {{EMPTY, piece_colors.at(S), piece_colors.at(S), EMPTY}},
    }};

    const UpcomingPiece upcoming_Z = {{
        {{EMPTY, EMPTY, EMPTY, EMPTY}},
        {{EMPTY, piece_colors.at(Z), piece_colors.at(Z), EMPTY}},
        {{piece_colors.at(Z), piece_colors.at(Z), EMPTY, EMPTY}},
    }};

    const UpcomingPiece upcoming_T = {{
        {{EMPTY, EMPTY, EMPTY, EMPTY}},
        {{piece_colors.at(T), piece_colors.at(T), piece_colors.at(T), EMPTY}},
        {{EMPTY, piece_colors.at(T), EMPTY, EMPTY}},
    }};

    const std::unordered_map<PieceType, UpcomingPiece> upcoming_map = {
        {I, upcoming_I},
        {O, upcoming_O},
        {J, upcoming_J},
        {L, upcoming_L},
        {S, upcoming_S},
        {Z, upcoming_Z},
        {T, upcoming_T},
    };

    UpcomingBoard upcoming_board;

    void initialize_game();
    void update_upcoming_board();
    void add_seven_pieces_to_queue();
    bool falling_piece_can_move(const MovementDirection);
    std::function<bool(const int, const int)> get_checker_function(const MovementDirection);
    void remove_falling_piece_from_board();
    void add_falling_piece_to_board();
    void clear_any_full_lines();
    bool line_is_full(int);
    void remove_lines(std::vector<int>);
    void add_empty_lines(int);
    void add_next_piece_to_board();
    void add_piece_to_board(PieceType);
    void initialize_falling_piece_positions(const PieceType);
    void move_falling_piece_down();
    void move_falling_piece_left();
    void move_falling_piece_right();
    void set_positions_to_color(const PiecePositions, const BoardSquareColor);
    void set_falling_piece_positions_to_one_lower();
    void get_left_rotated_positions_and_state(PiecePositions &new_positions, RotationState &);
    void get_right_rotated_positions_and_state(PiecePositions &new_positions, RotationState &);
    void rotate_falling_piece(RotationDirection);
    PiecePositions get_kicked_positions(PiecePositions, int, int);
    bool test_and_set_new_positions_and_state(PiecePositions, RotationState);
    bool new_positions_are_valid(PiecePositions);
    OffsetsMap get_offsets_map_for_piece_type(PieceType);
};