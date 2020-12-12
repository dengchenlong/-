#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <stack>
#include <queue>
#include <ctime>
using namespace std;

//定义全局常量

//棋盘行数
const int BOARD_ROWS = 4;
//棋盘列数
const int BOARD_COLS = 4;
//棋盘大小
const int BOARD_SIZE = BOARD_ROWS * BOARD_COLS;
//玩家1胜利标志
const int WIN_1 = 40;
//玩家2胜利标志
const int WIN_2 = 80;

//声明枚举、结构体、类

enum class Policy;
enum class SumType;
enum class PlayType;
struct Value;
struct comp;
struct Action;
class State;
class Player;
class AIPlayer;
class HumanPlayer;

//声明全局函数

void get_all_states_impl(State current_state, int current_symbol);
void get_all_states();
void save_all_states();
void load_all_states();
void train(int print_every_n_epochs, double step_size, Policy policy, double parameter, int time);
void train(int epochs, int print_every_n_epochs, double step_size, Policy policy, double parameter);
ostream& operator<<(ostream& out, Policy& policy);
void test(int epochs, Policy policy, double parameter);
void play(int order, Policy policy, double parameter);

//声明全局变量

extern double train_parameter;
extern double test_parameter;
extern double step_size;
extern int print_every_n_epochs;
extern bool use_simple_rule;
extern bool positive_init;
extern bool parameter_decline;
extern int last_epoch;
extern PlayType play_type;
extern map<int, State> all_states;
extern State temp_state;
extern Action temp_action;
extern Policy policy;

//定义类

