#include<bits/stdc++.h> 
using namespace std;
typedef long long ll;
#define all(x) (x).begin(),(x).end()
template<typename T1,typename T2> bool chmin(T1 &a,T2 b){if(a<=b)return 0; a=b; return 1;}
template<typename T1,typename T2> bool chmax(T1 &a,T2 b){if(a>=b)return 0; a=b; return 1;}

unsigned int randint() {
    static unsigned int tx = 123456789, ty=362436069, tz=521288629, tw=88675123;
    unsigned int tt = (tx^(tx<<11));
    tx = ty; ty = tz; tz = tw;
    return ( tw=(tw^(tw>>19))^(tt^(tt>>8)) );
}

struct Timer {
    chrono::high_resolution_clock::time_point st;

    Timer() { reset(); }

    void reset() {
        st = chrono::high_resolution_clock::now();
    }

    chrono::milliseconds::rep elapsed() {
        auto ed = chrono::high_resolution_clock::now();
        return chrono::duration_cast< chrono::milliseconds >(ed - st).count();
    }
};


struct MoveAction {
    int before_row, before_col, after_row, after_col;
    MoveAction(int before_row, int before_col, int after_row, int after_col) : 
        before_row(before_row), before_col(before_col), after_row(after_row), after_col(after_col) {}
};

struct ConnectAction {
    int c1_row, c1_col, c2_row, c2_col;
    ConnectAction(int c1_row, int c1_col, int c2_row, int c2_col) : 
        c1_row(c1_row), c1_col(c1_col), c2_row(c2_row), c2_col(c2_col) {}
};

struct Result {
    vector<MoveAction> move;
    vector<ConnectAction> connect;
    Result(const vector<MoveAction> &move, const vector<ConnectAction> &con) : move(move), connect(con) {}
};

struct Solver {
    static constexpr char USED = 'x';
    static constexpr int dx[4] = {0, 1, 0, -1};
    static constexpr int dy[4] = {1, 0, -1, 0};

    int N, K;
    int action_count_limit;
    mt19937 engine;
    vector<string> field;

    Solver(int N, int K, const vector<string> &field, int seed = 0) : 
        N(N), K(K), action_count_limit(K * 100), field(field){
        engine.seed(seed);
    }

    bool can_move(int row, int col, int dir) const{
        int nrow = row + dx[dir];
        int ncol = col + dy[dir];
        if (0 <= nrow && nrow < N && 0 <= ncol && ncol < N) {
            return field[nrow][ncol] == '0';
        }
        return false;
    }

    vector<MoveAction> move(int move_limit = -1){
        vector<MoveAction> ret;
        if (move_limit == -1) {
            move_limit = K * 50;
        }

        for (int i = 0; i < move_limit; i++) {
            int row = engine() % N;
            int col = engine() % N;
            int dir = engine() % 4;
            if (field[row][col] != '0' && can_move(row, col, dir)) {
                swap(field[row][col], field[row + dx[dir]][col + dy[dir]]);
                ret.emplace_back(row, col, row + dx[dir], col + dy[dir]);
                action_count_limit--;
            }
        }

        return ret;
    }

    array<int,2> can_connect(int row, int col, int dir) const{
        // can connect -> return {x, y}
        // cannot -> return {-1, -1}
        int nrow = row + dx[dir];
        int ncol = col + dy[dir];
        while (0 <= nrow && nrow < N && 0 <= ncol && ncol < N) {
            if (field[nrow][ncol] == field[row][col]) {
                return {nrow, ncol};
            } else if (field[nrow][ncol] != '0') {
                return {-1, -1};
            }
            nrow += dx[dir];
            ncol += dy[dir];
        }
        return {-1, -1};
    }

    ConnectAction line_fill(int row, int col, int dir){
        int nrow = row + dx[dir];
        int ncol = col + dy[dir];
        while (0 <= nrow && nrow < N && 0 <= ncol && ncol < N) {
            if (field[nrow][ncol] == field[row][col]) {
                return ConnectAction(row, col, nrow, ncol);
            }
            assert(field[nrow][ncol] == '0');
            field[nrow][ncol] = USED;
            nrow += dx[dir];
            ncol += dy[dir];
        }
        assert(false);
    }

    vector<ConnectAction> connect(){
        vector<ConnectAction> ret;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (field[i][j] != '0' && field[i][j] != 'x') {
                    for (int dir = 0; dir < 2; dir++) {
                        auto [x, y] = can_connect(i, j, dir);
                        if(x == -1) continue;
                        bool adjust = (abs(x-i) + abs(y-j) == 1);
                        // bool naname_1 = (abs(x-i) == 1 and abs(y-j) == 1);
                        bool kabe = ((x==i and (i==0 or i==N-1)) or (y==j and (j==0 or j==N-1)));
                        if(adjust or kabe) {
                            ret.push_back(ConnectAction(i, j, x, y));
                            action_count_limit--;
                            if (action_count_limit <= 0) {
                                return ret;
                            }
                        }
                    }
                }
            }
        }
        return ret;
    }

    Result solve(){
        // create random moves
        auto moves = move(0);

        // from each computer, connect to right and/or bottom if it will reach the same type
        auto connects = connect();
        return Result(moves, connects);
    }
};

struct UnionFind {
    map<pair<int,int>, pair<int, int>> parent;
    UnionFind() :parent() {}

    pair<int, int> find(pair<int, int> x){
        if (parent.find(x) == parent.end()) {
            parent[x] = x;
            return x;
        } else if (parent[x] == x) {
            return x;
        } else {
            parent[x] = find(parent[x]);
            return parent[x];
        }
    }

    void unite(pair<int, int> x, pair<int, int> y){
        x = find(x);
        y = find(y);
        if (x != y) {
            parent[x] = y;
        }
    }
};

int calc_score(int N, vector<string> field, const Result &res){
    for (auto r : res.move) {
        assert(field[r.before_row][r.before_col] != '0');
        assert(field[r.after_row][r.after_col] == '0');
        swap(field[r.before_row][r.before_col], field[r.after_row][r.after_col]);
    }

    UnionFind uf;
    for (auto r : res.connect) {
        pair<int, int> p1(r.c1_row, r.c1_col), p2(r.c2_row, r.c2_col);
        uf.unite(p1, p2);
    }

    vector<pair<int, int>> computers;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (field[i][j] != '0') {
                computers.emplace_back(i, j);
            }
        }
    }

    int score = 0;
    for (int i = 0; i < (int)computers.size(); i++) {
        for (int j = i+1; j < (int)computers.size(); j++) {
            auto c1 = computers[i];
            auto c2 = computers[j];
            if (uf.find(c1) == uf.find(c2)) {
                score += (field[c1.first][c1.second] == field[c2.first][c2.second]) ? 1 : -1;
            }
        }
    }

    return max(score, 0);
}

void print_answer(const Result &res){
    cout << res.move.size() << endl;
    for (auto m : res.move) {
        cout << m.before_row << " " << m.before_col << " "
            << m.after_row << " " << m.after_col << endl;
    }
    cout << res.connect.size() << endl;
    for (auto m : res.connect) {
        cout << m.c1_row << " " << m.c1_col << " "
            << m.c2_row << " " << m.c2_col << endl;
    }
}


int main(){
    int N, K;
    cin >> N >> K;
    vector<string> field(N);
    for (int i = 0; i < N; i++) {
        cin >> field[i];
    }

    Solver s(N, K, field);
    auto ret = s.solve();

    // cerr << "Score = " << calc_score(N, field, ret) << endl;

    print_answer(ret);

}