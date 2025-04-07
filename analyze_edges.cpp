// Author: Olaf Surgut (surgutti)
// Created on 07-04-2025 21:28:56
#ifndef LOCAL
#undef _GLIBCXX_DEBUG  // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline,unroll-loops,tracer,vpt,split-loops,unswitch-loops")
// #undef __cplusplus
// #define __cplusplus 202002L
#endif

#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>

#ifndef LOCAL
#pragma GCC target( \
    "aes,align-stringops,avx,avx2,bmi,bmi2,crc32,cx16,f16c,fma,fsgsbase,fxsr,hle,ieee-fp,lzcnt,mmx,movbe,mwait,pclmul,popcnt,rdrnd,sahf,sse,sse2,sse3,sse4,sse4.1,sse4.2,ssse3,xsave,xsaveopt")
#endif

using namespace std;

// #define int long long
#define ll long long
#define ld long double

#define endl '\n'
#define st first
#define nd second
#define pb push_back
#define eb emplace_back
#define sz(x) (int)(x).size()
#define all(x) begin(x),end(x)
#define FOR(i,l,r) for(int i=(l);i<=(r);i++)
#define ROF(i,r,l) for(int i=(r);i>=(l);i--)

auto& operator<<(auto &o, pair<auto, auto> p) {
	return o << "(" << p.st << ", " << p.nd << ")";}
auto operator<<(auto &o, auto x)->decltype(end(x), o) {
	o << "{"; int i=0; for (auto e : x) o << ","+!i++ << e;
	return o << "}";}

#ifdef LOCAL
#define debug(x...) cerr << "[" #x "]: ", [](auto...$) { \
	((cerr << $ << "; "),...) << endl; }(x)
#else
#define debug(...)
#endif

#define rep(i,a,b) for(int i = a; i < (b); i++)
using pii = pair<int, int>;
using vi = vector<int>;

typedef uint32_t Board;

const int N = 4183357 * 2;

int n, m;
unordered_map<Board, int> idx;
unordered_set<uint64_t> done_edges;
vi adj[N];
int deg[N];

vi order;

/*
int blocked[N];
int vis[N], czas = 1;
bool check(int ile) {
	rep(i, 0, n) {
		blocked[i] = 0;
	}

	int cnt = 0;
	ROF(i, n - 1, 0) {
		vi q{order[i]};
		czas++;
		
		rep(j, 0, sz(q)) {
			int u = q[j];

			if (blocked[u] == 0) {
				for (int v : adj[u]) if (vis[v] != czas) {
					vis[v] = czas;
					q.pb(v);
				}
			}

			if (sz(q) > ile) {
				break;
			}
		}
		
		if (sz(q) > ile) {
			blocked[order[i]] = 1;
			cnt++;
		}

		if (cnt > 50000) {
			cerr << "fail: " << ile << ' ' << i << ' ' << double(i) / n << '\n';
			return false;
		}
	}

	cerr << "> " << ile << " => " << cnt << '\n';
	return true;
}
*/

int block[N];
int d1[N], d2[N];

int calc() {
	rep(i, 0, n) {
		d1[i] = d2[i] = 0;
	}
		
	rep(i, 0, n) {
		int u = order[i];

		if (block[u])
			continue;

		for (int v : adj[u]) {
			d1[v] = max(d1[v], d1[u] + 1);
		}
	}

	ROF(i, n - 1, 0) {
		int u = order[i];

		if (block[u])
			continue;

		for (int v : adj[u]) if (!block[u]) {
			d2[u] = max(d2[u], d2[v] + 1);
		}
	}

	int best = -1;
	ll val = -1;
	rep(i, 0, n) {
		ll now = (ll) (1 + d1[i]) * (1 + d2[i]);
		if (val < now) {
			val = now;
			best = i;
		}
	}

	return best;
}

signed main() {
	cin.tie(0)->sync_with_stdio(0);

	idx.reserve(4183357);

	cin >> m;
	rep(i, 0, m) {
		Board b1, b2;
		int d1, d2;
		cin >> b1 >> d1 >> b2 >> d2;
		
		if (i % 100000 == 0) {
			cerr << i << "/" << m << ' ' << double(i) / m << '\n';
		}

		uint64_t hh = b1;
		hh <<= 32;
		hh |= b2;

		if (done_edges.count(hh))
			continue;
		done_edges.insert(hh);

		if (idx.count(b1) == 0) {
			idx[b1] = sz(idx);
		}

		if (idx.count(b2) == 0) {
			idx[b2] = sz(idx);
		}
		
		assert(sz(idx) < N);
		adj[idx[b1]].pb(idx[b2]);
		deg[idx[b2]]++;
	}

	n = sz(idx);

	cerr << sz(idx) << '\n';
	cerr << sz(done_edges) << '\n';

	rep(i, 0, n) {
		if (deg[i] == 0) {
			order.pb(i);
		}
	}

	rep(i, 0, sz(order)) {
		int u = order[i];
		for (int v : adj[u]) 
			if (--deg[v] == 0)
				order.pb(v);
	}

	vi nodes;
	rep(i, 0, 1000) {
		if (i % 10 == 5) {
			cerr << "i: " << i << '\n';
		}

		int u = calc();
		block[u] = true;
		nodes.pb(u);
	}

	cerr << nodes << '\n';

	return 0;
}

