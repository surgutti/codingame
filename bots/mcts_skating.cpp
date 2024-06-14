// *** Start of: /home/olaf/codingame/main.cpp *** 
#ifndef LOCAL
    #undef _GLIBCXX_DEBUG
    #pragma GCC optimize("Ofast,inline")
    #pragma GCC target("bmi,bmi2,lzcnt,popcnt")
    #pragma GCC target("movbe")
    #pragma GCC target("aes,pclmul,rdrnd")
    #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2")
#endif // LOCAL

 // *** Start of: /home/olaf/codingame/state.hpp *** 
 #ifndef STATE_HPP
 #define STATE_HPP
 
  // *** Start of: /home/olaf/codingame/minigames/hurdle_race.hpp *** 
  #ifndef HURDLE_RACE
  #define HURDLE_RACE
  
   // *** Start of: /home/olaf/codingame/const.hpp *** 
   #ifndef CONST_HPP
   #define CONST_HPP
   
   const int INF = 1'000'000;
   
   const int TRACK_LENGTH = 30;
   const int ARCHERY_LENGTH = 12 + 3; // 12 + random.nextInt(4);
   const int DIVING_LENGTH = 12 + 3; // 12 + random.nextInt(4);
   
   const int MCTSNODE_POOL = 8'500'000;
   
   const float C = 0.8f;
   
   #endif // CONST_HPP
   // *** End of: /home/olaf/codingame/const.hpp *** 
  
  #include <iostream>
  #include <cassert>
  
  struct HurdleRace {
      
      uint32_t track;
  
      int8_t pos[3], stun[3];
      bool end;
      int8_t places[3];
  
      void debug() const {
          std::cerr << "TRACK: ";
          for (int i = 0; i < TRACK_LENGTH; i++) {
              if (track & (1u << i))
                  std::cerr << '#';
              else
                  std::cerr << '.';
          }
          std::cerr << "|\n";
  
          for (int i = 0; i < 3; i++) {
              std::cerr << "i: " << i << " => " << int(pos[i]) << ' ' << int(stun[i]) << '\n';
          }
      }
  
      void play(const int8_t* move) {
          if (end) {
              return;
          }
  
          for (int i = 0; i < 3; i++) {
              if (stun[i]) {
                  stun[i]--;
              }
              else {
                  if (move[i] == 1) {
                      pos[i]++;
                  }
                  else
                  if (move[i] == 0) {
                      pos[i] += 2;
                  }
                  else
                  if (move[i] == 2) {
                      pos[i]++;
                      if (track & (1U << pos[i])) {
                          stun[i] = 2;
                      }
                      else {
                          pos[i]++;
                      }
                  }
                  else {
                  // if (move[i] == 3) {
                      pos[i]++;
                      if (track & (1U << pos[i])) {
                          stun[i] = 2;
                      }
                      else {
                          pos[i]++;
                          if (track & (1U << pos[i])) {
                              stun[i] = 2;
                          }
                          else {
                              pos[i]++;
                          }
                      }
                  }
                  // else {
                  //     assert(false);
                  // }
  
                  if (track & (1U << pos[i])) {
                      stun[i] = 2;
                  }
                  else
                  if (pos[i] >= TRACK_LENGTH - 1) {
                      end = true;
                  }
              }
          }
  
          if (end) {
              for (int i = 0; i < 3; i++) {
                  if (pos[i] >= TRACK_LENGTH - 1) {
                      places[i] = 3;
                  }
                  else
                  if (pos[i] < pos[(i + 1) % 3] && pos[i] < pos[(i + 2) % 3]) {
                      places[i] = 0;
                  }
                  else {
                      places[i] = 1;
                  }
              }
          }
      }
  
      bool proven_win(int p_idx) const {
          return false;
      }
  
      bool proven_lost(int p_idx) const {
          return false;
      }
  };
  
  #endif // HURDLE_RACE
  // *** End of: /home/olaf/codingame/minigames/hurdle_race.hpp *** 
  // *** Start of: /home/olaf/codingame/minigames/archery.hpp *** 
  #ifndef ARCHERY_HPP
  #define ARCHERY_HPP
  
  
  struct Archery {
  
      int8_t wind[ARCHERY_LENGTH];
      int8_t wind_index;
  
      int8_t x[3];
      int8_t y[3];
  
      bool end;
  
      int8_t places[3];
  
      void debug() const {
          std::cerr << "wind_index: " << wind_index << '\n';
          std::cerr << "wind: ";
          for (int i = wind_index; i >= 0; i--) {
              std::cerr << int(wind[i]) << ' ';
          }
          std::cerr << '\n';
          for (int i = 0; i < 3; i++) {
              std::cerr << "i: " << i << " => " << int(x[i]) << ' ' << int(y[i]) << '\n';
          }
      }
  
      void play(const int8_t* move) {
          if (end) {
              return;
          }
  
          static const int8_t dx[4] = {0, -1, 0, +1};
          static const int8_t dy[4] = {-1, 0, +1, 0};
  
          for (int i = 0; i < 3; i++) {
              x[i] += wind[wind_index] * dx[move[i]];
              y[i] += wind[wind_index] * dy[move[i]];
          
              if (x[i] < -20) x[i] = -20;
              if (x[i] > +20) x[i] = +20;
  
              if (y[i] < -20) y[i] = -20;
              if (y[i] > +20) y[i] = +20;
          }
  
          if (wind_index == 0) {
  
              int16_t scores[3];
              for (int i = 0; i < 3; i++) {
                  scores[i] = (int16_t) x[i] * x[i] + (int16_t) y[i] * y[i]; 
              }
  
              for (int i = 0; i < 3; i++) {
                  if (scores[i] <= scores[(i + 1) % 3] && scores[i] <= scores[(i + 2) % 3]) {
                      places[i] = 3;
                  }
                  else
                  if (scores[i] > scores[(i + 1) % 3] && scores[i] > scores[(i + 2) % 3]) {
                      places[i] = 0;
                  }
                  else {
                      places[i] = 1;
                  }
              }
  
              end = true;
          }
          else {
              wind_index--;
          }
      }
      
      bool proven_win(int p_idx) const {
          return false;
      }
  
      bool proven_lost(int p_idx) const {
          return false;
      }
  
  };
  
  #endif // ARCHERY_HPP
  // *** End of: /home/olaf/codingame/minigames/archery.hpp *** 
  // *** Start of: /home/olaf/codingame/minigames/roller_skating.hpp *** 
  #ifndef ROLLER_SKATING
  #define ROLLER_SKATING
  
   // *** Start of: /home/olaf/codingame/random.hpp *** 
   #ifndef RANDOM_HPP
   #define RANDOM_HPP
   
    // *** Start of: /home/olaf/codingame/utils.hpp *** 
    #ifndef UTILS_HPP
    #define UTILS_HPP
    
    #include <cassert>
    
    int to_move_index(char c) {
        if (c == 'U') {
            return 0;
        }
        if (c == 'L') {
            return 1;
        }
        if (c == 'D') {
            return 2;
        }
        if (c == 'R') {
            return 3;
        }
    
        assert(false);
    }
    
    #endif // UTILS_HPP
    // *** End of: /home/olaf/codingame/utils.hpp *** 
   
   static unsigned int g_seed = 2137;
   
   inline void fast_srand(int seed) {
   	g_seed = seed;
   }
   
   inline int fast_rand() {
   	unsigned int z = (g_seed += 0x39A3689AU);
   	z = (z ^ (z >> 14)) * 0x108E8AB8;
   	z = (z ^ (z >> 13)) * 0x2CD01FD6;
   	return int((z ^ (z >> 15)) >> 1);
   }
   
   inline int fast_rand(int a, int b) {
       return a + fast_rand() % (b - a + 1);
   }
   
   #endif
   
   // *** End of: /home/olaf/codingame/random.hpp *** 
  
  #include <iostream>
  #include <algorithm>
  
  struct RollerSkating {
  
      int8_t turns_left;
  
      int8_t dist[3];
      int8_t risk[3];
  
      int8_t places[3];
  
      int8_t order[4];
  
      bool end;
  
      void debug() const {
          std::cerr << "TURNS LEFT: " << int(turns_left) << '\n';
          for (int i = 0; i < 3; i++) {
              std::cerr << "i: " << i << ' ' << int(dist[i]) << ' ' << int(risk[i]) << '\n';
          }
          std::cerr << "order: ";
          for (int i = 0; i < 4; i++) {
              std::cerr << int(order[i]) << ' ';
          }
          std::cerr << '\n';
      }
  
      void play(const int8_t* move) {
          if (end) {
              return;
          }
  
          for (int i = 0; i < 3; i++) {
              if (risk[i] < 0) {
                  risk[i]++;
              }
              else
              if (order[move[i]] == 0) {
                  dist[i] += 1;
                  risk[i] -= 1;
              }
              else
              if (order[move[i]] == 1) {
                  dist[i] += 2;
              }
              else
              if (order[move[i]] == 2) {
                  dist[i] += 2;
                  risk[i] += 1;
              }
              else {
                  dist[i] += 3;
                  risk[i] += 2;
              }
          }
          
          if (risk[0] >= 0 &&
              (dist[0] % 10 == dist[1] % 10 ||
               dist[0] % 10 == dist[2] % 10)) {
              risk[0] += 2;
          }
  
          if (risk[1] >= 0 &&
              (dist[1] % 10 == dist[0] % 10 ||
              dist[1] % 10 == dist[2] % 10)) {
              risk[1] += 2;
          }
  
          if (risk[2] >= 0 &&
              (dist[2] % 10 == dist[0] % 10 ||
              dist[2] % 10 == dist[1] % 10)) {
              risk[2] += 2;
          }
  
          for (int i = 0; i < 3; i++) {
              if (risk[i] >= 5) {
                  risk[i] = -2;
              }
          }
  
          if (turns_left == 0) {
              for (int i = 0; i < 3; i++) {
                  if (dist[i] >= dist[(i + 1) % 3] && dist[i] >= dist[(i + 2) % 3]) {
                      places[i] = 3;
                  }
                  else
                  if (dist[i] < dist[(i + 1) % 3] && dist[i] < dist[(i + 2) % 3]) {
                      places[i] = 0;
                  }
                  else {
                      places[i] = 1;
                  }
              }
  
              end = true;
          }
          else {
              turns_left--;
  
              // new order
              std::swap(order[3], order[fast_rand() % 4]);
              std::swap(order[2], order[fast_rand() % 3]);
              std::swap(order[1], order[fast_rand() % 2]);
          }
      }
  
  };
  
  #endif // ROLLER_SPEED_SKATING
  // *** End of: /home/olaf/codingame/minigames/roller_skating.hpp *** 
  // *** Start of: /home/olaf/codingame/minigames/diving.hpp *** 
  #ifndef DIVING_HPP
  #define DIVING_HPP
  
  
  struct Diving {
  
      int32_t goal;
      int8_t goals_left;
  
      uint8_t score[3];
      int8_t combo[3];
  
      int8_t places[3];
  
      bool end;
  
      void debug() const {
          std::cerr << "goals_left: " << goals_left << '\n';
          std::cerr << "goal: ";
          for (int i = 0; i < goals_left; i++) {
              std::cerr << int((goal >> (i * 2)) & 3) << ' ';
          }
          std::cerr << '\n';
          for (int i = 0; i < 3; i++) {
              std::cerr << "i: " << i << " => " << int(score[i]) << ' ' << int(combo[i]) << '\n';
          }
      }
  
      void play(const int8_t* move) {
          if (end) {
              return;
          }
          
          for (int i = 0; i < 3; i++) {
              if (move[i] == (goal & 3)) {
                  combo[i]++;
                  score[i] += combo[i];
              }
              else {
                  combo[i] = 0;
              }
          }
  
          if (goals_left == 1) {
              end = true;
  
              for (int i = 0; i < 3; i++) {
                  if (score[i] >= score[(i + 1) % 3] && score[i] >= score[(i + 2) % 3]) {
                      places[i] = 3;
                  }
                  else
                  if (score[i] < score[(i + 1) % 3] && score[i] < score[(i + 2) % 3]) {
                      places[i] = 0;
                  }
                  else {
                      places[i] = 1;
                  }
              }
          }
          else {
              goal >>= 2;
              goals_left--;
          }
      }
  
      bool proven_win(int p_idx) const {
          return false;
      }
  
      bool proven_lost(int p_idx) const {
          return false;
      }
  };
  
  #endif // DIVING_HPP
  // *** End of: /home/olaf/codingame/minigames/diving.hpp *** 
 
 
 #include <iostream>
 #include <algorithm>
 #include <vector>
 #include <string>
 
 struct State {
 
     HurdleRace hurdle_race;
     uint8_t hurdle_race_score[3];
 
     Archery archery;
     uint8_t archery_score[3];
 
     RollerSkating roller_skating;
     uint8_t roller_skating_score[3];
 
     Diving diving;
     uint8_t diving_score[3];
 
     bool is_terminal() const {
         return hurdle_race.end &&
                archery.end &&
                roller_skating.end &&
                diving.end;
     }
 
     // return how much does a player earn from games
     void get_stats(float& r0, float& r1, float& r2) {
 
         for (int i = 0; i < 3; i++) {
             hurdle_race_score[i] += hurdle_race.places[i];
             archery_score[i] += archery.places[i];
             roller_skating_score[i] += roller_skating.places[i];
             diving_score[i] += diving.places[i];
         }
 
         int score0 = std::max<int>(1, hurdle_race_score[0]) * std::max<int>(1, archery_score[0]) * std::max<int>(1, roller_skating_score[0]) * std::max<int>(1, diving_score[0]);
         int score1 = std::max<int>(1, hurdle_race_score[1]) * std::max<int>(1, archery_score[1]) * std::max<int>(1, roller_skating_score[1]) * std::max<int>(1, diving_score[1]);
         int score2 = std::max<int>(1, hurdle_race_score[2]) * std::max<int>(1, archery_score[2]) * std::max<int>(1, roller_skating_score[2]) * std::max<int>(1, diving_score[2]);
 
         int sum = score0 + score1 + score2;
 
         r0 = (float) (score0 - score1 - score2) / sum;
         r1 = (float) (score1 - score0 - score2) / sum;
         r2 = (float) (score2 - score0 - score1) / sum;
     }
 
     void init(const std::vector<std::string>& gpu,
               const std::vector<std::vector<int>> reg) {
 
         if (gpu[0] == "GAME_OVER") {
             hurdle_race.end = true;
         }
         else { // hurdle_race
             hurdle_race.track = 0;
             for (int i = 0; i < (int) gpu[0].size(); i++) {
                 if (gpu[0][i] == '#') {
                     hurdle_race.track |= uint32_t(1) << i;
                 }
             }
 
             for (int i = 0; i < 3; i++) {
                 hurdle_race.pos[i] = reg[0][i];
                 hurdle_race.stun[i] = reg[0][i + 3];
                 hurdle_race.places[i] = -1;
             }
 
             hurdle_race.end = false;
         }
 
         if (gpu[1] == "GAME_OVER") {
             archery.end = true;
         }
         else { // archery
             archery.wind_index = (int) gpu[1].size() - 1;
             
             for (int i = 0; i < (int) gpu[1].size(); i++) {
                 archery.wind[archery.wind_index - i] = int(gpu[1][i] - '0');
             }
 
             for (int i = 0; i < 3; i++) {
                 archery.x[i] = reg[1][2 * i + 0];
                 archery.y[i] = reg[1][2 * i + 1];
                 archery.places[i] = -1;
             }
 
             archery.end = false;
         }
 
         if (gpu[2] == "GAME_OVER") {
             roller_skating.end = true;
         }
         else { // roller_skating
             for (int i = 0; i < 4; i++) {
                 roller_skating.order[to_move_index(gpu[3][i])] = i;
             }
 
             for (int i = 0; i < 3; i++) {
                 roller_skating.dist[i] = reg[3][i];
                 roller_skating.risk[i] = reg[3][i + 3];
             }
 
             roller_skating.turns_left = reg[3][6];
 
             roller_skating.end = false;
         }
 
         if (gpu[3] == "GAME_OVER") {
             diving.end = true;
         }
         else { // diving
             diving.goals_left = (int8_t) gpu[3].size();
 
             diving.goal = 0;
             for (int i = 0; i < (int) gpu[3].size(); i++) {
                 diving.goal |= uint32_t(to_move_index(gpu[3][i])) << (2 * i);
             }
             
             for (int i = 0; i < 3; i++) {
                 diving.score[i] = reg[3][i];
                 diving.combo[i] = reg[3][i + 3];
                 diving.places[i] = -1;
             }
 
             diving.end = false;
         }
     }
 
     void play(int8_t p0, int8_t p1, int8_t p2) {
         const int8_t move[3] = {p0, p1, p2};
 
         hurdle_race.play(move);
         archery.play(move);
         roller_skating.play(move);
         diving.play(move);
     }
 
     void debug() const {
         std::cerr << "state: " << hurdle_race.end << ' ' << archery.end << ' ' << roller_skating.end << ' ' << diving.end << '\n';
         hurdle_race.debug();
         archery.debug();
         roller_skating.debug();
         diving.debug();
     }
 
 };
 
 #endif // STATE_HPP
 // *** End of: /home/olaf/codingame/state.hpp *** 
