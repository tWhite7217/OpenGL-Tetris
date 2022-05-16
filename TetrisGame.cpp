#include <algorithm>
#include <random>
#include <chrono>

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

void TetrisGame::initialize_game()
{
    add_seven_pieces_to_queue();
    add_next_piece_to_board();
}

BoardSquareColor TetrisGame::get_square(const int i, const int j)
{
    return board[i][j];
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
    time++;
    // if (game_is_over())
    // {
    //     end_game();
    // } else
    if (falling_piece_can_move(DOWN))
    {
        move_falling_piece_down();
    }
    else
    {
        add_next_piece_to_board();
    }

    if (upcoming_pieces.empty())
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
    switch (direction)
    {
    case LEFT:
        return [this](const int i, const int j)
        { return (j == 0) || (board[i][j - 1] != EMPTY); };
    case RIGHT:
        return [this](const int i, const int j)
        { return (j == (board_width - 1)) || (board[i][j + 1] != EMPTY); };
    case DOWN:
        return [this](const int i, const int j)
        { return (i == 0) || (board[i - 1][j] != EMPTY); };
    default:
        exit(1);
    }
}

void TetrisGame::add_next_piece_to_board()
{
    PieceType next_piece_type = top_and_pop(upcoming_pieces);
    BoardSquareColor piece_color = piece_colors.at(next_piece_type);
    falling_piece.type = next_piece_type;
    initialize_falling_piece_positions(next_piece_type);
    set_positions_to_color(falling_piece.positions, piece_color);
}

void TetrisGame::initialize_falling_piece_positions(const PieceType type)
{
    switch (type)
    {
    case I:
        falling_piece.positions = {
            {board_height - 2, 3},
            {board_height - 2, 4},
            {board_height - 2, 5},
            {board_height - 2, 6},
        };
        break;

    case J:
        falling_piece.positions = {
            {board_height - 1, 3},
            {board_height - 2, 3},
            {board_height - 2, 4},
            {board_height - 2, 5},
        };
        break;

    case L:
        falling_piece.positions = {
            {board_height - 1, 5},
            {board_height - 2, 3},
            {board_height - 2, 4},
            {board_height - 2, 5},
        };
        break;

    case O:
        falling_piece.positions = {
            {board_height - 1, 4},
            {board_height - 1, 5},
            {board_height - 2, 4},
            {board_height - 2, 5},
        };
        break;

    case S:
        falling_piece.positions = {
            {board_height - 1, 4},
            {board_height - 1, 5},
            {board_height - 2, 3},
            {board_height - 2, 4},
        };
        break;

    case Z:
        falling_piece.positions = {
            {board_height - 1, 3},
            {board_height - 1, 4},
            {board_height - 2, 4},
            {board_height - 2, 5},
        };
        break;

    case T:
        falling_piece.positions = {
            {board_height - 1, 4},
            {board_height - 2, 3},
            {board_height - 2, 4},
            {board_height - 2, 5},
        };
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

    if (falling_piece_can_move(LEFT))
    {
        move_falling_piece_left();
    }
}

void TetrisGame::handle_right_input()
{
    if (falling_piece_can_move(RIGHT))
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
    while (falling_piece_can_move(DOWN))
    {
        move_falling_piece_down();
    }
    iterate_time();
}
