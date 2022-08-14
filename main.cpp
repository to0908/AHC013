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

struct SparseSolver {
    static constexpr int USED = 9;
    const int TIME_LIMIT = 2800;
    int dxy[4];

    int N, K;
    int _action_count_limit;
    array<int, 2> rev_field[2500]; // (N+2)**2 cells -> N*N cells (x, y)
    int hasi[2500];
    Timer time;

    int field[2500]; // field[pos] := -1/0/server
    vector<int> server_pos; // server_pos[server_id] := pos
    int field_server_id[2500]; // field_server_id[pos] := -1/server_id

    SparseSolver(int N, int K, const vector<string> &field_, Timer &time) : 
        N(N), K(K), _action_count_limit(K * 100), time(time){
        int cnt = 0;
        server_pos.resize(K*100, -1);
        for(int i=0;i<2500;i++) {
            field[i] = -1;
            field_server_id[i] = -1;
            hasi[i] = 0;
            rev_field[i] = {-1, -1};
        }
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
                    field_server_id[pos] = -1;
                }
                rev_field[pos] = {i-1, j-1};
                if(i == 1) hasi[pos] += 1;
                if(i == N) hasi[pos] += 2;
                if(j == 1) hasi[pos] += 4;
                if(j == N) hasi[pos] += 8;
            }
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

    int can_connect(int pos, int dir) {
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

        for(int i=0;i<2500;i++) {
            if(field[i] >= USED) field[i] = 0;
        }

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
                    int pos_x = get_server_pos(now, (dir==0)?3:2);
                    int pos_y = get_server_pos(now, (dir==0)?1:0);
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
                            int pos_x = get_server_pos(now, (dir==0)?3:2);
                            int pos_y = get_server_pos(now, (dir==0)?1:0);
                            int x = field[pos_x], y = field[pos_y];
                            assert(x != -1);
                            assert(y != -1);
                            if(x > 0 and x == y and !uf.same(field_server_id[pos_x], field_server_id[pos_y])) {
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


    vector<MoveAction> move(){
        vector<MoveAction> ret;


        return ret;
    }

    Result solve(){
        auto moves = move();

        // from each computer, connect to right and/or bottom if it will reach the same type
        int action_count_limit = _action_count_limit - (int)moves.size();

        // for(int i=0;i<(N+2)*(N+2);i++) {
        //     if()
        // }
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
    if(density >= DENSE) {
        cerr << "Solver: Dense" << "\n";
        cout << 0 << "\n";
        cout << 0 << "\n";
    }
    else {
        cerr << "Solver: Sparse" << "\n";
        SparseSolver s(N, K, field, time);
        auto ret = s.solve();
        s.print_answer(ret);
    }

    cerr << "Time = " << time.elapsed() << "\n";
}