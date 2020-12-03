#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

const int BOARD_ROWS = 4;
const int BOARD_COLS = 4;
const int BOARD_SIZE = BOARD_ROWS * BOARD_COLS;

//初始探索概率
double init_epsilon = 0.8;
//每n步显示一次
int print_every_n_epochs = 500;
//添加简单规则
bool use_simple_rule = true;
//乐观的初始值
bool positive_init = true;

class State;
class Player;
class AIPlayer;
class HumanPlayer;

//保存状态空间
map<int, State> all_states;

inline int winner2symbol(int winner)
{
    return 3 - 2 * winner;
}

inline int symbol2winner(int symbol)
{
    return (3 - symbol) / 2;
}

//状态类
class State
{
public:
    //棋盘
    int8_t data[BOARD_ROWS][BOARD_COLS] = { 0 };
    //计算哈希值
    int hash();
    //判断胜者
    int winner();
    //检查是否结束
    bool is_end();
    //求和。0：绝对总和；1：行和；2：列和；3：对角线和（1：\；2：/）
    int sum(int type = 0, int n = 0);
    //返回下一状态
    State* next_state(int i, int j, int symbol);
    void print_state();
    //深拷贝
    State& operator=(State& state);
    //重置状态
    void reset();
};
//用于临时保存状态
State temp_state;
int State::hash()
{
    int hashVal = 0;
    for (int i = 0; i < BOARD_ROWS; i++)
    {
        for (int j = 0; j < BOARD_COLS; j++)
        {
            hashVal = hashVal * 3 + data[i][j] + 1;
        }
    }
    return hashVal;
}
int State::winner()
{
    int sum_vals[BOARD_ROWS + BOARD_COLS + 2] = { 0 };
    int i = 0;
    //检查行
    for (int j = 1; j <= BOARD_ROWS; j++, i++)
        sum_vals[i] = sum(1, j);
    //检查列
    for (int j = 1; j <= BOARD_COLS; j++, i++)
        sum_vals[i] = sum(2, j);
    //检查对角线
    for (int j = 1; j <= 2; j++, i++)
        sum_vals[i] = sum(3, j);

    for (i = 0; i < BOARD_ROWS + BOARD_COLS + 2; i++)
    {
        if (sum_vals[i] == 4)
        {
            return 1;
        }
        if (sum_vals[i] == -4)
        {
            return 2;
        }
    }
    //是否为平局
    if (sum() == BOARD_SIZE)
    {
        return 0;
    }
    //没有结束
    return -1;
}
bool State::is_end()
{
    if (winner() == -1)
        return false;
    return true;
}
int State::sum(int type, int n)
{
    int sum = 0;
    int temp = 0;
    switch (type)
    {
    case 0:
        for (int i = 0; i < BOARD_ROWS; i++)
        {
            for (int j = 0; j < BOARD_COLS; j++)
            {
                sum += (temp = data[i][j]) < 0 ? -temp : temp;
            }
        }
        break;
    case 1:
        for (int i = 0; i < BOARD_COLS; i++)
        {
            sum += data[n - 1][i];
        }
        break;
    case 2:
        for (int i = 0; i < BOARD_COLS; i++)
        {
            sum += data[i][n - 1];
        }
        break;
    case 3:
        if (n == 1)
        {
            for (int i = 0; i < BOARD_COLS; i++)
            {
                sum += data[i][i];
            }
        }
        else if (n == 2)
        {
            for (int i = 0; i < BOARD_COLS; i++)
            {
                sum += data[3 - i][i];
            }
        }
        break;
    default:
        break;
    }
    return sum;
}
State* State::next_state(int i, int j, int symbol)
{
    temp_state = *this;
    temp_state.data[i][j] = symbol;
    return &temp_state;
}
void State::print_state()
{
    cout << "---------\n";
    for (int i = 0; i < BOARD_ROWS; i++)
    {
        for (int j = 0; j < BOARD_COLS; j++)
        {
            if (data[i][j] == 1)
            {
                cout << "|*";
            }
            else if (data[i][j] == -1)
            {
                cout << "|x";
            }
            else
            {
                cout << "|0";
            }
        }
        cout << "|\n---------\n";
    }
}
State& State::operator=(State& state)
{
    for (int i = 0; i < BOARD_ROWS; i++)
    {
        for (int j = 0; j < BOARD_COLS; j++)
        {
            this->data[i][j] = state.data[i][j];
        }
    }
    return *this;
}
void State::reset()
{
    for (int i = 0; i < BOARD_ROWS; i++)
    {
        for (int j = 0; j < BOARD_COLS; j++)
        {
            data[i][j] = 0;
        }
    }
}

