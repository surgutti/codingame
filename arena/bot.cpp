#pragma GCC optimize ("Ofast,inline")

#include "bits/stdc++.h"
using namespace std;

#define INLINE inline __attribute__((always_inline))

#define int long long
#define ll long long
#define ld long double

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
#define debug(...) do {} while (0)

#define rep(i,a,b) for(int i=(a);i<(b); i++)
using pii=pair<int,int>;
using vi=vector<int>;

const int inf = (int)4e18;
const int NEG = (int)-4e18;

int myId, w, h;
vector<string> g;
vector<int> myIds, oppIds;
unordered_set<int> mySet, oppSet;
int turnNo = 0;

int dx[4] = {0, 0, -1, 1};
int dy[4] = {-1, 1, 0, 0};
string dn[4] = {"UP", "DOWN", "LEFT", "RIGHT"};
int oppDir[4] = {1, 0, 3, 2};

mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());

struct Snake {
  int id;
  int owner;
  bool alive;
  vector<pii> body;
};

struct State {
  vector<Snake> snakes;
  vector<char> apple;
  int turn;
};

struct SimOne {
  bool alive;
  bool beheaded;
  bool eat;
  pii rawHead;
  vector<pii> body;
};

struct SearchInfo {
  array<int,4> firstDist;
  vector<int> appleDist;
  SearchInfo() {}
  SearchInfo(int n) {
    firstDist.fill(inf);
    appleDist.assign(n, inf);
  }
};

struct SupportPathInfo {
  array<int,4> firstDist;
  int bestAny;
  SupportPathInfo() {
    firstDist.fill(inf);
    bestAny = inf;
  }
};

struct MoveInfo {
  int dir;
  int score;
  pii rawHead;
  pii finalHead;
  bool eat;
  bool nearEnemy;
  bool alive;
  bool beheaded;
  bool ownTail;
  bool strictSafe;
  bool routeRisk;
  int outCnt;
  int traffic;
};

struct Combo {
  vector<int> act;
  int prior;
  int key;
};

struct Stat {
  ld sum = 0;
  int cnt = 0;
};

INLINE bool inside(int x, int y) {
  return 0 <= x && x < w && 0 <= y && y < h;
}

INLINE bool inside(pii p) {
  return inside(p.st, p.nd);
}

INLINE int idxy(int x, int y) {
  return y * w + x;
}

INLINE int idxy(pii p) {
  return p.nd * w + p.st;
}

INLINE uint64_t key_xy(int x, int y) {
  return ((uint64_t)(x + 256) << 16) ^ (uint64_t)(y + 256);
}

INLINE uint64_t key_xy(pii p) {
  return key_xy(p.st, p.nd);
}

INLINE bool wall(pii p) {
  return inside(p) && g[p.nd][p.st] == '#';
}

INLINE int manh(pii a, pii b) {
  return abs(a.st - b.st) + abs(a.nd - b.nd);
}

INLINE bool same(pii a, pii b) {
  return a.st == b.st && a.nd == b.nd;
}

INLINE int apple_count(const State &state) {
  int cnt = 0;
  for (char x : state.apple) cnt += x;
  return cnt;
}

vector<pii> parse_body(const string &s) {
  vector<pii> body;
  string cur;
  vector<string> parts;
  for (char c : s) {
    if (c == ':') {
      parts.pb(cur);
      cur.clear();
    } else cur += c;
  }
  parts.pb(cur);

  for (auto &t : parts) {
    int p = t.find(',');
    int x = stoll(t.substr(0, p));
    int y = stoll(t.substr(p + 1));
    body.pb({x, y});
  }
  return body;
}

INLINE int face_dir(const vector<pii> &body) {
  if (sz(body) < 2) return 0;
  int xx = body[0].st - body[1].st;
  int yy = body[0].nd - body[1].nd;
  rep(k,0,4) if (dx[k] == xx && dy[k] == yy) return k;
  return 0;
}

INLINE bool is_vertical(const vector<pii> &body) {
  if (body.empty()) return false;
  int x = body[0].st;
  for (auto p : body) if (p.st != x) return false;
  return true;
}

INLINE bool has_cell(const vector<pii> &body, pii c) {
  for (auto p : body) if (same(p, c)) return true;
  return false;
}

INLINE bool has_cell_skip_head(const vector<pii> &body, pii c) {
  rep(i,1,sz(body)) if (same(body[i], c)) return true;
  return false;
}

INLINE bool has_apple(const vector<char> &apple, pii p) {
  return inside(p) && apple[idxy(p)];
}

int nearest_apple_manh(const State &state, pii p) {
  int ans = inf;
  rep(y,0,h) rep(x,0,w) if (state.apple[idxy(x, y)]) {
    ans = min(ans, manh(p, {x, y}));
  }
  return ans == inf ? w + h + 5 : ans;
}

INLINE int count_outside_parts(const vector<pii> &body) {
  int ans = 0;
  for (auto p : body) if (!inside(p)) ans++;
  return ans;
}

INLINE int top_value(pii p) {
  if (!inside(p)) return 0;
  return (h - 1 - p.nd);
}

int snake_height_metric(const vector<pii> &body) {
  if (body.empty()) return 0;
  int hh = top_value(body[0]);
  int sum = 0;
  for (auto p : body) sum += top_value(p);
  int avg = sum / max<int>(1, sz(body));
  return 10 * hh + 2 * avg;
}

int body_score_diff(const State &state) {
  int myScore = 0, oppScore = 0;
  for (auto &snake : state.snakes) if (snake.alive) {
    if (snake.owner == myId) myScore += sz(snake.body);
    else oppScore += sz(snake.body);
  }
  return myScore - oppScore;
}

INLINE bool guaranteed_win_state(const State &state) {
  int diff = body_score_diff(state);
  int rem = apple_count(state);
  return diff > rem;
}

bool side_dead(const State &state, int owner) {
  for (auto &snake : state.snakes) if (snake.owner == owner && snake.alive) return false;
  return true;
}

bool no_apples(const State &state) {
  for (char x : state.apple) if (x) return false;
  return true;
}

INLINE bool terminal(const State &state) {
  return no_apples(state) || side_dead(state, 0) || side_dead(state, 1) || state.turn >= 200;
}

