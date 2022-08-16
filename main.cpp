// #define _GLIBCXX_DEBUG
// #pragma GCC target("avx2")
// #pragma GCC optimize("O3")
// #pragma GCC optimize("unroll-loops")
#include<bits/stdc++.h> 
using namespace std;
typedef unsigned long long ll;
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

// 固定
// K=2 かつ 15≤N≤39
// K=3 かつ 18≤N≤42
// K=4 かつ 21≤N≤45
// K=5 かつ 24≤N≤48
static constexpr int N_MIN[4] = {15, 18, 21, 24};
static constexpr int N_MAX[4] = {39, 42, 45, 48};

// パラメータ
const int SPARSE_TIME_LIMIT = 2850; // 提出するときは2850にする
const int DENSE_TIME_LIMIT = 2780;
int target_range = 5;
static constexpr int DENSE_THRESHOLD[4] = {3, 3, 3, 3};  // N <= MIN + x -> DENSE

// DENSE
static constexpr int DENSE_BREADTH[4] = {30, 25, 18, 13};
static constexpr int DENSE_SEARCH_LIMIT[4] = {30, 25, 17, 12}; 
static constexpr int DENSE_MAX_MOVE_COUNT[4] = {80, 160, 225, 295};
static constexpr int DENSE_START_LIMIT[4] = {2, 3, 3, 3};
int breadth;
int search_limit;

// SPARSE
static constexpr int SPARSE_BREADTH[4] = {15, 12, 9, 11};
static constexpr int SPARSE_SEARCH_LIMIT[4] = {15, 11, 8, 10}; 
static constexpr int SPARSE_MAX_MOVE_COUNT[4] = {80, 165, 200, 320};
static constexpr int SPARSE_START_LIMIT[4] = {1, 1, 2, 2};


struct BaseSolver {
    const int max_iter = 1000;
    static constexpr int USED = 9;
    const int TIME_LIMIT_CONNECT = 2850;
    int dxy[4];

    int N, K;
    bool Kis2;
    int _action_count_limit;
    array<int, 2> rev_field[2500]; // (N+2)**2 cells -> N*N cells (x, y)
    int raw_field[2500]; // (N+2)**2 cells pos -> N*N cells pos
    int hasi[2500];
    vector<int> field_list; // inner field list (N*N cells)
    Timer time;

    int field[2500]; // field[pos] := -1/0/server
    vector<int> server_pos; // server_pos[server_id] := pos
    int field_server_id[2500]; // field_server_id[pos] := -1/server_id
    int field_empty_id[2500]; // field_empty_id[pos] := -1/empty_id
    vector<int> empty_pos; // empty_pos[empty_id] := pos
    array<int,2> vertical_server_pair[2][2500]; // [dir(0/1)][pos] := {pos1, pos2} (empty or server)
    vector<int> calc_target_server_pos;
    int calc_target_field_server_id[2500]; // field_server_id[pos] := -1/server_id

    // ---------------- beam ------------------
    mt19937_64 engine;
    ll rand[2500][6];

    struct State{
        int score;
        ll field_hash;
        // TODO: int field_id;
        vector<MoveAction> move;

        State(int score, ll field_hash, vector<MoveAction> &move) : 
            score(score), field_hash(field_hash), move(move) {}

        // scoreが大きい順にソートしたい
        bool operator<(const State &s) const{
            return score < s.score;
        }
    };

    ll field_hash() {
        ll ret = 0;
        for(int i=1;i<=N;i++){
            for(int j=1;j<=N;j++) {
                int pos = i * (N+2) + j;
                int x = field[pos];
                ret ^= rand[pos][x];
            }
        }
        return ret;
    }

