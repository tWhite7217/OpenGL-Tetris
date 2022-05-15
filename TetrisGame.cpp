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
    BoardSquareColor piece_color = piece_colors.at(falling_piece.type);
    set_positions_to_color(falling_piece.positions, EMPTY);
    if (next_piece_should_drop())
    {
        set_positions_to_color(falling_piece.positions, piece_color);
        add_next_piece_to_board();
    }
    else
    {
        move_falling_piece_down();
        set_positions_to_color(falling_piece.positions, piece_color);
    }

    if (upcoming_pieces.empty())
    {
        add_seven_pieces_to_queue();
    }
}

bool TetrisGame::next_piece_should_drop()
{
    for (const auto [i, j] : falling_piece.positions)
    {
        if (square_cannot_move_down(i, j))
        {
            return true;
        }
    }
    return false;
}

bool TetrisGame::square_cannot_move_down(const int i, const int j)
{
    return (i == 0) || (board[i - 1][j] != EMPTY);
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
    BoardSquareColor piece_color = piece_colors.at(falling_piece.type);
    set_falling_piece_positions_to_one_lower();
    set_positions_to_color(falling_piece.positions, piece_color);
}

void TetrisGame::set_positions_to_color(const PiecePositions positions, const BoardSquareColor color)
{
    for (const auto [i, j] : positions)
    {
        board[i][j] = color;
    }
}

void TetrisGame::set_falling_piece_positions_to_one_lower()
{
    for (int x = 0; x < falling_piece.positions.size(); x++)
    {
        falling_piece.positions[x].first--;
    }
}