bool solid_under_cell(const State &state, pii c, const vector<pii> &ignore) {
  pii below = {c.st, c.nd + 1};
  if (has_cell(ignore, below)) return false;
  if (wall(below)) return true;
  for (auto &snake : state.snakes) if (snake.alive && has_cell(snake.body, below)) return true;
  if (has_apple(state.apple, below)) return true;
  return false;
}

bool solid_under_single(const vector<char> &apple, const vector<char> &block, pii c, const vector<pii> &ignore, bool ate, pii eatenCell) {
  pii below = {c.st, c.nd + 1};
  if (has_cell(ignore, below)) return false;
  if (wall(below)) return true;
  if (inside(below) && block[idxy(below)]) return true;
  if (has_apple(apple, below) && !(ate && same(below, eatenCell))) return true;
  return false;
}

SimOne one_snake_step(const Snake &snake, int dir, const vector<char> &block, const vector<char> &apple) {
  SimOne ret;
  ret.alive = false;
  ret.beheaded = false;
  ret.eat = false;
  ret.rawHead = snake.body[0];
  ret.body.clear();

  if (!snake.alive) return ret;

  pii head = snake.body[0];
  pii nh = {head.st + dx[dir], head.nd + dy[dir]};
  bool eat = has_apple(apple, nh);

  vector<pii> body;
  body.reserve(sz(snake.body) + 1);
  body.pb(nh);
  rep(i,0,sz(snake.body) - 1) body.pb(snake.body[i]);
  if (eat) body.pb(snake.body.back());

  bool hit = false;
  if (wall(nh)) hit = true;
  if (inside(nh) && block[idxy(nh)]) hit = true;
  if (has_cell_skip_head(body, nh)) hit = true;

  bool beheaded = false;
  if (hit) {
    if (sz(body) <= 3) {
      ret.rawHead = nh;
      ret.eat = eat;
      return ret;
    }
    beheaded = true;
    body.erase(body.begin());
  }

  while (1) {
    bool supported = false;
    for (auto p : body) {
      if (solid_under_single(apple, block, p, body, eat, nh)) {
        supported = true;
        break;
      }
    }
    if (supported) break;
    for (auto &p : body) p.nd++;
    bool out = true;
    for (auto p : body) if (p.nd < h + 1) out = false;
    if (out) {
      ret.rawHead = nh;
      ret.eat = eat;
      ret.beheaded = beheaded;
      return ret;
    }
  }

  ret.alive = true;
  ret.beheaded = beheaded;
  ret.eat = eat;
  ret.rawHead = nh;
  ret.body = move(body);
  return ret;
}

uint64_t hash_body(const vector<pii> &body) {
  uint64_t hsh = 1469598103934665603ULL;
  for (auto [x, y] : body) {
    uint64_t v = ((uint64_t)(x + 256) << 10) ^ (uint64_t)(y + 256);
    hsh ^= v + 0x9e3779b97f4a7c15ULL + (hsh << 6) + (hsh >> 2);
  }
  hsh ^= (uint64_t)sz(body) * 11995408973635179863ULL;
  return hsh;
}

vector<int> legal_dirs(const Snake &snake) {
  vector<int> dirs;
  if (!snake.alive) return dirs;
  int face = face_dir(snake.body);
  rep(dir,0,4) if (dir != oppDir[face]) dirs.pb(dir);
  return dirs;
}

vector<char> build_block(const State &state, int idx) {
  vector<char> block(w * h, 0);
  rep(i,0,sz(state.snakes)) if (i != idx && state.snakes[i].alive) {
    for (auto p : state.snakes[i].body) if (inside(p)) block[idxy(p)] = 1;
  }
  return block;
}

SearchInfo search_snake(const State &state, int idx, int maxDepth = 9) {
  SearchInfo info(w * h);
  auto snake = state.snakes[idx];
  if (!snake.alive) return info;

  vector<char> block = build_block(state, idx);

  struct Node {
    vector<pii> body;
    int dep;
    int first;
  };

  queue<Node> q;
  unordered_set<uint64_t> seen;
  q.push({snake.body, 0, -1});
  seen.insert(hash_body(snake.body));

  while (!q.empty()) {
    Node cur = move(q.front());
    q.pop();

    pii head = cur.body[0];
    if (inside(head) && state.apple[idxy(head)]) {
      info.appleDist[idxy(head)] = min(info.appleDist[idxy(head)], cur.dep);
      if (cur.first != -1) info.firstDist[cur.first] = min(info.firstDist[cur.first], cur.dep);
    }

    if (cur.dep == maxDepth) continue;

    Snake now;
    now.id = snake.id;
    now.owner = snake.owner;
    now.alive = true;
    now.body = cur.body;

    int face = face_dir(cur.body);
    rep(dir,0,4) {
      if (dir == oppDir[face]) continue;

      auto nxt = one_snake_step(now, dir, block, state.apple);
      if (!nxt.alive) continue;
      if (!inside(nxt.rawHead)) continue;
      if (nxt.beheaded) continue;
      if (count_outside_parts(nxt.body) > 0) continue;

      int firstDir = (cur.first == -1 ? dir : cur.first);

      if (nxt.eat && inside(nxt.rawHead)) {
        info.firstDist[firstDir] = min(info.firstDist[firstDir], cur.dep + 1);
        info.appleDist[idxy(nxt.rawHead)] = min(info.appleDist[idxy(nxt.rawHead)], cur.dep + 1);
      }

      uint64_t hsh = hash_body(nxt.body);
      if (!seen.insert(hsh).nd) continue;
      q.push({nxt.body, cur.dep + 1, firstDir});
    }
  }

  return info;
}

INLINE int idx4(int hx, int hy, int sx, int sy) {
  return ((hx * h + hy) * w + sx) * h + sy;
}

void build_static_grids(const State &state, int idx, vector<char> &block, vector<char> &solid, vector<char> &apple) {
  block.assign(w * h, 0);
  solid.assign(w * h, 0);
  apple.assign(w * h, 0);

  rep(y,0,h) rep(x,0,w) if (g[y][x] == '#') {
    block[idxy(x, y)] = 1;
    solid[idxy(x, y)] = 1;
  }

  rep(y,0,h) rep(x,0,w) if (state.apple[idxy(x, y)]) {
    apple[idxy(x, y)] = 1;
    solid[idxy(x, y)] = 1;
  }

  rep(i,0,sz(state.snakes)) if (i != idx && state.snakes[i].alive) {
    for (auto p : state.snakes[i].body) if (inside(p)) {
      block[idxy(p)] = 1;
      solid[idxy(p)] = 1;
    }
  }
}