    ll calc_hash(ll hash, MoveAction &move) {
        assert(field[move.pos1] < 6);
        assert(field[move.pos2] < 6);
        assert(field[move.pos2] >= 0);
        assert(field[move.pos1] >= 0);
        assert(move.pos1 < (N+2)*(N+2));
        assert(move.pos2 < (N+2)*(N+2));
        assert(move.pos2 >= 0);
        assert(move.pos1 >= 0);
        hash ^= rand[move.pos1][field[move.pos1]];
        hash ^= rand[move.pos2][field[move.pos2]];
        hash ^= rand[move.pos2][field[move.pos1]];
        hash ^= rand[move.pos1][field[move.pos2]];
        return hash;
    }

    // ---------------- beam ------------------

    BaseSolver(int N, int K, const vector<string> &field_, Timer &time) : 
        N(N), K(K), _action_count_limit(K * 100), time(time){
        Kis2 = (K==2);
        int cnt = 0;
        server_pos.resize(K*100, -1);
        random_device seed_gen;
        engine.seed(seed_gen());
        for(int i=0;i<(N+2)*(N+2);i++){
            for(int j=0;j<6;j++){
                rand[i][j] = engine();
            }
        }
        for(int i=0;i<2500;i++)calc_target_field_server_id[i]=-1;
        for(int i=1;i<=N;i++){
            for(int j=1;j<=N;j++){
                int pos = i*(N+2)+j;
                field[pos] = field_[i-1][j-1] - '0';
                if(field[pos] != 0) {
                    server_pos[cnt] = pos;
                    field_server_id[pos] = cnt;
                    field_empty_id[pos] = -1;
                    cnt++;
                    if(field[pos] <= target_range){
                        calc_target_field_server_id[pos] = calc_target_server_pos.size();
                        calc_target_server_pos.emplace_back(pos);
                    }
                }
                else {
                    field_empty_id[pos] = empty_pos.size();
                    empty_pos.emplace_back(pos);
                    field_server_id[pos] = -1;
                }
                field_list.emplace_back(pos);
                rev_field[pos] = {i-1, j-1};
                raw_field[pos] = (i-1)*N + j-1;
                hasi[pos] = 0;
                if(i == 1) hasi[pos] += 1;
                if(i == N) hasi[pos] += 2;
                if(j == 1) hasi[pos] += 4;
                if(j == N) hasi[pos] += 8;
            }
            field[i*(N+2)] = -1;
            field[(i+1)*(N+2)-1] = -1;
            field_server_id[i*(N+2)] = -1;
            field_server_id[(i+1)*(N+2)-1] = -1;
        }
        for(int i=0;i<N+2;i+=N+1){
            for(int j=0;j<N+2;j++){
                int pos = i*(N+2)+j;
                field[pos] = -1;
                field_server_id[pos] = -1;
            }
        }
        // RDLU
        dxy[0] = 1;
        dxy[1] = N+2;
        dxy[2] = -1;
        dxy[3] = -N-2;

        for(int i=0;i<2;i++)for(int j=0;j<2500;j++)vertical_server_pair[i][j] = {0, 0};

        for(int dir=0;dir<2;dir++){
            for(auto &pos : empty_pos) {
                if(vertical_server_pair[dir][pos][0] != 0) continue;
                // 常にindexが(min, max)になるようにしておく
                update_vertical_info(pos, dir);
            }
        }
    }

    bool can_move(int pos, int dir) const{
        int npos = pos + dxy[dir];
        return field[npos] == 0;
    }

