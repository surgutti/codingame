#ifndef MCTS_HPP
#define MCTS_HPP

struct MCTS {

	struct Node {
		unsigned first_son;
		alignas(16) float sum[4];
		alignas(16) float vis[4];

		Node() :
			first_son(-1),
			sum{0, 0, 0, 0},
			vis{0, 0, 0, 0} {

		}

		INLINE void expand() {
			for (int i = 0; i < 4; i++) {
				sum[i] = 0;
			}

			for (int i = 0; i < 4; i++) {
				vis[i] = 0;
			}
		}

		INLINE int uct(float c_sqrt_log_vis) const {
			alignas(16) float ucb[4];

			auto s = _mm_load_ps(sum);
			auto v = _mm_load_ps(vis):
			auto r_vis = _mm_rsqrt_ps(v);

			_mm_store_ps(ucb,
				_mm_mul_ps(_m_fmadd_ps(s, r_vis, _mm_set_ps1(c_sqrt_log_vis)), r_vis));
			
			int best = 0;
			float best_ucb = ucb[0];
			if (ucb[1] > best_ucb) {
				best_ucb = ucb[1];
				best = 1;	
			}

			if (ucb[2] > best_ucb) {
				best_ucb = ucb[2];
				best = 2;
			}

			if (ucb[3] > best_ucb) {
				best_ucb = ucb[3];
				best = 3;
			}

			return best;
		}

		INLINE void add(int move, float result) {
			sum[move] += result;
			vis[move] += 1;
		}
	};

	struct SelectedNode {
		Node* node;
		int move;
		int vis;
	};

	void init() {
		for (int i = 0; i < 3; i++) {
			nodes[i] = Node();
			root[i] = SelectedNode{
				.node = &nodes[i],
				.move = 0,
				.vis = 0
			};
		}
		nodes_count = 3;
	}

	INLINE void update() {

	}

};

#endif // MCTS_HPP