pii get_support_start(const State &state, int idx, const vector<char> &solid) {
  auto &body = state.snakes[idx].body;

  pii ret = body.back();
  for (auto p : body) if (p.nd > ret.nd) ret = p;

  rep(i,0,sz(body)) {
    pii below = {body[i].st, body[i].nd + 1};
    if (inside(below) && solid[idxy(below)]) ret = body[i];
  }

  return ret;
}

bool support_transition(
  const vector<char> &block,
  const vector<char> &solid,
  const vector<char> &apple,
  int len,
  int hx, int hy, int sx, int sy, int dir,
  int &nhx, int &nhy, int &nsx, int &nsy,
  bool &eat
) {
  pii nh = {hx + dx[dir], hy + dy[dir]};
  if (!inside(nh)) return false;
  if (block[idxy(nh)]) return false;

  eat = apple[idxy(nh)];

  pii support = {sx, sy};

  auto solid_below = [&](pii p) -> bool {
    pii below = {p.st, p.nd + 1};
    if (!inside(below)) return false;
    if (eat && same(below, nh)) return false;
    return solid[idxy(below)];
  };

  if (solid_below(nh)) {
    support = nh;
  } else {
    while (manh(nh, support) > len - 1) {
      if (support.st < nh.st) support.st++;
      else if (support.st > nh.st) support.st--;
      else if (support.nd < nh.nd) support.nd++;
      else if (support.nd > nh.nd) support.nd--;
    }
    if (!inside(support)) return false;
  }

  while (!solid_below(support)) {
    nh.nd++;
    support.nd++;
    if (!inside(nh)) return false;
    if (!inside(support)) return false;
  }

  if (solid_below(nh)) support = nh;

  nhx = nh.st;
  nhy = nh.nd;
  nsx = support.st;
  nsy = support.nd;
  return true;
}

SupportPathInfo support_path_search(const State &state, int idx, int maxExpand = 80000, int extraDepth = 10) {
  SupportPathInfo info;
  auto &snake = state.snakes[idx];
  if (!snake.alive || no_apples(state)) return info;
  if (snake.body.empty()) return info;
  if (!inside(snake.body[0])) return info;

  vector<char> block, solid, apple;
  build_static_grids(state, idx, block, solid, apple);

  pii head = snake.body[0];
  pii support = get_support_start(state, idx, solid);
  int len = sz(snake.body);

  static vector<int> dist;
  static vector<int> first;
  int S = w * h * w * h;
  dist.assign(S, -1);
  first.assign(S, -1);

  struct Node {
    int hx, hy, sx, sy;
  };

  queue<Node> q;
  int startId = idx4(head.st, head.nd, support.st, support.nd);
  dist[startId] = 0;
  q.push({head.st, head.nd, support.st, support.nd});

  int best = inf;
  int expanded = 0;

  while (!q.empty()) {
    auto cur = q.front();
    q.pop();

    int curId = idx4(cur.hx, cur.hy, cur.sx, cur.sy);
    int dep = dist[curId];

    if (dep > best + extraDepth) break;
    if (++expanded > maxExpand) break;

    auto relax_dir = [&](int dir) {
      int nhx, nhy, nsx, nsy;
      bool eat = false;

      if (!support_transition(block, solid, apple, len, cur.hx, cur.hy, cur.sx, cur.sy, dir, nhx, nhy, nsx, nsy, eat)) return;

      int nd = dep + 1;
      int firstDir = (dep == 0 ? dir : first[curId]);

      if (eat) {
        info.bestAny = min(info.bestAny, nd);
        info.firstDist[firstDir] = min(info.firstDist[firstDir], nd);
        best = min(best, nd);
      }

      int nxtId = idx4(nhx, nhy, nsx, nsy);
      if (dist[nxtId] != -1) return;

      dist[nxtId] = nd;
      first[nxtId] = firstDir;
      q.push({nhx, nhy, nsx, nsy});
    };

    if (dep == 0) {
      auto rootDirs = legal_dirs(snake);
      for (int dir : rootDirs) relax_dir(dir);
    } else {
      rep(dir,0,4) relax_dir(dir);
    }
  }

  return info;
}

vector<pii> get_enemy_heads(const State &state, int owner) {
  vector<pii> heads;
  for (auto &snake : state.snakes) if (snake.alive && snake.owner != owner) heads.pb(snake.body[0]);
  return heads;
}

INLINE bool near_enemy_head(pii c, const vector<pii> &enemyHeads) {
  for (auto p : enemyHeads) if (manh(c, p) == 1) return true;
  return false;
}

int strict_safe_moves_count_local(const State &state, int idx, const vector<pii> &body, const vector<char> &apple, bool onlySides) {
  Snake snake = state.snakes[idx];
  snake.body = body;
  snake.alive = true;
  auto block = build_block(state, idx);
  int ans = 0;

  for (int dir : legal_dirs(snake)) {
    if (onlySides && dir < 2) continue;
    pii rawHead = {snake.body[0].st + dx[dir], snake.body[0].nd + dy[dir]};
    if (!inside(rawHead)) continue;
    if (wall(rawHead)) continue;
    auto sim = one_snake_step(snake, dir, block, apple);
    if (!sim.alive) continue;
    if (sim.beheaded) continue;
    if (count_outside_parts(sim.body) > 0) continue;
    ans++;
  }
  return ans;
}

vector<int> strict_safe_dirs_from_body(const State &state, int idx, const vector<pii> &body, const vector<char> &apple, bool onlySides = false) {
  vector<int> dirs;
  Snake snake = state.snakes[idx];
  snake.alive = true;
  snake.body = body;

  auto block = build_block(state, idx);

  for (int dir : legal_dirs(snake)) {
    if (onlySides && dir < 2) continue;

    pii rawHead = {body[0].st + dx[dir], body[0].nd + dy[dir]};
    if (!inside(rawHead)) continue;
    if (wall(rawHead)) continue;

    auto sim = one_snake_step(snake, dir, block, apple);
    if (!sim.alive) continue;
    if (sim.beheaded) continue;
    if (count_outside_parts(sim.body) > 0) continue;

    dirs.pb(dir);
  }

  return dirs;
}

