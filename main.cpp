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
    MoveAction(int pos1, int pos2) : pos1(pos1), pos2(pos2) {}
};

struct ConnectAction {
    int pos1, pos2;
    ConnectAction(int pos1, int pos2) : pos1(pos1), pos2(pos2) {}
};

struct Result {
    vector<MoveAction> move;
    vector<ConnectAction> connect;
    Result(const vector<MoveAction> &move, const vector<ConnectAction> &con) : move(move), connect(con) {}
};

struct BaseSolver {
    static constexpr int USED = 9;
    const int TIME_LIMIT = 2800;
    int dxy[4];

    int N, K;
    int action_count_limit;
    int field[2500];
    array<int, 2> rev_field[2500];
    int raw_field[2500];
    bool hasi[2500];
    vector<int> field_list;
    int server_pos[5][100];
    vector<int> empty_pos;

    BaseSolver(int N, int K, const vector<string> &field_) : 
        N(N), K(K), action_count_limit(K * 100){
        int cnt[K] = {};
        for(int i=1;i<=N;i++){
            for(int j=1;j<=N;j++){
                int pos = i*(N+2)+j;
                field[pos] = field_[i-1][j-1] - '0';
                if(field[pos] != 0) {
                    server_pos[field[pos]-1][cnt[field[pos]-1]] = pos;
                    cnt[field[pos]-1]++;
                }
                else {
                    empty_pos.emplace_back(pos);
                }
                field_list.emplace_back(pos);
                rev_field[pos] = {i-1, j-1};
                raw_field[pos] = (i-1)*N + j-1;
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
        dxy[0] = 1;
        dxy[1] = N+2;
        dxy[2] = -1;
        dxy[3] = -N-2;
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
            uf.unite(raw_field[pos1], raw_field[pos2]);
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

    int get_server_pos(int pos, int dir) {
        int npos = pos + dxy[dir];
        while (field[npos] != -1) {
            if(field[npos] != 0) return npos;
            npos += dxy[dir];
        }
        return npos - dxy[dir];
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
        /*
        無害なConnectだけをやっている
        */
        vector<ConnectAction> ret;
        UnionFind uf(N*N);
        array<int,2> vertical_server_pair[2][2500]={};
        for(int dir=0;dir<2;dir++){
            for(auto &pos : empty_pos) {
                if(vertical_server_pair[dir][raw_field[pos]][0] != 0) continue;
                int x = get_server_pos(pos, dir+1);
                int y = get_server_pos(pos, dir?0:3);
                int d = dir?dxy[0]:dxy[1];
                for(int npos=min(x, y); npos <= max(x, y); npos += d){
                    vertical_server_pair[dir][raw_field[npos]] = {x, y};
                }
            }
        }
        for(int i=0;i<K;i++){
            for(auto pos : server_pos[i]){
                for (int dir = 0; dir < 2; dir++) {
                    int npos = can_connect(pos, dir);
                    if(npos == -1) continue;
                    bool is_adjust = (abs(npos - pos) == 1 or abs(npos - pos) == N+2);
                    bool is_hasi = hasi[npos] & hasi[pos];
                    if((is_adjust or is_hasi) and uf.unite(raw_field[pos], raw_field[npos])) {
                        ret.push_back(ConnectAction(pos, npos));
                        action_count_limit--;
                        if (action_count_limit <= 0) {
                            return ret;
                        }
                        continue;
                    }

                    bool is_only_this_pair = true;
                    int now = pos + dxy[dir];
                    while(now != npos) {
                        auto [x, y] = vertical_server_pair[dir][now];
                        x = field[x];
                        y = field[y];
                        if(x != 0 and x == y and !uf.same(x, y)) {
                            is_only_this_pair = false;
                            break;
                        }
                        now += dxy[dir];
                    }
                    if(is_only_this_pair and uf.unite(raw_field[pos], raw_field[npos])) {
                        ret.push_back(line_fill(pos, dir));
                        action_count_limit--;
                        if (action_count_limit <= 0) {
                            return ret;
                        }
                    }
                }
            }
        }


        // TODO:
        // ここで回数がoverする時は最後までやって得られるScoreが小さいものを分解していくみたいな感じが良いか
        // 分解する際にも、いきなり真ん中で割るとスコアが大きく下がってしまう。端の方から分解するのが良い。

        return ret;
    }

    Result base_solve(){
        // create random moves
        auto moves = move(0);

        // from each computer, connect to right and/or bottom if it will reach the same type
        auto connects = connect();
        return Result(moves, connects);
    }

    void print_answer(const Result &res){
        cout << res.move.size() << "\n";
        for (auto m : res.move) {
            auto [x1, y1] = rev_field[m.pos1];
            auto [x2, y2] = rev_field[m.pos2];
            cout << x1 << " " << y1 << " "
                << x2 << " " << y2 << "\n";
        }
        cout << res.connect.size() << "\n";
        for (auto m : res.connect) {
            auto [x1, y1] = rev_field[m.pos1];
            auto [x2, y2] = rev_field[m.pos2];
            cout << x1 << " " << y1 << " "
                << x2 << " " << y2 << "\n";
        }
    }
};


struct DenseSolver : public BaseSolver{

    DenseSolver(int N, int K, const vector<string> &field_) : BaseSolver(N, K, field_) {}

    Result solve(){
        // create random moves
        auto moves = move(0);

        // from each computer, connect to right and/or bottom if it will reach the same type
        auto connects = connect();
        return Result(moves, connects);
    }
};


int main(){

    Timer time;

    int N, K;
    cin >> N >> K;
    vector<string> field(N);
    for (int i = 0; i < N; i++) {
        cin >> field[i];
    }

    if(N*N - K * 100 < 100) {
        cerr << "Dense Solver" << endl;
        DenseSolver s(N, K, field);
        auto ret = s.solve();
        s.print_answer(ret);
    }
    else{
        BaseSolver s(N, K, field);
        auto ret = s.base_solve();
        s.print_answer(ret);
    }

    cerr << "Time = " << time.elapsed() << endl;
}