//玩家
class Player
{
public:
    int symbol = 0;
    double epsilon = 0.1;
    virtual void set_symbol(int symbol, int play_type) {}
    virtual void get_action(int& i, int& j) {}
    virtual void reset() {}
    virtual void set_state(State* state) {}
    virtual void load_policy() {}
};

//裁判
class Judger
{
public:
    Player* p1;
    Player* p2;
    Player* current_player;
    int p1_symbol = 1;
    int p2_symbol = -1;
    int current_symbol = 1;
    State* current_state = NULL;
    //play_type:0为play(默认),1为train,2为test,3为continue_train
    int play_type = 0;
    Judger(Player* player1, Player* player2, int play_type = 0)
    {
        p1 = player1;
        p2 = player2;
        current_player = p1;
        this->play_type = play_type;
        p1->set_symbol(p1_symbol, play_type);
        p2->set_symbol(p2_symbol, play_type);
    }
    void reset()
    {
        p1->reset();
        p2->reset();
        current_symbol = 1;
    }
    Player* alternate()
    {
        return current_symbol == 1 ? (current_symbol = -current_symbol, p1) : (current_symbol = -current_symbol, p2);
    }
    int play()
    {
        int i = 0;
        int j = 0;
        int current_state_hash = 0;
        current_state = new State();
        reset();
        current_state_hash = current_state->hash();
        p1->set_state(&all_states.at(current_state_hash));
        p2->set_state(&all_states.at(current_state_hash));
        //如果不是train,则显示棋盘
        if (play_type != 1)
        {
            current_state->print_state();
        }
        //如果是test,第一步就设为随机
        if (play_type == 2)
        {
            current_player = alternate();
            double temp = current_player->epsilon;
            current_player->epsilon = 1;
            current_player->get_action(i, j);
            current_player->epsilon = temp;
            *current_state = *current_state->next_state(i, j, current_player->symbol);
            current_state_hash = current_state->hash();
            p1->set_state(&all_states.at(current_state_hash));
            p2->set_state(&all_states.at(current_state_hash));
            if (play_type != 1)
            {
                current_state->print_state();
            }
            if (current_state->is_end())
            {
                int winner = current_state->winner();
                delete current_state;
                return winner;
            }
        }
        while (true)
        {
            current_player = alternate();
            current_player->get_action(i, j);
            *current_state = *current_state->next_state(i, j, current_player->symbol);
            current_state_hash = current_state->hash();
            p1->set_state(&all_states.at(current_state_hash));
            p2->set_state(&all_states.at(current_state_hash));
            //如果不是train,则显示棋盘
            if (play_type != 1)
            {
                current_state->print_state();
            }
            //游戏结束
            if (current_state->is_end())
            {
                int winner = current_state->winner();
                delete current_state;
                return winner;
            }
        }
    }
};