bool can_survive_k_local(const State &state, int idx, const vector<pii> &body, const vector<char> &apple, int depth) {
  if (depth == 0) return true;

  Snake snake = state.snakes[idx];
  snake.alive = true;
  snake.body = body;

  auto block = build_block(state, idx);

  for (int dir : legal_dirs(snake)) {
    pii rawHead = {body[0].st + dx[dir], body[0].nd + dy[dir]};
    if (!inside(rawHead)) continue;
    if (wall(rawHead)) continue;

    auto sim = one_snake_step(snake, dir, block, apple);
    if (!sim.alive) continue;
    if (sim.beheaded) continue;
    if (count_outside_parts(sim.body) > 0) continue;

    vector<char> nextApple = apple;
    if (sim.eat && inside(sim.rawHead)) nextApple[idxy(sim.rawHead)] = 0;

    if (can_survive_k_local(state, idx, sim.body, nextApple, depth - 1)) return true;
  }

  return false;
}

INLINE int safe_branching_local(const State &state, int idx, const vector<pii> &body, const vector<char> &apple) {
  return sz(strict_safe_dirs_from_body(state, idx, body, apple, false));
}

INLINE int loopiness_bonus(const vector<pii> &body) {
  if (sz(body) < 3) return 0;
  pii head = body[0];
  pii tail = body.back();
  int d = manh(head, tail);

  if (d == 1) return 140;
  if (d == 2) return 70;
  if (d == 3) return 20;
  return 0;
}

int area_from_body_state(const State &state, int idx, const vector<pii> &body, int cap) {
  if (body.empty()) return 0;
  pii head = body[0];
  if (!inside(head) || wall(head)) return 0;

  vector<char> bad(w * h, 0);
  rep(i,0,sz(state.snakes)) if (i != idx && state.snakes[i].alive) {
    for (auto p : state.snakes[i].body) if (inside(p)) bad[idxy(p)] = 1;
  }
  rep(i,1,sz(body)) if (inside(body[i])) bad[idxy(body[i])] = 1;

  queue<pii> q;
  q.push(head);
  bad[idxy(head)] = 1;
  int cnt = 1;

  while (!q.empty() && cnt < cap) {
    pii v = q.front();
    q.pop();
    rep(k,0,4) {
      pii u = {v.st + dx[k], v.nd + dy[k]};
      if (!inside(u)) continue;
      if (wall(u)) continue;
      int id = idxy(u);
      if (bad[id]) continue;
      bad[id] = 1;
      q.push(u);
      cnt++;
      if (cnt >= cap) break;
    }
  }
  return cnt;
}

bool cell_in_other_routes(const State &state, int idx, pii c) {
  rep(j,0,sz(state.snakes)) if (j != idx && state.snakes[j].alive) {
    auto &other = state.snakes[j];
    for (int dir : legal_dirs(other)) {
      pii q = {other.body[0].st + dx[dir], other.body[0].nd + dy[dir]};
      if (!inside(q)) continue;
      if (wall(q)) continue;
      if (same(q, c)) return true;
    }
  }
  return false;
}

int traffic_pressure(const State &state, int idx, pii c) {
  int pen = 0;
  rep(j,0,sz(state.snakes)) if (j != idx && state.snakes[j].alive) {
    auto &other = state.snakes[j];
    pii head = other.body[0];
    int dh = manh(c, head);
    if (dh == 1) pen += 120;
    else if (dh == 2) pen += 40;

    int face = face_dir(other.body);
    pii ahead = {head.st + dx[face], head.nd + dy[face]};
    if (inside(ahead) && same(ahead, c)) pen += 140;

    for (auto p : other.body) {
      int d = manh(c, p);
      if (d == 1) pen += 18;
      else if (d == 2) pen += 6;
    }
  }
  return pen;
}

int head_spread_penalty(const vector<pii> &heads) {
  int pen = 0;
  rep(i,0,sz(heads)) rep(j,i + 1,sz(heads)) {
    int d = manh(heads[i], heads[j]);
    if (d < 10) pen += 8 * (10 - d) * (10 - d);
  }
  return pen;
}

int eval_state(const State &state) {
  int val = 2600 * body_score_diff(state);

  vector<pii> myHeads, oppHeads;

  rep(i,0,sz(state.snakes)) if (state.snakes[i].alive) {
    auto &snake = state.snakes[i];
    int sign = (snake.owner == myId ? 1 : -1);
    int len = sz(snake.body);

    int cur = 0;
    cur += snake_height_metric(snake.body);

    int safeCnt = strict_safe_moves_count_local(state, i, snake.body, state.apple, false);
    int sideCnt = is_vertical(snake.body) ? strict_safe_moves_count_local(state, i, snake.body, state.apple, true) : 0;
    int area = area_from_body_state(state, i, snake.body, min<int>(28, len + 12));

    if (safeCnt == 0) cur -= 1500;
    else if (safeCnt == 1) cur -= 360;
    else if (safeCnt == 2) cur += 30;
    else cur += 90;

    if (area <= len) cur -= 900;
    else if (area <= len + 2) cur -= 320;
    else if (area <= len + 5) cur -= 110;

    if (is_vertical(snake.body) && face_dir(snake.body) == 0 && sideCnt > 0) cur -= 180;

    if (len <= 4) {
      cur -= 2 * traffic_pressure(state, i, snake.body[0]);
      if (cell_in_other_routes(state, i, snake.body[0])) cur -= 220;
    }

    int aCnt = apple_count(state);
    if (aCnt <= 3 && aCnt > 0) {
      int dAp = nearest_apple_manh(state, snake.body[0]);

      if (sign == 1) {
        cur -= 110 * dAp;
        if (len >= 8) cur -= 55 * dAp;
        else if (len >= 6) cur -= 25 * dAp;
      } else {
        cur += 40 * dAp;
      }
    }

    if (guaranteed_win_state(state) && sign == 1) {
      cur += 2 * snake_height_metric(snake.body);
      int safeCnt2 = strict_safe_moves_count_local(state, i, snake.body, state.apple, false);
      cur += 120 * safeCnt2;
      cur += loopiness_bonus(snake.body);
    }

    val += sign * cur;

    if (snake.owner == myId) myHeads.pb(snake.body[0]);
    else oppHeads.pb(snake.body[0]);
  }

  val -= head_spread_penalty(myHeads);
  val += head_spread_penalty(oppHeads) / 2;

  return val;
}

