#pragma GCC optimize ("Ofast,inline")
// surgutti, winter-challenge-2026, 10-03-2026
#include "bits/stdc++.h"
using namespace std;

// #define int long long
// #define ll long long
// #define ld long double

#define LOCAL

#define endl '\n'
#define st first
#define nd second
#define mp make_pair
#define pb push_back
#define eb emplace_back
#define sz(x) (int)(x).size()
#define all(x) begin(x),end(x)
#define FOR(i,l,r) for(int i=(l);i<=(r);i++)
#define ROF(i,r,l) for(int i=(r);i>=(l);i--)

#define _$ auto operator<<(auto&o,auto x)->decltype
_$(x.st,o){return o<<"("<<x.st<<", "<<x.nd<<")";}
_$(end(x),o){o<<"{";for(int i=0;auto e:x)o<<","+!i++<<e;return o<<"}";}

#ifdef LOCAL
#define debug(x...) cerr<<"["#x"]: ",[](auto...$){\
  ((cerr<<$<<"; "),...)<<'\n';}(x)
#else
#define debug(...)
#endif

#define rep(i,a,b) for(int i=(a);i<(b); i++)

using i8  = char;
using u8  = unsigned char;
using i16 = short;
using u16 = unsigned short;
using i32 = int;
using u32 = unsigned int;
using i64 = long long;
using u64 = unsigned long long;

using pii=pair<i32,i32>;
using vi=vector<i32>;

const i32 inf = 1e9+7;

const i32 WIDTH = 45;
const i32 HEIGHT = 30;
const i32 SNAKES_PER_PLAYER = 4;
const i32 SNAKES = 8;

const string ds[] = {"LEFT", "UP", "RIGHT", "DOWN"};
const i32 dx[] = {-1, 0, +1, 0};
const i32 dy[] = {0, -1, 0, +1};

i32 myId;
i32 width;
i32 height;
string g[HEIGHT];
i32 snakesPerPlayer;
i32 myIds[SNAKES_PER_PLAYER];
i32 opIds[SNAKES_PER_PLAYER];
i32 powerCnt;
vector<pii> powers;
mt19937 rng(2137);

const i32 MAX_GEN_LEN = 5;
const i32 MAX_SHAPES = 600;

struct Snake {
  i32 sId;
  vector<pii> body;
  i32 shapeId;
  bool isMy;
  bool isAlive;

  i32 action=-1;
} snakes[SNAKES];

i32 random_action(Snake& snake) {
  auto [x,y]=snake.body[0];

  vi dirs;
  rep(d,0,4){
    i32 xx = x+dx[d];
    i32 yy = y+dy[d];

    if (0 <= xx && xx < width && 0 <= yy && yy < height && g[xx][yy] == '#') {
      continue;
    }
    
    bool ok=true;
    rep(j,1,sz(snake.body)){
      if (mp(xx,yy)==snake.body[j]){
        ok=false;
      }
    }
    if (!ok){
      continue;
    }
  }

  if (sz(dirs)) {
    uniform_int_distribution<int> dist(0, sz(dirs)-1);
    return dirs[dist(rng)];
  }

  return -1;
}

struct GenerateShapes {

  map<vector<pii>, int> shape_id;
  vector<vector<pii>> shapes[MAX_GEN_LEN+1];
  vector<pii> path;
  vector<array<i32, 4>> go[MAX_GEN_LEN+1], go_prev[MAX_GEN_LEN+1];

  void gen(i32 x, i32 y) {
    path.eb(x,y);

    if (sz(path)<=MAX_GEN_LEN){
      shape_id[path] = sz(shapes[sz(path)]);
      shapes[sz(path)].pb(path);

      if (sz(path) == MAX_GEN_LEN) {
        path.pop_back();
        return;
      }
    }

    rep(d,0,4){
      i32 xx = x + dx[d];
      i32 yy = y + dy[d];
      
      bool ok=true;
      rep(j,0,sz(path)){
        if (path[j] == mp(xx,yy)){
          ok=false;
          break;
        }
      }

      if (ok) {
        gen(xx,yy);
      }
    }

    path.pop_back();
  }