struct Value
{
    double estimation = 0;
    int position_i = 0;
    int position_j = 0;
};
static bool comp(const Value& a, const Value& b)
{
    return b.estimation < a.estimation;
}
class AIPlayer :public Player
{
public:
    map<int, double> estimations;
    double step_size;
    vector<State*> states;
    vector<bool> greedy;
    vector<int> hash_vals;
    AIPlayer(double epsilon = 0.8, bool positive_init = false, double step_size = 0.1) :Player(), estimations(), states(), greedy(), hash_vals()
    {
        this->step_size = step_size;
        this->epsilon = epsilon;
    }
    void reset()
    {
        states.clear();
        greedy.clear();
        hash_vals.clear();
    }
    void set_state(State* state)
    {
        states.push_back(state);
        greedy.push_back(true);
        hash_vals.push_back(state->hash());
    }
    void set_symbol(int symbol, int play_type)
    {
        this->symbol = symbol;
        //如果是train,则初始化值估计;如果是test和play,则直接加载策略
        if (play_type == 1)
        {
            for (map<int, State>::iterator it = all_states.begin(); it != all_states.end(); it++)
            {
                int winner = it->second.winner();
                //自己胜利
                if (winner == (3 - symbol) / 2)
                {
                    estimations[it->first] = 1.0;
                }
                //对方胜利
                else if (winner == (3 + symbol) / 2)
                {
                    estimations[it->first] = 0.0;
                }
                //平局或未结束
                else
                {
                    estimations[it->first] = 0.5;
                }
            }
            //乐观的初始值,将第一步16个值估计都设置为1，可以保证第一步可以遍历所有情况
            if (positive_init && symbol == 1)
            {
                estimations[1 + 21523360] = 1;
                estimations[3 + 21523360] = 1;
                estimations[9 + 21523360] = 1;
                estimations[27 + 21523360] = 1;
                estimations[81 + 21523360] = 1;
                estimations[273 + 21523360] = 1;
                estimations[int(pow(3, 6)) + 21523360] = 1;
                estimations[int(pow(3, 7)) + 21523360] = 1;
                estimations[int(pow(3, 8)) + 21523360] = 1;
                estimations[int(pow(3, 9)) + 21523360] = 1;
                estimations[int(pow(3, 10)) + 21523360] = 1;
                estimations[int(pow(3, 11)) + 21523360] = 1;
                estimations[int(pow(3, 12)) + 21523360] = 1;
                estimations[int(pow(3, 13)) + 21523360] = 1;
                estimations[int(pow(3, 14)) + 21523360] = 1;
                estimations[int(pow(3, 15)) + 21523360] = 1;
            }
        }
        else
        {
            load_policy();
        }
    }
    void backup()
    {
        double td_error = 0;
        for (int i = (int)states.size() - 2; i >= 0; i--)
        {
            td_error = greedy[i] * (estimations[hash_vals[(size_t)i + 1]] - estimations[hash_vals[i]]);
            estimations[hash_vals[i]] += step_size * td_error;
        }
    }
    void get_action(int& i, int& j)
    {
        State* state = states[states.size() - 1];
        vector<int> next_states;
        vector<int> next_positions_i;
        vector<int> next_positions_j;
        int position_opponent_will_win[2] = { -1, -1 };
        for (int i = 0; i < BOARD_ROWS; i++)
        {
            for (int j = 0; j < BOARD_COLS; j++)
            {
                if (state->data[i][j] == 0)
                {
                    next_positions_i.push_back(i);
                    next_positions_j.push_back(j);
                    next_states.push_back(state->next_state(i, j, symbol)->hash());

                    if (use_simple_rule)
                    {
                        //看看对面是不是差一子赢
                        if (position_opponent_will_win[0] == -1)
                        {
                            state->data[i][j] = -this->symbol;
                            if (state->winner() == symbol2winner(-this->symbol))
                            {
                                position_opponent_will_win[0] = i;
                                position_opponent_will_win[1] = j;
                            }
                            state->data[i][j] = 0;
                        }
                    }
                }
            }
        }
        if (position_opponent_will_win[0] == -1 && rand() % 100 / 100.0 < epsilon)
        {
            int n = rand() % next_states.size();
            i = next_positions_i[n];
            j = next_positions_j[n];
            greedy[greedy.size() - 1] = false;
            return;
        }
        Value value = {};
        vector<Value> values;
        int size = (int)next_states.size();
        for (int i = 0; i < size; i++)
        {
            value.estimation = estimations.at(next_states[i]);
            value.position_i = next_positions_i[i];
            value.position_j = next_positions_j[i];
            values.push_back(value);
        }
        sort(values.begin(), values.end(), comp);
        if (use_simple_rule)
        {
            //如果有必胜下法就下
            if (values[0].estimation == 1)
            {
                i = values[0].position_i;
                j = values[0].position_j;
            }
            //否则，如果对方下一步就要赢了就堵
            else if (position_opponent_will_win[0] != -1)
            {
                i = position_opponent_will_win[0];
                j = position_opponent_will_win[1];
            }
            //否则，选择值估计最大的走法
            else
            {
                i = values[0].position_i;
                j = values[0].position_j;
            }
        }
        else
        {
            i = values[0].position_i;
            j = values[0].position_j;
        }
        return;
    }
    void save_policy()
    {
        ofstream out;
        string name = "policy_";
        name += symbol == 1 ? "first" : "second";
        out.open(name, ios::out | ios::binary);
        for (map<int, double>::iterator iter = estimations.begin(); iter != estimations.end(); iter++)
        {
            out.write((const char*)&iter->first, sizeof(int));
            out.write((const char*)&iter->second, sizeof(double));
        }
        out.close();
    }
    void load_policy()
    {
        ifstream in;
        string name = "policy_";
        name += symbol == 1 ? "first" : "second";
        in.open(name, ios::in | ios::binary);
        double estimation = 0;
        int hash = 0;
        while (!in.eof())
        {
            in.read((char*)&hash, sizeof(int));
            in.read((char*)&estimation, sizeof(double));
            estimations[hash] = estimation;
        }
    }
};