    void empty_move_operation(int emp_id, int npos){
        int pos = empty_pos[emp_id];
        assert(field[npos] > 0 and field[npos] <= K);
        assert(field[pos] == 0);
        assert(field_server_id[pos] == -1);
        assert(field_server_id[npos] >= 0);
        assert(field_server_id[npos] < K*100);

        swap(field[pos], field[npos]);
        swap(field_server_id[pos], field_server_id[npos]);
        swap(field_empty_id[pos], field_empty_id[npos]);
        swap(calc_target_field_server_id[pos], calc_target_field_server_id[npos]);
        empty_pos[emp_id] = npos;
        server_pos[field_server_id[pos]] = pos;
        if(field[pos] <= target_range) calc_target_server_pos[calc_target_field_server_id[pos]] = pos;

        assert(field_server_id[pos] >= 0);
        assert(field_server_id[npos] == -1);
        // server
        for(int dir=0;dir<4;dir++){
            int nx = pos + dxy[dir];
            while(field[nx] == 0) {
                // RDLU
                // 0101
                vertical_server_pair[(dir&1)^1][nx][(dir<=1)?0:1] = pos;
                nx += dxy[dir];
            }
        }
        // empty
        for(int dir=0;dir<2;dir++){
            update_vertical_info(npos, dir);
        }
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

    vector<ConnectAction> base_connect(int action_count_limit){

        Timer time;
        vector<ConnectAction> ret;
        UnionFind uf(K*100);


        // 無害な連結
        for(auto pos : server_pos){
            for (int dir = 0; dir < 2; dir++) {
                int npos = can_connect(pos, dir);
                if(npos == -1) continue;
                bool is_adjust = (abs(npos - pos) == 1 or abs(npos - pos) == N+2);
                bool is_hasi = hasi[npos] & hasi[pos];
                if((is_adjust or is_hasi) and uf.unite(field_server_id[pos], field_server_id[npos])) {
                    ret.push_back(ConnectAction(pos, npos));
                    action_count_limit--;
                    continue;
                }

                bool is_only_this_pair = true;
                int now = pos + dxy[dir];
                while(now != npos) {
                    assert(field[now] == 0);
                    auto [pos_x, pos_y] = vertical_server_pair[dir][now];
                    int x = field[pos_x], y = field[pos_y];
                    if(x > 0 and x == y and !uf.same(field_server_id[pos_x], field_server_id[pos_y])) {
                        is_only_this_pair = false;
                        break;
                    }
                    now += dxy[dir];
                }
                if(is_only_this_pair and uf.unite(field_server_id[pos], field_server_id[npos])) {
                    ret.push_back(line_fill(pos, dir));
                    action_count_limit--;
                }
            }
        }

        // 貪欲に最も無害なのを繋げる (多分無害？わからんぜ) <- 最適ではありません…
        if(1){
            while(true){
                bool connected = false;
                for(auto pos : server_pos){
                    for (int dir = 0; dir < 2; dir++) {
                        int npos = can_connect(pos, dir);
                        if(npos == -1) continue;
                        if(uf.same(field_server_id[pos], field_server_id[npos])) continue;
                        int sz1 = uf.size(field_server_id[pos]), sz2 = uf.size(field_server_id[npos]);
                        int score = sz1 * sz2;
                        int now = pos + dxy[dir];
                        while(now != npos) {
                            auto [pos_x, pos_y] = vertical_server_pair[dir][now];
                            int x = field[pos_x], y = field[pos_y];
                            if(x != 0 and x == y and !uf.same(field_server_id[pos_x], field_server_id[pos_y])) {
                                score -= uf.size(field_server_id[pos_x]) * uf.size(field_server_id[pos_y]);
                            }
                            now += dxy[dir];
                        }
                        if(score > 0 and uf.unite(field_server_id[pos], field_server_id[npos])) {
                            connected=true;
                            ret.push_back(line_fill(pos, dir));
                            action_count_limit--;
                        }
                    }
                }
                if(!connected) break;
            }
        }

        // 貪欲にスコアが高いものから連結 (有害)
        if(1){
            vector<array<int, 4>> edge;
            for(auto pos : server_pos){
                for (int dir = 0; dir < 2; dir++) {
                    int npos = can_connect(pos, dir);
                    if(npos == -1) continue;
                    if(uf.same(field_server_id[pos], field_server_id[npos])) continue;
                    int sz1 = uf.size(field_server_id[pos]), sz2 = uf.size(field_server_id[npos]);
                    int score = sz1 * sz2;
                    edge.push_back({score, dir, pos, npos});
                }
            }
            sort(all(edge), greater<array<int,4>>());
            for(auto &[score, dir, pos, npos] : edge){
                if(npos != can_connect(pos, dir)) continue;
                if(uf.unite(field_server_id[pos], field_server_id[npos])){
                    ret.push_back(line_fill(pos, dir));
                    action_count_limit--;
                }
            }
        }


        if(action_count_limit < 0) {
            vector<array<int, 2>> sz;
            for(int i=0;i<K*100;i++) {
                if(i == uf.root(i) and uf.size(i) > 1) sz.push_back({uf.size(i), i});
            }
            sort(all(sz));
            vector<ConnectAction> nret;
            for(int i=0;i<(int)sz.size();i++){
                action_count_limit += sz[i][0]-1;
                if(action_count_limit >= 0) {
                    vector<ConnectAction> tmp;
                    for(auto &action : ret) {
                        bool ok = true;
                        for(int j=0;j<=i;j++){
                            if(uf.same(sz[j][1], field_server_id[action.pos1])) {
                                if(action_count_limit > 1 and i == j){
                                    tmp.push_back(action);
                                }
                                else if(action_count_limit == 1 and i == j) {
                                    action_count_limit--;
                                    break;
                                }
                                ok = false;
                                break;
                            }
                        }
                        if(ok) nret.push_back(action);
                    }
                    if(action_count_limit > 1) {
                        queue<int> que;
                        que.push(tmp[0].pos1);
                        que.push(tmp[0].pos2);
                        action_count_limit--;
                        nret.push_back(tmp[0]);
                        bool used[(int)tmp.size()] = {};
                        while(que.size()) {
                            if(action_count_limit == 0) break;
                            int now = que.front(); que.pop();
                            for(int c=1;c<(int)tmp.size();c++) {
                                if(used[c]) continue;
                                if(tmp[c].pos1 == now or tmp[c].pos2 == now) {
                                    used[c] = 1;
                                    nret.push_back(tmp[c]);
                                    action_count_limit--;
                                    if(action_count_limit == 0) break;
                                    if(tmp[c].pos1 != now) que.push(tmp[c].pos1);
                                    if(tmp[c].pos2 != now) que.push(tmp[c].pos2);
                                } 
                            }
                        }
                    }
                    break;
                }
            }
            swap(ret, nret);
        }

        // // TODO: 時間いっぱいConnectを切ってMoveで再Connect
        // while(time.elapsed() < TIME_LIMIT_CONNECT){
        //     // TODO
        // }

        cerr << "Connect Time = " << time.elapsed() << "\n";
        return ret;
    }

    int calc_connect_score(int action_count_limit){
        int score = 0;
        UnionFind uf(K*100);
        vector<int> used;

        // 無害な連結
        vector<array<int,3>> connect_pair;
        for(auto pos : calc_target_server_pos){
            for (int dir = 0; dir < 2; dir++) {
                int npos = can_connect(pos, dir);
                if(npos == -1) continue;
                bool is_adjust = (abs(npos - pos) == 1 or abs(npos - pos) == N+2);
                bool is_hasi = hasi[npos] & hasi[pos];
                int sz1 = uf.size(field_server_id[pos]);
                int sz2 = uf.size(field_server_id[npos]);
                if((is_adjust or is_hasi) and uf.unite(field_server_id[pos], field_server_id[npos])) {
                    score += sz1 * sz2;
                    action_count_limit--;
                    continue;
                }

                bool is_only_this_pair = true;
                int now = pos + dxy[dir];
                while(now != npos) {
                    auto [pos_x, pos_y] = vertical_server_pair[dir][now];
                    int x = field[pos_x], y = field[pos_y];
                    if(x > 0 and x == y and !uf.same(field_server_id[pos_x], field_server_id[pos_y])) {
                        is_only_this_pair = false;
                        break;
                    }
                    now += dxy[dir];
                }
                if(is_only_this_pair and uf.unite(field_server_id[pos], field_server_id[npos])) {
                    score += sz1 * sz2;
                    int ps = pos + dxy[dir];
                    while(ps != npos) {
                        if(field[ps] != 0) break;
                        field[ps] = USED;
                        used.push_back(ps);
                        ps += dxy[dir];
                    }
                    action_count_limit--;
                }
                else if(!is_only_this_pair and !uf.same(field_server_id[pos], field_server_id[npos])) {
                    connect_pair.push_back({pos, npos, dir});
                }
            }
        }

        // 貪欲に最も無害なのを繋げる (多分無害？わからんぜ)
        {
            while(true){
                bool connected = false;
                for(auto it = connect_pair.begin(); it != connect_pair.end();){
                    auto &[pos, npos, dir] = *it;
                    if(uf.same(field_server_id[pos], field_server_id[npos])) {
                        it = connect_pair.erase(it);
                        continue;
                    }
                    int nnpos = can_connect(pos, dir);
                    if(nnpos == -1) {
                        it = connect_pair.erase(it);
                        continue;
                    }
                    int sz1 = uf.size(field_server_id[pos]), sz2 = uf.size(field_server_id[npos]);
                    int score2 = sz1 * sz2;
                    int now = pos + dxy[dir];
                    vector<array<int,3>> era;
                    while(now != npos) {
                        auto [pos_x, pos_y] = vertical_server_pair[dir][now];
                        int x = field[pos_x], y = field[pos_y];
                        if(x != 0 and x == y and !uf.same(field_server_id[pos_x], field_server_id[pos_y])) {
                            score2 -= uf.size(field_server_id[pos_x]) * uf.size(field_server_id[pos_y]);
                            era.push_back({pos_x, pos_y, dir?1:0});
                        }
                        now += dxy[dir];
                    }
                    if(score2 > 0 and uf.unite(field_server_id[pos], field_server_id[npos])) {
                        connected=true;
                        score += sz1 * sz2;
                        int ps = pos + dxy[dir];
                        while(ps != npos) {
                            if(field[ps] != 0) break;
                            field[ps] = USED;
                            used.push_back(ps);
                            ps += dxy[dir];
                        }
                        action_count_limit--;
                        it = connect_pair.erase(it);
                    }
                    else{
                        ++it;
                    }
                }
                if(!connected) break;
            }
        }


        // 貪欲にスコアが高いものから連結 (有害)
        // K=2では十分回るので正確性が重視され、K>=3ではより深く(Moveを多く使う)方が重視されるので省く、みたいな？
        {
            vector<array<int, 4>> edge;
            for(auto it = connect_pair.begin(); it != connect_pair.end();++it){
                auto &[pos, npos, dir] = *it;
                if(uf.same(field_server_id[pos], field_server_id[npos])) {
                    continue;
                }
                int nnpos = can_connect(pos, dir);
                if(nnpos == -1) {
                    continue;
                }
                int sz1 = uf.size(field_server_id[pos]), sz2 = uf.size(field_server_id[npos]);
                int score2 = sz1 * sz2;
                edge.push_back({score2, dir, pos, npos});
            }
            sort(all(edge), greater<array<int,4>>());
            for(auto &[score2, dir, pos, npos] : edge){
                if(npos != can_connect(pos, dir)) continue;
                if(uf.unite(field_server_id[pos], field_server_id[npos])){
                    score += score2;
                    int ps = pos + dxy[dir];
                    while(ps != npos) {
                        if(field[ps] != 0) break;
                        field[ps] = USED;
                        used.push_back(ps);
                        ps += dxy[dir];
                    }
                    action_count_limit--;
                }
            }
        }


        if(action_count_limit < 0) {
            vector<array<int, 2>> sz;
            for(int i=0;i<K*100;i++) {
                if(i == uf.root(i) and uf.size(i) > 1) sz.push_back({uf.size(i), i});
            }
            sort(all(sz));
            for(int i=0;i<(int)sz.size();i++){
                action_count_limit += sz[i][0]-1;
                if(action_count_limit < 0) score -= sz[i][0] * (sz[i][0]-1) / 2;
                else {
                    score -= sz[i][0] * (sz[i][0]-1) / 2;
                    score += action_count_limit * (action_count_limit+1) / 2;
                    break;
                }
            }
        }
        for(auto i:used) field[i] = 0;

        return score;
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

private:
    void update_vertical_info(int pos, int dir){
        if(dir == 0) {
            int x = get_server_pos(pos, 3);
            int y = get_server_pos(pos, 1);
            int d = dxy[1];
            assert(x <= y);
            assert(field[x] != -1);
            assert(field[y] != -1);
            for(int npos=x; npos <= y; npos += d){
                vertical_server_pair[dir][npos] = {x, y};
            }
        }
        else{
            int x = get_server_pos(pos, 2);
            int y = get_server_pos(pos, 0);
            int d = dxy[0];
            assert(x <= y);
            assert(field[x] != -1);
            assert(field[y] != -1);
            for(int npos=x; npos <= y; npos += d){
                vertical_server_pair[dir][npos] = {x, y};
            }
        }
    }
};


struct DenseSolver : public BaseSolver{

    int max_move_size;
    vector<int> rand_empty_permutation;

    DenseSolver(int N, int K, const vector<string> &field_, Timer &time) : BaseSolver(N, K, field_, time) {
        rand_empty_permutation.resize((int)empty_pos.size());
        for(int i=0;i<(int)rand_empty_permutation.size();i++)rand_empty_permutation[i] = i;
        shuffle(rand_empty_permutation.begin(), rand_empty_permutation.end(), engine);
    }

    void dfs(int limit, int emp_id, int pre, 
            State &state, priority_queue<State> *pq, unordered_map<ll, bool> &used, int depth) {
        if(depth+1 == max_move_size)return;

        int pos = empty_pos[emp_id];
        for(int dir = 0; dir < 4; dir++){
            int npos = pos + dxy[dir];
            if(npos == pre) continue;
            if(field[npos] > 0){
                assert(field[npos] != USED);
                assert(field[npos] <= K);

                MoveAction mv = MoveAction(npos, pos);
                ll nhash = calc_hash(state.field_hash, mv);
                if(used[nhash]) continue;
                used[nhash]=1;

                empty_move_operation(emp_id, npos);
                int nscore = calc_connect_score(_action_count_limit - (int)state.move.size() - 1);
                
                state.move.push_back(mv);
                pq[depth+1].push(State(nscore, nhash, state.move));
                if(limit != 1) dfs(limit-1, emp_id, pos, state, pq, used, depth);
                state.move.pop_back();
                empty_move_operation(emp_id, pos);
            }
        }
    }

    vector<MoveAction> move(){

        max_move_size = DENSE_MAX_MOVE_COUNT[K-2];
        priority_queue<State> pq[max_move_size + 1];
        unordered_map<ll, bool> used;

        int best_score = 0;
        ll hash = field_hash();
        vector<MoveAction> best_move;
        State initial_state = State(0, hash, best_move);
        pq[0].push(initial_state);

        int depth = 0;
        int depth_cnt = 0;
        int idx = 0;
        int limit = DENSE_START_LIMIT[K-2];
        while(int ti = time.elapsed() < DENSE_TIME_LIMIT) {
            if(ti > 2000) limit = 2;
            if(depth == max_move_size) break;
            if(pq[depth].empty()) {
                depth++;
                depth_cnt = 0;
                continue;
            }
            State state = pq[depth].top();
            pq[depth].pop();
            if(depth_cnt == 0 and chmax(best_score, state.score)) {
                // cerr << depth << " " << state.score << " " << state.field_hash << "\n";
                best_move = state.move;
            }
            depth_cnt++;

            for(auto &mv : state.move) {
                // cerr << "-> " << mv.pos1 << " " << mv.pos2 << "\n";
                empty_move_operation(field_empty_id[mv.pos2], mv.pos1);
            }

            // empの個数が小さい場合、結構衝突することが予想される。
            // ランダムに順番を設定し、その順でやるか？
            for(int iter=0;iter<min(search_limit, (int)rand_empty_permutation.size());iter++){
                int emp_id = rand_empty_permutation[idx];
                idx++;
                if(idx == (int)rand_empty_permutation.size())idx = 0;
                dfs(limit, emp_id, -1, state, pq, used, depth);
            }

            
            for (auto itr = state.move.rbegin(); itr != state.move.rend(); ++itr) {
                // cerr << "<- " << itr->pos1 << " " << itr->pos2 << "\n";
                empty_move_operation(field_empty_id[itr->pos1], itr->pos2);
            }

            if(depth_cnt == breadth) {
                depth_cnt = 0;
                depth++;
            }
        }


        for(int i=depth;i<depth+2;i++) {
            if(i >= max_move_size) break;
            if(pq[i].empty()) continue;
            State state = pq[i].top();
            if(chmax(best_score, state.score)) {
                // cerr << i << " " << state.score << " " << state.field_hash << "\n";
                best_move = state.move;
            }
        }
        return best_move;
    }

    Result solve(){
        auto moves = move();
        for(auto &mv : moves) {
            empty_move_operation(field_empty_id[mv.pos2], mv.pos1);
        }

        int action_count_limit = _action_count_limit - (int)moves.size();

        auto connects = base_connect(action_count_limit);

        return Result(moves, connects);
    }
};


struct SparseSolver : public BaseSolver{
    int max_move_size;
    SparseSolver(int N, int K, const vector<string> &field_, Timer &time) : BaseSolver(N, K, field_, time) {}


    Result solve(){
        auto moves = move();

        int action_count_limit = _action_count_limit - (int)moves.size();
        for(auto &mv : moves) {
            empty_move_operation(field_empty_id[mv.pos2], mv.pos1);
        }

        auto connects = base_connect(action_count_limit);

        return Result(moves, connects);
    }

private:

    void dfs(int limit, int server_id, int pre, 
            State &state, priority_queue<State> *pq, unordered_map<ll, bool> &used, int depth) {
        if(depth+1 == max_move_size)return;

        int pos = server_pos[server_id];
        for(int dir=0;dir<4;dir++) {
            if(can_move(pos, dir) == false) continue;
            int npos = pos + dxy[dir];
            if(npos == pre) continue;
            MoveAction mv = MoveAction(pos, npos);
            ll nhash = calc_hash(state.field_hash, mv);
            if(used[nhash]) continue;
            used[nhash]=1;

            empty_move_operation(field_empty_id[npos], pos);
            int nscore = calc_connect_score(_action_count_limit - (int)state.move.size() - 1);

            state.move.push_back(mv);
            pq[depth+1].push(State(nscore, nhash, state.move));
            if(limit != 1) dfs(limit-1, server_id, pos, state, pq, used, depth);
            empty_move_operation(field_empty_id[pos], npos);
            state.move.pop_back();
        }
    }

    vector<MoveAction> move(){
        vector<MoveAction> ret;

        max_move_size = SPARSE_MAX_MOVE_COUNT[K-2];
        priority_queue<State> pq[max_move_size + 1];
        unordered_map<ll, bool> used;

        int best_score = 0;
        ll hash = field_hash();
        vector<MoveAction> best_move;
        State initial_state = State(0, hash, best_move);
        pq[0].push(initial_state);

        int depth = 0;
        int limit = SPARSE_START_LIMIT[K-2];
        int depth_cnt = 0;
        while(int ti = time.elapsed() < SPARSE_TIME_LIMIT) {
            if(depth == max_move_size) break;
            if(ti > 2400) limit = 1;
            if(pq[depth].empty()) {
                depth++;
                depth_cnt = 0;
                continue;
            }
            State state = pq[depth].top();
            pq[depth].pop();
            if(depth_cnt == 0 and chmax(best_score, state.score)) {
                best_move = state.move;
            }
            depth_cnt++;

            for(auto &mv : state.move) {
                empty_move_operation(field_empty_id[mv.pos2], mv.pos1);
            }

            for(int iter=0;iter<search_limit;iter++){
                int server_id = randint() % (int)server_pos.size();
                // int pos = server_pos[server_id];
                // for(int dir=0;dir<4;dir++) {
                //     if(can_move(pos, dir) == false) continue;
                //     int npos = pos + dxy[dir];
                //     MoveAction mv = MoveAction(pos, npos);
                //     ll nhash = calc_hash(state.field_hash, mv);
                //     if(used[depth+1][nhash]) continue;
                //     used[depth+1][nhash]=1;

                //     empty_move_operation(field_empty_id[npos], pos);
                //     int nscore = calc_connect_score(_action_count_limit - (int)state.move.size() - 1);
                //     empty_move_operation(field_empty_id[pos], npos);

                //     state.move.push_back(mv);
                //     pq[depth+1].push(State(nscore, nhash, state.move));
                //     state.move.pop_back();
                // }
                dfs(limit, server_id, -1, state, pq, used, depth);
            }

            
            for (auto itr = state.move.rbegin(); itr != state.move.rend(); ++itr) {
                empty_move_operation(field_empty_id[itr->pos1], itr->pos2);
            }

            if(depth_cnt == breadth) {
                depth_cnt = 0;
                depth++;
            }
        }


        for(int i=depth;i<depth+2;i++) {
            if(i >= max_move_size) break;
            if(pq[i].empty()) continue;
            State state = pq[i].top();
            if(chmax(best_score, state.score)) {
                // cerr << i << " " << state.score << " " << state.field_hash << "\n";
                best_move = state.move;
            }
        }
        return best_move;
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

    if(N - N_MIN[K-2] <= DENSE_THRESHOLD[K-2]) {
        cerr << "Solver: Dense" << "\n";

        // if(K == 5) target_range = 3; // 3


        breadth = DENSE_BREADTH[K-2];
        search_limit = DENSE_SEARCH_LIMIT[K-2];

        // DenseSolver s(N, K, field, time);
        // auto ret = s.solve();
        // s.print_answer(ret);

        cout << 0 << endl;
        cout << 0 << endl;
    }
    else {
        cerr << "Solver: Sparse" << "\n";

        if(K == 5) target_range = 3; // 3
        
        breadth = SPARSE_BREADTH[K-2];
        search_limit = SPARSE_SEARCH_LIMIT[K-2];

        int margin = N_MAX[K-2] - N;
        int t = (K>=4)?3:1;
        if(K == 2){
            int tmp = margin;
            margin -= 1;
            if(margin >= 15) margin *= 1.5;
            else if(margin <= 5) margin += 2;
            if(tmp >= 18) margin += 3;
        }
        else if(K == 3){
            if(margin >= 18) margin++;
            if(margin <= 4) margin += 2;
            margin -= 2;
            if(margin >= 10) margin -= 2;
        }
        else if(K == 4){
            if(margin == 20) margin += 3;
            if(margin >= 15) margin += 12;
            margin -= 3;
        }
        else if(K == 5){
            if(margin == 20) margin += 9;
            else if(margin >= 18) margin += 6;
            else if(margin >= 15) margin += 3;
            else margin -= 6;
        }
        breadth += margin / t;
        search_limit += margin / t;


        if(K < 4) {
            cout << 0 << endl;
            cout << 0 << endl;
        }
        else{
            SparseSolver s(N, K, field, time);
            auto ret = s.solve();
            s.print_answer(ret);
        }

    }

    cerr << "Time = " << time.elapsed() << "\n";
}