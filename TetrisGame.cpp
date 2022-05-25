#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>

#include "TetrisGame.h"

template <typename T>
T top_and_pop(std::queue<T> &queue)
{
    T top = queue.front();
    queue.pop();
    return top;
}

TetrisGame::TetrisGame()
{
    initialize_game();
}

int TetrisGame::get_score()
{
    return score;
}

PieceType TetrisGame::get_held_piece()
{
    return held_piece;
}

bool TetrisGame::get_whether_a_piece_is_held()
{
    return a_piece_is_held;
}

void TetrisGame::initialize_game()
{
    add_seven_pieces_to_queue();
    add_next_piece_to_board();
}

BoardSquareColor TetrisGame::get_square(const int i, const int j)
{
    return board[i][j];
}

BoardSquareColor TetrisGame::get_upcoming_square(const int i, const int j, const int k)
{
    return upcoming_board[i][j][k];
}

void TetrisGame::hold_piece()
{
    if (!a_piece_was_held_this_turn)
    {
        a_piece_was_held_this_turn = true;
        remove_falling_piece_from_board();
        auto prev_piece_type = falling_piece.type;
        if (a_piece_is_held)
        {
            add_piece_to_board(held_piece);
        }
        else
        {
            add_next_piece_to_board();
            a_piece_is_held = true;
        }
        held_piece = prev_piece_type;
    }
}

void TetrisGame::update_upcoming_board()
{
    auto queue_copy = upcoming_pieces;
    for (int i = num_upcoming_pieces_shown - 1; i >= 0; i--)
    {
        upcoming_board[i] = upcoming_map.at(queue_copy.front());
        queue_copy.pop();
    }
}

void TetrisGame::add_seven_pieces_to_queue()
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(seven_bag.begin(), seven_bag.end(), std::default_random_engine(seed));
    for (auto piece : seven_bag)
    {
        upcoming_pieces.push(piece);
    }
}

void TetrisGame::iterate_time()
{
    bool falling_piece_moved_down = move_falling_piece_if_possible(MovementDirection::DOWN);
    if (!falling_piece_moved_down)
    {
        clear_any_full_lines();
        add_next_piece_to_board();
        a_piece_was_held_this_turn = false;
    }

    if (upcoming_pieces.size() <= num_upcoming_pieces_shown)
    {
        add_seven_pieces_to_queue();
    }
}

bool TetrisGame::move_falling_piece_if_possible(MovementDirection direction)
{
    remove_falling_piece_from_board();
    auto positions_to_test = get_moved_positions(direction);
    bool piece_moved = test_and_set_new_positions_and_state(positions_to_test, falling_piece.rotation_state);
    add_falling_piece_to_board();
    return piece_moved;
}

PiecePositions TetrisGame::get_moved_positions(MovementDirection direction)
{
    auto position_modification_function = get_position_modification_function(direction);
    PiecePositions moved_positions;
    for (int x = 0; x < falling_piece.positions.size(); x++)
    {
        moved_positions[x] = position_modification_function(falling_piece.positions[x]);
    }
    return moved_positions;
}

void TetrisGame::remove_falling_piece_from_board()
{
    set_positions_to_color(falling_piece.positions, EMPTY);
}

void TetrisGame::add_falling_piece_to_board()
{
    BoardSquareColor piece_color = piece_colors.at(falling_piece.type);
    set_positions_to_color(falling_piece.positions, piece_color);
}

void TetrisGame::clear_any_full_lines()
{
    std::vector<int> full_lines;
    for (int i = board_height - 1; i >= 0; i--)
    {
        if (line_is_full(i))
        {
            full_lines.push_back(i);
        }
    }

    remove_lines(full_lines);

    int num_lines_cleared = full_lines.size();
    add_empty_lines(num_lines_cleared);
    score += num_lines_cleared;
}

bool TetrisGame::line_is_full(int i)
{
    for (auto square : board[i])
    {
        if (square == EMPTY)
        {
            return false;
        }
    }
    return true;
}

void TetrisGame::remove_lines(std::vector<int> line_nums)
{
    for (auto line_num : line_nums)
    {
        for (int i = line_num + 1; i < board_height; i++)
        {
            for (int j = 0; j < board_width; j++)
            {
                board[i - 1][j] = board[i][j];
            }
        }
    }
}

void TetrisGame::add_empty_lines(int num_lines)
{
    for (int i = board_height - 1; i >= board_height - num_lines; i--)
    {
        board[i] = empty_line;
    }
}

void TetrisGame::add_next_piece_to_board()
{
    PieceType next_piece_type = top_and_pop(upcoming_pieces);
    add_piece_to_board(next_piece_type);
    update_upcoming_board();
}

void TetrisGame::add_piece_to_board(PieceType type)
{
    BoardSquareColor piece_color = piece_colors.at(type);
    falling_piece.type = type;
    falling_piece.rotation_state = RotationState::_0;
    initialize_falling_piece_positions(type);
    set_positions_to_color(falling_piece.positions, piece_color);
}