vector<MoveInfo> build_moves(const State &state, int idx, const SearchInfo *searchInfo, const SupportPathInfo *pathInfo) {
  vector<MoveInfo> res;
  auto &snake = state.snakes[idx];
  if (!snake.alive) return res;

  auto enemyHeads = get_enemy_heads(state, snake.owner);
  auto block = build_block(state, idx);
  int face = face_dir(snake.body);
  bool vertical = is_vertical(snake.body);
  int len = sz(snake.body);
  int sideCntNow = vertical ? strict_safe_moves_count_local(state, idx, snake.body, state.apple, true) : 0;
  int aCnt = apple_count(state);
  bool winLock = guaranteed_win_state(state);
  bool rootMode = (searchInfo != nullptr || pathInfo != nullptr);

  for (int dir : legal_dirs(snake)) {
    pii rawHead = {snake.body[0].st + dx[dir], snake.body[0].nd + dy[dir]};
    bool eat = has_apple(state.apple, rawHead);
    bool ownTail = !eat && same(rawHead, snake.body.back());
    bool riskyNear = near_enemy_head(rawHead, enemyHeads) && !eat;

    auto sim = one_snake_step(snake, dir, block, state.apple);
    int score = 0;

    int outCnt = sim.alive ? count_outside_parts(sim.body) : 99;
    bool strictSafe = inside(rawHead) && !wall(rawHead) && sim.alive && !sim.beheaded && outCnt == 0;
    bool routeRisk = cell_in_other_routes(state, idx, rawHead);
    int traffic = traffic_pressure(state, idx, rawHead);
    if (sim.alive) traffic += traffic_pressure(state, idx, sim.body[0]) / 2;

    if (!inside(rawHead)) score -= 1000000;
    if (wall(rawHead)) score -= 1000000;
    if (!sim.alive) score -= 1000000;
    if (sim.beheaded) score -= 450000;
    if (outCnt > 0) score -= 200000 + 800 * outCnt;

    if (strictSafe) {
      pii finalHead = sim.body[0];

      if (eat) score += 900;
      if (ownTail) score += 70;
      if (dir == face) score += 8;

      if (vertical && (dir == 2 || dir == 3)) score += 60;
      if (vertical && (face == 0 || face == 1) && (dir == 2 || dir == 3)) score += 50;
      if (vertical && face == 0 && dir == 0 && !eat && sideCntNow > 0) score -= 260;

      if (riskyNear) score -= 260;

      score += snake_height_metric(sim.body);

      if (searchInfo != nullptr && searchInfo->firstDist[dir] < inf) {
        score += 320 - 24 * searchInfo->firstDist[dir];
      } else {
        score -= 7 * nearest_apple_manh(state, finalHead);
      }

      if (pathInfo != nullptr && pathInfo->bestAny < inf) {
        int Wpath = 0;
        if (aCnt <= 1) Wpath = 620;
        else if (aCnt <= 3) Wpath = 430;
        else if (aCnt <= 6) Wpath = 260;
        else Wpath = 150;

        if (pathInfo->firstDist[dir] < inf) {
          score += Wpath - 18 * pathInfo->firstDist[dir];
          if (pathInfo->firstDist[dir] == pathInfo->bestAny) score += 120;
        } else {
          if (searchInfo == nullptr || searchInfo->firstDist[dir] == inf) score -= 80;
        }
      }

      if (aCnt <= 3 && aCnt > 0) {
        int dAp = nearest_apple_manh(state, finalHead);
        score -= 120 * dAp;
        if (len >= 8) score -= 45 * dAp;
        if (eat && len >= 6) score += 180;
      }

      vector<char> nextApple = state.apple;
      if (eat && inside(rawHead)) nextApple[idxy(rawHead)] = 0;

      int replyCnt = strict_safe_moves_count_local(state, idx, sim.body, nextApple, false);
      int sideCnt = is_vertical(sim.body) ? strict_safe_moves_count_local(state, idx, sim.body, nextApple, true) : 0;
      int area = area_from_body_state(state, idx, sim.body, min<int>(24, len + 10));

      if (replyCnt == 0) score -= 1000;
      else if (replyCnt == 1) score -= 180;

      if (area <= len) score -= 900;
      else if (area <= len + 2) score -= 300;
      else if (area <= len + 5) score -= 100;

      if (is_vertical(sim.body) && sideCnt == 0) score -= 700;
      if (is_vertical(sim.body) && face_dir(sim.body) == 0 && sideCnt > 0) score -= 220;
      if (is_vertical(sim.body) && (sim.body[0].st == 0 || sim.body[0].st == w - 1)) score -= 180;

      if (len <= 4) {
        score -= 2 * traffic;
        if (routeRisk) score -= 340;
      } else {
        score -= traffic / 2;
      }

      rep(j,0,sz(state.snakes)) if (j != idx && state.snakes[j].alive && state.snakes[j].owner == snake.owner) {
        int d = manh(finalHead, state.snakes[j].body[0]);
        if (d < 10) score -= 12 * (10 - d);
      }

      if (rootMode) {
        bool esc2 = can_survive_k_local(state, idx, sim.body, nextApple, 2);
        bool esc3 = can_survive_k_local(state, idx, sim.body, nextApple, 3);
        int futureWidth = safe_branching_local(state, idx, sim.body, nextApple);

        if (!esc2) score -= 700;
        if (!esc3) score -= 1200;

        if (futureWidth == 0) score -= 900;
        else if (futureWidth == 1) score -= 140;
        else if (futureWidth >= 3) score += 40;
      }

      if (winLock) {
        int dAp = nearest_apple_manh(state, finalHead);

        vector<char> nextApple2 = state.apple;
        if (eat && inside(rawHead)) nextApple2[idxy(rawHead)] = 0;

        bool esc2 = can_survive_k_local(state, idx, sim.body, nextApple2, 2);
        bool esc3 = can_survive_k_local(state, idx, sim.body, nextApple2, 3);
        int futureWidth = safe_branching_local(state, idx, sim.body, nextApple2);

        score += 700 * esc2;
        score += 1200 * esc3;
        score += 140 * futureWidth;

        score += loopiness_bonus(sim.body);
        if (ownTail) score += 260;

        if (eat) score += 700;

        score += snake_height_metric(sim.body);
        score -= 36 * dAp;
        if (aCnt <= 3) score -= 80 * dAp;
        score -= traffic;
        if (riskyNear) score -= 400;

        if (vertical && face == 0 && dir == 0 && sideCntNow > 0 && !eat) score -= 450;
      }
    }

    MoveInfo cur;
    cur.dir = dir;
    cur.score = score;
    cur.rawHead = rawHead;
    cur.finalHead = sim.alive ? sim.body[0] : rawHead;
    cur.eat = eat;
    cur.nearEnemy = riskyNear;
    cur.alive = sim.alive;
    cur.beheaded = sim.beheaded;
    cur.ownTail = ownTail;
    cur.strictSafe = strictSafe;
    cur.routeRisk = routeRisk;
    cur.outCnt = outCnt;
    cur.traffic = traffic;
    res.pb(cur);
  }

  bool hasStrict = false;
  for (auto mv : res) if (mv.strictSafe) hasStrict = true;
  if (hasStrict) {
    vector<MoveInfo> tmp;
    for (auto mv : res) if (mv.strictSafe) tmp.pb(mv);
    if (!tmp.empty()) res.swap(tmp);
  }

  if (len <= 4) {
    bool hasClean = false;
    for (auto mv : res) if (!mv.routeRisk) hasClean = true;
    if (hasClean) {
      vector<MoveInfo> tmp;
      for (auto mv : res) if (!mv.routeRisk) tmp.pb(mv);
      if (!tmp.empty()) res.swap(tmp);
    }
  }

  if (vertical && face == 0 && sideCntNow > 0) {
    bool hasGoodSide = false;
    for (auto mv : res) if ((mv.dir == 2 || mv.dir == 3) && mv.strictSafe) hasGoodSide = true;
    if (hasGoodSide) {
      vector<MoveInfo> tmp;
      for (auto mv : res) if (!(mv.dir == 0 && !mv.eat)) tmp.pb(mv);
      if (!tmp.empty()) res.swap(tmp);
    }
  }

  sort(all(res), [](const MoveInfo &A, const MoveInfo &B) {
    if (A.score != B.score) return A.score > B.score;
    return A.dir < B.dir;
  });
  return res;
}

