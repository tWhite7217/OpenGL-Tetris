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
            // std::cout << "here\n";
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
    // if (game_is_over())
    // {
    //     end_game();
    // } else
    if (falling_piece_can_move(MovementDirection::DOWN))
    {
        move_falling_piece_down();
    }
    else
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

bool TetrisGame::falling_piece_can_move(const MovementDirection direction)
{
    auto square_movement_is_possible = get_checker_function(direction);

    remove_falling_piece_from_board();

    bool result = true;
    for (const auto [i, j] : falling_piece.positions)
    {
        if (square_movement_is_possible(i, j))
        {
            result = false;
            break;
        }
    }

    add_falling_piece_to_board();
    return result;
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

std::function<bool(const int, const int)> TetrisGame::get_checker_function(const MovementDirection direction)
{
    using MD = MovementDirection;

    switch (direction)
    {
    case MD::LEFT:
        return [this](const int i, const int j)
        { return (j == 0) || (board[i][j - 1] != EMPTY); };
    case MD::RIGHT:
        return [this](const int i, const int j)
        { return (j == (board_width - 1)) || (board[i][j + 1] != EMPTY); };
    case MD::DOWN:
        return [this](const int i, const int j)
        { return (i == 0) || (board[i - 1][j] != EMPTY); };
    default:
        exit(1);
    }
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
    switch (type)
    {
    case I:
        falling_piece.positions = {{
            {board_height - 2, 3},
            {board_height - 2, 4},
            {board_height - 2, 5},
            {board_height - 2, 6},
        }};
        break;

    case J:
        falling_piece.positions = {{
            {board_height - 1, 3},
            {board_height - 2, 3},
            {board_height - 2, 4},
            {board_height - 2, 5},
        }};
        break;

    case L:
        falling_piece.positions = {{
            {board_height - 1, 5},
            {board_height - 2, 3},
            {board_height - 2, 4},
            {board_height - 2, 5},
        }};
        break;

    case O:
        falling_piece.positions = {{
            {board_height - 1, 4},
            {board_height - 1, 5},
            {board_height - 2, 4},
            {board_height - 2, 5},
        }};
        break;

    case S:
        falling_piece.positions = {{
            {board_height - 1, 4},
            {board_height - 1, 5},
            {board_height - 2, 3},
            {board_height - 2, 4},
        }};
        break;

    case Z:
        falling_piece.positions = {{
            {board_height - 1, 3},
            {board_height - 1, 4},
            {board_height - 2, 4},
            {board_height - 2, 5},
        }};
        break;

    case T:
        falling_piece.positions = {{
            {board_height - 1, 4},
            {board_height - 2, 3},
            {board_height - 2, 4},
            {board_height - 2, 5},
        }};
        break;

    default:
        break;
    }
}

void TetrisGame::move_falling_piece_down()
{
    remove_falling_piece_from_board();
    for (int x = 0; x < falling_piece.positions.size(); x++)
    {
        falling_piece.positions[x].first--;
    }
    add_falling_piece_to_board();
}

void TetrisGame::move_falling_piece_left()
{
    remove_falling_piece_from_board();
    for (int x = 0; x < falling_piece.positions.size(); x++)
    {
        falling_piece.positions[x].second--;
    }
    add_falling_piece_to_board();
}

void TetrisGame::move_falling_piece_right()
{
    remove_falling_piece_from_board();
    for (int x = 0; x < falling_piece.positions.size(); x++)
    {
        falling_piece.positions[x].second++;
    }
    add_falling_piece_to_board();
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

    if (falling_piece_can_move(MovementDirection::LEFT))
    {
        move_falling_piece_left();
    }
}

void TetrisGame::handle_right_input()
{
    if (falling_piece_can_move(MovementDirection::RIGHT))
    {
        move_falling_piece_right();
    }
}

void TetrisGame::soft_drop()
{
    iterate_time();
}

void TetrisGame::hard_drop()
{
    while (falling_piece_can_move(MovementDirection::DOWN))
    {
        move_falling_piece_down();
    }
    iterate_time();
}

void TetrisGame::rotate_left()
{
    if (falling_piece.type == O)
    {
        return;
    }

    remove_falling_piece_from_board();
    rotate_falling_piece(RotationDirection::LEFT);
    add_falling_piece_to_board();
}

void TetrisGame::rotate_right()
{
    if (falling_piece.type == O)
    {
        return;
    }

    remove_falling_piece_from_board();
    rotate_falling_piece(RotationDirection::RIGHT);
    add_falling_piece_to_board();
}

void TetrisGame::rotate_falling_piece(RotationDirection direction)
{
    RotationState current_rotation_state = falling_piece.rotation_state;

    PiecePositions possible_new_positions;
    RotationState possible_new_rotation_state;

    if (direction == RotationDirection::LEFT)
    {
        get_left_rotated_positions_and_state(possible_new_positions, possible_new_rotation_state);
    }
    else
    {
        get_right_rotated_positions_and_state(possible_new_positions, possible_new_rotation_state);
    }

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
    bool valid = new_positions_are_valid(positions_to_test);
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

bool TetrisGame::new_positions_are_valid(PiecePositions positions)
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

void TetrisGame::get_left_rotated_positions_and_state(PiecePositions &new_positions, RotationState &new_state)
{
    using RS = RotationState;

    auto [i, j] = falling_piece.positions[0];

    switch (falling_piece.type)
    {
    case I:
        int new_j;
        int new_i;
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_L;
            new_j = j + 1;
            new_positions = {{
                {i + 1, new_j},
                {i, new_j},
                {i - 1, new_j},
                {i - 2, new_j},
            }};
            break;

        case RS::_L:
            new_state = RS::_2;
            new_i = i - 2;
            new_positions = {{
                {new_i, j - 1},
                {new_i, j},
                {new_i, j + 1},
                {new_i, j + 2},
            }};
            break;

        case RS::_2:
            new_state = RS::_R;
            new_j = j + 2;
            new_positions = {{
                {i + 2, new_j},
                {i + 1, new_j},
                {i, new_j},
                {i - 1, new_j},
            }};
            break;

        case RS::_R:
            new_state = RS::_0;
            new_i = i - 1;
            new_positions = {{
                {new_i, j - 2},
                {new_i, j - 1},
                {new_i, j},
                {new_i, j + 1},
            }};
            break;

        default:
            break;
        }
        break;

    case J:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_L;
            new_positions = {{
                {i, j + 1},
                {i - 1, j + 1},
                {i - 2, j},
                {i - 2, j + 1},
            }};
            break;

        case RS::_L:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
                {i - 2, j + 1},
            }};
            break;

        case RS::_2:
            new_state = RS::_R;
            new_positions = {{
                {i + 1, j + 1},
                {i + 1, j + 2},
                {i, j + 1},
                {i - 1, j + 1},
            }};
            break;

        case RS::_R:
            new_state = RS::_0;
            new_positions = {{
                {i, j - 1},
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
            }};
            break;

        default:
            break;
        }
        break;

    case L:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_L;
            new_positions = {{
                {i, j - 2},
                {i, j - 1},
                {i - 1, j - 1},
                {i - 2, j - 1},
            }};
            break;

        case RS::_L:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j},
                {i - 1, j + 1},
                {i - 1, j + 2},
                {i - 2, j},
            }};
            break;

        case RS::_2:
            new_state = RS::_R;
            new_positions = {{
                {i + 1, j + 1},
                {i, j + 1},
                {i - 1, j + 1},
                {i - 1, j + 2},
            }};
            break;

        case RS::_R:
            new_state = RS::_0;
            new_positions = {{
                {i, j + 1},
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
            }};
            break;

        default:
            break;
        }
        break;

    case S:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_L;
            new_positions = {{
                {i, j - 1},
                {i - 1, j - 1},
                {i - 1, j},
                {i - 2, j},
            }};
            break;

        case RS::_L:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j + 1},
                {i - 1, j + 2},
                {i - 2, j},
                {i - 2, j + 1},
            }};
            break;

        case RS::_2:
            new_state = RS::_R;
            new_positions = {{
                {i + 1, j},
                {i, j},
                {i, j + 1},
                {i - 1, j + 1},
            }};
            break;

        case RS::_R:
            new_state = RS::_0;
            new_positions = {{
                {i, j},
                {i, j + 1},
                {i - 1, j - 1},
                {i - 1, j},
            }};
            break;

        default:
            break;
        }
        break;

    case Z:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_L;
            new_positions = {{
                {i, j + 1},
                {i - 1, j},
                {i - 1, j + 1},
                {i - 2, j},
            }};
            break;

        case RS::_L:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j - 1},
                {i - 1, j},
                {i - 2, j},
                {i - 2, j + 1},
            }};
            break;

        case RS::_2:
            new_state = RS::_R;
            new_positions = {{
                {i + 1, j + 2},
                {i, j + 1},
                {i, j + 2},
                {i - 1, j + 1},
            }};
            break;

        case RS::_R:
            new_state = RS::_0;
            new_positions = {{
                {i, j - 2},
                {i, j - 1},
                {i - 1, j - 1},
                {i - 1, j},
            }};
            break;

        default:
            break;
        }
        break;

    case T:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_L;
            new_positions = {{
                {i, j},
                {i - 1, j - 1},
                {i - 1, j},
                {i - 2, j},
            }};
            break;

        case RS::_L:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
                {i - 2, j},
            }};
            break;

        case RS::_2:
            new_state = RS::_R;
            new_positions = {{
                {i + 1, j + 1},
                {i, j + 1},
                {i, j + 2},
                {i - 1, j + 1},
            }};
            break;

        case RS::_R:
            new_state = RS::_0;
            new_positions = {{
                {i, j},
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
            }};
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