//策略枚举
enum class Policy
{
    e_greedy, ucb, boltzmann
};
//求和方式枚举
enum class SumType
{
    ROW, COL, LDIAG, RDIAG
};
//play类型枚举
enum class PlayType
{
    play, train, test
};
//行为结构体
struct Action
{
    int row;
    int col;
    Action() :row(-1), col(-1) {}
    Action(int n)
    {
        row = n / BOARD_COLS;
        col = n % BOARD_COLS;
    }
    Action(int r, int c) :row(r), col(c) {}
    void set(int n)
    {
        row = n / BOARD_COLS;
        col = n % BOARD_COLS;
        return;
    }
    void set(int r, int c)
    {
        row = r;
        col = c;
        return;
    }
    Action& operator=(const Action& a)
    {
        row = a.row;
        col = a.col;
        return *this;
    }
    bool isEmpty()
    {
        return row == -1 && col == -1;
    }
};
//行为值结构体
struct Value
{
    Action action;
    double estimation = 0;
    bool operator<(const Value& a)
    {
        return estimation < a.estimation;
    }
};
struct comp
{
    bool operator()(const Value& a, const Value& b)
    {
        return a.estimation < b.estimation;
    }
};
//状态
class State
{
public:
    //棋盘
    char board[BOARD_ROWS][BOARD_COLS] = { 0 };
    //计算哈希值
    int getHash()
    {
        int hashVal = 0;
        int i = 0;
        int j = 0;

        i = 0;
        while (i < BOARD_ROWS)
        {
            j = 0;
            while (j < BOARD_COLS)
            {
                hashVal = hashVal * 3 + board[i][j];
                j++;
            }
            i++;
        }
        return hashVal;
    }
    bool update(Action& action, int symbol)
    {
        if (!action.isEmpty())
        {
            if (board[action.row][action.col] == 0)
            {
                board[action.row][action.col] = symbol;
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    //判断胜者。返回winner:0,未结束;1,玩家1胜;2,玩家2胜;3,平局。
    int getWinner()
    {
        //定义局部变量
        struct
        {
            int row[BOARD_ROWS] = { 0 };
            int col[BOARD_COLS] = { 0 };
            int left_diagonal = 0;
            int right_diagonal = 0;
        }check;
        int* a = (int*)&check;
        int i = 0;
        int j = 0;

        //行
        i = 0;
        while (i < BOARD_ROWS)
        {
            check.row[i] = getSum(SumType::ROW, i);
            i++;
        }

        //列
        j = 0;
        while (j < BOARD_COLS)
        {
            check.col[j] = getSum(SumType::COL, j);
            j++;
        }

        //对角线
        check.left_diagonal = getDiagSum(SumType::LDIAG);
        check.right_diagonal = getDiagSum(SumType::RDIAG);

        //如果游戏结束，返回winner
        i = 0;
        while (i < BOARD_ROWS + BOARD_COLS + 2)
        {
            if (a[i] == WIN_1)
            {
                return 1;
            }
            if (a[i] == WIN_2)
            {
                return 2;
            }
            i++;
        }

        //如果为平局
        if (getNum() == BOARD_SIZE)
        {
            return 3;
        }

        //没有结束
        return 0;
    }
    //检查是否结束
    bool isEnd()
    {
        if (getWinner() == 0)
            return false;
        return true;
    }
    //求对角线和。左对角线\；右对角线/
    int getDiagSum(SumType type)
    {
        int sum = 0;
        int i = 0;

        switch (type)
        {
        case SumType::LDIAG:
            i = 0;
            while (i < BOARD_COLS)
            {
                sum = sum * 3 + board[i][i];
                i++;
            }
            break;
        case SumType::RDIAG:
            i = 0;
            while (i < BOARD_ROWS)
            {
                sum = sum * 3 + board[BOARD_ROWS - 1 - i][i];
                i++;
            }
        default:
            break;
        }

        return sum;
    }
    //求行和列和
    int getSum(SumType type, int n)
    {
        int sum = 0;
        int i = 0;

        switch (type)
        {
        case SumType::ROW:
            i = 0;
            while (i < BOARD_COLS)
            {
                sum = sum * 3 + board[n][i];
                i++;
            }
            break;
        case SumType::COL:
            i = 0;
            while (i < BOARD_COLS)
            {
                sum = sum * 3 + board[i][n];
                i++;
            }
            break;
        default:
            break;
        }

        return sum;
    }
    //求落子数
    int getNum()
    {
        int num = 0;
        int i = 0;
        int j = 0;

        i = 0;
        while (i < BOARD_ROWS)
        {
            j = 0;
            while (j < BOARD_COLS)
            {
                if (board[i][j])
                    num++;
                j++;
            }
            i++;
        }

        return num;
    }
    //返回下一状态
    void getNextState(Action& action, int symbol, State& state)
    {
        state = *this;
        state.board[action.row][action.col] = symbol;
    }
    void printState()
    {
        int i = 0;
        int j = 0;

        if (play_type == PlayType::play)
        {
            cout << "---------\n";
            i = 0;
            while (i < BOARD_ROWS)
            {
                j = 0;
                while (j < BOARD_COLS)
                {
                    if (board[i][j] == 1)
                    {
                        cout << "|*";
                    }
                    else if (board[i][j] == 2)
                    {
                        cout << "|x";
                    }
                    else
                    {
                        cout << "|0";
                    }
                    j++;
                }
                cout << "|\n---------\n";
                i++;
            }
        }
    }
    //重载赋值运算符为深拷贝
    State& operator=(State& state)
    {
        int i = 0;
        int j = 0;

        i = 0;
        while (i < BOARD_ROWS)
        {
            j = 0;
            while (j < BOARD_COLS)
            {
                this->board[i][j] = state.board[i][j];
                j++;
            }
            i++;
        }

        return *this;
    }
    //重置状态
    void reset()
    {
        int i = 0;
        int j = 0;

        i = 0;
        while (i < BOARD_ROWS)
        {
            j = 0;
            while (j < BOARD_COLS)
            {
                board[i][j] = 0;
                j++;
            }
            i++;
        }
    }
};
//玩家
class Player
{
public:
    //标记棋子类型。1先手，2后手。
    int symbol;
    Player()
    {
        this->symbol = 1;
    }
    Player(int symbol)
    {
        this->symbol = symbol;
    }
    virtual void getAction(State& current_state, Action& action) {}
    virtual void saveGreedy() {}
    virtual void saveState(int hash) {}
    virtual void loadPolicy() {}
    virtual void savePolicy() {}
    virtual void reset() {}
    virtual void resetPolicy() {}
};
//AI玩家
class AIPlayer :public Player
{
public:
    //策略参数，贪婪策略是探索概率，ucb是c_ratio
    double parameter;
    //状态值函数，这里是给定状态下的估计胜率
    map<int, double> Q;
    //动作选择策略
    Policy policy;
    //保存走过的状态中哪几步采取了贪心策略
    vector<bool> greedy;
    //保存走过的状态
    vector<int> hash_vals;
    AIPlayer() :Player(), greedy(), hash_vals(), parameter(0.1), policy(Policy::e_greedy) {}
    AIPlayer(Policy _policy, double _parameter, int _symbol) :Player(_symbol), greedy(), hash_vals(), parameter(_parameter), policy(_policy) {}
    virtual void getAction(State& current_state, Action& action)
    {
        //定义局部变量
        int i = 0;
        int j = 0;
        vector<int> next_states;
        vector<Action> next_actions;
        Action action_opponent_will_win;
        int size = 0;
        //行为值函数
        Value action_val;
        priority_queue<Value, vector<Value>, comp> action_vals;
        int n = 0;

        //遍历next_states
        i = 0;
        while (i < BOARD_ROWS)
        {
            j = 0;
            while (j < BOARD_COLS)
            {
                if (current_state.board[i][j] == 0)
                {
                    temp_action.set(i, j);
                    next_actions.push_back(temp_action);
                    current_state.getNextState(temp_action, symbol, temp_state);
                    next_states.push_back(temp_state.getHash());

                    //使用简单规则play
                    if (use_simple_rule && play_type == PlayType::play)
                    {
                        //看看对面是不是差一子赢
                        if (action_opponent_will_win.isEmpty())
                        {
                            current_state.board[i][j] = 3 - symbol;
                            if (current_state.getWinner() == 3 - symbol)
                            {
                                action_opponent_will_win.set(i, j);
                            }
                            current_state.board[i][j] = 0;
                        }
                    }
                }
                j++;
            }
            i++;
        }

        //分三种policy
        if (policy == Policy::e_greedy)
        {
            //对面下一步不会胜，才考虑探索
            if (action_opponent_will_win.isEmpty() && rand() % 100 / 100.0 < parameter)
            {
                action = next_actions[rand() % next_actions.size()];

                if (play_type == PlayType::train)
                    greedy[greedy.size() - 1] = false;

                return;
            }

            //将next_states的值估计排序
            size = (int)next_states.size();
            i = 0;
            while (i < size)
            {
                action_val.estimation = Q.at(next_states[i]);
                action_val.action = next_actions[i];
                action_vals.push(action_val);
                i++;
            }

            //动作选择
            if (use_simple_rule && play_type != PlayType::train)
            {
                //如果有必胜下法就下
                if (action_vals.top().estimation == 1)
                {
                    action = action_vals.top().action;
                }
                //否则，如果对方下一步就要赢了就堵
                else if (!action_opponent_will_win.isEmpty())
                {
                    action = action_opponent_will_win;
                }
                //否则，选择值估计最大的走法
                else
                {
                    action = action_vals.top().action;
                }
            }
            else
            {
                action = action_vals.top().action;
            }
        }

        return;
    }
    virtual void saveGreedy()
    {
        greedy.push_back(true);
    }
    virtual void saveState(int hash)
    {
        hash_vals.push_back(hash);
    }
    virtual void reset()
    {
        greedy.clear();
        hash_vals.clear();
    }
    virtual void loadPolicy()
    {
        ifstream in;
        string name;
        double estimation = 0;
        int hash = 0;

        name = "policy_";
        name += (symbol == 1 ? "first.policy" : "second.policy");
        in.open(name, ios::in | ios::binary);
        while (!in.eof())
        {
            in.read((char*)&hash, sizeof(int));
            in.read((char*)&estimation, sizeof(double));
            Q[hash] = estimation;
        }
        in.close();
    }
    virtual void resetPolicy()
    {
        map<int, State>::iterator it = all_states.begin();
        int winner = 0;

        while (it != all_states.end())
        {
            winner = it->second.getWinner();
            //自己胜利
            if (winner == symbol)
            {
                Q[it->first] = 1.0;
            }
            //对方胜利
            else if (winner == 3 - symbol)
            {
                Q[it->first] = 0.0;
            }
            //平局或未结束
            else
            {
                Q[it->first] = 0.5;
            }
            it++;
        }
        //乐观的初始值,将第一步16个值估计都设置为1，可以保证第一步可以遍历所有情况
        if (positive_init && symbol == 1)
        {
            Q[1] = 1;
            Q[3] = 1;
            Q[9] = 1;
            Q[27] = 1;
            Q[81] = 1;
            Q[243] = 1;
            Q[int(pow(3, 6))] = 1;
            Q[int(pow(3, 7))] = 1;
            Q[int(pow(3, 8))] = 1;
            Q[int(pow(3, 9))] = 1;
            Q[int(pow(3, 10))] = 1;
            Q[int(pow(3, 11))] = 1;
            Q[int(pow(3, 12))] = 1;
            Q[int(pow(3, 13))] = 1;
            Q[int(pow(3, 14))] = 1;
            Q[int(pow(3, 15))] = 1;
        }
    }
    virtual void savePolicy()
    {
        ofstream out;
        string name;
        map<int, double>::iterator iter;

        name = "policy_";
        name += (symbol == 1 ? "first.policy" : "second.policy");
        out.open(name, ios::out | ios::binary);
        iter = Q.begin();
        while (iter != Q.end())
        {
            out.write((const char*)&iter->first, sizeof(int));
            out.write((const char*)&iter->second, sizeof(double));
            iter++;
        }
        out.close();
    }
    void backup(double step_size)
    {
        double td_error = 0;
        int i = 0;

        i = (int)hash_vals.size() - 2;
        while (i >= 0)
        {
            td_error = greedy[i] * (Q[hash_vals[(size_t)i + 1]] - Q[hash_vals[i]]);
            Q[hash_vals[i]] += step_size * td_error;
            i--;
        }
    }
};
//人类玩家
class HumanPlayer :public Player
{
public:
    char keys[16] = { '1', '2', '3', '4', 'q', 'w', 'e', 'r', 'a', 's', 'd', 'f', 'z', 'x', 'c', 'v' };
    HumanPlayer(int _symbol) :Player(_symbol) {}
    virtual void getAction(State& current_state, Action& action)
    {
        //定义局部变量
        char key = 0;
        int num = 0;

        do
        {
            num = 0;

            cout << "Input your position : ";
            cin >> key;

            if (key == '/')
            {
                cout << "Game over!\n";
                action.set(-1, -1);
                break;
            }

            while (keys[num] != key && num < 16)
            {
                num++;
            }

            if (num == 16)
                continue;

            action.set(num);
        } while (current_state.board[action.row][action.col] != 0);

        return;
    }
};

//定义全局函数

//递归获取所有状态
void get_all_states_impl(State current_state, int current_symbol)
{
    int i, j;

    if (current_state.isEnd())
        return;
    int hash = 0;

    i = 0;
    while (i < BOARD_ROWS)
    {
        j = 0;
        while (j < BOARD_COLS)
        {
            if (current_state.board[i][j] == 0)
            {
                temp_action.set(i, j);
                current_state.getNextState(temp_action, current_symbol, temp_state);
                hash = temp_state.getHash();

                map<int, State>::iterator it = all_states.find(hash);
                if (it == all_states.end())
                {
                    all_states.insert(map<int, State>::value_type(hash, temp_state));
                    get_all_states_impl(temp_state, 3 - current_symbol);
                }
            }
            j++;
        }
        i++;
    }
}
void get_all_states()
{
    temp_state.reset();
    all_states[temp_state.getHash()] = temp_state;
    get_all_states_impl(temp_state, 1);
}
void save_all_states()
{
    //定义局部变量
    ofstream out;
    string name;
    map<int, State>::iterator iter;

    //保存状态
    name = "all_states.state";
    out.open(name, ios::out | ios::binary);
    iter = all_states.begin();
    while (iter != all_states.end())
    {
        out.write((const char*)&iter->first, sizeof(int));
        iter++;
    }
    out.close();
}
void load_all_states()
{
    ifstream in;
    string name;
    int hash = 0;
    int i = 0;
    int j = 0;

    //加载先手状态空间
    name = "all_states.state";
    in.open(name, ios::in | ios::binary);
    //int Board[BOARD_ROWS][BOARD_COLS] = {};
    while (!in.eof())
    {
        in.read((char*)&hash, sizeof(int));
        //in.read((char*)&temp_state.Board, sizeof(int) * BOARD_SIZE);
        //将哈希值转换成棋盘
        i = 0;
        while (i < BOARD_ROWS)
        {
            j = 0;
            while (j < BOARD_COLS)
            {
                temp_state.board[i][j] = hash / int(pow(3, (3 - i) * 4 + 3 - j)) % 3;
                j++;
            }
            i++;
        }
        all_states[hash] = temp_state;
    }
    in.close();
}
//train_time单位是秒
void train(int print_every_n_epochs, double step_size, Policy policy, double parameter, int train_time)
{
    play_type = PlayType::train;

    //定义局部变量
    static int all_win_times[3] = { 0 };
    int win_times[3] = { 0 };
    int winner = 0;
    int i = 0;
    AIPlayer p[2];
    p[0] = AIPlayer(policy, parameter, 1);
    p[1] = AIPlayer(policy, parameter, 2);
    State Board = State();
    ofstream train_data;
    int current_symbol = 1;
    time_t begin, end;

    //初始化训练数据保存文件
    if (last_epoch)
    {
        train_data.open("train_data.txt", ios::out | ios::app);
        p[0].loadPolicy();
        p[1].loadPolicy();
    }
    else
    {
        train_data.open("train_data.txt", ios::out | ios::trunc);
        p[0].resetPolicy();
        p[1].resetPolicy();
        train_data << "policy: " << policy << endl << "step size: " << step_size << endl << "epochs\tp1 all win rate\tp1 win rate\tp1 parameter\tp2 all win rate\tp2 win rate\tp2 parameter\n";
    }

    begin = time(NULL);
    i = 0;
    while ((end = time(NULL), difftime(end, begin) < train_time))
    {
        while (!Board.isEnd())
        {
            p[0].saveGreedy();
            p[1].saveGreedy();
            p[current_symbol - 1].getAction(Board, temp_action);
            Board.update(temp_action, current_symbol);
            p[0].saveState(Board.getHash());
            p[1].saveState(Board.getHash());
            current_symbol = 3 - current_symbol;
        }

        winner = Board.getWinner();

        win_times[winner - 1] += 1;
        all_win_times[winner - 1] += 1;

        p[0].backup(step_size);
        p[1].backup(step_size);

        if ((i + 1) % print_every_n_epochs == 0)
        {
            //每训练1周期,parameter都会减小,最后趋近于0.01
            if (parameter_decline)
            {
                p[0].parameter = 500.0 / (((int64_t)i + 1) / 10.0 + 600) + 0.01;
                p[1].parameter = 500.0 / (((int64_t)i + 1) / 10.0 + 600) + 0.01;
            }

            //显示训练数据
            cout << "Epoch " << (int64_t)last_epoch + i + 1 << ", player 1 all win rate: " << (double)all_win_times[0] / ((int64_t)last_epoch + i + 1) << ", player 2 all win rate: " << (double)all_win_times[1] / ((int64_t)last_epoch + i + 1) << endl;
            cout << "Epoch " << (int64_t)last_epoch + i + 1 << ", player 1 win rate: " << (double)win_times[0] / print_every_n_epochs << ", player 2 win rate: " << (double)win_times[1] / print_every_n_epochs << endl;
            cout << "Epoch " << (int64_t)last_epoch + i + 1 << ", player 1 parameter: " << p[0].parameter << ", player 2 parameter: " << p[1].parameter << endl;

            //保存训练数据
            train_data << last_epoch + i + 1 << '\t' << (double)all_win_times[0] / ((int64_t)last_epoch + i + 1) << '\t' << (double)win_times[0] / print_every_n_epochs << '\t' << p[0].parameter << '\t' << (double)all_win_times[1] / ((int64_t)last_epoch + i + 1) << '\t' << (double)win_times[1] / print_every_n_epochs << '\t' << p[1].parameter << endl;

            win_times[0] = 0;
            win_times[1] = 0;
        }
        winner = 0;
        p[0].reset();
        p[1].reset();
        Board.reset();
        i++;
        current_symbol = 1;
    }
    last_epoch += i;
    train_data.close();

    //保存策略
    p[0].savePolicy();
    p[1].savePolicy();
    train_parameter = p[0].parameter;
}
void train(int epochs, int print_every_n_epochs, double step_size, Policy policy, double parameter)
{
    play_type = PlayType::train;

    //定义局部变量
    static int all_win_times[3] = { 0 };
    int win_times[3] = { 0 };
    int winner = 0;
    int i = 0;
    AIPlayer p[2];
    p[0] = AIPlayer(policy, parameter, 1);
    p[1] = AIPlayer(policy, parameter, 2);
    State Board = State();
    ofstream train_data;
    int current_symbol = 1;

    //初始化训练数据保存文件
    if (last_epoch)
    {
        train_data.open("train_data.txt", ios::out | ios::app);
        p[0].loadPolicy();
        p[1].loadPolicy();
    }
    else
    {
        train_data.open("train_data.txt", ios::out | ios::trunc);
        p[0].resetPolicy();
        p[1].resetPolicy();
        train_data << "policy: " << policy << endl << "step size: " << step_size << endl << "epochs\tp1 all win rate\tp1 win rate\tp1 parameter\tp2 all win rate\tp2 win rate\tp2 parameter\n";
    }

    i = 0;
    while (i < epochs)
    {
        while (!Board.isEnd())
        {
            p[0].saveGreedy();
            p[1].saveGreedy();
            p[current_symbol - 1].getAction(Board, temp_action);
            Board.update(temp_action, current_symbol);
            p[0].saveState(Board.getHash());
            p[1].saveState(Board.getHash());
            current_symbol = 3 - current_symbol;
        }

        winner = Board.getWinner();

        win_times[winner - 1] += 1;
        all_win_times[winner - 1] += 1;

        p[0].backup(step_size);
        p[1].backup(step_size);

        if ((i + 1) % print_every_n_epochs == 0)
        {
            //每训练1周期,parameter都会减小,最后趋近于0.01
            if (parameter_decline)
            {
                p[0].parameter = 500.0 / (((int64_t)i + 1) / 10.0 + 600) + 0.01;
                p[1].parameter = 500.0 / (((int64_t)i + 1) / 10.0 + 600) + 0.01;
            }

            //显示训练数据
            cout << "Epoch " << (int64_t)last_epoch + i + 1 << ", player 1 all win rate: " << (double)all_win_times[0] / ((int64_t)last_epoch + i + 1) << ", player 2 all win rate: " << (double)all_win_times[1] / ((int64_t)last_epoch + i + 1) << endl;
            cout << "Epoch " << (int64_t)last_epoch + i + 1 << ", player 1 win rate: " << (double)win_times[0] / print_every_n_epochs << ", player 2 win rate: " << (double)win_times[1] / print_every_n_epochs << endl;
            cout << "Epoch " << (int64_t)last_epoch + i + 1 << ", player 1 parameter: " << p[0].parameter << ", player 2 parameter: " << p[1].parameter << endl;

            //保存训练数据
            train_data << last_epoch + i + 1 << '\t' << (double)all_win_times[0] / ((int64_t)last_epoch + i + 1) << '\t' << (double)win_times[0] / print_every_n_epochs << '\t' << p[0].parameter << '\t' << (double)all_win_times[1] / ((int64_t)last_epoch + i + 1) << '\t' << (double)win_times[1] / print_every_n_epochs << '\t' << p[1].parameter << endl;

            win_times[0] = 0;
            win_times[1] = 0;
        }
        winner = 0;
        p[0].reset();
        p[1].reset();
        Board.reset();
        i++;
        current_symbol = 1;
    }
    last_epoch += epochs;
    train_data.close();

    //保存策略
    p[0].savePolicy();
    p[1].savePolicy();
    train_parameter = p[0].parameter;
}
void test(int epochs, Policy policy, double parameter)
{
    play_type = PlayType::test;

    //定义局部变量
    int i = 0;
    int win_times[3] = { 0 };
    int winner = 0;
    Action action;
    AIPlayer p[2];
    p[0] = AIPlayer(policy, parameter, 1);
    p[1] = AIPlayer(policy, parameter, 2);
    State Board = State();
    int current_symbol = 1;

    p[0].loadPolicy();
    p[1].loadPolicy();

    i = 0;
    while (i < epochs)
    {
        //测试第一步随机
        //Board.printState();
        temp_action.set(rand() % 16);
        Board.update(temp_action, current_symbol);

        current_symbol = 3 - current_symbol;

        while (!Board.isEnd())
        {
            //Board.printState();
            p[current_symbol - 1].getAction(Board, action);
            Board.update(action, current_symbol);

            current_symbol = 3 - current_symbol;
        }
        //Board.printState();

        winner = Board.getWinner();

        win_times[winner - 1] += 1;

        winner = 0;
        Board.reset();
        i++;
        current_symbol = 1;
    }

    cout << epochs << " epochs, player 1 win rate: " << win_times[0] / (double)epochs << ", player 2 win rate: " << win_times[1] / (double)epochs << endl;
}
//order：1，玩家先手；2，玩家后手。
void play(int order, Policy policy, double parameter)
{
    play_type = PlayType::play;

    //定义局部变量
    bool isEnd = false;
    int human = order - 1;
    int AI = 1 - human;
    int winner = 0;
    Action action;
    Player* p[2];
    p[human] = new HumanPlayer(order);
    p[AI] = new AIPlayer(policy, parameter, 3 - order);
    State Board = State();
    int current_symbol = 1;

    p[0]->loadPolicy();
    p[1]->loadPolicy();

    while (true)
    {
        while (!Board.isEnd())
        {
            Board.printState();
            p[current_symbol - 1]->getAction(Board, action);
            isEnd = !Board.update(action, current_symbol);
            if (isEnd)
                break;
            current_symbol = 3 - current_symbol;
        }
        if (isEnd)
            break;
        Board.printState();

        winner = Board.getWinner();

        if (winner == human + 1)
        {
            cout << "You win!\n";
        }
        else if (winner == AI + 1)
        {
            cout << "You lose!\n";
        }
        else
        {
            cout << "It is a tie!\n";
        }
        winner = 0;
        p[0]->reset();
        p[1]->reset();
        Board.reset();
        current_symbol = 1;
    }

    delete p[0];
    delete p[1];
}
ostream& operator<<(ostream& out, Policy& policy)
{
    switch (policy)
    {
    case Policy::e_greedy:
        out << "e_greedy";
        break;
    case Policy::ucb:
        out << "ucb";
        break;
    case Policy::boltzmann:
        out << "boltzmann";
        break;
    default:
        break;
    }
    return out;
}

//定义全局变量

//策略
Policy policy = Policy::e_greedy;
//策略参数
double train_parameter = 0.8;
double test_parameter = 0.0;
//更新步长
double step_size = 0.2;
//每n幕打印1次
int print_every_n_epochs = 1000;
//添加简单规则
bool use_simple_rule = true;
//乐观的初始值
bool positive_init = true;
//自适应控制parameter
bool parameter_decline = true;
//共享变量，全局唯一
PlayType play_type;
map<int, State> all_states;
int last_epoch = 0;
//传递变量，用于过渡，没有实际含义
State temp_state;
Action temp_action;

//int main()
//{
//    get_all_states();
//    save_all_states();
//
//    load_all_states();
//    train((int)5e5, print_every_n_epochs, step_size, policy, train_parameter);
//
//    load_all_states();
//    train(print_every_n_epochs, step_size, policy, train_parameter, 7200);
//
//    test(10, policy, test_parameter);
//
//    play(2, policy, test_parameter);
//
//    return 0;
//}