void step(State &state, const vector<int> &act) {
  int n = sz(state.snakes);
  vector<vector<pii>> afterMove(n);

  rep(i,0,n) {
    auto &snake = state.snakes[i];
    if (!snake.alive) continue;

    int face = face_dir(snake.body);
    int dir = act[i];
    if (dir < 0) dir = face;
    if (dir == oppDir[face]) dir = face;

    pii nh = {snake.body[0].st + dx[dir], snake.body[0].nd + dy[dir]};
    bool eat = has_apple(state.apple, nh);

    afterMove[i].reserve(sz(snake.body) + 1);
    afterMove[i].pb(nh);
    rep(j,0,sz(snake.body) - 1) afterMove[i].pb(snake.body[j]);
    if (eat) afterMove[i].pb(snake.body.back());
  }

  rep(i,0,n) if (state.snakes[i].alive) state.snakes[i].body = move(afterMove[i]);

  vector<char> eaten(w * h, 0);
  rep(i,0,n) if (state.snakes[i].alive) {
    pii head = state.snakes[i].body[0];
    if (has_apple(state.apple, head)) eaten[idxy(head)] = 1;
  }
  rep(i,0,w * h) if (eaten[i]) state.apple[i] = 0;

  vector<int> behead;
  rep(i,0,n) if (state.snakes[i].alive) {
    auto &snake = state.snakes[i];
    pii head = snake.body[0];
    bool inWall = wall(head);
    bool inBird = false;

    rep(j,0,n) if (state.snakes[j].alive) {
      auto &other = state.snakes[j];
      if (!has_cell(other.body, head)) continue;
      if (j != i) {
        inBird = true;
        break;
      }
      if (has_cell_skip_head(other.body, head)) {
        inBird = true;
        break;
      }
    }

    if (inWall || inBird) behead.pb(i);
  }

  for (int i : behead) {
    auto &snake = state.snakes[i];
    if (!snake.alive) continue;
    if (sz(snake.body) <= 3) {
      snake.alive = false;
      snake.body.clear();
    } else {
      snake.body.erase(snake.body.begin());
    }
  }

  auto can_fall_bird = [&](int idx) {
    auto &snake = state.snakes[idx];
    if (!snake.alive) return false;
    for (auto p : snake.body) if (solid_under_cell(state, p, snake.body)) return false;
    return true;
  };

  auto birds_touch = [&](int a, int b) {
    for (auto p : state.snakes[a].body) for (auto q : state.snakes[b].body)
      if (manh(p, q) == 1) return true;
    return false;
  };

  auto do_intercoiled = [&]() {
    bool fellAtLeastOnce = false;
    bool something = true;
    while (something) {
      something = false;
      vector<int> live;
      rep(i,0,n) if (state.snakes[i].alive) live.pb(i);
      vector<int> vis(n, 0);

      for (int src : live) {
        if (vis[src]) continue;
        vector<int> comp;
        queue<int> q;
        q.push(src);
        vis[src] = 1;
        while (!q.empty()) {
          int v = q.front();
          q.pop();
          comp.pb(v);
          for (int u : live) if (!vis[u] && birds_touch(v, u)) {
            vis[u] = 1;
            q.push(u);
          }
        }
        if (sz(comp) <= 1) continue;

        vector<pii> meta;
        for (int v : comp) for (auto p : state.snakes[v].body) meta.pb(p);
        bool canFall = true;
        for (auto p : meta) if (solid_under_cell(state, p, meta)) {
          canFall = false;
          break;
        }
        if (!canFall) continue;

        something = true;
        fellAtLeastOnce = true;
        for (int v : comp) {
          for (auto &p : state.snakes[v].body) p.nd++;
          if (state.snakes[v].body[0].nd >= h) {
            state.snakes[v].alive = false;
            state.snakes[v].body.clear();
          }
        }
      }
    }
    return fellAtLeastOnce;
  };

  bool somethingFell = true;
  while (somethingFell) {
    while (somethingFell) {
      somethingFell = false;
      rep(i,0,n) if (can_fall_bird(i)) {
        somethingFell = true;
        for (auto &p : state.snakes[i].body) p.nd++;
        bool out = true;
        for (auto p : state.snakes[i].body) if (p.nd < h + 1) out = false;
        if (out) {
          state.snakes[i].alive = false;
          state.snakes[i].body.clear();
        }
      }
    }
    somethingFell |= do_intercoiled();
  }

  state.turn++;
}