// #include "search.hpp"
 // *** Start of: /home/olaf/codingame/mcts.hpp *** 
 #ifndef MCTS_HPP
 #define MCTS_HPP
 
  // *** Start of: /home/olaf/codingame/timer.hpp *** 
  #ifndef TIMER_HPP
  #define TIMER_HPP
  
  #include <chrono>
  
  class Timer {
  public:
  
      void start() {
          start_time = std::chrono::steady_clock::now();
      }
  
      inline auto get_elapsed() {
          using namespace std::chrono;
          milliseconds elapsed = duration_cast<milliseconds>(steady_clock::now() - start_time);
          return elapsed.count();
      }
  
  private:
      std::chrono::steady_clock::time_point start_time;
  } timer;
  
  #endif
  
  // *** End of: /home/olaf/codingame/timer.hpp *** 
 
 #include <cstdint>
 #include <cmath>
 #include <iostream>
 
 struct MCTSNode {
     static MCTSNode pool[MCTSNODE_POOL];
     static int last_node;
 
     int first_son;
 
     uint8_t last_moves;
 
     float avg[3][4];
     unsigned vis[3][4];
 
     unsigned node_vis;
 
     void init(const uint8_t& _last_moves) {
         last_moves = _last_moves;
         first_son = -1;
         node_vis = 0;
 
         for (int i = 0; i < 3; i++) {
             for (int j = 0; j < 4; j++) {
                 avg[i][j] = 0;
                 vis[i][j] = 0;
             }
         }
     }
 
     void expand() {
         first_son = last_node;
         for (uint8_t moves = 0; moves < 64; moves++) {
             pool[last_node++].init(moves);
         }
     }
 
     int best_move_per_player(int player_idx) const {
         float best_score = -INF;
         int best_move = -1;
 
         for (int move = 0; move < 4; move++) {
             float node_score = vis[player_idx][move]; // avg[player_idx][move];
 
             if (best_score < node_score) {
                 best_score = node_score;
                 best_move = move;
             }
         }
 
         return best_move;
     }
 
     int select_per_player(int player_idx) const {
         float best_score = -INF;
         int best_move = -1;
         
         float log_node_vis = std::log(node_vis);
         for (int move = 0; move < 4; move++) {
             if (vis[player_idx][move] == 0) {
                 return move;
             }
 
             float node_score = avg[player_idx][move] + C * std::sqrt(log_node_vis / vis[player_idx][move]);
         
             if (best_score < node_score) {
                 best_score = node_score;
                 best_move = move;
             }
         }
 
         return best_move;
     }
 
     MCTSNode* select() const {
         return &pool[first_son + select_per_player(0) * 1
                                + select_per_player(1) * 4
                                + select_per_player(2) * 16];
     }
 
     void apply(uint8_t moves, float r0, float r1, float r2) {
         int m0 = (moves >> 0) & 3;
         int m1 = (moves >> 2) & 3;
         int m2 = (moves >> 4) & 3;
 
         avg[0][m0] *= vis[0][m0];
         avg[0][m0] += r0;
         vis[0][m0] += 1;
         avg[0][m0] /= vis[0][m0];
 
         avg[1][m1] *= vis[1][m1];
         avg[1][m1] += r1;
         vis[1][m1] += 1;
         avg[1][m1] /= vis[1][m1];
 
         avg[2][m2] *= vis[2][m2];
         avg[2][m2] += r2;
         vis[2][m2] += 1;
         avg[2][m2] /= vis[2][m2];
 
         node_vis += 1;
     }
 
     void debug() const {
         std::cerr << "NODE VIS: " << node_vis << '\n';
         for (int i = 0; i < 3; i++) {
             std::cerr << "PLAYER: " << i << '\n';
             for (int move = 0; move < 4; move++) {
                 std::cerr << avg[i][move] << '/' << vis[i][move] << ' ';
             }
             std::cerr << '\n';
         }
     }
 };
 
 MCTSNode MCTSNode::pool[MCTSNODE_POOL];
 int      MCTSNode::last_node = 0;
 
 struct MCTS {
     MCTSNode* root;
 
     void rollout(State& state, float& r0, float& r1, float& r2) {
         do {
             state.play(fast_rand() & 3, fast_rand() & 3, fast_rand() & 3);
         } while (!state.is_terminal());
 
         state.get_stats(r0, r1, r2);
     }
 
     void mcts(MCTSNode* node, State& state, float& r0, float& r1, float& r2) {
         if (state.is_terminal()) {
             state.get_stats(r0, r1, r2);
             return;
         }
 
         if (node->node_vis == 0) {
             rollout(state, r0, r1, r2);
             node->node_vis++;
             return;
         }
 
         if (node->first_son == -1) {
             node->expand();
         }
 
         MCTSNode* child = node->select();
 
         state.play((child->last_moves >> 0) & 3, 
                    (child->last_moves >> 2) & 3,
                    (child->last_moves >> 4) & 3);
 
         mcts(child, state, r0, r1, r2);
 
         node->apply(child->last_moves, r0, r1, r2);
     }
 
     void reset() {
         MCTSNode::last_node = 0;
         MCTSNode::pool[MCTSNode::last_node].init(0);
         root = &MCTSNode::pool[MCTSNode::last_node];
         MCTSNode::last_node++;
     }
 
     int best_move(int player_idx) const {
         return root->best_move_per_player(player_idx);
     }
 
     void debug() {
         root->debug();
     }
 
     void run(const State& root_state, int timeout) {
         reset();
 
         // root_state.debug();
         // return;
 
         do {
             State state = root_state;
             float r0, r1, r2;
             mcts(root, state, r0, r1, r2);
         } while (timer.get_elapsed() < timeout &&
                  MCTSNode::last_node + 64 < MCTSNODE_POOL);
     }
 };
 
 #endif // MCTS_HPP
 // *** End of: /home/olaf/codingame/mcts.hpp *** 

