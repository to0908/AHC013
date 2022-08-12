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
    int _action_count_limit;
    array<int, 2> rev_field[2500];
    int raw_field[2500];
    bool hasi[2500];
    vector<int> field_list;

    int field[2500];
    vector<int> server_pos;
    int field_server_id[2500];
    vector<int> empty_pos;

    BaseSolver(int N, int K, const vector<string> &field_) : 
        N(N), K(K), _action_count_limit(K * 100){
        int cnt = 0;
        server_pos.resize(K*100, -1);
        for(int i=1;i<=N;i++){
            for(int j=1;j<=N;j++){
                int pos = i*(N+2)+j;
                field[pos] = field_[i-1][j-1] - '0';
                if(field[pos] != 0) {
                    server_pos[cnt] = pos;
                    field_server_id[pos] = cnt;
                    cnt++;
                }
                else {
                    empty_pos.emplace_back(pos);
                    field_server_id[pos] = -1;
                }
                field_list.emplace_back(pos);
                rev_field[pos] = {i-1, j-1};
                raw_field[pos] = (i-1)*N + j-1;
                if(i == 1 or i == N or j == 1 or j == N) hasi[pos] = true;
                else hasi[pos] = false;
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
    }

    bool can_move(int pos, int dir) const{
        int npos = pos + dxy[dir];
        return field[npos] == 0;
    }

    vector<MoveAction> base_move(int move_limit = -1){
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

    vector<ConnectAction> connect(int action_count_limit){
        /*
        無害なConnectだけをやっている
        */
        Timer time;
        vector<ConnectAction> ret;
        UnionFind uf(N*N);
        array<int,2> vertical_server_pair[2][2305]={};
        // 前計算
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

        // 無害な連結
        for(auto pos : server_pos){
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
                    auto [pos_x, pos_y] = vertical_server_pair[dir][now];
                    int x = field[pos_x], y = field[pos_y];
                    if(x != 0 and x == y and !uf.same(raw_field[pos_x], raw_field[pos_y])) {
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

        // 貪欲に最も無害なのを繋げる (多分無害？わからんぜ)
        if(1){
            while(true){
                bool connected = false;
                for(auto pos : server_pos){
                    for (int dir = 0; dir < 2; dir++) {
                        int npos = can_connect(pos, dir);
                        if(npos == -1) continue;
                        if(uf.same(raw_field[pos], raw_field[npos])) continue;
                        int sz1 = uf.size(raw_field[pos]), sz2 = uf.size(raw_field[npos]);
                        int score = sz1 * sz2;
                        int now = pos + dxy[dir];
                        while(now != npos) {
                            auto [pos_x, pos_y] = vertical_server_pair[dir][raw_field[now]];
                            int x = field[pos_x], y = field[pos_y];
                            if(x != 0 and x == y and !uf.same(raw_field[pos_x], raw_field[pos_y])) {
                                score -= uf.size(raw_field[pos_x]) * uf.size(raw_field[pos_y]);
                            }
                            now += dxy[dir];
                        }
                        if(score > 0 and uf.unite(raw_field[pos], raw_field[npos])) {
                            connected=true;
                            ret.push_back(line_fill(pos, dir));
                            action_count_limit--;
                            if (action_count_limit <= 0) {
                                return ret;
                            }
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
                    if(uf.same(raw_field[pos], raw_field[npos])) continue;
                    int sz1 = uf.size(raw_field[pos]), sz2 = uf.size(raw_field[npos]);
                    int score = sz1 * sz2;
                    edge.push_back({score, dir, pos, npos});
                }
            }
            sort(all(edge), greater<array<int,4>>());
            for(auto &[score, dir, pos, npos] : edge){
                if(npos != can_connect(pos, dir)) continue;
                if(uf.unite(raw_field[pos], raw_field[npos])){
                    ret.push_back(line_fill(pos, dir));
                    action_count_limit--;
                    if (action_count_limit <= 0) {
                        return ret;
                    }
                }
            }
        }


        // TODO:
        // ここで回数がoverする時は最後までやって得られるScoreが小さいものを分解していくみたいな感じが良いか
        // 分解する際にも、いきなり真ん中で割るとスコアが大きく下がってしまう。端の方から分解するのが良い。
        cerr << "Connect Time = " << time.elapsed() << "\n";
        return ret;
    }


    Result base_solve(){
        // create random moves
        auto moves = base_move(0);
        int action_count_limit = _action_count_limit - (int)moves.size();
        // from each computer, connect to right and/or bottom if it will reach the same type
        auto connects = connect(action_count_limit);
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

    int connect2(int action_count_limit){
        /*
        無害なConnectだけをやっている
        */
        int score = 0;
        UnionFind uf(N*N);
        array<int,2> vertical_server_pair[2][2305]={};
        vector<int> used;

        // 前計算
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

        // 無害な連結
        for(auto pos : server_pos){
            for (int dir = 0; dir < 2; dir++) {
                int npos = can_connect(pos, dir);
                if(npos == -1) continue;
                bool is_adjust = (abs(npos - pos) == 1 or abs(npos - pos) == N+2);
                bool is_hasi = hasi[npos] & hasi[pos];
                int sz1 = uf.size(raw_field[pos]);
                int sz2 = uf.size(raw_field[npos]);
                if((is_adjust or is_hasi) and uf.unite(raw_field[pos], raw_field[npos])) {
                    score += sz1 * sz2;
                    action_count_limit--;
                    if (action_count_limit <= 0) {
                        for(auto i:used) field[i] = 0;
                        return score;
                    }
                    continue;
                }

                bool is_only_this_pair = true;
                int now = pos + dxy[dir];
                while(now != npos) {
                    auto [pos_x, pos_y] = vertical_server_pair[dir][now];
                    int x = field[pos_x], y = field[pos_y];
                    if(x != 0 and x == y and !uf.same(raw_field[pos_x], raw_field[pos_y])) {
                        is_only_this_pair = false;
                        break;
                    }
                    now += dxy[dir];
                }
                if(is_only_this_pair and uf.unite(raw_field[pos], raw_field[npos])) {
                    score += sz1 * sz2;
                    int ps = pos + dxy[dir];
                    while(ps != npos) {
                        if(field[ps] != 0) break;
                        field[ps] = USED;
                        used.push_back(ps);
                        ps += dxy[dir];
                    }
                    action_count_limit--;
                    if (action_count_limit <= 0) {
                        for(auto i:used) field[i] = 0;
                        return score;
                    }
                }
            }
        }

        // 貪欲に最も無害なのを繋げる (多分無害？わからんぜ)
        if(1){
            while(true){
                bool connected = false;
                for(auto pos : server_pos){
                    for (int dir = 0; dir < 2; dir++) {
                        int npos = can_connect(pos, dir);
                        if(npos == -1) continue;
                        if(uf.same(raw_field[pos], raw_field[npos])) continue;
                        int sz1 = uf.size(raw_field[pos]), sz2 = uf.size(raw_field[npos]);
                        int score2 = sz1 * sz2;
                        int now = pos + dxy[dir];
                        while(now != npos) {
                            auto [pos_x, pos_y] = vertical_server_pair[dir][raw_field[now]];
                            int x = field[pos_x], y = field[pos_y];
                            if(x != 0 and x == y and !uf.same(raw_field[pos_x], raw_field[pos_y])) {
                                score2 -= uf.size(raw_field[pos_x]) * uf.size(raw_field[pos_y]);
                            }
                            now += dxy[dir];
                        }
                        if(score2 > 0 and uf.unite(raw_field[pos], raw_field[npos])) {
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
                            if (action_count_limit <= 0) {
                                for(auto i:used) field[i] = 0;
                                return score;
                            }
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
                    if(uf.same(raw_field[pos], raw_field[npos])) continue;
                    int sz1 = uf.size(raw_field[pos]), sz2 = uf.size(raw_field[npos]);
                    int score2 = sz1 * sz2;
                    edge.push_back({score2, dir, pos, npos});
                }
            }
            sort(all(edge), greater<array<int,4>>());
            for(auto &[score2, dir, pos, npos] : edge){
                if(npos != can_connect(pos, dir)) continue;
                if(uf.unite(raw_field[pos], raw_field[npos])){
                    score += score2;
                    int ps = pos + dxy[dir];
                    while(ps != npos) {
                        if(field[ps] != 0) break;
                        field[ps] = USED;
                        used.push_back(ps);
                        ps += dxy[dir];
                    }
                    action_count_limit--;
                    if (action_count_limit <= 0) {
                        for(auto i:used) field[i] = 0;
                        return score;
                    }
                }
            }
        }

        for(auto i:used) field[i] = 0;

        // TODO:
        // ここで回数がoverする時は最後までやって得られるScoreが小さいものを分解していくみたいな感じが良いか
        // 分解する際にも、いきなり真ん中で割るとスコアが大きく下がってしまう。端の方から分解するのが良い。

        return score;
    }

    void empty_move_operation(int emp_idx, int npos){
        int pos = empty_pos[emp_idx];
        assert(field[npos] > 0 and field[npos] <= K);
        assert(field[pos] == 0);
        assert(field_server_id[pos] == -1);
        assert(field_server_id[npos] >= 0);
        assert(field_server_id[npos] < K*100);

        swap(field[pos], field[npos]);
        swap(field_server_id[pos], field_server_id[npos]);
        empty_pos[emp_idx] = npos;
        
        assert(field_server_id[pos] >= 0);
        assert(field_server_id[npos] == -1);
        server_pos[field_server_id[pos]] = pos;
    }

    vector<int> dfs(int limit, int emp_idx, int pre, int action_count_limit) {
        if(limit == 0) {
            return {connect2(action_count_limit)};
        }
        int ma = -1;
        vector<int> op = {-1};
        int pos = empty_pos[emp_idx];
        assert(field[pos] == 0);
        for(int dir=0;dir<4;dir++) {
            int npos = pos + dxy[dir];
            if(npos == pre) continue;
            if(field[npos] > 0){
                assert(field[npos] != USED);
                assert(field[npos] <= K);
                empty_move_operation(emp_idx, npos);
                assert(empty_pos[emp_idx] == npos);
                auto v = dfs(limit-1, emp_idx, pos, action_count_limit-1);
                assert((int)v.size() >= 1);
                int score = v[0];
                if(chmax(ma, score)){
                    op = v;
                    op.push_back(npos);
                }
                empty_move_operation(emp_idx, pos);
            }
        }
        return op;
    }

    vector<MoveAction> move(){
        vector<MoveAction> ret;
        /*
        State:
        - field[(N+2)*(N+2)] <- *int (?)
        - field_hash <- long long (デカいので衝突が怖い)
        - 空白の位置 <- vector<int> ? (長さは固定なので) *intでいけるならそっちのが良い 
        - 操作列 <- vector<int> ?
        - Score <- int
        */
        int action_count_limit = _action_count_limit;
        int score = 0;
        // 実験として、雑なDFSでやる。これで上手くいくならビームを撃つ
        for(int iter=0;iter<100;iter++) {
            int limit = randint() % 7 + 1;
            if(limit > action_count_limit) continue;
            int emp_idx = randint() % (int)empty_pos.size(); // TODO: <- emp_idxはrandomじゃなくて順番でええか
            assert(emp_idx >= 0 and emp_idx < (int)empty_pos.size());
            auto v = dfs(limit, emp_idx, -1, action_count_limit);
            assert((int)v.size() == 1 or (int)v.size() == limit+1);
            if(chmax(score, v[0])) {
                action_count_limit -= (int)v.size() - 1;
                for(int i=(int)v.size()-1;i>=1;i--){
                    int pos = empty_pos[emp_idx];
                    int npos = v[i];
                    {
                        bool ok = false;
                        for(int dir=0;dir<4;dir++) if(pos + dxy[dir] == npos) ok = 1;
                        assert(ok);
                    }
                    assert(pos != npos);
                    assert(field[pos] == 0);
                    assert(field[npos] != -1);
                    assert(field[npos] <= K);
                    ret.push_back(MoveAction(npos, pos));
                    empty_move_operation(emp_idx, npos);
                }
            }
            cerr << iter << " " << score << " " << action_count_limit << "\n";
        }
        return ret;
    }

    Result solve(){
        // create random moves
        cerr << "BEGIN " << connect2(_action_count_limit) << "\n";
        auto moves = move();
        // auto moves = base_move();

        // from each computer, connect to right and/or bottom if it will reach the same type
        int action_count_limit = _action_count_limit - (int)moves.size();

        auto connects = connect(action_count_limit);

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

    if(N*N - K * 100 < 150) {
        cerr << "Solver: Dense" << "\n";
        DenseSolver s(N, K, field);
        auto ret = s.solve();
        s.print_answer(ret);
    }
    else{
        cerr << "Solver: Base" << "\n";
        BaseSolver s(N, K, field);
        auto ret = s.base_solve();
        s.print_answer(ret);
    }

    cerr << "Time = " << time.elapsed() << "\n";
}