class HumanPlayer :public Player
{
public:
    char keys[16] = { '1', '2', '3', '4', 'q', 'w', 'e', 'r', 'a', 's', 'd', 'f', 'z', 'x', 'c', 'v' };
    void set_symbol(int symbol, int play_type)
    {
        this->symbol = symbol;
    }
    void get_action(int& i, int& j)
    {
        char key = 0;
        cout << "Input your position : ";
        cin >> key;
        int data = 0;
        while (keys[data] != key)
        {
            data++;
        }
        i = data / BOARD_COLS;
        j = data % BOARD_COLS;
    }
};

void get_all_states_impl(State current_state, int8_t current_symbol)
{
    if (current_state.is_end())
        return;
    int hash = 0;
    for (int i = 0; i < BOARD_ROWS; i++)
    {
        for (int j = 0; j < BOARD_COLS; j++)
        {
            if (current_state.data[i][j] == 0)
            {
                current_state.next_state(i, j, current_symbol);
                hash = temp_state.hash();
                map<int, State>::iterator it = all_states.find(hash);
                if (it == all_states.end())
                {
                    all_states.insert(map<int, State>::value_type(hash, temp_state));
                    get_all_states_impl(temp_state, -current_symbol);
                }
            }
        }
    }
}

void get_all_states()
{
    int8_t current_symbol = 1;
    State current_state;
    all_states[current_state.hash()] = current_state;
    get_all_states_impl(current_state, current_symbol);
}

void save_all_states()
{
    ofstream out;
    string name = "all_states";
    out.open(name, ios::out | ios::binary);
    for (map<int, State>::iterator iter = all_states.begin(); iter != all_states.end(); iter++)
    {
        out.write((const char*)&iter->first, sizeof(int));
        out.write((const char*)&iter->second.data, sizeof(int8_t) * BOARD_SIZE);
    }
    out.close();
}

void load_all_states()
{
    ifstream in;
    string name = "all_states";
    in.open(name, ios::in | ios::binary);
    //int8_t data[BOARD_ROWS][BOARD_COLS] = {};
    int hash = 0;
    while (!in.eof())
    {
        in.read((char*)&hash, sizeof(int));
        in.read((char*)&temp_state.data, sizeof(int8_t) * BOARD_SIZE);
        //for (int i = 0; i < BOARD_ROWS; i++)
        //{
        //    for (int j = 0; j < BOARD_COLS; j++)
        //    {
                //temp_state.data[i][j] = hash / int(pow(3, (3 - i) * 4 + 3 - j)) % 3 - 1;
        //        temp_state.data[i][j] = data[i][j];
        //    }
        //}
        all_states[hash] = temp_state;
    }
}