  GenerateShapes() {
    gen(0,0);

    cerr << "SHAPES: " << sz(shape_id) << '\n';

    FOR(len,1,MAX_GEN_LEN){
      go[len].resize(sz(shapes[len]), {-1, -1, -1, -1});
      go_prev[len] = go[len];
      
      cerr << "len: " << len << '\n';
      rep(i,0,sz(shapes[len])) {
        assert(sz(shapes[len]) <= MAX_SHAPES);
        vector<pii> ss = shapes[len][i];
        reverse(all(ss));
        ss.eb(0,0);
        reverse(all(ss));
        ss.pop_back();

        rep(d,0,4){
          vector<pii> shape = ss;

          bool ok = true;
          rep(j,1,sz(shape)){
            shape[j].st -= dx[d];
            shape[j].nd -= dy[d];
            if(shape[j]==shape[0]){
              ok=false;
              break;
            }
          }

          if (ok) {
            auto it = shape_id.find(shape);
            assert(it != shape_id.end());
            assert(sz(it->st) == sz(shapes[len][i]));
            go[len][i][d]=it->nd;
            go_prev[len][it->nd][d]=i;
          } else {
            go[len][i][d] = -1;
          }
        }
      }
    }

    cerr << "done\n";
  }

  i32 get_shapeId(vector<pii> body) {
    debug(body);
    assert(sz(body) >= 3);
    pii head = body[0];

    for (auto &[x, y] : body) {
      x -= head.first;
      y -= head.second;
    }

    auto it = shape_id.find(body);
    assert(it != shape_id.end());
    return it->second;
  }

  i32 next_shape(i32 len, i32 shape, i32 dir) {
    return go[len][shape][dir];
  }

  i32 prev_shape(i32 len, i32 shape, i32 dir) {
    return go_prev[len][shape][dir];
  }
} go;

struct ShortDistances {

  i32 dp[WIDTH][HEIGHT][MAX_SHAPES];
  i8 prev[WIDTH][HEIGHT][MAX_SHAPES];
  i32 vis[WIDTH][HEIGHT][MAX_SHAPES];
  i32 dist[WIDTH][HEIGHT], dist_vis[WIDTH][HEIGHT];
  i8 dist_prev[WIDTH][HEIGHT];
  i32 dist_shape[WIDTH][HEIGHT];

  i32 tim=0, len;
  array<i32,3> q[WIDTH*HEIGHT*MAX_SHAPES];

  void init(i32 _len, i32 sx, i32 sy, i32 ss) {
    len = _len;
    tim++;
    i32 h=0;
    q[h++]={sx,sy,ss};
    dp[sx][sy][ss]=0;
    vis[sx][sy][ss]=tim;
    dist_vis[sx][sy]=tim;

    rep(t,0,h){
      auto [x,y,s]=q[t];

      rep(d,0,4){
        i32 xx = x+dx[d], yy = y+dy[d];

        if (xx < 0 || xx >= width || yy < 0 || yy >= height || g[xx][yy] == '#') {
          continue;
        }

        i32 nxt = go.next_shape(len,ss,d);
        if (nxt == -1) {
          continue;
        }

        vector<pii> const& body = go.shapes[len][nxt];
        // debug(xx, yy, nxt, body);
        i32 drop = HEIGHT;
        for (auto [_x, _y] : body) {
          _x += xx;
          _y += yy;
          _y++;
          i32 cur = 0;
          while (g[_x][_y]!='#') {
            _y++;
            cur++;
          }
          drop=min(drop,cur);
        }
        // debug(drop);
        assert(drop != HEIGHT);
        yy += drop;

        if (nxt != -1 && vis[xx][yy][nxt] != tim) {
          vis[xx][yy][nxt] = tim;
          dp[xx][yy][nxt] = dp[x][y][s]+1;
          prev[xx][yy][nxt] = d;

          if (dist_vis[xx][yy]!=tim){
            dist[xx][yy]=dp[xx][yy][nxt];
            dist_shape[xx][yy]=nxt;
            dist_prev[xx][yy]=d;
            dist_vis[xx][yy]=tim;
          }
          q[h++]={xx,yy,nxt};
        }
      }
    }

    debug("head:",h);
    rep(y,0,height){
      rep(x,0,width){
        if (g[x][y]=='#')
          cerr << '#';
        else if (x == sx && y == sy) {
          cerr << 's';
        } else if(query_dist(x,y)!=-1){
          cerr<<'x';
        } else {
          cerr<<'.';
        }
      }
      cerr<<'\n';
    }
    cerr<<'\n';
  }