void TetrisGame::initialize_falling_piece_positions(const PieceType type)
{
    falling_piece.positions = falling_piece_initial_positions.at(type);
}

std::function<SquarePosition(SquarePosition)> TetrisGame::get_position_modification_function(MovementDirection direction)
{
    switch (direction)
    {
    case MovementDirection::LEFT:
        return [](SquarePosition position)
        { return SquarePosition{position.first, position.second - 1}; };
    case MovementDirection::RIGHT:
        return [](SquarePosition position)
        { return SquarePosition{position.first, position.second + 1}; };
    case MovementDirection::DOWN:
        return [](SquarePosition position)
        { return SquarePosition{position.first - 1, position.second}; };
    }
}

void TetrisGame::set_positions_to_color(const PiecePositions positions, const BoardSquareColor color)
{
    for (const auto [i, j] : positions)
    {
        board[i][j] = color;
    }
}

void TetrisGame::handle_left_input()
{
    move_falling_piece_if_possible(MovementDirection::LEFT);
}

void TetrisGame::handle_right_input()
{
    move_falling_piece_if_possible(MovementDirection::RIGHT);
}

void TetrisGame::soft_drop()
{
    iterate_time();
}

void TetrisGame::hard_drop()
{
    while (move_falling_piece_if_possible(MovementDirection::DOWN))
    {
        ;
    }
    iterate_time();
}

void TetrisGame::rotate_left()
{
    rotate_falling_piece(RotationDirection::LEFT);
}

void TetrisGame::rotate_right()
{
    rotate_falling_piece(RotationDirection::RIGHT);
}

void TetrisGame::rotate_falling_piece(RotationDirection direction)
{
    if (falling_piece.type == O)
    {
        return;
    }

    remove_falling_piece_from_board();
    set_falling_piece_positions_to_rotated_values(direction);
    add_falling_piece_to_board();
}

void TetrisGame::set_falling_piece_positions_to_rotated_values(RotationDirection direction)
{
    RotationState current_rotation_state = falling_piece.rotation_state;

    PiecePositions possible_new_positions;
    RotationState possible_new_rotation_state;

    get_rotated_positions_and_state(possible_new_positions, possible_new_rotation_state, direction);

    auto positions_to_test = possible_new_positions;
    if (test_and_set_new_positions_and_state(positions_to_test, possible_new_rotation_state))
    {
        return;
    }

    const auto offsets_map = get_offsets_map_for_piece_type(falling_piece.type);

    for (auto [i, j] : offsets_map.at({current_rotation_state, possible_new_rotation_state}))
    {
        positions_to_test = get_kicked_positions(possible_new_positions, i, j);
        if (test_and_set_new_positions_and_state(positions_to_test, possible_new_rotation_state))
        {
            return;
        }
    }
}

TetrisGame::OffsetsMap TetrisGame::get_offsets_map_for_piece_type(PieceType type)
{
    if (type == I)
    {
        return I_kick_offsets;
    }
    else
    {
        return standard_kick_offsets;
    }
}

bool TetrisGame::test_and_set_new_positions_and_state(PiecePositions positions_to_test, RotationState new_rotation_state)
{
    bool valid = positions_are_valid(positions_to_test);
    if (valid)
    {
        falling_piece.positions = positions_to_test;
        falling_piece.rotation_state = new_rotation_state;
    }
    return valid;
}

PiecePositions TetrisGame::get_kicked_positions(PiecePositions possible_new_positions, int i, int j)
{
    PiecePositions offset_positions;
    int x = 0;
    for (auto [k, l] : possible_new_positions)
    {
        offset_positions[x] = {i + k, j + l};
        x++;
    }
    return offset_positions;
}

bool TetrisGame::positions_are_valid(PiecePositions positions)
{
    for (auto [i, j] : positions)
    {
        if (i < 0 || i >= board_height ||
            j < 0 || j >= board_width ||
            board[i][j] != EMPTY)
        {
            return false;
        }
    }
    return true;
};

void TetrisGame::get_rotated_positions_and_state(PiecePositions &new_positions, RotationState &new_state, RotationDirection direction)
{
    const auto [i, j] = falling_piece.positions[0];

    new_state = get_new_rotation_state(direction);

    auto rotation_offsets = rotation_offsets_based_on_previous_top_left_square.at(falling_piece.type).at({falling_piece.rotation_state, new_state});

    for (int x = 0; x < new_positions.size(); x++)
    {
        const auto [i_offset, j_offset] = rotation_offsets[x];
        new_positions[x] = {i + i_offset, j + j_offset};
    }
}

TetrisGame::RotationState TetrisGame::get_new_rotation_state(RotationDirection direction)
{
    if (direction == RotationDirection::LEFT)
    {
        return static_cast<RotationState>(std::min(static_cast<unsigned int>(falling_piece.rotation_state) - 1, (unsigned)3));
    }
    else
    {
        return static_cast<RotationState>((static_cast<int>(falling_piece.rotation_state) + 1) % 4);
    }
}