void train(int epochs, int print_every_n_epochs = 500)
{
    AIPlayer player1(init_epsilon, positive_init);
    AIPlayer player2(init_epsilon);
    Judger judger(&player1, &player2, 1);
    int player1_win_times = 0;
    int player2_win_times = 0;
    int winner = 0;
    for (int i = 0; i < epochs; i++)
    {
        winner = judger.play();
        if (winner == 1)
        {
            player1_win_times += 1;
        }
        else if (winner == 2)
        {
            player2_win_times += 1;
        }
        if ((i + 1) % print_every_n_epochs == 0)
        {
            cout << "Epoch " << i + 1 << ", player 1 win rate: " << (double)player1_win_times / print_every_n_epochs << ", player 2 win rate: " << (double)player2_win_times / print_every_n_epochs << endl;
            player1_win_times = 0;
            player2_win_times = 0;
            //每训练1周期,epsilon都会减小,最后趋近于0.01
            player1.epsilon = 500.0 / (((int64_t)i + 1) / 10.0 + 600) + 0.01;
        }
        player1.backup();
        player2.backup();
        judger.reset();
    }
    player1.save_policy();
    player2.save_policy();
}

void continue_train(int epochs, int print_every_n_epochs = 500)
{
    AIPlayer player1(init_epsilon, positive_init);
    AIPlayer player2(init_epsilon);
    Judger judger(&player1, &player2, 3);
    int player1_win_times = 0;
    int player2_win_times = 0;
    int winner = 0;
    for (int i = 0; i < epochs; i++)
    {
        winner = judger.play();
        if (winner == 1)
        {
            player1_win_times += 1;
        }
        else if (winner == 2)
        {
            player2_win_times += 1;
        }
        if ((i + 1) % print_every_n_epochs == 0)
        {
            cout << "Epoch " << i + 1 << ", player 1 win rate: " << (double)player1_win_times / print_every_n_epochs << ", player 2 win rate: " << (double)player2_win_times / print_every_n_epochs << endl;
            player1_win_times = 0;
            player2_win_times = 0;
            //每训练1周期,epsilon都会减小,最后趋近于0.01
            player1.epsilon = 500.0 / (((int64_t)i + 1) / 10.0 + 600) + 0.01;
        }
        player1.backup();
        player2.backup();
        judger.reset();
    }
    player1.save_policy();
    player2.save_policy();
}

void test(int turns)
{
    AIPlayer player1(0);
    AIPlayer player2(0);
    Judger judger(&player1, &player2, 2);
    int player1_win = 0;
    int player2_win = 0;
    int winner = 0;
    for (int i = 0; i < turns; i++)
    {
        winner = judger.play();
        if (winner == 1)
        {
            player1_win += 1;
        }
        else if (winner == 2)
        {
            player2_win += 1;
        }
        judger.reset();
    }
    cout << turns << " turns, player 1 win rate: " << player1_win / (double)turns << ", player 2 win rate: " << player2_win / (double)turns << endl;
}

void play()
{
    Player* player1 = new HumanPlayer;
    Player* player2 = new AIPlayer(0);
    Judger judger(player1, player2);
    int winner = 0;
    while (true)
    {
        winner = judger.play();
        if (winner == symbol2winner(player2->symbol))
        {
            cout << "You lose!\n";
        }
        else if (winner == symbol2winner(player1->symbol))
        {
            cout << "You win!\n";
        }
        else
        {
            cout << "It is a tie!\n";
        }
    }
}

int main()
{
    //get_all_states();
    //save_all_states();

    load_all_states();
    train((int)5e5, print_every_n_epochs);
    test(10);
    play();

    return 0;
}