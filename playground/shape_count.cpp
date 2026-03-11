// surgutti, winter-challenge-2026, 10-03-2026
#include "bits/stdc++.h"
using namespace std;

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
using pii=pair<int,int>;
using vi=vector<int>;

const int inf = 1e9+7;
const int dx[] = {-1, 0, +1, 0};
const int dy[] = {0, -1, 0, +1};

using u64=unsigned long long;

int max_length;
vector<vector<pii>> cache;
vector<pii> path;

u64 hh(vector<pii> p) {
  auto it = *min_element(all(p));
  for (auto&[x,y]:p){
    x-=it.st;
    y-=it.nd;
    x += 100;
    y += 100;
  }

  u64 h = 0;
  for (auto [x,y]:p){
    h += x;
    h *= 2137;
    h += y;
    h *= 2137;
  }
  return h;
}

void gen(int x, int y) {
  path.eb(x,y);
  
  if (sz(path) == max_length) {
    cache.emplace_back(path);
    path.pop_back();
    return; 
  }

  rep(d,0,4){
    int xx = x + dx[d];
    int yy = y + dy[d];

    bool ok = true;
    rep(j,0,sz(path)-1)
      if(mp(xx,yy)==path[j])
        ok=false;;
    if (!ok)
      continue;

    gen(xx, yy);
  }
  path.pop_back();
}

signed main() {

  for (max_length=3;max_length<=20;max_length++) {
    cache.clear();
    gen(0,0);
    cerr<<"length:"<<max_length<<": "<<sz(cache)<<' '<<sz(cache)*45*30*4*4<<'\n';
  }
  return 0;
}
