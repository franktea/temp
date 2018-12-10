#ifndef CHESSGRID_H
#define CHESSGRID_H
#include <iostream>
#include <vector>
#include <utility>
#include <cstring>
#include <stack>
#include <algorithm>
#include <iomanip>
#include <random>

using namespace std;

static const int dx[] = { 1,  2, 2, 1, -1, -2, -2, -1};
static const int dy[] = {-2, -1, 1, 2,  2,  1, -1, -2};

struct StackItem
{
    StackItem(int xx, int yy, int l):x(xx), y(yy), level(l)
    {
        memset(visited, 0, sizeof(visited));
    }
    int x;
    int y;
    int level;
    bool visited[8];
    inline int Count()
    {
        int ret = 0;
        for(bool b: visited)
            if(b) ++ret;
        return ret;
    }
};

class ChessGrid
{
    friend class ChessBoard;
public:
    explicit ChessGrid(int x, int y)
    {
        x0 = x;
        y0 = y;
        level = 1;
        current_x = x;
        current_y = y;
        memset(grid, 0, sizeof(grid));
        grid[y0][x0] = level;
        path.push(StackItem(x, y, 1));
        points.push_back(std::make_pair(x0, y0));
    }

    bool Finished()
    {
        if(path.empty())
            return true;

        if(path.top().level >= 90)
        {
            for(int y = 0; y < 10; ++y)
            {
                for(int x = 0; x < 9; ++x)
                {
                    cout<<std::setw(2)<<grid[y][x]<<" ";
                }
                cout<<"\n";
            }
            return true;
        }

        return false;
    }

    void Step()
    {
        if(path.empty())
        {
            cout<<"empty path!!!!\n";
            return;
        }
        StackItem& si = path.top();
        //cout<<si.x<<", "<<si.y<<", level="<<si.level<<", index="<<si.Count()<<"\n";
        int next_index = NextIndex(si);
        if(next_index < 0)
        {
            grid[si.y][si.x] = 0;
            path.pop();
            points.pop_back();
            return;
        }
        si.visited[next_index] = true;
        StackItem new_item(si.x + dx[next_index], si.y + dy[next_index], si.level+1);
        path.push(new_item);
        grid[new_item.y][new_item.x] = new_item.level;
        points.push_back(std::make_pair(new_item.x, new_item.y));
    }
private:
    int NextIndex(StackItem& si)
    {
        int ret = -1;
        int max_way = 100;
        static int arr[] = {0, 1, 2, 3, 4, 5, 6, 7};
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(begin(arr), end(arr), g);

        for(int index = 0; index < 8; ++index)
        {
            int i = arr[index];
            int x = si.x + dx[i];
            int y = si.y + dy[i];
            if(x >= 0 && x < 9 && y >= 0 && y < 10 && !grid[y][x] && !si.visited[i])
            {
                return i;
                /*
                int wo = WayOut(x, y);
                if(wo < max_way)
                {
                    max_way = wo;
                    ret = i;
                }*/
            }
        }

        return ret;
    }
    int WayOut(int xvalue, int yvalue)
    {
        int ret = 0;
        for(int i = 0; i < 8; ++i)
        {
            int x = xvalue + dx[i];
            int y = yvalue + dy[i];
            if(x >= 0 && x < 9 && y >= 0 && y < 10 && !grid[y][x])
            {
                ++ret;
            }
        }
        return ret;
    }
private:
    int x0;
    int y0;
    int level;
    int current_x;
    int current_y;
    int grid[10][9];
    stack<StackItem> path;
    vector<std::pair<int, int>> points;
};

#endif // CHESSGRID_H