void TetrisGame::get_right_rotated_positions_and_state(PiecePositions &new_positions, RotationState &new_state)
{
    using RS = RotationState;

    auto [i, j] = falling_piece.positions[0];

    switch (falling_piece.type)
    {
    case I:
        int new_j;
        int new_i;
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_R;
            new_j = j + 2;
            new_positions = {{
                {i + 1, new_j},
                {i, new_j},
                {i - 1, new_j},
                {i - 2, new_j},
            }};
            break;

        case RS::_R:
            new_state = RS::_2;
            new_i = i - 2;
            new_positions = {{
                {new_i, j - 2},
                {new_i, j - 1},
                {new_i, j},
                {new_i, j + 1},
            }};
            break;

        case RS::_2:
            new_state = RS::_L;
            new_j = j + 1;
            new_positions = {{
                {i + 2, new_j},
                {i + 1, new_j},
                {i, new_j},
                {i - 1, new_j},
            }};
            break;

        case RS::_L:
            new_state = RS::_0;
            new_i = i - 1;
            new_positions = {{
                {new_i, j - 1},
                {new_i, j},
                {new_i, j + 1},
                {new_i, j + 2},
            }};
            break;

        default:
            break;
        }
        break;

    case J:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_R;
            new_positions = {{
                {i, j + 1},
                {i, j + 2},
                {i - 1, j + 1},
                {i - 2, j + 1},
            }};
            break;

        case RS::_R:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
                {i - 2, j + 1},
            }};
            break;

        case RS::_2:
            new_state = RS::_L;
            new_positions = {{
                {i + 1, j + 1},
                {i, j + 1},
                {i - 1, j},
                {i - 1, j + 1},
            }};
            break;

        case RS::_L:
            new_state = RS::_0;
            new_positions = {{
                {i, j - 1},
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
            }};
            break;

        default:
            break;
        }
        break;

    case L:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_R;
            new_positions = {{
                {i, j - 1},
                {i - 1, j - 1},
                {i - 2, j - 1},
                {i - 2, j},
            }};
            break;

        case RS::_R:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
                {i - 2, j - 1},
            }};
            break;

        case RS::_2:
            new_state = RS::_L;
            new_positions = {{
                {i + 1, j},
                {i + 1, j + 1},
                {i, j + 1},
                {i - 1, j + 1},
            }};
            break;

        case RS::_L:
            new_state = RS::_0;
            new_positions = {{
                {i, j + 2},
                {i - 1, j},
                {i - 1, j + 1},
                {i - 1, j + 2},
            }};
            break;

        default:
            break;
        }
        break;

    case S:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_R;
            new_positions = {{
                {i, j},
                {i - 1, j},
                {i - 1, j + 1},
                {i - 2, j + 1},
            }};
            break;

        case RS::_R:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j},
                {i - 1, j + 1},
                {i - 2, j - 1},
                {i - 2, j},
            }};
            break;

        case RS::_2:
            new_state = RS::_L;
            new_positions = {{
                {i + 1, j - 1},
                {i, j - 1},
                {i, j},
                {i - 1, j},
            }};
            break;

        case RS::_L:
            new_state = RS::_0;
            new_positions = {{
                {i, j + 1},
                {i, j + 2},
                {i - 1, j},
                {i - 1, j + 1},
            }};
            break;

        default:
            break;
        }
        break;

    case Z:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_R;
            new_positions = {{
                {i, j + 2},
                {i - 1, j + 1},
                {i - 1, j + 2},
                {i - 2, j + 1},
            }};
            break;

        case RS::_R:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j - 2},
                {i - 1, j - 1},
                {i - 2, j - 1},
                {i - 2, j},
            }};
            break;

        case RS::_2:
            new_state = RS::_L;
            new_positions = {{
                {i + 1, j + 1},
                {i, j},
                {i, j + 1},
                {i - 1, j},
            }};
            break;

        case RS::_L:
            new_state = RS::_0;
            new_positions = {{
                {i, j - 1},
                {i, j},
                {i - 1, j},
                {i - 1, j + 1},
            }};
            break;

        default:
            break;
        }
        break;

    case T:
        switch (falling_piece.rotation_state)
        {
        case RS::_0:
            new_state = RS::_R;
            new_positions = {{
                {i, j},
                {i - 1, j},
                {i - 1, j + 1},
                {i - 2, j},
            }};
            break;

        case RS::_R:
            new_state = RS::_2;
            new_positions = {{
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
                {i - 2, j},
            }};
            break;

        case RS::_2:
            new_state = RS::_L;
            new_positions = {{
                {i + 1, j + 1},
                {i, j},
                {i, j + 1},
                {i - 1, j + 1},
            }};
            break;

        case RS::_L:
            new_state = RS::_0;
            new_positions = {{
                {i, j},
                {i - 1, j - 1},
                {i - 1, j},
                {i - 1, j + 1},
            }};
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}