vector<int> pick_policy_dirs(const State &state, int owner, bool deterministic, const vector<SearchInfo> *rootSearch = nullptr) {
  int n = sz(state.snakes);
  vector<int> act(n, -1);
  unordered_set<uint64_t> reservedNext;
  unordered_set<uint64_t> reservedFinal;

  vector<int> order;
  rep(i,0,n) if (state.snakes[i].alive && state.snakes[i].owner == owner) order.pb(i);
  sort(all(order), [&](int a, int b) {
    return sz(state.snakes[a].body) > sz(state.snakes[b].body);
  });

  auto take_move = [&](vector<MoveInfo> cand) {
    vector<MoveInfo> ok = cand;

    vector<MoveInfo> good;
    for (auto mv : ok) {
      if (reservedNext.count(key_xy(mv.rawHead))) continue;
      if (reservedFinal.count(key_xy(mv.finalHead))) continue;
      good.pb(mv);
    }
    if (!good.empty()) ok = good;

    bool hasNonRisk = false;
    for (auto mv : ok) if (!mv.nearEnemy || mv.eat) hasNonRisk = true;
    if (hasNonRisk) {
      vector<MoveInfo> tmp;
      for (auto mv : ok) if (!mv.nearEnemy || mv.eat) tmp.pb(mv);
      if (!tmp.empty()) ok = tmp;
    }

    if (deterministic) return ok[0];

    int best = ok[0].score;
    vector<int> pref;
    for (auto mv : ok) {
      int wgh = max<int>(1, 60 + mv.score - best);
      rep(i,0,wgh) pref.pb(mv.dir);
    }
    int dir = pref[rng() % sz(pref)];
    for (auto mv : ok) if (mv.dir == dir) return mv;
    return ok[0];
  };

  for (int idx : order) {
    const SearchInfo *ptr = nullptr;
    if (rootSearch != nullptr) ptr = &(*rootSearch)[idx];
    auto cand = build_moves(state, idx, ptr, nullptr);
    if (cand.empty()) continue;
    auto mv = take_move(cand);
    act[idx] = mv.dir;
    reservedNext.insert(key_xy(mv.rawHead));
    reservedFinal.insert(key_xy(mv.finalHead));
  }

  return act;
}

int rollout(State state, const vector<int> &myRoot, const vector<SearchInfo> &rootSearchMy, const vector<SearchInfo> &rootSearchOpp, int depth) {
  auto oppRoot = pick_policy_dirs(state, 1 - myId, false, &rootSearchOpp);

  vector<int> rootAct(sz(state.snakes), -1);
  rep(i,0,sz(state.snakes)) {
    if (state.snakes[i].alive && state.snakes[i].owner == myId) rootAct[i] = myRoot[i];
    else rootAct[i] = oppRoot[i];
  }
  step(state, rootAct);

  for (int d = 1; d < depth && !terminal(state); d++) {
    auto myAct = pick_policy_dirs(state, myId, false, nullptr);
    auto oppAct = pick_policy_dirs(state, 1 - myId, false, nullptr);
    vector<int> joint(sz(state.snakes), -1);
    rep(i,0,sz(state.snakes)) {
      if (state.snakes[i].alive && state.snakes[i].owner == myId) joint[i] = myAct[i];
      else joint[i] = oppAct[i];
    }
    step(state, joint);
  }

  return eval_state(state);
}

vector<Combo> build_root_combos(const State &state, const vector<SearchInfo> &rootSearchMy, const vector<SupportPathInfo> &rootPath) {
  int n = sz(state.snakes);
  vector<int> mine;
  rep(i,0,n) if (state.snakes[i].alive && state.snakes[i].owner == myId) mine.pb(i);

  vector<vector<MoveInfo>> allMoves(n);
  for (int idx : mine) allMoves[idx] = build_moves(state, idx, &rootSearchMy[idx], &rootPath[idx]);

  int K = sz(mine) <= 3 ? 3 : 2;
  vector<Combo> res;
  vector<int> cur(n, -1);
  vector<pii> chosenFinal(n, {-1000, -1000});
  unordered_set<uint64_t> reservedNext;
  unordered_set<uint64_t> reservedFinal;

  function<void(int,int)> dfs = [&](int at, int prior) {
    if (at == sz(mine)) {
      int spreadPenalty = 0;
      rep(i,0,sz(mine)) rep(j,i + 1,sz(mine)) {
        pii a = chosenFinal[mine[i]];
        pii b = chosenFinal[mine[j]];
        int d = manh(a, b);
        if (d < 10) spreadPenalty += 20 * (10 - d);
      }

      int key = 0;
      int mul = 1;
      for (int idx : mine) {
        key += mul * (cur[idx] + 1);
        mul *= 5;
      }
      res.pb({cur, prior - spreadPenalty, key});
      return;
    }

    int idx = mine[at];
    int lim = min<int>(K, sz(allMoves[idx]));
    bool any = false;

    rep(i,0,lim) {
      auto mv = allMoves[idx][i];
      if (reservedNext.count(key_xy(mv.rawHead))) continue;
      if (reservedFinal.count(key_xy(mv.finalHead))) continue;
      any = true;

      cur[idx] = mv.dir;
      chosenFinal[idx] = mv.finalHead;
      reservedNext.insert(key_xy(mv.rawHead));
      reservedFinal.insert(key_xy(mv.finalHead));

      dfs(at + 1, prior + mv.score);

      reservedNext.erase(key_xy(mv.rawHead));
      reservedFinal.erase(key_xy(mv.finalHead));
      chosenFinal[idx] = {-1000, -1000};
      cur[idx] = -1;
    }

    if (!any) {
      rep(i,0,lim) {
        auto mv = allMoves[idx][i];
        if (reservedNext.count(key_xy(mv.rawHead))) continue;

        cur[idx] = mv.dir;
        chosenFinal[idx] = mv.finalHead;
        reservedNext.insert(key_xy(mv.rawHead));

        dfs(at + 1, prior + mv.score - 50);

        reservedNext.erase(key_xy(mv.rawHead));
        chosenFinal[idx] = {-1000, -1000};
        cur[idx] = -1;
      }
    }
  };

  dfs(0, 0);

  if (res.empty()) {
    unordered_set<uint64_t> used;
    vector<int> act(n, -1);
    int prior = 0;
    for (int idx : mine) {
      auto mv = allMoves[idx][0];
      if (used.count(key_xy(mv.rawHead))) {
        for (auto alt : allMoves[idx]) if (!used.count(key_xy(alt.rawHead))) {
          mv = alt;
          break;
        }
      }
      act[idx] = mv.dir;
      used.insert(key_xy(mv.rawHead));
      prior += mv.score;
    }
    int key = 0, mul = 1;
    for (int idx : mine) {
      key += mul * (act[idx] + 1);
      mul *= 5;
    }
    res.pb({act, prior, key});
  }

  sort(all(res), [](const Combo &A, const Combo &B) {
    if (A.prior != B.prior) return A.prior > B.prior;
    return A.key < B.key;
  });

  vector<Combo> uniq;
  unordered_set<int> seen;
  for (auto x : res) if (!seen.count(x.key)) {
    seen.insert(x.key);
    uniq.pb(x);
  }

  int KEEP = min<int>(24, sz(uniq));
  uniq.resize(KEEP);
  return uniq;
}

