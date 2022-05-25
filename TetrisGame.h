#pragma once

#include <queue>
#include <unordered_map>
#include <array>
#include <functional>

class TetrisGame
{
public:
    static const int board_height = 22;
    static const int board_width = 10;

    static const int upcoming_board_width = 4;
    static const int upcoming_board_lines_per_piece = 3;
    static const int num_upcoming_pieces_shown = 5;

    enum class PieceType
    {
        I,
        J,
        L,
        O,
        S,
        Z,
        T
    };

    enum class BoardSquareColor
    {
        LIGHT_BLUE,
        DARK_BLUE,
        ORANGE,
        YELLOW,
        GREEN,
        RED,
        MAGENTA,
        EMPTY
    };

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
    bool get_whether_a_piece_is_held();

private:
    using BSC = BoardSquareColor;
    using SquarePosition = std::pair<int, int>;
    using PiecePositions = std::array<SquarePosition, 4>;
    using BoardLine = std::array<BoardSquareColor, board_width>;
    using TetrisBoard = std::array<BoardLine, board_height>;
    using UpcomingPiece = std::array<std::array<BoardSquareColor, upcoming_board_width>, upcoming_board_lines_per_piece>;
    using UpcomingBoard = std::array<UpcomingPiece, num_upcoming_pieces_shown>;

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
        _2,
        _L,
    };

    inline static const std::unordered_map<PieceType, BoardSquareColor> piece_colors = {
        {PieceType::I, BSC::LIGHT_BLUE},
        {PieceType::J, BSC::DARK_BLUE},
        {PieceType::L, BSC::ORANGE},
        {PieceType::O, BSC::YELLOW},
        {PieceType::S, BSC::GREEN},
        {PieceType::Z, BSC::RED},
        {PieceType::T, BSC::MAGENTA},
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
    std::array<PieceType, 7> seven_bag = {PieceType::I, PieceType::J, PieceType::L, PieceType::O, PieceType::S, PieceType::Z, PieceType::T};

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

    inline static const std::unordered_map<PieceType, PiecePositions> falling_piece_initial_positions = {
        {PieceType::I, {{{board_height - 2, 3}, {board_height - 2, 4}, {board_height - 2, 5}, {board_height - 2, 6}}}},
        {PieceType::J, {{{board_height - 1, 3}, {board_height - 2, 3}, {board_height - 2, 4}, {board_height - 2, 5}}}},
        {PieceType::L, {{{board_height - 1, 5}, {board_height - 2, 3}, {board_height - 2, 4}, {board_height - 2, 5}}}},
        {PieceType::O, {{{board_height - 1, 4}, {board_height - 1, 5}, {board_height - 2, 4}, {board_height - 2, 5}}}},
        {PieceType::S, {{{board_height - 1, 4}, {board_height - 1, 5}, {board_height - 2, 3}, {board_height - 2, 4}}}},
        {PieceType::Z, {{{board_height - 1, 3}, {board_height - 1, 4}, {board_height - 2, 4}, {board_height - 2, 5}}}},
        {PieceType::T, {{{board_height - 1, 4}, {board_height - 2, 3}, {board_height - 2, 4}, {board_height - 2, 5}}}}};

    typedef std::unordered_map<std::pair<RotationState, RotationState>, PiecePositions, RotationStatePairHashFunction> OffsetsMap;

    inline static const std::unordered_map<PieceType, OffsetsMap> rotation_offsets_based_on_previous_top_left_square =
        {
            {PieceType::I, {
                               {{RotationState::_0, RotationState::_R}, {{{1, 2}, {0, 2}, {-1, 2}, {-2, 2}}}},
                               {{RotationState::_R, RotationState::_0}, {{{-1, -2}, {-1, -1}, {-1, 0}, {-1, 1}}}},
                               {{RotationState::_R, RotationState::_2}, {{{-2, -2}, {-2, -1}, {-2, 0}, {-2, 1}}}},
                               {{RotationState::_2, RotationState::_R}, {{{2, 2}, {1, 2}, {0, 2}, {-1, 2}}}},
                               {{RotationState::_2, RotationState::_L}, {{{2, 1}, {1, 1}, {0, 1}, {-1, 1}}}},
                               {{RotationState::_L, RotationState::_2}, {{{-2, -1}, {-2, 0}, {-2, 1}, {-2, 2}}}},
                               {{RotationState::_L, RotationState::_0}, {{{-1, -1}, {-1, 0}, {-1, 1}, {-1, 2}}}},
                               {{RotationState::_0, RotationState::_L}, {{{1, 1}, {0, 1}, {-1, 1}, {-2, 1}}}},
                           }},
            {PieceType::J, {
                               {{RotationState::_0, RotationState::_R}, {{{0, 1}, {0, 2}, {-1, 1}, {-2, 1}}}},
                               {{RotationState::_R, RotationState::_0}, {{{0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}}},
                               {{RotationState::_R, RotationState::_2}, {{{-1, -1}, {-1, 0}, {-1, 1}, {-2, 1}}}},
                               {{RotationState::_2, RotationState::_R}, {{{1, 1}, {1, 2}, {0, 1}, {-1, 1}}}},
                               {{RotationState::_2, RotationState::_L}, {{{1, 1}, {0, 1}, {-1, 0}, {-1, 1}}}},
                               {{RotationState::_L, RotationState::_2}, {{{-1, -1}, {-1, 0}, {-1, 1}, {-2, 1}}}},
                               {{RotationState::_L, RotationState::_0}, {{{0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}}},
                               {{RotationState::_0, RotationState::_L}, {{{0, 1}, {-1, 1}, {-2, 0}, {-2, 1}}}},
                           }},
            {PieceType::L, {
                               {{RotationState::_0, RotationState::_R}, {{{0, -1}, {-1, -1}, {-2, -1}, {-2, 0}}}},
                               {{RotationState::_R, RotationState::_0}, {{{0, 1}, {-1, -1}, {-1, 0}, {-1, 1}}}},
                               {{RotationState::_R, RotationState::_2}, {{{-1, -1}, {-1, 0}, {-1, 1}, {-2, -1}}}},
                               {{RotationState::_2, RotationState::_R}, {{{1, 1}, {0, 1}, {-1, 1}, {-1, 2}}}},
                               {{RotationState::_2, RotationState::_L}, {{{1, 0}, {1, 1}, {0, 1}, {-1, 1}}}},
                               {{RotationState::_L, RotationState::_2}, {{{-1, 0}, {-1, 1}, {-1, 2}, {-2, 0}}}},
                               {{RotationState::_L, RotationState::_0}, {{{0, 2}, {-1, 0}, {-1, 1}, {-1, 2}}}},
                               {{RotationState::_0, RotationState::_L}, {{{0, -2}, {0, -1}, {-1, -1}, {-2, -1}}}},
                           }},
            {PieceType::S, {
                               {{RotationState::_0, RotationState::_R}, {{{0, 0}, {-1, 0}, {-1, 1}, {-2, 1}}}},
                               {{RotationState::_R, RotationState::_0}, {{{0, 0}, {0, 1}, {-1, -1}, {-1, 0}}}},
                               {{RotationState::_R, RotationState::_2}, {{{-1, 0}, {-1, 1}, {-2, -1}, {-2, 0}}}},
                               {{RotationState::_2, RotationState::_R}, {{{1, 0}, {0, 0}, {0, 1}, {-1, 1}}}},
                               {{RotationState::_2, RotationState::_L}, {{{1, -1}, {0, -1}, {0, 0}, {-1, 0}}}},
                               {{RotationState::_L, RotationState::_2}, {{{-1, 1}, {-1, 2}, {-2, 0}, {-2, 1}}}},
                               {{RotationState::_L, RotationState::_0}, {{{0, 1}, {0, 2}, {-1, 0}, {-1, 1}}}},
                               {{RotationState::_0, RotationState::_L}, {{{0, -1}, {-1, -1}, {-1, 0}, {-2, 0}}}},
                           }},
            {PieceType::Z, {
                               {{RotationState::_0, RotationState::_R}, {{{0, 2}, {-1, 1}, {-1, 2}, {-2, 1}}}},
                               {{RotationState::_R, RotationState::_0}, {{{0, -2}, {0, -1}, {-1, -1}, {-1, 0}}}},
                               {{RotationState::_R, RotationState::_2}, {{{-1, -2}, {-1, -1}, {-2, -1}, {-2, 0}}}},
                               {{RotationState::_2, RotationState::_R}, {{{1, 2}, {0, 1}, {0, 2}, {-1, 1}}}},
                               {{RotationState::_2, RotationState::_L}, {{{1, 1}, {0, 0}, {0, 1}, {-1, 0}}}},
                               {{RotationState::_L, RotationState::_2}, {{{-1, -1}, {-1, 0}, {-2, 0}, {-2, 1}}}},
                               {{RotationState::_L, RotationState::_0}, {{{0, -1}, {0, 0}, {-1, 0}, {-1, 1}}}},
                               {{RotationState::_0, RotationState::_L}, {{{0, 1}, {-1, 0}, {-1, 1}, {-2, 0}}}},
                           }},
            {PieceType::T, {
                               {{RotationState::_0, RotationState::_R}, {{{0, 0}, {-1, 0}, {-1, 1}, {-2, 0}}}},
                               {{RotationState::_R, RotationState::_0}, {{{0, 0}, {-1, -1}, {-1, 0}, {-1, 1}}}},
                               {{RotationState::_R, RotationState::_2}, {{{-1, -1}, {-1, 0}, {-1, 1}, {-2, 0}}}},
                               {{RotationState::_2, RotationState::_R}, {{{1, 1}, {0, 1}, {0, 2}, {0, 2}}}},
                               {{RotationState::_2, RotationState::_L}, {{{1, 1}, {0, 0}, {0, 1}, {-1, 1}}}},
                               {{RotationState::_L, RotationState::_2}, {{{-1, -1}, {-1, 0}, {-1, 1}, {-2, 0}}}},
                               {{RotationState::_L, RotationState::_0}, {{{0, 0}, {-1, -1}, {-1, 0}, {-1, 1}}}},
                               {{RotationState::_0, RotationState::_L}, {{{0, 0}, {-1, -1}, {-1, 0}, {-2, 0}}}},
                           }},
    };

    inline static const OffsetsMap I_kick_offsets =
        {
            {{RotationState::_0, RotationState::_R}, {{{-2, 0}, {1, 0}, {-2, -1}, {1, 2}}}},
            {{RotationState::_R, RotationState::_0}, {{{2, 0}, {-1, 0}, {2, 1}, {-1, -2}}}},
            {{RotationState::_R, RotationState::_2}, {{{-1, 0}, {2, 0}, {-1, 2}, {2, -1}}}},
            {{RotationState::_2, RotationState::_R}, {{{1, 0}, {-2, 0}, {1, -2}, {-2, 1}}}},
            {{RotationState::_2, RotationState::_L}, {{{2, 0}, {-1, 0}, {2, 1}, {-1, -2}}}},
            {{RotationState::_L, RotationState::_2}, {{{-2, 0}, {1, 0}, {-2, -1}, {1, 2}}}},
            {{RotationState::_L, RotationState::_0}, {{{1, 0}, {-2, 0}, {1, -2}, {-2, 1}}}},
            {{RotationState::_0, RotationState::_L}, {{{-1, 0}, {2, 0}, {-1, 2}, {2, -1}}}},
    };

    inline static const OffsetsMap standard_kick_offsets =
        {
            {{RotationState::_0, RotationState::_R}, {{{-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}}},
            {{RotationState::_R, RotationState::_0}, {{{1, 0}, {1, -1}, {0, 2}, {1, 2}}}},
            {{RotationState::_R, RotationState::_2}, {{{1, 0}, {1, -1}, {0, 2}, {1, 2}}}},
            {{RotationState::_2, RotationState::_R}, {{{-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}}},
            {{RotationState::_2, RotationState::_L}, {{{1, 0}, {1, 1}, {0, -2}, {1, -2}}}},
            {{RotationState::_L, RotationState::_2}, {{{-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}}},
            {{RotationState::_L, RotationState::_0}, {{{-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}}},
            {{RotationState::_0, RotationState::_L}, {{{1, 0}, {1, 1}, {0, -2}, {1, -2}}}},
    };

    inline static const BoardLine empty_line = {{BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}};

    TetrisBoard board = {{
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
        empty_line,
    }};

    inline static const UpcomingPiece upcoming_I = {{
        {{BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}},
        {{piece_colors.at(PieceType::I), piece_colors.at(PieceType::I), piece_colors.at(PieceType::I), piece_colors.at(PieceType::I)}},
        {{BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}},
    }};

    inline static const UpcomingPiece upcoming_O = {{
        {{BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}},
        {{BSC::EMPTY, piece_colors.at(PieceType::O), piece_colors.at(PieceType::O), BSC::EMPTY}},
        {{BSC::EMPTY, piece_colors.at(PieceType::O), piece_colors.at(PieceType::O), BSC::EMPTY}},
    }};

    inline static const UpcomingPiece upcoming_J = {{
        {{BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}},
        {{piece_colors.at(PieceType::J), piece_colors.at(PieceType::J), piece_colors.at(PieceType::J), BSC::EMPTY}},
        {{piece_colors.at(PieceType::J), BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}},
    }};

    inline static const UpcomingPiece upcoming_L = {{
        {{BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}},
        {{piece_colors.at(PieceType::L), piece_colors.at(PieceType::L), piece_colors.at(PieceType::L), BSC::EMPTY}},
        {{BSC::EMPTY, BSC::EMPTY, piece_colors.at(PieceType::L), BSC::EMPTY}},
    }};

    inline static const UpcomingPiece upcoming_S = {{
        {{BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}},
        {{piece_colors.at(PieceType::S), piece_colors.at(PieceType::S), BSC::EMPTY, BSC::EMPTY}},
        {{BSC::EMPTY, piece_colors.at(PieceType::S), piece_colors.at(PieceType::S), BSC::EMPTY}},
    }};

    inline static const UpcomingPiece upcoming_Z = {{
        {{BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}},
        {{BSC::EMPTY, piece_colors.at(PieceType::Z), piece_colors.at(PieceType::Z), BSC::EMPTY}},
        {{piece_colors.at(PieceType::Z), piece_colors.at(PieceType::Z), BSC::EMPTY, BSC::EMPTY}},
    }};

    inline static const UpcomingPiece upcoming_T = {{
        {{BSC::EMPTY, BSC::EMPTY, BSC::EMPTY, BSC::EMPTY}},
        {{piece_colors.at(PieceType::T), piece_colors.at(PieceType::T), piece_colors.at(PieceType::T), BSC::EMPTY}},
        {{BSC::EMPTY, piece_colors.at(PieceType::T), BSC::EMPTY, BSC::EMPTY}},
    }};

    inline static const std::unordered_map<PieceType, UpcomingPiece> upcoming_map = {
        {PieceType::I, upcoming_I},
        {PieceType::O, upcoming_O},
        {PieceType::J, upcoming_J},
        {PieceType::L, upcoming_L},
        {PieceType::S, upcoming_S},
        {PieceType::Z, upcoming_Z},
        {PieceType::T, upcoming_T},
    };

    UpcomingBoard upcoming_board;

    void initialize_game();
    void update_upcoming_board();
    void add_seven_pieces_to_queue();
    std::function<bool(const int, const int)> get_movement_checker_function(const MovementDirection);
    void remove_falling_piece_from_board();
    void add_falling_piece_to_board();
    void clear_any_full_lines();
    bool line_is_full(int);
    void remove_lines(std::vector<int>);
    void add_empty_lines(int);
    void add_next_piece_to_board();
    void add_piece_to_board(PieceType);
    void initialize_falling_piece_positions(const PieceType);
    bool move_falling_piece_if_possible(MovementDirection);
    PiecePositions get_moved_positions(MovementDirection);
    std::function<SquarePosition(SquarePosition)> get_position_mover_function(MovementDirection);
    void set_positions_to_color(const PiecePositions, const BoardSquareColor);
    void set_falling_piece_positions_to_one_lower();
    void get_rotated_positions_and_state(PiecePositions &, RotationState &, RotationDirection);
    RotationState get_new_rotation_state(RotationDirection);
    void rotate_falling_piece(RotationDirection);
    void set_falling_piece_positions_to_rotated_values(RotationDirection);
    PiecePositions get_kicked_positions(PiecePositions, int, int);
    bool test_and_set_new_positions_and_state(PiecePositions, RotationState);
    bool test_and_set_new_positions(PiecePositions);
    bool positions_are_valid(PiecePositions);
    OffsetsMap get_offsets_map_for_piece_type(PieceType);
};