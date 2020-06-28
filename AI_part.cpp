#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <map>
#include <sstream>
#include <cassert>

struct Point
{
    int x, y;
    Point() : Point(0, 0) {}
    Point(int x, int y) : x(x), y(y) {}
    bool operator==(const Point &rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const Point &rhs) const
    {
        return !operator==(rhs);
    }
    Point operator+(const Point &rhs) const
    {
        return Point(x + rhs.x, y + rhs.y);
    }
    Point operator-(const Point &rhs) const
    {
        return Point(x - rhs.x, y - rhs.y);
    }
};

class OthelloBoard
{
public:
    enum SPOT_STATE
    {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{Point(-1, -1), Point(-1, 0), Point(-1, 1),
                                           Point(0, -1), /*{0, 0}, */ Point(0, 1),
                                           Point(1, -1), Point(1, 0), Point(1, 1)}};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;

private:
    int get_next_player(int player) const
    {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const
    {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const
    {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc)
    {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const
    {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const
    {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir : directions)
        {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY)
            {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center)
    {
        for (Point dir : directions)
        {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY)
            {
                if (is_disc_at(p, cur_player))
                {
                    for (Point s : discs)
                    {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }

public:
    OthelloBoard()
    {
        reset();
    }
    void reset()
    {
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8 * 8 - 4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const
    {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p)
    {
        if (!is_spot_valid(p))
        {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0)
        {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0)
            {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs)
                    winner = EMPTY;
                else if (black_discs > white_discs)
                    winner = BLACK;
                else
                    winner = WHITE;
            }
        }
        return true;
    }
    std::string encode_player(int state)
    {
        if (state == BLACK)
            return "O";
        if (state == WHITE)
            return "X";
        return "Draw";
    }
    std::string encode_spot(int x, int y)
    {
        if (is_spot_valid(Point(x, y)))
            return ".";
        if (board[x][y] == BLACK)
            return "O";
        if (board[x][y] == WHITE)
            return "X";
        return " ";
    }
    std::string encode_output(bool fail = false)
    {
        int i, j;
        std::stringstream ss;
        ss << "Timestep #" << (8 * 8 - 4 - disc_count[EMPTY] + 1) << "\n";
        ss << "O: " << disc_count[BLACK] << "; X: " << disc_count[WHITE] << "\n";
        if (fail)
        {
            ss << "Winner is " << encode_player(winner) << " (Opponent performed invalid move)\n";
        }
        else if (next_valid_spots.size() > 0)
        {
            ss << encode_player(cur_player) << "'s turn\n";
        }
        else
        {
            ss << "Winner is " << encode_player(winner) << "\n";
        }
        ss << "+---------------+\n";
        for (i = 0; i < SIZE; i++)
        {
            ss << "|";
            for (j = 0; j < SIZE - 1; j++)
            {
                ss << encode_spot(i, j) << " ";
            }
            ss << encode_spot(i, j) << "|\n";
        }
        ss << "+---------------+\n";
        ss << next_valid_spots.size() << " valid moves: {";
        if (next_valid_spots.size() > 0)
        {
            Point p = next_valid_spots[0];
            ss << "(" << p.x << "," << p.y << ")";
        }
        for (size_t i = 1; i < next_valid_spots.size(); i++)
        {
            Point p = next_valid_spots[i];
            ss << ", (" << p.x << "," << p.y << ")";
        }
        ss << "}\n";
        ss << "=================\n";
        return ss.str();
    }
    std::string encode_state()
    {
        int i, j;
        std::stringstream ss;
        ss << cur_player << "\n";
        for (i = 0; i < SIZE; i++)
        {
            for (j = 0; j < SIZE - 1; j++)
            {
                ss << board[i][j] << " ";
            }
            ss << board[i][j] << "\n";
        }
        ss << next_valid_spots.size() << "\n";
        for (size_t i = 0; i < next_valid_spots.size(); i++)
        {
            Point p = next_valid_spots[i];
            ss << p.x << " " << p.y << "\n";
        }
        return ss.str();
    }
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board_orgi;
std::vector<Point> next_valid_spots_orgi;

void read_board(std::ifstream &fin)
{
    fin >> player;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            fin >> board_orgi[i][j];
        }
    }
}

void read_valid_spots(std::ifstream &fin)
{
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++)
    {
        fin >> x >> y;
        next_valid_spots_orgi.push_back({x, y});
    }
}

bool is_corner(Point p)
{
    if (p.x == 0 || p.x == 7)
    {
        if (p.y == 0 || p.y == 7)
            return true;
    }
    return false;
}

bool is_deadedge(Point p)
{
    if (p.x == 1 || p.x == 6)
    {
        if (p.y == 0 || p.y == 7)
            return true;
    }
    else if (p.y == 1 || p.y == 6)
    {
        if (p.x == 0 || p.x == 7)
            return true;
    }
    return false;
}

bool is_edge(Point p)
{
    if (p.x > 1 && p.x < 6)
    {
        if (p.y == 0 || p.y == 7)
            return true;
    }
    else if (p.y > 1 && p.y < 6)
    {
        if (p.x == 0 || p.x == 7)
            return true;
    }
    return false;
}

bool is_goodedge(Point p)
{
    if (p.x > 2 && p.x < 5)
    {
        if (p.y == 2 || p.y == 5)
            return true;
    }
    else if (p.y > 2 && p.y < 5)
    {
        if (p.x == 2 || p.x == 5)
            return true;
    }
    return false;
}

bool is_notgoodedge(Point p)
{
    if (p.x > 1 && p.x < 6)
    {
        if (p.y == 1 || p.y == 6)
            return true;
    }
    else if (p.y > 1 && p.y < 6)
    {
        if (p.x == 1 || p.x == 6)
            return true;
    }
    return false;
}

bool is_notgoodcorner(Point p)
{
    if (p.x == 1 && p.y == 1)
        return true;
    else if (p.x == 6 && p.y == 1)
        return true;
    else if (p.x == 1 && p.y == 6)
        return true;
    else if (p.x == 6 && p.y == 6)
        return true;
    return false;
}

bool is_goodcorner(Point p)
{
    if (p.x == 2 && p.y == 5)
        return true;
    else if (p.x == 5 && p.y == 2)
        return true;
    else if (p.x == 2 && p.y == 5)
        return true;
    else if (p.x == 5 && p.y == 5)
        return true;
    return false;
}

int find_heur(OthelloBoard curboard)
{
    int curheur = 0;
    std::vector<Point> curvalid = curboard.get_valid_spots();
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            Point curp(i, j);
            if (is_corner(curp)) //1
                curheur += 500000;
            else if (is_edge(curp)) //2
                curheur += 1000;
            else if (is_notgoodcorner(curp)) //6
                curheur -= 5000;
            else if (is_deadedge(curp)) //6
                curheur -= 5000;
            else if (is_goodcorner(curp)) //4
                curheur += 50;
            else if (is_notgoodedge(curp)) //5
                curheur += 0;
            else if (is_goodedge(curp)) //3
                curheur += 100;
        }
    }
    curheur -= curvalid.size() * 10;
    return curheur;
}

int minimax(OthelloBoard curboard, int depth, int alpha, int beta, int curplayer)
{
    if (depth == 0 || curboard.done)
    {
        return find_heur(curboard);
    }
    if (curboard.cur_player == curplayer)
    {
        int maxheur = -10000000;
        for (int i = 0; i < (int)curboard.get_valid_spots().size(); i++)
        {
            OthelloBoard newboard;
            Point p = curboard.get_valid_spots()[i];
            newboard.board = curboard.board;
            newboard.cur_player = curboard.cur_player;
            newboard.put_disc(p);
            int curheur = minimax(newboard, depth - 1, alpha, beta, curplayer);
            maxheur = std::max(maxheur, curheur);
            alpha = std::max(alpha, curheur);
            if (beta <= alpha)
                break;
        }
        return maxheur;
    }
    else
    {
        int minheur = 10000000;
        for (int i = 0; i < (int)curboard.get_valid_spots().size(); i++)
        {
            OthelloBoard newboard;
            Point p = curboard.get_valid_spots()[i];
            newboard.board = curboard.board;
            newboard.cur_player = curboard.cur_player;
            newboard.put_disc(p);
            int curheur = minimax(newboard, depth - 1, alpha, beta, 3-curplayer);
            if (is_corner(p))
                curheur -= 50000;
            // else if (is_edge(p))
            //     curheur -= 2000;
            // else if (is_notgoodcorner(p))
            //     curheur += 100;
            // else if (is_deadedge(p))
            //     curheur += 100;
            // else if (is_goodcorner(p))
            //     curheur -= 300;
            // else if (is_notgoodedge(p))
            //     curheur -= 200;
            // else if (is_goodedge(p))
            //     curheur -= 400;
            minheur = std::min(minheur, curheur);
            beta = std::min(beta, curheur);
            if (beta <= alpha)
                break;
        }
        return minheur;
    }
}

void write_valid_spot(std::ofstream &fout)
{
    OthelloBoard curboard;
    curboard.board = board_orgi;
    curboard.cur_player = player;
    int n_valid_spots = (int)next_valid_spots_orgi.size();
    int afa = -10000000, beta = 10000000;
    // Choose the spot.
    int index, heur = 0;
    for (int i = 0; i < n_valid_spots; i++)
    {
        OthelloBoard newboard;
        newboard.board = curboard.board;
        newboard.cur_player = curboard.cur_player;
        Point p = newboard.get_valid_spots()[i];
        newboard.put_disc(p);
        int cur_heur = minimax(newboard, 3, afa, beta, player);
        if (cur_heur > heur)
        {
            index = i;
            heur = cur_heur;
        }
    }

    Point p = next_valid_spots_orgi[index];
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char **argv)
{
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
