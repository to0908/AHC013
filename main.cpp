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

class UnionFind{
    public:
    //親の番号を格納する。親だった場合は-(その集合のサイズ)
    vector<int> parent;

    UnionFind(int N){
        parent = vector<int>(N,-1);
    }

    int root(int A){
        if(parent[A] < 0) return A;
        return parent[A]=root(parent[A]);
    }

    int size(int A){
        return -parent[root(A)];
    }

    bool unite(int A, int B) {
        A = root(A), B = root(B);
        if(A == B) return false; 

        if(size(A) < size(B)) swap(A,B);
        parent[A] += parent[B];
        parent[B] = A;
        return true;
    }

    bool same(int A, int B){
        return root(A)==root(B);
    } 
};

struct MoveAction {
    int pos1, pos2;
    MoveAction(int pos1, int pos2) : 
        pos1(pos1), pos2(pos2) {}
};

struct ConnectAction {
    int pos1, pos2;
    ConnectAction(int pos1, int pos2) : 
        pos1(pos1), pos2(pos2) {}
};

struct Result {
    vector<MoveAction> move;
    vector<ConnectAction> connect;
    Result(const vector<MoveAction> &move, const vector<ConnectAction> &con) : move(move), connect(con) {}
};

struct Solver {
    static constexpr int USED = 9;
    int dxy[4];

    int N, K;
    int action_count_limit;
    mt19937 engine;
    int field[2500];
    array<int, 2> rev_field[2500];
    bool hasi[2500];
    vector<int> field_list;

    Solver(int N, int K, const vector<string> &field_, int seed = 0) : 
        N(N), K(K), action_count_limit(K * 100){
        for(int i=1;i<=N;i++){
            for(int j=1;j<=N;j++){
                int pos = i*(N+2)+j;
                field[pos] = field_[i-1][j-1] - '0';
                field_list.emplace_back(pos);
                rev_field[pos] = {i-1, j-1};
                if(i == 1 or i == N or j == 1 or j == N) hasi[pos] = true;
                else hasi[pos] = false;
            }
            field[i*(N+2)] = -1;
            field[(i+1)*(N+2)-1] = -1;
        }
        for(int i=0;i<N+2;i+=N+1){
            for(int j=0;j<N+2;j++){
                field[i*(N+2)+j] = -1;
            }
        }
        // RDLU
        // dxy = {1, N+2, -1, -N-2};
        dxy[0] = 1;
        dxy[1] = N+2;
        dxy[2] = -1;
        dxy[3] = -N-2;
        engine.seed(seed);
    }

    int calc_score(const Result &res){
        for (auto r : res.move) {
            assert(field[r.pos1] != 0);
            assert(field[r.pos2] == 0);
            swap(field[r.pos1], field[r.pos2]);
        }

        UnionFind uf(N*N);

        for (auto r : res.connect) {
            int pos1 = r.pos1;
            int pos2 = r.pos2;
            // TODO: ufのこの処理はバグの要因になりそうなのでどうにかする
            uf.unite(pos1-N-3, pos2-N-3);
        }

        vector<int> computers;
        for(auto &pos : field_list){
            if (field[pos] != 0) {
                computers.emplace_back(pos);
            }
        }

        int score = 0;
        for (int i = 0; i < (int)computers.size(); i++) {
            for (int j = i+1; j < (int)computers.size(); j++) {
                auto c1 = computers[i];
                auto c2 = computers[j];
                if (uf.root(c1) == uf.root(c2)) {
                    score += (field[c1] == field[c2]) ? 1 : -1;
                }
            }
        }

        return max(score, 0);
    }

    bool can_move(int pos, int dir) const{
        int npos = pos + dxy[dir];
        return field[npos] == 0;
    }

    vector<MoveAction> move(int move_limit = -1){
        vector<MoveAction> ret;
        if (move_limit == -1) {
            move_limit = K * 50;
        }

        // for (int i = 0; i < move_limit; i++) {
        //     int row = engine() % N;
        //     int col = engine() % N;
        //     int dir = engine() % 4;
        //     if (field[row][col] != '0' && can_move(row, col, dir)) {
        //         swap(field[row][col], field[row + dx[dir]][col + dy[dir]]);
        //         ret.emplace_back(row, col, row + dx[dir], col + dy[dir]);
        //         action_count_limit--;
        //     }
        // }

        return ret;
    }

    int can_connect(int pos, int dir) const{
        // can connect -> return npos
        // cannot -> return -1
        int npos = pos + dxy[dir];
        while (field[npos] != -1) {
            if (field[npos] == field[pos]) {
                return npos;
            } else if (field[npos] != 0) {
                return -1;
            }
            npos += dxy[dir];
        }
        return -1;
    }

    ConnectAction line_fill(int pos, int dir){
        int npos = pos + dxy[dir];
        while (field[npos] != -1) {
            if (field[npos] == field[pos]) {
                return ConnectAction(pos, npos);
            }
            assert(field[npos] == 0);
            field[npos] = USED;
            npos += dxy[dir];
        }
        assert(false);
    }

    vector<ConnectAction> connect(){
        vector<ConnectAction> ret;
        UnionFind uf(N*N);
        for(auto &pos : field_list){
            if (field[pos] != 0 && field[pos] != USED) {
                for (int dir = 0; dir < 2; dir++) {
                    int npos = can_connect(pos, dir);
                    if(npos == -1) continue;
                    bool is_adjust = (abs(npos - pos) == 1 or abs(npos - pos) == N+2);
                    bool is_hasi = hasi[npos] & hasi[pos];
                    if(is_adjust or is_hasi) {
                        ret.push_back(ConnectAction(pos, npos));
                        action_count_limit--;
                        if (action_count_limit <= 0) {
                            return ret;
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

    void print_answer(const Result &res){
        cout << res.move.size() << endl;
        for (auto m : res.move) {
            auto [x1, y1] = rev_field[m.pos1];
            auto [x2, y2] = rev_field[m.pos2];
            cout << x1 << " " << y1 << " "
                << x2 << " " << y2 << endl;
        }
        cout << res.connect.size() << endl;
        for (auto m : res.connect) {
            auto [x1, y1] = rev_field[m.pos1];
            auto [x2, y2] = rev_field[m.pos2];
            cout << x1 << " " << y1 << " "
                << x2 << " " << y2 << endl;
        }
    }
};



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

    s.print_answer(ret);

}