signed main() {
  cin >> myId;
  cin >> w >> h;

  g.resize(h);
  rep(i,0,h) cin >> g[i];

  int snakesPerPlayer;
  cin >> snakesPerPlayer;

  myIds.resize(snakesPerPlayer);
  oppIds.resize(snakesPerPlayer);
  rep(i,0,snakesPerPlayer) {
    cin >> myIds[i];
    mySet.insert(myIds[i]);
  }
  rep(i,0,snakesPerPlayer) {
    cin >> oppIds[i];
    oppSet.insert(oppIds[i]);
  }

  while (1) {
    int powerCnt;
    cin >> powerCnt;
    if (!cin) break;

    State root;
    root.apple.assign(w * h, 0);
    root.turn = turnNo;

    rep(i,0,powerCnt) {
      int x, y;
      cin >> x >> y;
      if (inside(x, y)) root.apple[idxy(x, y)] = 1;
    }

    int snakeCnt;
    cin >> snakeCnt;

    root.snakes.clear();
    root.snakes.reserve(snakeCnt);

    rep(i,0,snakeCnt) {
      int id;
      string body;
      cin >> id >> body;

      Snake snake;
      snake.id = id;
      snake.owner = mySet.count(id) ? myId : (1 - myId);
      snake.alive = true;
      snake.body = parse_body(body);
      root.snakes.pb(snake);
    }

    auto startTime = chrono::steady_clock::now();
    int budgetMs = (turnNo == 0 ? 900 : 42);

    auto time_left = [&]() {
      auto now = chrono::steady_clock::now();
      int spent = chrono::duration_cast<chrono::milliseconds>(now - startTime).count();
      return budgetMs - spent;
    };

    vector<SearchInfo> rootSearch(sz(root.snakes), SearchInfo(w * h));
    vector<SearchInfo> rootSearchOpp(sz(root.snakes), SearchInfo(w * h));
    vector<SupportPathInfo> rootPath(sz(root.snakes));

    rep(i,0,sz(root.snakes)) if (root.snakes[i].alive) {
      if (time_left() > 20) rootSearch[i] = search_snake(root, i, turnNo == 0 ? 9 : 7);
      else rootSearch[i] = SearchInfo(w * h);
      rootSearchOpp[i] = rootSearch[i];
    }

    rep(i,0,sz(root.snakes)) if (root.snakes[i].alive && root.snakes[i].owner == myId) {
      if (time_left() > 10) {
        int maxExpand = (turnNo == 0 ? 120000 : 70000);
        rootPath[i] = support_path_search(root, i, maxExpand, 10);
      }
    }

    auto combos = build_root_combos(root, rootSearch, rootPath);
    if (combos.empty()) {
      cout << "WAIT" << endl;
      turnNo++;
      continue;
    }

    unordered_map<int, Stat> stats;

    auto eval_combo_once = [&](const Combo &combo, bool deterministic) {
      int depth = deterministic ? 2 : 3;
      return rollout(root, combo.act, rootSearch, rootSearchOpp, depth);
    };

    for (auto &combo : combos) {
      if (time_left() <= 4) break;
      int val = eval_combo_once(combo, true);
      stats[combo.key].sum += val;
      stats[combo.key].cnt++;
    }

    int totalRoll = 0;
    for (auto &combo : combos) totalRoll += stats[combo.key].cnt;

    auto ucb_value = [&](const Combo &combo, int total) {
      auto cur = stats[combo.key];
      ld avg = cur.cnt ? cur.sum / cur.cnt : -1e18L;
      ld bonus = 120.0L * sqrt(log((ld)max<int>(2, total)) / (ld)(cur.cnt + 1));
      return avg + bonus + combo.prior * 0.02L;
    };

    while (time_left() > 2) {
      int best = 0;
      rep(i,1,sz(combos)) {
        if (ucb_value(combos[i], totalRoll + 1) > ucb_value(combos[best], totalRoll + 1)) best = i;
      }
      int val = eval_combo_once(combos[best], false);
      stats[combos[best].key].sum += val;
      stats[combos[best].key].cnt++;
      totalRoll++;
    }

    int best = 0;
    auto mean_value = [&](const Combo &combo) {
      auto cur = stats[combo.key];
      if (!cur.cnt) return (ld)-1e18;
      return cur.sum / cur.cnt;
    };

    rep(i,1,sz(combos)) {
      ld A = mean_value(combos[i]);
      ld B = mean_value(combos[best]);
      if (A > B + 1e-12) best = i;
      else if (fabsl(A - B) <= 1e-12 && combos[i].prior > combos[best].prior) best = i;
    }

    cerr << "turn=" << turnNo
         << " mcts_iters=" << totalRoll
         << " combos=" << sz(combos)
         << " best_mean=" << (double)mean_value(combos[best])
         << '\n';

    vector<string> out;
    auto bestAct = combos[best].act;
    bool wroteInfo = false;

    rep(i,0,sz(root.snakes)) if (root.snakes[i].alive && root.snakes[i].owner == myId) {
      int dir = bestAct[i];
      if (dir == -1) dir = face_dir(root.snakes[i].body);
      string cmd = to_string(root.snakes[i].id) + " " + dn[dir];
      if (!wroteInfo) {
        cmd += " mcts=" + to_string(totalRoll);
        wroteInfo = true;
      }
      out.pb(cmd);
    }

    if (out.empty()) out.pb("WAIT");

    rep(i,0,sz(out)) {
      if (i) cout << ';';
      cout << out[i];
    }
    cout << endl;

    turnNo++;
  }

  return 0;
}