#include <iostream>
#include <string>
#include <vector>

int TURN;
int PLAYER_IDX;
int NB_GAMES;

int main() {

    std::cerr << sizeof(State) << '\n';

    std::cin >> PLAYER_IDX;
    std::cin.ignore();
    std::cin >> NB_GAMES;
    std::cin.ignore();

    timer.start();

    MCTS mcts;

    for (TURN = 0; ; TURN++) {
        State current_state;
        
        for (int i = 0; i < 3; i++) {
            int final_score;
            std::cin >> final_score;

            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.hurdle_race_score[i] = 3 * gold + silver;
            }
            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.archery_score[i] = 3 * gold + silver;
            }
            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.roller_skating_score[i] = 3 * gold + silver;
            }
            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.diving_score[i] = 3 * gold + silver;
            }
        }

        if (TURN != 0) {
            timer.start();
        }

        {
            std::vector<std::string> gpu(NB_GAMES);
            std::vector<std::vector<int>> reg(NB_GAMES);

            for (int i = 0; i < NB_GAMES; i++) {
                std::cin >> gpu[i];

                reg[i].resize(7);
                for (int j = 0; j < 7; j++) {
                    std::cin >> reg[i][j];
                }
                std::cin.ignore();
            }

            current_state.init(gpu, reg);
        }

        std::cerr << "MCTS START\n";

        mcts.run(current_state, (TURN == 0 ? 950 : 45));

        mcts.debug();
        std::cerr << "timer: " << timer.get_elapsed() << '\n';
        std::cerr << "pool: " << (float) MCTSNode::last_node / MCTSNODE_POOL << '\n';
        std::cerr << "last: " << MCTSNode::last_node << '\n';

        int move = mcts.best_move(PLAYER_IDX);
        
        std::vector<std::string> move_list = {
            "UP", "LEFT", "DOWN", "RIGHT"
        };

        std::cout << move_list[move] << std::endl;

        // return 0;
    }

}
// *** End of: /home/olaf/codingame/main.cpp *** 