  i32 query_dist(i32 x, i32 y) {
    if (dist_vis[x][y] != tim) {
      return -1;
    }

    return dist[x][y];
  }

  vi query_moves(i32 x, i32 y) {
    assert(dist_vis[x][y] == tim);
    i32 s = dist_shape[x][y];
    vi moves;

    while (dp[x][y][s] != 0){
      moves.pb(prev[x][y][s]);
      i32 d = prev[x][y][s];
      x -= dx[d];
      y -= dy[d];
      s = go.prev_shape(len, s, d);
    }
    assert(sz(moves));
    reverse(all(moves));
    return moves;
  }
} dist[SNAKES];

vector<pii> parse_pairs(const string& s) {
  vector<pii>v;
  stringstream ss(s);
  string t;
  while (getline(ss, t, ':')) {
    int a, b;
    sscanf(t.c_str(), "%d,%d", &a, &b);
    v.emplace_back(a, b);
  }
  return v;
}

signed main() {

  cin >> myId;
  cin >> width >> height;

  rep(y,0,height){
    rep(x,0,width){
      cin >> g[x][y];
    }
  }

  cin >> snakesPerPlayer;
  rep(i,0,snakesPerPlayer) {
    cin >> myIds[i];
    snakes[myIds[i]].isMy=true;
    snakes[myIds[i]].isAlive=true;
  }
  rep(i,0,snakesPerPlayer) {
    cin >> opIds[i];
    snakes[opIds[i]].isAlive=true;
  }

  cerr << "hello there\n";

  for (;;) {
    cin >> powerCnt;

    powers.clear();
    rep(i,0,powerCnt) {
      i32 x, y;
      cin >> x >> y;
      powers.eb(x,y);
    }

    i32 snakesCount;
    cin >> snakesCount;
    
    rep(i,0,SNAKES){
      snakes[i].isAlive=false;
    }
    rep(i,0,snakesCount) {
      i32 sId;
      cin >> sId;

      string bodyS;
      cin >> bodyS;

      vector<pii>body = parse_pairs(bodyS);
      
      debug("input body", body);
      
      snakes[sId].body = body;
      snakes[sId].isAlive=true;
      snakes[sId].shapeId=go.get_shapeId(body);
      snakes[sId].action=-1;
    }

    vector<pii> moves, marks;

    rep(i,0,SNAKES){
      if (snakes[i].isAlive == false || snakes[i].isMy == false) {
        continue;
      }
      if (sz(snakes[i].body) > MAX_GEN_LEN) {
        continue;
      }

      Snake const& snake = snakes[i];
      dist[i].init(sz(snake.body), snake.body[0].st, snake.body[0].nd, snake.shapeId);
    
      vector<array<i32,3>> possible;
      for (auto [x, y] : powers) {
        i32 d = dist[i].query_dist(x,y);
        if (d != -1) {
          possible.pb({d, x, y});
        }
      }  

      if (sz(possible)) {
        sort(all(possible));
        
        auto [_, tx, ty] = possible[0];
        
        i32 act = dist[i].query_moves(tx, ty)[0];
        snakes[i].action = act;

        cerr << "snake(" << i << ") => " << tx << ' ' << ty << ' ' << _ << '\n';

        moves.eb(i, act);
        marks.eb(tx, ty);
      }
    }

    debug(moves);
    debug(marks);

    if (sz(moves)) {
      rep(i,0,sz(moves)){
        if (i != 0) {
          cout << ';';
        }
        auto [id, act] = moves[i];
        cout << id << ' ' << ds[act];
      }
    } else {
      cout << "WAIT";
    }

    for (auto [x, y] : marks) {
      cout << ";MARK " << x << ' ' << y;
    }

    cout << endl;
  }

  return 0;
}
