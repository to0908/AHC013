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
    ConnectAction(){}
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
    vector<int> field_list;
    array<int, 2> rev_field[2500]; // (N+2)**2 cells -> N*N cells (x, y)
    bool hasi[2500];
    Timer time;

    int field[2500]; // field[pos] := -1/0/server
    vector<int> server_pos; // server_pos[server_id] := pos
    int field_server_id[2500]; // field_server_id[pos] := -1/server_id
    int field_empty_id[2500]; // field_empty_id[pos] := -1/empty_id
    vector<int> empty_pos; // empty_pos[empty_id] := pos
    array<int,2> vertical_server_pair[2][2500]; // [dir(0/1)][pos] := {pos1, pos2} (empty or server)

    BaseSolver(int N, int K, const vector<string> &field_, Timer &time) : 
        N(N), K(K), _action_count_limit(K * 100), time(time){
        int cnt = 0;
        server_pos.resize(K*100, -1);
        for(int i=1;i<=N;i++){
            for(int j=1;j<=N;j++){
                int pos = i*(N+2)+j;
                field[pos] = field_[i-1][j-1] - '0';
                if(field[pos] != 0) {
                    server_pos[cnt] = pos;
                    field_server_id[pos] = cnt;
                    field_empty_id[pos] = -1;
                    cnt++;
                }
                else {
                    field_empty_id[pos] = empty_pos.size();
                    empty_pos.emplace_back(pos);
                    field_server_id[pos] = -1;
                }
                field_list.emplace_back(pos);
                rev_field[pos] = {i-1, j-1};
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

    void empty_move_operation(int emp_id, int npos, bool is_update_vertical_info=true){
        int pos = empty_pos[emp_id];
        assert(field[npos] > 0 and field[npos] <= K);
        assert(field[pos] == 0);
        assert(field_server_id[pos] == -1);
        assert(field_server_id[npos] >= 0);
        assert(field_server_id[npos] < K*100);

        swap(field[pos], field[npos]);
        swap(field_server_id[pos], field_server_id[npos]);
        swap(field_empty_id[pos], field_empty_id[npos]);
        empty_pos[emp_id] = npos;
        server_pos[field_server_id[pos]] = pos;

        assert(field_server_id[pos] >= 0);
        assert(field_server_id[npos] == -1);

        if(is_update_vertical_info){
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

    vector<ConnectAction> base_connect(int action_count_limit){
        /*
        無害なConnectだけをやっている
        */
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

        cerr << "Connect Time = " << time.elapsed() << "\n";
        return ret;
    }

    int calc_connect_score(int action_count_limit){
        int score = 0;
        UnionFind uf(K*100);
        vector<int> used;

        // 無害な連結

        for(auto pos : server_pos){
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
                        if(uf.same(field_server_id[pos], field_server_id[npos])) continue;
                        int sz1 = uf.size(field_server_id[pos]), sz2 = uf.size(field_server_id[npos]);
                        int score2 = sz1 * sz2;
                        int now = pos + dxy[dir];
                        while(now != npos) {
                            auto [pos_x, pos_y] = vertical_server_pair[dir][now];
                            int x = field[pos_x], y = field[pos_y];
                            if(x != 0 and x == y and !uf.same(field_server_id[pos_x], field_server_id[pos_y])) {
                                score2 -= uf.size(field_server_id[pos_x]) * uf.size(field_server_id[pos_y]);
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
                    int score2 = sz1 * sz2;
                    edge.push_back({score2, dir, pos, npos});
                }
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
                if(action_count_limit < 0) score -= ((sz[i][0] * (sz[i][0]-1)) >> 1);
                else {
                    score -= ((sz[i][0] * (sz[i][0]-1))>>1);
                    score += ((action_count_limit * (action_count_limit+1)) >> 1);
                    break;
                }
            }
        }
        for(auto i:used) field[i] = 0;

        return score;
    }

    Result base_solve(){
        // create random moves
        auto moves = base_move(0);
        int action_count_limit = _action_count_limit - (int)moves.size();
        // from each computer, connect to right and/or bottom if it will reach the same type
        auto connects = base_connect(action_count_limit);
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
    
    const int max_iter = 100;

    DenseSolver(int N, int K, const vector<string> &field_, Timer &time) : BaseSolver(N, K, field_, time) {}

    vector<int> dfs(int limit, int emp_idx, int pre, int action_count_limit) {
        if(limit == 0) {
            return {calc_connect_score(action_count_limit)};
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
                auto v = dfs(limit-1, emp_idx, pos, action_count_limit);
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
        int iter = 0;
        // while(time.elapsed() < TIME_LIMIT) {
        while(iter < max_iter){ // ローカルで動かす時にスコアが安定するように
            int limit = randint() % 7 + 1;
            if(limit >= action_count_limit) continue;
            int emp_idx = randint() % (int)empty_pos.size(); // TODO: <- emp_idxはrandomじゃなくて順番でええか
            auto v = dfs(limit, emp_idx, -1, action_count_limit - limit);
            if(chmax(score, v[0])) {
                action_count_limit -= (int)v.size() - 1;
                for(int i=(int)v.size()-1;i>=1;i--){
                    int pos = empty_pos[emp_idx];
                    int npos = v[i];
                    ret.push_back(MoveAction(npos, pos));
                    empty_move_operation(emp_idx, npos);
                }
                cerr << iter << " " << score << " " << action_count_limit << "\n";
            }
            iter++;
        }
        return ret;
    }

    Result solve(){
        // create random moves
        cerr << "BEGIN " << calc_connect_score(_action_count_limit) << "\n";
        auto moves = move();
        // auto moves = base_move();

        // from each computer, connect to right and/or bottom if it will reach the same type
        int action_count_limit = _action_count_limit - (int)moves.size();

        auto connects = base_connect(action_count_limit);

        return Result(moves, connects);
    }
};

struct Graph;
vector<Graph> make_new_graphs(vector<ConnectAction> &vc, int except_vertex);

struct Graph {
    
    vector<ConnectAction> v;
    unordered_map<int, int> mp;
    vector<int> vertices;

    Graph(){}

    int edge_size(){ return v.size(); }
    int vertex_size(){ return vertices.size(); }

    void ade_edge(int x, int y) {
        v.push_back(ConnectAction(x, y));
        if(mp[x] == 0) {
            mp[x] = 1;
            vertices.push_back(x);
        }
        if(mp[y] == 0) {
            mp[y] = 1;
            vertices.push_back(y);
        }
    }
    void ade_edge(ConnectAction &e) {
        v.push_back(e);
        if(mp[e.pos1] == 0) {
            mp[e.pos1] = 1;
            vertices.push_back(e.pos1);
        }
        if(mp[e.pos2] == 0) {
            mp[e.pos2] = 1;
            vertices.push_back(e.pos2);
        }
    }
    void add_vertex(int x) {
        mp[x] = 1;
        vertices.push_back(x);
    }

    vector<Graph> erase_vertex(int erase_vertex) {
        // 頂点xを削除して、その結果グラフが分解された場合は1つだけ残して新しくできたグラフを返す
        // 一番大きいグラフがそのまま残る
        // 分解されなければ空のvectorが返る

        vector<Graph> ret = make_new_graphs(v, erase_vertex);
        v = ret[0].v;
        ret.erase(ret.begin());
        mp[erase_vertex] = 0;
        for(int i=0;i<(int)vertices.size();i++){
            if(vertices[i] == erase_vertex) {
                vertices.erase(vertices.begin() + i);
                break;
            }
        }
        return ret;
    }
    bool contain(int vertex) {
        return mp[vertex];
    }

    int score(int action_count_limit=100){
        if(action_count_limit >= edge_size()) return (vertex_size() * (vertex_size() - 1)) >> 1;
        else return (action_count_limit * (action_count_limit + 1)) >> 1;
    }

};

vector<Graph> make_new_graphs(vector<ConnectAction> &vc, int except_vertex = -1) {
    vector<Graph> ret;
    int cnt = 0;
    unordered_map<int, int> mp;
    for(auto &c : vc) {
        if(c.pos1 == except_vertex or c.pos2 == except_vertex) continue;
        if(mp[c.pos1]==0)mp[c.pos1]=++cnt;
        if(mp[c.pos2]==0)mp[c.pos2]=++cnt;
    }
    bool used[mp.size()] = {};
    for(auto &pp:mp) {
        if(used[pp.second-1]) continue;
        Graph g;
        g.add_vertex(pp.first);
        queue<int> que;
        que.push(pp.first);
        used[pp.second-1] = true;
        while(que.size()){
            int now = que.front();
            que.pop();
            for(int i=0;i<(int)vc.size();i++) {
                if(vc[i].pos1 == except_vertex or vc[i].pos2 == except_vertex) continue;
                if(used[mp[vc[i].pos1]-1] and used[mp[vc[i].pos2]-1]) continue;
                if(vc[i].pos1 == now) {
                    g.ade_edge(vc[i]);
                    used[mp[vc[i].pos2]-1] = true;
                    que.push(vc[i].pos2);
                } 
                else if(vc[i].pos2 == now) {
                    g.ade_edge(vc[i]);
                    used[mp[vc[i].pos1]-1] = true;
                    que.push(vc[i].pos1);
                } 
            } 
        }
        ret.push_back(g);
    }
    return ret;
}

struct SparseSolver : public BaseSolver{
    const int max_iter = 1000;
    
    vector<vector<Graph>> g;

    void reset_graph(int action_count_limit) {
        g.clear();
        g.resize(K);
        vector<ConnectAction> ca = base_connect(action_count_limit);
        for(auto &i : field_list) if(field[i] == USED) field[i] = 0;
        vector<Graph> all_g = make_new_graphs(ca);
        for(Graph & v : all_g) {
            g[field[v.vertices[0]] - 1].push_back(v);
        }

        for(auto &i : field_list){
            if(field[i] <= 0) continue;
            bool is_contain = false;
            for(auto &v : g[field[i] - 1]) {
                if(v.contain(i)) {
                    is_contain = true;
                }
            }
            if(!is_contain) {
                Graph new_g;
                new_g.add_vertex(i);
                g[field[i] - 1].push_back(new_g);
            }
        }
    }

    SparseSolver(int N, int K, const vector<string> &field_, Timer &time) : BaseSolver(N, K, field_, time) {
        reset_graph(_action_count_limit);
    }

    int calc_strict_score(int action_count_limit, vector<vector<Graph>> &v) {
        // XXX: これ、strictじゃないんだよな
        // というのも、vは辺の交差を無視して作っているので
        int score = 0;
        vector<int> sz;
        for(int i=0;i<K;i++)for(auto &g1 : v[i]) {
            sz.push_back(g1.vertex_size());
        }
        sort(all(sz), greater<int>());
        for(auto i : sz) {
            if(i - 1 <= action_count_limit) {
                score += ((i*(i-1))>>1);
                action_count_limit -= i-1;
            }
            else{
                score += ((action_count_limit * (action_count_limit + 1)) >> 1);
                break;
            }
        }
        return score;
    }

    int estimate_score_diff(vector<vector<Graph>> &vs, int color, int graph_id, 
            int target_id, int from_pos, int to_pos, int action_count_limit, bool update_graph) {
        // 交差で壊れるんだよな、やばいです
        // 最終判定だけちゃんとしたcalc_scoreでええか

        int diff = 0;

        // STEP 1 & 2
        if(!update_graph){
            // step1: graph decomposition
            vector<Graph> new_v = make_new_graphs(vs[color][graph_id].v, from_pos);
            diff -= ((vs[color][graph_id].vertex_size() * (vs[color][graph_id].vertex_size() - 1)) >> 1);
            
            // step2 : to_pos connection
            set<int> st;
            for(int dir=0;dir<4;dir++) {
                int npos = can_connect(to_pos, dir);
                if(npos == -1) continue;
                for(int i=0;i<(int)vs[color].size();i++) {
                    if(vs[color][i].contain(npos)) {
                        st.insert(i);
                    }
                }
            }
            int sz = 1;
            for(auto i : st) {
                diff -= vs[color][i].score();
                sz += vs[color][i].vertex_size();
            }
            diff += ((sz * (sz - 1)) >> 1);
        }
        
        // これは実際に操作をする時に行う処理なので、スコア計算の際には省ける
        // 引数で操作を行うかを与えるようにするか？
        if(update_graph){
            if(update_graph){
                vs[color][graph_id].erase_vertex(target_id);
            }

            vector<array<int,3>> connect; // {dir, npos, graph_id}
            for(int dir=0;dir<4;dir++) {
                int npos = can_connect(to_pos, dir);
                if(npos == -1) continue;
                for(int i=0;i<(int)vs[color].size();i++) {
                    if(vs[color][i].contain(npos)) {
                        connect.push_back({dir, npos, i});
                    }
                }
            }
            if(connect.size()){
                vector<int> connected_graph_id;
                for(int i=0;i<(int)connect.size();i++){
                    bool used = false;
                    for(int j=0;j<i;j++) {
                        if(connect[i][2] == connect[j][2]) {
                            used = 1;
                            break;
                        }
                    }
                    if(used) continue;
                    int mi_idx = i;
                    int dist = (connect[i][1] - to_pos) / dxy[connect[i][0]];
                    // 同じグラフなら近い方を優先
                    for(int j=i+1;j<(int)connect.size();j++){
                        if(connect[i][2] == connect[j][2]) {
                            int dist2 = (connect[j][1] - to_pos) / dxy[connect[j][0]];
                            if(chmin(dist, dist2)) {
                                mi_idx = j;
                            }
                        }
                    }
                    connected_graph_id.push_back(connect[mi_idx][2]);
                    vs[color][connect[mi_idx][2]].ade_edge(to_pos, connect[i][1]);
                }

                if(connected_graph_id.size() > 1){
                    // これでグラフサイズの降順になっていると思う
                    sort(all(connected_graph_id), [&vs, &color](int &s1, int &s2) {return vs[color][s1].edge_size() > vs[color][s2].edge_size();});
                    if(connected_graph_id.size() > 1) {
                        // 降順になっているかチェック
                        assert(vs[color][connected_graph_id[0]].edge_size() >= vs[color][connected_graph_id[1]].edge_size());
                    }
                    vector<int> era;
                    for(int i=1;i<(int)connected_graph_id.size();i++) {
                        auto &v = vs[color][connected_graph_id[i]].v;
                        for(auto &e : v) {
                            vs[color][connected_graph_id[0]].ade_edge(e);
                        }
                        era.push_back(connected_graph_id[i]);
                    }
                    sort(all(era),greater<int>());
                    for(auto i:era) vs[color].erase(vs[color].begin() + i);
                }
            }
        }

        // step3: from_posがemptyになることによって繋がるものを見る
        // RDLU
        vector<int> adj;
        for(int dir=0;dir<4;dir++){
            int npos = from_pos + dxy[dir];
            bool ok = false;
            while(field[npos] != -1) {
                if(field[npos] > 0) {
                    adj.push_back(npos);
                    ok = 1;
                    break;
                }
                npos += dxy[dir];
            }
            if(!ok) adj.push_back(-1);
        }
        if(!update_graph) {
            if(adj[0] != -1 and adj[2] != -1 and field[adj[0]] == field[adj[2]]) {
                int c = field[adj[0]]-1;
                int gid1 = 0, gid2 = 0;
                for(int i=0;i<(int)vs[c].size();i++) {
                    if(vs[c][i].contain(adj[0])) gid1 = i;
                    if(vs[c][i].contain(adj[2])) gid2 = i;
                }
                if(gid1 != gid2) diff += vs[c][gid1].vertex_size() * vs[c][gid2].vertex_size();
            }
            if(adj[1] != -1 and adj[3] != -1 and field[adj[1]] == field[adj[3]]) {
                int c = field[adj[1]]-1;
                int gid1 = 0, gid2 = 0;
                for(int i=0;i<(int)vs[c].size();i++) {
                    if(vs[c][i].contain(adj[1])) gid1 = i;
                    if(vs[c][i].contain(adj[3])) gid2 = i;
                }
                if(gid1 != gid2) diff += vs[c][gid1].vertex_size() * vs[c][gid2].vertex_size();
            }
        }
        // 実際に操作するときの処理
        // 交差が発生するのを無視して上下左右どっちも繋げる。
        // 交差が発生するが、最終的に強い方が選ばれると考えるとこれでいいかという気持ちに
        if(update_graph){
        } 
        return diff;
    }

    vector<MoveAction> move(){
        vector<MoveAction> ret;
        /*
        State:
        - field[(N+2)*(N+2)] <- *int (?)
        - field_hash <- long long (デカいので衝突が怖い)
        - graph <- vector<vector<array<int,2>>> (vector<Graph>, Graph:=edge(pos1,pos2))
        - 操作列 <- vector<int> ?
        - Score <- int
        */
        int action_count_limit = _action_count_limit;
        int best_score = calc_connect_score(0);
        int iter = 0;
        const int limit = 3;
        int dxy2[] = {1, limit*2+1, -1, -limit*2-1};
        // while(time.elapsed() < TIME_LIMIT) {
        while(iter < max_iter){ // ローカルで動かす時にスコアが安定するように
            
            // このやり方だと大きいサイズの頂点は選ばれにくいので良い気がする
            int color = randint() % K;
            int graph_id = randint() % (int)g[color].size();
            int target_id = randint() % (int)g[color][graph_id].vertex_size();

            int target_pos = g[color][graph_id].vertices[target_id];

            int next_pos = -1;
            int next_comp_pos = -1;
            // target_posからlimit歩以内で行けるマスを全探索(BFS)
            int visited[(limit*2+1)*(limit*2+1)] = {};
            queue<array<int,3>> que;
            que.push({target_pos, limit*(limit+1) + limit, 0});
            visited[limit*(limit+1) + limit] = -1;
            int best_estimate_diff = 0;
            int best_dist = 0;
            while(que.size()){
                auto [pos, comp_pos, dist] = que.front();
                que.pop();
                if(dist >= action_count_limit) break;
                if(dist) {
                    empty_move_operation(field_empty_id[pos], target_pos, false);
                    // 交差で壊れるんだよな、やばいです
                    // 最終判定だけちゃんとしたcalc_scoreでええか
                    // int s = calc_connect_score(action_count_limit - dist);
                    // if(chmax(best_score, s)) {
                    //     next_pos = pos;
                    //     next_comp_pos = comp_pos;
                    // }
                    int s = estimate_score_diff(g, color, graph_id, target_id, target_pos, pos, action_count_limit-dist, false);
                    if(chmax(best_estimate_diff, s)) {
                        next_pos = pos;
                        next_comp_pos = comp_pos;
                        best_dist = dist;
                    }
                    empty_move_operation(field_empty_id[target_pos], pos, false);
                }
                if(dist == limit) break;
                for(int dir=0;dir<4;dir++){
                    if(can_move(pos, dir) == false) continue;
                    int npos = pos + dxy[dir];
                    int comp_npos = comp_pos + dxy2[dir];
                    if(visited[comp_npos] != 0) continue;
                    visited[comp_npos] = dir + 1;
                    que.push({npos, comp_npos, dist+1});
                }
            }

            if(next_pos != -1) {
                empty_move_operation(field_empty_id[next_pos], target_pos, true);
                int strict_score = calc_connect_score(action_count_limit - best_dist);
                empty_move_operation(field_empty_id[target_pos], next_pos, true);
                if(chmax(best_score, strict_score) == false) {
                    iter++;
                    continue;
                }

                vector<MoveAction> tmp;
                while(visited[next_comp_pos] != -1) {
                    int dir = visited[next_comp_pos] - 1;
                    int next_pos2 = next_pos - dxy[dir];
                    int next_comp_pos2 = next_comp_pos - dxy2[dir];
                    tmp.push_back(MoveAction(next_pos2, next_pos));
                    swap(next_pos2, next_pos);
                    swap(next_comp_pos2, next_comp_pos);
                }
                for(int i=(int)tmp.size() - 1;i>=0;i--){
                    empty_move_operation(field_empty_id[tmp[i].pos2], tmp[i].pos1);
                    ret.push_back(tmp[i]);
                }
                action_count_limit -= (int)tmp.size();
                cerr << "est diff " << best_estimate_diff << "\n";
                cerr << iter << " " << best_score << " " << action_count_limit << "\n";
                reset_graph(action_count_limit);
            }

            
            iter++;
        }
        return ret;
    }

    Result solve(){
        // create random moves
        cerr << "BEGIN " << calc_connect_score(_action_count_limit) << "\n";
        auto moves = move();
        // auto moves = base_move();

        // from each computer, connect to right and/or bottom if it will reach the same type
        int action_count_limit = _action_count_limit - (int)moves.size();

        auto connects = base_connect(action_count_limit);

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

    double density = double(K*100) / double(N*N);
    const double DENSE = 0.55;
    const double SPARSE = 0.55;
    if(density >= DENSE) {
        cerr << "Solver: Dense" << "\n";
        DenseSolver s(N, K, field, time);
        auto ret = s.solve();
        s.print_answer(ret);
    }
    else if(density > SPARSE){
        cerr << "Solver: Middle" << "\n";
        BaseSolver s(N, K, field, time);
        auto ret = s.base_solve();
        s.print_answer(ret);
    }
    else {
        cerr << "Solver: Sparse" << "\n";
        SparseSolver s(N, K, field, time);
        auto ret = s.solve();
        s.print_answer(ret);
    }

    // while(time.elapsed() < 2800)continue;

    cerr << "Time = " << time.elapsed() << "\n";
}