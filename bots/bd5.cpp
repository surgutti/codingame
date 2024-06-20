// *** Start of: /home/olaf/codingame/brain.cpp *** 
#ifndef LOCAL
    #undef _GLIBCXX_DEBUG
    #pragma GCC optimize("Ofast,inline")
    #pragma GCC target("bmi,bmi2,lzcnt,popcnt")
    #pragma GCC target("movbe")
    #pragma GCC target("aes,pclmul,rdrnd")
    #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2")
#endif // LOCAL

 // *** Start of: /home/olaf/codingame/const.hpp *** 
 #ifndef CONST_HPP
 #define CONST_HPP
 
 const int INF = 1'000'000;
 
 const int TRACK_LENGTH = 30;
 const int ARCHERY_LENGTH = 12 + 3; // 12 + random.nextInt(4);
 const int DIVING_LENGTH = 12 + 3; // 12 + random.nextInt(4);
 
 const int MCTSNODE_POOL = 6'000'000;
 const int BRAIN_POOL = 6'000'000;
 
 // TODO: run psyleague with different C values
 const float C = 0.4f;
 
 int PLAYER_IDX;
 
 #endif // CONST_HPP
 // *** End of: /home/olaf/codingame/const.hpp *** 
 // *** Start of: /home/olaf/codingame/state.hpp *** 
 #ifndef STATE_HPP
 #define STATE_HPP
 
  // *** Start of: /home/olaf/codingame/minigames/hurdle_race.hpp *** 
  #ifndef HURDLE_RACE
  #define HURDLE_RACE
  
   // *** Start of: /home/olaf/codingame/random.hpp *** 
   #ifndef RANDOM_HPP
   #define RANDOM_HPP
   
    // *** Start of: /home/olaf/codingame/utils.hpp *** 
    #ifndef UTILS_HPP
    #define UTILS_HPP
    
    #include <cstdint>
    #include <cassert>
    #include <x86intrin.h>
    
    inline int to_move_index(char c) {
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
    
    inline float fastlogf(const float& x) { union { float f; uint32_t i; } vx = { x }; float y = vx.i; y *= 8.2629582881927490e-8f; return(y - 87.989971088f); }
    inline float fastsqrtf(const float& x) { union { int i; float x; } u; u.x = x; u.i = (1 << 29) + (u.i >> 1) - (1 << 22); return(u.x); }
    inline float rsqrt_fast(float x) { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x))); }
    
    #endif // UTILS_HPP
    // *** End of: /home/olaf/codingame/utils.hpp *** 
   
   static unsigned int g_seed = 2137420;
   
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
  #include <cassert>
  #include <cmath>
  
  const int tracks_count = 640;
  const uint32_t all_tracks[tracks_count] = {8947848,17336456,559240,559240,17860744,34637960,1083528,1083528,34952,34952,34952,34952,17893512,34670728,1116296,1116296,35719304,69273736,2164872,2164872,67720,67720,67720,67720,2184,2184,2184,2184,2184,2184,2184,2184,17895560,34672776,1118344,1118344,35721352,69275784,2166920,2166920,69768,69768,69768,69768,35786888,69341320,2232456,2232456,71438472,138547336,4329608,4329608,135304,135304,135304,135304,4232,4232,4232,4232,4232,4232,4232,4232,17895688,34672904,1118472,1118472,35721480,69275912,2167048,2167048,69896,69896,69896,69896,35787016,69341448,2232584,2232584,71438600,138547464,4329736,4329736,135432,135432,135432,135432,4360,4360,4360,4360,4360,4360,4360,4360,35791112,69345544,2236680,2236680,71442696,138551560,4333832,4333832,139528,139528,139528,139528,71573768,138682632,4464904,4464904,142876936,277094664,8659208,8659208,270600,270600,270600,270600,8456,8456,8456,8456,8456,8456,8456,8456,17895696,34672912,1118480,1118480,35721488,69275920,2167056,2167056,69904,69904,69904,69904,35787024,69341456,2232592,2232592,71438608,138547472,4329744,4329744,135440,135440,135440,135440,4368,4368,4368,4368,4368,4368,4368,4368,35791120,69345552,2236688,2236688,71442704,138551568,4333840,4333840,139536,139536,139536,139536,71573776,138682640,4464912,4464912,142876944,277094672,8659216,8659216,270608,270608,270608,270608,8464,8464,8464,8464,8464,8464,8464,8464,35791376,69345808,2236944,2236944,71442960,138551824,4334096,4334096,139792,139792,139792,139792,71574032,138682896,4465168,4465168,142877200,277094928,8659472,8659472,270864,270864,270864,270864,8720,8720,8720,8720,8720,8720,8720,8720,71582224,138691088,4473360,4473360,142885392,277103120,8667664,8667664,279056,279056,279056,279056,143147536,277365264,8929808,8929808,285753872,17318416,17318416,17318416,541200,541200,541200,541200,16912,16912,16912,16912,16912,16912,16912,16912,35791392,69345824,2236960,2236960,71442976,138551840,4334112,4334112,139808,139808,139808,139808,71574048,138682912,4465184,4465184,142877216,277094944,8659488,8659488,270880,270880,270880,270880,8736,8736,8736,8736,8736,8736,8736,8736,71582240,138691104,4473376,4473376,142885408,277103136,8667680,8667680,279072,279072,279072,279072,143147552,277365280,8929824,8929824,285753888,17318432,17318432,17318432,541216,541216,541216,541216,16928,16928,16928,16928,16928,16928,16928,16928,71582752,138691616,4473888,4473888,142885920,277103648,8668192,8668192,279584,279584,279584,279584,143148064,277365792,8930336,8930336,285754400,17318944,17318944,17318944,541728,541728,541728,541728,17440,17440,17440,17440,17440,17440,17440,17440,143164448,277382176,8946720,8946720,285770784,17335328,17335328,17335328,558112,558112,558112,558112,286295072,17859616,17859616,17859616,34636832,34636832,34636832,34636832,1082400,1082400,1082400,1082400,33824,33824,33824,33824,33824,33824,33824,33824,71582784,138691648,4473920,4473920,142885952,277103680,8668224,8668224,279616,279616,279616,279616,143148096,277365824,8930368,8930368,285754432,17318976,17318976,17318976,541760,541760,541760,541760,17472,17472,17472,17472,17472,17472,17472,17472,143164480,277382208,8946752,8946752,285770816,17335360,17335360,17335360,558144,558144,558144,558144,286295104,17859648,17859648,17859648,34636864,34636864,34636864,34636864,1082432,1082432,1082432,1082432,33856,33856,33856,33856,33856,33856,33856,33856,143165504,277383232,8947776,8947776,285771840,17336384,17336384,17336384,559168,559168,559168,559168,286296128,17860672,17860672,17860672,34637888,34637888,34637888,34637888,1083456,1083456,1083456,1083456,34880,34880,34880,34880,34880,34880,34880,34880,286328896,17893440,17893440,17893440,34670656,34670656,34670656,34670656,1116224,1116224,1116224,1116224,35719232,35719232,35719232,35719232,69273664,69273664,69273664,69273664,2164800,2164800,2164800,2164800,67648,67648,67648,67648,67648,67648,67648,67648,143165568,277383296,8947840,8947840,285771904,17336448,17336448,17336448,559232,559232,559232,559232,286296192,17860736,17860736,17860736,34637952,34637952,34637952,34637952,1083520,1083520,1083520,1083520,34944,34944,34944,34944,34944,34944,34944,34944,286328960,17893504,17893504,17893504,34670720,34670720,34670720,34670720,1116288,1116288,1116288,1116288,35719296,35719296,35719296,35719296,69273728,69273728,69273728,69273728,2164864,2164864,2164864,2164864,67712,67712,67712,67712,67712,67712,67712,67712,286331008,17895552,17895552,17895552,34672768,34672768,34672768,34672768,1118336,1118336,1118336,1118336,35721344,35721344,35721344,35721344,69275776,69275776,69275776,69275776,2166912,2166912,2166912,2166912,69760,69760,69760,69760,69760,69760,69760,69760,35786880,35786880,35786880,35786880,69341312,69341312,69341312,69341312,2232448,2232448,2232448,2232448,71438464,71438464,71438464,71438464,138547328,138547328,138547328,138547328,4329600,4329600,4329600,4329600,135296,135296,135296,135296,135296,135296,135296,135296};
  
  struct HurdleRace {
      
      static int dp[TRACK_LENGTH + 2]; // shortest time to finish
      static int pd[TRACK_LENGTH + 2]; // longest time to finish
      static uint8_t dp_opt[TRACK_LENGTH];
  
      uint32_t track;
  
      int8_t pos[3], stun[3];
      bool end;
  
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
  
      inline int expected_end() const {
          if (pos[0] > pos[1] && pos[0] > pos[2])
              return TRACK_LENGTH - 1 - pos[0];
          else
          if (pos[1] > pos[1] && pos[1] > pos[2])
              return TRACK_LENGTH - 1 - pos[1];
          return TRACK_LENGTH - 1 - pos[2];
      }
  
      inline void generate_places(int8_t* places) const {
          if (pos[0] >= TRACK_LENGTH - 1) {
              places[0] = 3;
          }
          else
          if (pos[0] < pos[1] && pos[0] < pos[2]) {
              places[0] = 0;
          }
          else {
              places[0] = 1;
          }
  
          if (pos[1] >= TRACK_LENGTH - 1) {
              places[1] = 3;
          }
          else
          if (pos[1] < pos[0] && pos[1] < pos[2]) {
              places[1] = 0;
          }
          else {
              places[1] = 1;
          }
  
          if (pos[2] >= TRACK_LENGTH - 1) {
              places[2] = 3;
          }
          else
          if (pos[2] < pos[0] && pos[2] < pos[1]) {
              places[2] = 0;
          }
          else {
              places[2] = 1;
          }
      }
  
      void randomize() {
  
          track = all_tracks[fast_rand() % tracks_count];
  
          for (int i = 0; i < 3; i++) {
              pos[i] = 0;
              stun[i] = 0;
          }
  
          end = false;
      }
  
      inline void play(const int8_t* move) {
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
  
                  if (track & (1U << pos[i])) {
                      stun[i] = 2;
                  }
                  else
                  if (pos[i] >= TRACK_LENGTH - 1) {
                      end = true;
                  }
              }
          }
      }
  
      bool in_waiting(const int8_t player_idx) const {
          return stun[player_idx] > 0;
      }
  
      void build_dp() {
          dp[TRACK_LENGTH - 1] = 0;
          dp[TRACK_LENGTH + 0] = 0;
          dp[TRACK_LENGTH + 1] = 0;
  
          dp_opt[TRACK_LENGTH - 1] = 0b1111;
          
          pd[TRACK_LENGTH - 1] = 0;
          pd[TRACK_LENGTH + 0] = 0;
          pd[TRACK_LENGTH + 1] = 0;
  
          for (int i = TRACK_LENGTH - 2; i >= 0; i--) {
  
              dp[i] = 1000;
              pd[i] = 0;
              dp_opt[i] = 0;
              for (int8_t move = 0; move < 4; move++) {
                  int now = 0;
  
                  int p = i;
  
                  if (move == 1) {
                      p++;
                      now += 1;
  
                      if (track & (1U << p)) {
                          now += 2;
                      }
                  }
                  else
                  if (move == 0) {
                      p += 2;
                      now += 1;
                      
                      if (track & (1U << p)) {
                          now += 2;
                      }
                  }
                  else
                  if (move == 2) {
                      p++;
                      if (track & (1U << p)) {
                          now += 1 + 2;
                      }
                      else {
                          p++;
                          now += 1;
  
                          if (track & (1U << p)) {
                              now += 2;
                          }
                      }
                  }
                  else {
                      p++;
                      if (track & (1U << p)) {
                          now += 1 + 2;
                      }
                      else {
                          p++;
                          if (track & (1U << p)) {
                              now += 1 + 2;
                          }
                          else {
                              p++;
                              now += 1;
  
                              if (track & (1U << p)) {
                                  now += 2;
                              }
                          }
                      }
                  }
  
                  if (dp[i] > now + dp[p]) {
                      dp[i] = now + dp[p];
                      dp_opt[i] = 1 << move;
                  }
                  else
                  if (dp[i] == now + dp[p]) {
                      dp_opt[i] |= 1 << move;
                  }
  
                  if (pd[i] < now + pd[p]) {
                      pd[i] = now + pd[p]; // ! need to use the pd value, not the dp in now!!!
                  }
              }
          }
  
          // std::cerr << "dp: ";
          // for (int i = 0; i < TRACK_LENGTH; i++) {
          //     std::cerr << int(dp[i]) << ' ';
          // }
          // std::cerr << '\n';
  
          // std::cerr << "dp_opt: ";
          // for (int i = 0; i < TRACK_LENGTH; i++) {
          //     std::cerr << int(dp_opt[i]) << ' ';
          // }
          // std::cerr << '\n';
  
          // std::cerr << "pd: ";
          // for (int i = 0; i < TRACK_LENGTH; i++) {
          //     std::cerr << int(pd[i]) << ' ';
          // }
          // std::cerr << '\n';
      }
  
      inline bool playable(const int8_t player_idx) const {
          // if (stun[player_idx]) {
          //     return false;
          // }
  
          const int8_t enemy1_idx = (player_idx + 1) % 3;
          const int8_t enemy2_idx = (player_idx + 2) % 3;
  
          if (pd[pos[player_idx]] + stun[player_idx] <=
              dp[pos[enemy1_idx]] + stun[enemy1_idx] &&
              
              pd[pos[player_idx]] + stun[player_idx] <=
              dp[pos[enemy2_idx]] + stun[enemy2_idx]) {
              return false; // inevitable 1st place
          }
  
          if (dp[pos[player_idx]] + stun[player_idx] >
              pd[pos[enemy1_idx]] + stun[enemy1_idx] &&
  
              dp[pos[player_idx]] + stun[player_idx] >
              pd[pos[enemy2_idx]] + stun[enemy2_idx]) {
              return false; // inevitable 3rd place
          }
  
          // ranking: <enemy1> <player> <enemy2>
          if (dp[pos[player_idx]] + stun[player_idx] >
              pd[pos[enemy1_idx]] + stun[enemy1_idx] &&
              
              pd[pos[player_idx]] + stun[player_idx] <=
              dp[pos[enemy2_idx]] + stun[enemy2_idx]) {
              return false; // inevitable 2nd place
          }
  
          // ranking: <enemy2> <player> <enemy1>
          if (dp[pos[player_idx]] + stun[player_idx] >
              pd[pos[enemy2_idx]] + stun[enemy2_idx] &&
              
              pd[pos[player_idx]] + stun[player_idx] <=
              dp[pos[enemy1_idx]] + stun[enemy1_idx]) {
              return false; // inevitable 2nd place
          }
  
          return true;
      }
  
      inline uint8_t greedy_moves(const int8_t player_idx) const {
          if (end || stun[player_idx])
              return 0;
  
          return dp_opt[pos[player_idx]];
      }
  };
  
  int HurdleRace::dp[TRACK_LENGTH + 2];
  int HurdleRace::pd[TRACK_LENGTH + 2];
  uint8_t HurdleRace::dp_opt[TRACK_LENGTH];
  
  #endif // HURDLE_RACE
  // *** End of: /home/olaf/codingame/minigames/hurdle_race.hpp *** 
  // *** Start of: /home/olaf/codingame/minigames/archery.hpp *** 
  #ifndef ARCHERY_HPP
  #define ARCHERY_HPP
  
  
  #include <vector>
  
  struct Archery {
  
      static int dp[ARCHERY_LENGTH][41][41];
      static int pd[ARCHERY_LENGTH][41][41];
      static uint8_t dp_opt[ARCHERY_LENGTH][41][41];
  
      int8_t wind[ARCHERY_LENGTH];
      int8_t wind_index;
  
      int8_t x[3];
      int8_t y[3];
  
      bool end;
  
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
  
      inline int expected_end() const {
          return wind_index + 1;
      }
  
      void randomize() {
          int8_t sx = 5 + fast_rand() % 5;
          int8_t sy = 5 + fast_rand() % 5;
  
          if (fast_rand() & 1)
              sx = -sx;
          
          if (fast_rand() & 1)
              sy = -sy;
          
          wind_index = 12 + fast_rand() % 4 - 1;
  
          for (int i = 0; i < 3; i++) {
              x[i] = sx;
              y[i] = sy;
          }
  
          constexpr float weights[10] = { 0, 2, 2, 2, 0.5, 0.5, 0.25, 0.25, 0.25, 0.2 };
          constexpr float sum = weights[0] + weights[1] + weights[2] + 
                                weights[3] + weights[4] + weights[5] + 
                                weights[6] + weights[7] + weights[8] + 
                                weights[9];
  
          for (int i = 0; i <= wind_index; i++) {
              wind[i] = 9;
  
              float w = (fast_rand() % 1000) * sum;
  
              for (int j = 1; j < 9; j++) {
                  w -= weights[j] * 1000;
  
                  if (w < 0) {
                      wind[i] = j;
                      break;
                  }
              }
          }
  
          end = false;
      }
  
      inline void generate_places(int8_t* places) const {
          int16_t score[3];
          for (int i = 0; i < 3; i++) {
              score[i] = (int16_t) x[i] * x[i] + (int16_t) y[i] * y[i]; 
          }
  
          if (score[0] <= score[1] && score[0] <= score[2]) {
              places[0] = 3;
          }
          else
          if (score[0] > score[1] && score[0] > score[2]) {
              places[0] = 0;
          }
          else {
              places[0] = 1;
          }
  
          if (score[1] <= score[0] && score[1] <= score[2]) {
              places[1] = 3;
          }
          else
          if (score[1] > score[0] && score[1] > score[2]) {
              places[1] = 0;
          }
          else {
              places[1] = 1;
          }
  
          if (score[2] <= score[0] && score[2] <= score[1]) {
              places[2] = 3;
          }
          else
          if (score[2] > score[0] && score[2] > score[1]) {
              places[2] = 0;
          }
          else {
              places[2] = 1;
          }
      }
  
      inline void play(const int8_t* move) {
          if (end) {
              return;
          }
  
          static constexpr int8_t dx[4] = {0, -1, 0, +1};
          static constexpr int8_t dy[4] = {-1, 0, +1, 0};
  
          const int8_t wind_strength = wind[wind_index];
  
          for (int i = 0; i < 3; i++) {
              x[i] += wind_strength * dx[move[i]];
              y[i] += wind_strength * dy[move[i]];
  
              if (x[i] > +20) x[i] = +20;
              else
              if (x[i] < -20) x[i] = -20;
          
              if (y[i] > +20) y[i] = +20;
              else
              if (y[i] < -20) y[i] = -20;
          }
  
          if (wind_index == 0) {
              end = true;
          }
          else {
              wind_index--;
          }
      }
  
      void build_dp() {
          for (int xxx = -20; xxx <= +20; xxx++) {
              for (int yyy = -20; yyy <= +20; yyy++) {
                  static constexpr int8_t dx[4] = {0, -1, 0, +1};
                  static constexpr int8_t dy[4] = {-1, 0, +1, 0};
  
                  int wind_strength = wind[0];
  
                  int& value = dp[0][20 + xxx][20 + yyy];
                  int& worst = pd[0][20 + xxx][20 + yyy];
                  uint8_t& opt = dp_opt[0][20 + xxx][20 + yyy];
  
                  opt = 0;
                  value = 10000;
                  worst = 0;
                  for (int8_t move = 0; move < 4; move++) {
                      int xx = xxx + wind_strength * dx[move];
                      int yy = yyy + wind_strength * dy[move];
  
                      if (xx < -20)   xx = -20;
                      if (xx > +20)   xx = +20;
                      if (yy < -20)   yy = -20;
                      if (yy > +20)   yy = +20;
  
                      int now = xx * xx + yy * yy;
  
                      if (value > now) {
                          value = now;
                          opt = 1 << move;
                      }
                      else
                      if (value == now) {
                          opt |= 1 << move;
                      }
  
                      if (worst < now) {
                          worst = now;
                      }
                  }
  
              }
          }
  
          for (int i = 1; i <= wind_index; i++) {
              for (int xxx = -20; xxx <= +20; xxx++) {
                  for (int yyy = -20; yyy <= +20; yyy++) {
  
                      static constexpr int8_t dx[4] = {0, -1, 0, +1};
                      static constexpr int8_t dy[4] = {-1, 0, +1, 0};
  
                      int wind_strength = wind[i];
  
                      int& value = dp[i][20 + xxx][20 + yyy];
                      int& worst = pd[i][20 + xxx][20 + yyy];
  
                      uint8_t& opt = dp_opt[i][20 + xxx][20 + yyy];
                      
                      opt = 0;
                      value = 10000;
                      worst = 0;
                      for (int8_t move = 0; move < 4; move++) {
                          int xx = xxx + wind_strength * dx[move];
                          int yy = yyy + wind_strength * dy[move];
  
                          if (xx < -20)   xx = -20;
                          if (xx > +20)   xx = +20;
                          if (yy < -20)   yy = -20;
                          if (yy > +20)   yy = +20;
  
                          int now = dp[i - 1][xx + 20][yy + 20];
  
                          if (value > now) {
                              value = now;
                              opt = 1 << move;
                          }
                          else
                          if (value == now) {
                              opt |= 1 << move;
                          }
  
                          if (worst < pd[i - 1][xx + 20][yy + 20]) {
                              worst = pd[i - 1][xx + 20][yy + 20];
                          }
                      }
                  }
              }
          }
      }
  
      bool in_waiting(const int8_t player_idx) const {
          return false;
      }
  
      inline bool playable(const int8_t player_idx) const {
          // too little enchancement to hope for early win
  
          const int8_t enemy1_idx = (player_idx + 1) % 3;
          const int8_t enemy2_idx = (player_idx + 2) % 3;
  
          if (pd[wind_index][x[player_idx] + 20][y[player_idx] + 20] <=
              dp[wind_index][x[enemy1_idx] + 20][y[enemy1_idx] + 20] &&
  
              pd[wind_index][x[player_idx] + 20][y[player_idx] + 20] <=
              dp[wind_index][x[enemy2_idx] + 20][y[enemy2_idx] + 20]) {
              return false; // inevitable 1st place
          }
  
          if (dp[wind_index][x[player_idx] + 20][y[player_idx] + 20] >
              pd[wind_index][x[enemy1_idx] + 20][y[enemy1_idx] + 20] &&
              
              dp[wind_index][x[player_idx] + 20][y[player_idx] + 20] >
              pd[wind_index][x[enemy2_idx] + 20][y[enemy2_idx] + 20]) {
              return false; // inevitable 3rd place
          }
  
          if (pd[wind_index][x[player_idx] + 20][y[player_idx] + 20] <=
              dp[wind_index][x[enemy1_idx] + 20][y[enemy1_idx] + 20] &&
  
              dp[wind_index][x[player_idx] + 20][y[player_idx] + 20] >
              pd[wind_index][x[enemy2_idx] + 20][y[enemy2_idx] + 20]) {
              return false; // inevitable 2nd place
          }
  
          if (pd[wind_index][x[player_idx] + 20][y[player_idx] + 20] <=
              dp[wind_index][x[enemy2_idx] + 20][y[enemy2_idx] + 20] &&
              
              dp[wind_index][x[player_idx] + 20][y[player_idx] + 20] >
              pd[wind_index][x[enemy1_idx] + 20][y[enemy1_idx] + 20]) {
              return false; // inevitable 2nd place
          }
  
          return true;
      }
  
      inline uint8_t greedy_moves(const int8_t player_idx) const {
          if (end)
              return 0;
          
          return dp_opt[wind_index][x[player_idx] + 20][y[player_idx] + 20];
      }
  };
  
  int Archery::dp[ARCHERY_LENGTH][41][41];
  int Archery::pd[ARCHERY_LENGTH][41][41];
  uint8_t Archery::dp_opt[ARCHERY_LENGTH][41][41];
  
  #endif // ARCHERY_HPP
  // *** End of: /home/olaf/codingame/minigames/archery.hpp *** 
  // *** Start of: /home/olaf/codingame/minigames/roller_skating.hpp *** 
  #ifndef ROLLER_SKATING
  #define ROLLER_SKATING
  
  
  #include <iostream>
  #include <algorithm>
  
  constexpr uint8_t all_permutations[24] = {
      27,
      30,
      39,
      45,
      54,
      57,
      75,
      78,
      99,
      108,
      114,
      120,
      135,
      141,
      147,
      156,
      177,
      180,
      198,
      201,
      210,
      216,
      225,
      228,
  };
  
  // const uint8_t second_in_permutation[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,3,0,0,0,0,0,3,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
  // const uint8_t last_in_permutation[256] =   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,3,0,0,0,0,0,0,0,0,3,0,0,0,0,0,3,0,0,0,0,0,0,0,0,3,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
  
  // void index_in_permtation_init() {
  //     for (int i = 0; i < 24; i++) {
  //         for (int j = 0; j < 4; j++) {
  //             // if (((all_permutations[i] >> (2 * j)) & 3) == 1) {
  //             //     second_in_permutation[all_permutations[i]] = j;
  //             // }
  
  //             // if (((all_permutations[i] >> (2 * j)) & 3) == 3) {
  //             //     last_in_permutation[all_permutations[i]] = j;
  //             // }
  //         }
  //     }
  
  //     for (int i = 0; i < 256; i++) {
  //         std::cerr << int(last_in_permutation[i]) << ',';
  //     }
  //     std::cerr << '\n';
  // }
  
  // constexpr int8_t all_permutations[24][4] = {
  //     {0, 1, 2, 3},
  //     {0, 1, 3, 2},
  //     {0, 2, 1, 3},
  //     {0, 2, 3, 1},
  //     {0, 3, 1, 2},
  //     {0, 3, 2, 1},
  //     {1, 0, 2, 3},
  //     {1, 0, 3, 2},
  //     {1, 2, 0, 3},
  //     {1, 2, 3, 0},
  //     {1, 3, 0, 2},
  //     {1, 3, 2, 0},
  //     {2, 0, 1, 3},
  //     {2, 0, 3, 1},
  //     {2, 1, 0, 3},
  //     {2, 1, 3, 0},
  //     {2, 3, 0, 1},
  //     {2, 3, 1, 0},
  //     {3, 0, 1, 2},
  //     {3, 0, 2, 1},
  //     {3, 1, 0, 2},
  //     {3, 1, 2, 0},
  //     {3, 2, 0, 1},
  //     {3, 2, 1, 0}
  // };
  
  // (order >> move[i]) & 3 == 0
  
  struct RollerSkating {
  
      int8_t turns_left;
      // int8_t turns_done;
  
      int8_t dist_div10[3];
      int8_t dist_mod10[3];
      int8_t risk[3];
  
      uint8_t order;
  
      bool end;
  
      void debug() const {
          std::cerr << "TURNS LEFT: " << int(turns_left) << '\n';
          for (int i = 0; i < 3; i++) {
              std::cerr << "i: " << i << ' ' << int(dist_div10[i] * 10 + dist_mod10[i]) << ' ' << int(risk[i]) << '\n';
          }
          std::cerr << "order: ";
          for (int i = 0; i < 4; i++) {
              std::cerr << int((order >> (i << 1)) & 3) << ' ';
          }
          std::cerr << '\n';
      }
  
      inline int expected_end() const {
          return turns_left;
      }
  
      inline void generate_places(int8_t* places) {
          const int8_t d0 = dist_div10[0] * 10 + dist_mod10[0];
          const int8_t d1 = dist_div10[1] * 10 + dist_mod10[1];
          const int8_t d2 = dist_div10[2] * 10 + dist_mod10[2];
  
          if (d0 >= d1 && d0 >= d2) {
              places[0] = 3;
          }
          if (d0 < d1 && d0 < d2) {
              places[0] = 0;
          }
          else {
              places[0] = 1;
          }
  
          if (d1 >= d0 && d1 >= d2) {
              places[1] = 3;
          }
          else
          if (d1 < d0 && d1 < d2) {
              places[1] = 0;
          }
          else {
              places[1] = 1;
          }
  
          if (d2 >= d0 && d2 >= d1) {
              places[2] = 3;
          }
          else
          if (d2 < d0 && d2 < d1) {
              places[2] = 0;
          }
          else {
              places[2] = 1;
          }
      }
  
      void randomize() {
          turns_left = 15;
  
          order = all_permutations[fast_rand() % 24];
  
          for (int i = 0; i < 3; i++) {
              dist_div10[i] = 0;
              dist_mod10[i] = 0;
              risk[i] = 0;
          }
  
          end = false;
      }
  
      // TODO:
      // after few turns (5?) just take the places as dist shows
      // gather some statistics on starting positions
      // the most important fact is that players will jump on max and get higest risk
      inline void play(const int8_t* move) {
          if (end) {
              return;
          }
  
          for (int i = 0; i < 3; i++) {
              if (risk[i] < 0) {
                  risk[i]++;
              }
              else {
                  const int8_t index = (order >> (move[i] << 1)) & 3;
                  
                  dist_mod10[i] += 2;
  
                  if (index == 0) {
                      dist_mod10[i]--;
                      risk[i]--;
                  }
                  else
                  if (index == 2) {
                      risk[i]++;
                  }
                  else
                  if (index == 3) {
                      dist_mod10[i]++;
                      risk[i] += 2;
                  }
  
                  if (dist_mod10[i] >= 10) {
                      dist_mod10[i] -= 10;
                      dist_div10[i]++;
                  }
              }
          }
  
          if (risk[0] >= 0 && (dist_mod10[0] == dist_mod10[1] || dist_mod10[0] == dist_mod10[2])) {
              risk[0] += 2;
          }
  
          if (risk[1] >= 0 && (dist_mod10[1] == dist_mod10[0] || dist_mod10[1] == dist_mod10[2])) {
              risk[1] += 2;
          }
  
          if (risk[2] >= 0 && (dist_mod10[2] == dist_mod10[0] || dist_mod10[2] == dist_mod10[1])) {
              risk[2] += 2;
          }
  
          for (int i = 0; i < 3; i++) {
              if (risk[i] >= 5) {
                  risk[i] = -2;
              }
          }
  
          if (turns_left == 0 /*|| turns_done == 10*/) {
              end = true;
          }
          else {
              // turns_done++;
              turns_left--;
  
              order = all_permutations[fast_rand() % 24];
          }
      }
  
      bool in_waiting(const int8_t player_idx) const {
          return risk[player_idx] < 0;
      }
  
      inline bool playable(const int8_t player_idx) const {
          return true; // return risk[player_idx] >= 0;
      }
  
      // maybe if for not being stun'ed
      // inline uint8_t greedy_moves(const int8_t player_idx) const {
      //     if (end || risk[player_idx] < 0)
      //         return 0;
  
      //     // std::cerr << "order: " << int(order) << ' ' << int(risk[player_idx]) << '\n';
      //     if (risk[player_idx] + 2 < 5) {
      //         // std::cerr << "take risk\n";
      //         return uint8_t(1) << last_in_permutation[order];
      //         // return uint8_t(1) << ((order >> (2 * 3)) & 3); // if have risk + 2 < 5 then rush 3
      //     }
      //     return uint8_t(1) << second_in_permutation[order];
      //     // return uint8_t(1) << ((order >> (1 * 2)) & 3); // else go 2
      // }
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
  
      inline int expected_end() const {
          return goals_left;
      }
  
      inline void generate_places(int8_t* places) const {
          if (score[0] >= score[1] && score[0] >= score[2]) {
              places[0] = 3;
          }
          else
          if (score[0] < score[1] && score[0] < score[2]) {
              places[0] = 0;
          }
          else {
              places[0] = 1;
          }
  
          if (score[1] >= score[0] && score[1] >= score[2]) {
              places[1] = 3;
          }
          else
          if (score[1] < score[0] && score[1] < score[2]) {
              places[1] = 0;
          }
          else {
              places[1] = 1;
          }
  
          if (score[2] >= score[0] && score[2] >= score[1]) {
              places[2] = 3;
          }
          else
          if (score[2] < score[0] && score[2] < score[1]) {
              places[2] = 0;
          }
          else {
              places[2] = 1;
          }
      }
  
      void randomize() {
          goals_left = 12 + fast_rand() % 4;
          goal = fast_rand(); // dont care about the rest? -> just slowing down
  
          for (int i = 0; i < 3; i++) {
              score[i] = 0;
              combo[i] = 0;
          }
  
          end = false;
      }
  
      inline void play(const int8_t* move) {
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
          }
          else {
              goal >>= 2;
              goals_left--;
          }
      }
  
      bool in_waiting(const int8_t player_idx) const {
          return false;
      }
  
      inline bool playable(int8_t player_idx) const {
          const int8_t enemy1_idx = (player_idx + 1) % 3;
          const int8_t enemy2_idx = (player_idx + 2) % 3;
  
          int best_player = score[player_idx] + combo[player_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1);
          int worst_player = score[player_idx];
  
          int best_enemy1 = score[enemy1_idx] + combo[enemy1_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1);
          int worst_enemy1 = score[enemy1_idx];
  
          int best_enemy2 = score[enemy2_idx] + combo[enemy2_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1);
          int worst_enemy2 = score[enemy2_idx];
  
          if (worst_player >= best_enemy1 && worst_player >= best_enemy2) {
              return false; // inevitable 1st place
          }
  
          if (best_player < worst_enemy1 && best_player < worst_enemy2) {
              return false; // inevitable 3rd place
          }
  
          if (worst_player >= best_enemy1 && best_player < worst_enemy2) {
              return false; // inevitable 2nd place
          }
  
          if (best_player < worst_enemy1 && worst_player >= best_enemy2) {
              return false; // inevitable 2nd place
          }
  
          // assert (int(goals_left) * (goals_left + 1) < 256);
          // if (score[player_idx] >= score[enemy1_idx] + combo[enemy1_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1) &&
          //     score[player_idx] >= score[enemy2_idx] + combo[enemy2_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1)) {
          //     return false; // inevitable 1st place
          // }
  
          // assert (int(goals_left) * (goals_left + 1) < 256);
          // if (score[player_idx] + combo[player_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1) < score[enemy1_idx] &&
          //     score[player_idx] + combo[player_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1) < score[enemy2_idx]) {
          //     return false; // inevitable 3rd place
          // }
  
          return true;
      }
  
      inline uint8_t greedy_moves(const int8_t player_idx) const {
          if (end)
              return 0;
          
          return uint8_t(1) << (goal & 3);
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
     Archery archery;
     RollerSkating roller_skating;
     Diving diving;
 
     uint8_t hurdle_race_score[3];
     int8_t hurdle_race_left;
     
     uint8_t archery_score[3];
     int8_t archery_left;
 
     uint8_t roller_skating_score[3];
     int8_t roller_skating_left;
 
     uint8_t diving_score[3];
     int8_t diving_left;
 
     int8_t turn;
 
     bool is_terminal() const {
         return (hurdle_race.end && hurdle_race_left == 0 &&
                 archery.end && archery_left == 0 &&
                 roller_skating.end && roller_skating_left == 0 &&
                 diving.end && diving_left == 0) ||
                turn >= 100;
     }
 
     // return how much does a player earn from games
     void get_stats(float& r0, float& r1, float& r2) {
 
         int8_t places[3];
 
         if (hurdle_race.end) {
             hurdle_race.generate_places(places);
             for (int i = 0; i < 3; i++) {
                 hurdle_race_score[i] += places[i];
             }
         }
 
         if (archery.end) {
             archery.generate_places(places);
             for (int i = 0; i < 3; i++) {
                 archery_score[i] += places[i];
             }
         }
 
         if (roller_skating.end) {
             roller_skating.generate_places(places);
             for (int i = 0; i < 3; i++) {
                 roller_skating_score[i] += places[i];
             }
         }
 
         if (diving.end) {
             diving.generate_places(places);
             for (int i = 0; i < 3; i++) {
                 diving_score[i] += places[i];
             }
         }
 
         int score0 = std::max<int>(1, hurdle_race_score[0]) * std::max<int>(1, archery_score[0]) * std::max<int>(1, roller_skating_score[0]) * std::max<int>(1, diving_score[0]);
         int score1 = std::max<int>(1, hurdle_race_score[1]) * std::max<int>(1, archery_score[1]) * std::max<int>(1, roller_skating_score[1]) * std::max<int>(1, diving_score[1]);
         int score2 = std::max<int>(1, hurdle_race_score[2]) * std::max<int>(1, archery_score[2]) * std::max<int>(1, roller_skating_score[2]) * std::max<int>(1, diving_score[2]);
 
         // maybe change the enemy to win with him?
         // get some values from gameplay? (with whom likely to win at the end)
         // int sum = score0 + score1 + score2;
 
         // maybe the place could be weighted after few turns (on the beginning just focus on own scores?)
 
         // if (turn == 100) {
         //     // if simulated to the end of the game, just get the current leaderboard
 
         // }
         // else {
             // assuming that opponents are litle against me
         r0 = (float) (score0 - score1 - score2) / (score0 + score1 + score2); // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
         r1 = (float) (score1 - score0 - score2) / (score1 + score0 + score2); // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
         r2 = (float) (score2 - score0 - score1) / (score2 + score0 + score1); // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);
         
         // r0 += COEFFICIENT2 * ((score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2));
         // r1 += COEFFICIENT2 * ((score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2));
         // r2 += COEFFICIENT2 * ((score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1));
 
         // r0 += COEFFICIENT2 * (hurdle_race_score[0] - hurdle_race_score[1] - hurdle_race_score[2]);
         // r1 += COEFFICIENT2 * (hurdle_race_score[1] - hurdle_race_score[0] - hurdle_race_score[2]);
         // r2 += COEFFICIENT2 * (hurdle_race_score[2] - hurdle_race_score[0] - hurdle_race_score[1]);
 
         // r0 += COEFFICIENT2 * (archery_score[0] - archery_score[1] - archery_score[2]);
         // r1 += COEFFICIENT2 * (archery_score[1] - archery_score[0] - archery_score[2]);
         // r2 += COEFFICIENT2 * (archery_score[2] - archery_score[0] - archery_score[1]);
 
         // r0 += COEFFICIENT2 * (roller_skating_score[0] - roller_skating_score[1] - roller_skating_score[2]);
         // r1 += COEFFICIENT2 * (roller_skating_score[1] - roller_skating_score[0] - roller_skating_score[2]);
         // r2 += COEFFICIENT2 * (roller_skating_score[2] - roller_skating_score[0] - roller_skating_score[1]);
 
         // r0 += COEFFICIENT2 * (diving_score[0] - diving_score[1] - diving_score[2]);
         // r1 += COEFFICIENT2 * (diving_score[1] - diving_score[0] - diving_score[2]);
         // r2 += COEFFICIENT2 * (diving_score[2] - diving_score[0] - diving_score[1]);
         
 
         //     if (PLAYER_IDX == 0) {
         //         r0 = (float) (score0 - score1 - score2) / sum; // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
         //         r1 = (float) (score1 - 2 * score0 - score2) / sum; // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
         //         r2 = (float) (score2 - 2 * score0 - score1) / sum; // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);
         //     }
         //     else
         //     if (PLAYER_IDX == 1) {
         //         r0 = (float) (score0 - 2 * score1 - score2) / sum; // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
         //         r1 = (float) (score1 - score0 - score2) / sum; // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
         //         r2 = (float) (score2 - score0 - 2 * score1) / sum; // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);
         //     }
         //     else {
         //         r0 = (float) (score0 - score1 - 2 * score2) / sum; // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
         //         r1 = (float) (score1 - score0 - 2 * score2) / sum; // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
         //         r2 = (float) (score2 - score0 - score1) / sum; // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);
         //     }
         // }
 
         // losing -> attack the weakest link (secret strategy: dont care about the winning guy)
         // if (PLAYER_IDX == 0 && score0 < score1 && score0 < score2) {
         //     r0 = (float) (score0 - std::min(score1, score2)) / (score0 + std::min(score1, score2));
         // }
         // else
         // if (PLAYER_IDX == 1 && score1 < score0 && score1 < score2) {
         //     r1 = (float) (score1 - std::min(score0, score2)) / (score1 + std::min(score0, score2));
         // }
         // else
         // if (PLAYER_IDX == 2 && score2 < score0 && score2 < score1) {
         //     r2 = (float) (score2 - std::min(score0, score1)) / (score2 + std::min(score0, score1));
         // }
 
         // float score0 = std::max<float>(1, hurdle_race_score[0]) * std::max<float>(0.95, archery_score[0]) * std::max<float>(0.93, roller_skating_score[0]) * std::max<float>(1, diving_score[0]);
         // float score1 = std::max<float>(1, hurdle_race_score[1]) * std::max<float>(0.95, archery_score[1]) * std::max<float>(0.93, roller_skating_score[1]) * std::max<float>(1, diving_score[1]);
         // float score2 = std::max<float>(1, hurdle_race_score[2]) * std::max<float>(0.95, archery_score[2]) * std::max<float>(0.93, roller_skating_score[2]) * std::max<float>(1, diving_score[2]);
 
         // float sum = score0 + score1 + score2;
 
         // r0 = (float) (score0 - score1 - score2) / sum;
         // r1 = (float) (score1 - score0 - score2) / sum;
         // r2 = (float) (score2 - score0 - score1) / sum;
     }
 
     void init(const std::vector<std::string>& gpu,
               const std::vector<std::vector<int>> reg) {
 
         if (gpu[0] == "GAME_OVER") {
             hurdle_race.end = true;
 
             hurdle_race_left = 1;
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
             }
 
             hurdle_race.end = false;
 
             hurdle_race.build_dp();
             
             // can't skip the game if number of its turns is important
             // if (!hurdle_race.playable(0) &&
             //     !hurdle_race.playable(1) &&
             //     !hurdle_race.playable(2)) {    
             //     hurdle_race.end = true;
             // }
 
             if (hurdle_race.expected_end() <= 5) {
                 hurdle_race_left = 1;
                 std::cerr << "Hurdle once more\n";
             }
             else
                 hurdle_race_left = 0; // 1;
         }
 
         if (gpu[1] == "GAME_OVER") {
             archery.end = true;
 
             archery_left = 0; // 1;
         }
         else { // archery
             archery.wind_index = (int) gpu[1].size() - 1;
             
             for (int i = 0; i < (int) gpu[1].size(); i++) {
                 archery.wind[archery.wind_index - i] = int(gpu[1][i] - '0');
             }
 
             for (int i = 0; i < 3; i++) {
                 archery.x[i] = reg[1][2 * i + 0];
                 archery.y[i] = reg[1][2 * i + 1];
             }
 
             archery.end = false;
 
             archery.build_dp();
 
             // if (!archery.playable(0) &&
             //     !archery.playable(1) &&
             //     !archery.playable(2)) {
             //     archery.end = true;
             // }
 
             if (archery.expected_end() <= 5) {
                 archery_left = 1;
                 std::cerr << "Archery once more\n";
             }
             else
                 archery_left = 0; // 1;
         }
 
         if (gpu[2] == "GAME_OVER") {
             roller_skating.end = true;
 
             roller_skating_left = 1;
         }
         else { // roller_skating
             roller_skating.order = 0;
             for (int i = 0; i < 4; i++) {
                 roller_skating.order |= uint8_t(i) << (to_move_index(gpu[2][i]) << 1);
             }
 
             for (int i = 0; i < 3; i++) {
                 roller_skating.dist_div10[i] = reg[2][i] / 10;
                 roller_skating.dist_mod10[i] = reg[2][i] % 10;
 
                 roller_skating.risk[i] = reg[2][i + 3];
             }
 
             roller_skating.turns_left = reg[2][6];
             // roller_skating.turns_done = 0;
 
             roller_skating.end = false;
 
             // if (!roller_skating.playable(0) &&
             //     !roller_skating.playable(1) &&
             //     !roller_skating.playable(2)) {
             //     roller_skating.end = true;
             // }
 
             if (roller_skating.expected_end() <= 5) {
                 roller_skating_left = 1;
                 std::cerr << "Skating once more\n";
             }
             else
                 roller_skating_left = 0; // 1;
         }
 
         if (gpu[3] == "GAME_OVER") {
             diving.end = true;
             diving_left = 0; // 1;
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
             }
 
             diving.end = false;
             
             // if (!diving.playable(0) &&
             //     !diving.playable(1) &&
             //     !diving.playable(2)) {
             //     diving.end = true;
             // }
 
             if (diving.expected_end() <= 5) {
                 diving_left = 1;
                 std::cerr << "Diving once more\n";
             }
             else
                 diving_left = 0; // 1;
         }
     }
     
     inline bool still_playing() const {
         return !hurdle_race.end ||
                !archery.end ||
                !roller_skating.end ||
                !diving.end;
     }
 
     // inline void greedy_moves(int8_t* move) const {
     //     for (int i = 0; i < 3; i++) {
 
     //         uint8_t moves = hurdle_race.greedy_moves(i) |
     //                         archery.greedy_moves(i) |
     //                         roller_skating.greedy_moves(i) |
     //                         diving.greedy_moves(i) | 
     //                         (uint8_t(1) << (fast_rand() & 3));
             
     //         uint8_t order = all_permutations[fast_rand() % 24];
 
     //         move[i] = -1;
     //         for (int j = 0; j < 4; j++) {
     //             if ((moves >> (order & 3)) & 1) {
     //                 move[i] = j;
     //                 break;
     //             }
 
     //             order >>= 2;
     //         }
 
     //         // assert(0 <= move[i] && move[i] <= 3);
 
     //         // // std::cerr << "player: " << i << '\n';
 
     //         // int eval[4] = {0, 0, 0, 0};
 
     //         // if (hurdle_race.playable(i)) {
     //         //     uint8_t good_moves = hurdle_race.greedy_moves(i);
     //         //     for (int j = 0; j < 4; j++) {
     //         //         if (good_moves & (1 << j)) {
     //         //             eval[j] += int(archery_score[i]) * roller_skating_score[i] * diving_score[i] + 1;
     //         //         }
     //         //     }
     //         // }
     //         // // else {
     //         // //     std::cerr << "hurdles useless\n";
     //         // // }
 
     //         // if (archery.playable(i)) {
     //         //     uint8_t good_moves = archery.greedy_moves(i);
     //         //     for (int j = 0; j < 4; j++) {
     //         //         if (good_moves & (1 << j)) {
     //         //             eval[j] += int(hurdle_race_score[i]) * roller_skating_score[i] * diving_score[i] + 1;
     //         //         }
     //         //     }
     //         // }
     //         // // else {
     //         // //     std::cerr << "archery useless\n";
     //         // // }
 
     //         // if (roller_skating.playable(i)) {
     //         //     uint8_t good_moves = roller_skating.greedy_moves(i);
     //         //     for (int j = 0; j < 4; j++) {
     //         //         if (good_moves & (1 << j)) {
     //         //             eval[j] += int(hurdle_race_score[i]) * archery_score[i] * diving_score[i] + 1;
     //         //         }
     //         //     }
     //         // }
     //         // // else {
     //         // //     std::cerr << "skating useless\n";
     //         // // }
 
     //         // if (diving.playable(i)) {
     //         //     uint8_t good_moves = diving.greedy_moves(i);
     //         //     for (int j = 0; j < 4; j++) {
     //         //         if (good_moves & (1 << j)) {
     //         //             eval[j] += int(hurdle_race_score[i]) * archery_score[i] * roller_skating_score[i] + 1;
     //         //         }
     //         //     }
     //         // }
     //         // // else {
     //         // //     std::cerr << "diving useless\n";
     //         // // }
 
     //         // // std::cerr << "eval: ";
     //         // // for (int j = 0; j < 4; j++) {
     //         //     // std::cerr << int(eval[j]) << ' ';
     //         // // }
     //         // // std::cerr << '\n';
 
     //         // int best_score = -1;
     //         // for (int j = 0; j < 4; j++) {
     //         //     if (best_score < eval[j]) {
     //         //         best_score = eval[j];
     //         //         move[i] = j;
     //         //     }
     //         //     else
     //         //     if (best_score == eval[j] && fast_rand() & 1) {
     //         //         best_score = eval[j];
     //         //         move[i] = j;
     //         //     }
     //         // }
     //     }
     // }
 
     // void play_greedy() {
     //     int8_t move[3];
 
     //     greedy_moves(move);
     //     play(move[0], move[1], move[2]);
     // }
 
     void play(int8_t p0, int8_t p1, int8_t p2) {
 
         const int8_t move[3] = {p0, p1, p2};
 
         static int8_t places[3];
         bool was_hurdle_race_end = hurdle_race.end;
         bool was_archery_end = archery.end;
         bool was_roller_skating_end = roller_skating.end;
         bool was_diving_end = diving.end;
 
         hurdle_race.play(move);
         archery.play(move);
         roller_skating.play(move);
         diving.play(move);
         turn++;
 
         if (was_hurdle_race_end && hurdle_race_left) {
             hurdle_race.generate_places(places);
             for (int i = 0; i < 3; i++) {
                 hurdle_race_score[i] += places[i];
             }
 
             hurdle_race.randomize();
             hurdle_race_left--;
         }
 
         if (was_archery_end && archery_left) {
             archery.generate_places(places);
             for (int i = 0; i < 3; i++) {
                 archery_score[i] += places[i];
             }
 
             archery.randomize();
             archery_left--;
         }
 
         if (was_roller_skating_end && roller_skating_left) {
             roller_skating.generate_places(places);
             for (int i = 0; i < 3; i++) {
                 roller_skating_score[i] += places[i];
             }
 
             roller_skating.randomize();
             roller_skating_left--;
         }
 
         if (was_diving_end && diving_left) {
             diving.generate_places(places);
             for (int i = 0; i < 3; i++) {
                 diving_score[i] += places[i];
             }
 
             diving.randomize();
             diving_left--;
         }
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
 // *** Start of: /home/olaf/codingame/search/brain.hpp *** 
 #ifndef BRAIN_HPP
 #define BRAIN_HPP
 
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
 
 struct BrainNode {
     
     static BrainNode pool[BRAIN_POOL];
     static uint32_t  last;
 
     BrainNode* sons;
 
     float    avg;
     // float    var;
     unsigned vis;
 
     uint8_t last_move;
 
     inline void init(const uint8_t _last_move) {
         sons = 0;
         avg = 0;
         // var = 0;
         vis = 0;
         last_move = _last_move;
     }
 
     inline void expand() {
         sons = (pool + last);
 
         for (uint8_t move = 0; move < 4; move++) {
             pool[last++].init(move);
         }
 
         // random shuffle sons
         std::swap(pool[last - 1], pool[last - 1 - fast_rand() % 4]);
         std::swap(pool[last - 2], pool[last - 2 - fast_rand() % 3]);
         std::swap(pool[last - 3], pool[last - 3 - fast_rand() % 2]);
     }
 
     inline BrainNode* select() const {
         for (int8_t move = 0; move < 4; move++) {
             if ((sons + move)->vis == 0) {
                 return sons + move;
             }
         }
 
         float best_score = -INF;
         BrainNode* best_node = 0;
 
         float sqrt_log_node_vis = C * fastsqrtf(fastlogf(vis));
         for (int8_t move = 0; move < 4; move++) {
             BrainNode* node = sons + move;
 
             // float reward_variance = node->var / node->vis;
             // float variance_term = reward_variance + fastsqrtf(2 * fastlogf(vis) / node->vis);
             // float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis) * std::min(0.25f, variance_term);
 
             float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis);
             // float ucb_score = avg[player_idx][move] + sqrt_log_node_vis * rsqrt_fast(vis[player_idx][move]); // C * std::sqrt(log_node_vis / vis[player_idx][move]);
 
             if (best_score < ucb_score) {
                 best_score = ucb_score;
                 best_node = node;
             }
         }
         return best_node;
     }
 
     inline BrainNode* random_select() const {
         return (sons + (fast_rand() & 3));
     }
 
     inline int8_t best_move() const {
         float best_score = -INF;
         int8_t best_move = -1;
 
         for (int8_t move = 0; move < 4; move++) {
             BrainNode* node = (sons + move);
 
             if (best_score < node->vis) {
                 best_score = node->vis;
                 best_move = node->last_move;
             }
         }
 
         return best_move;
     }
 
     inline void apply(float reward) {
         // float delta = reward - avg;
 
         avg *= vis;
         avg += reward;
         vis += 1;
         avg /= vis;
 
         // var += delta * (reward - avg);
     }
 
     void debug() const {
         std::cerr << "VIS: " << vis << '\n';
         std::cerr << "AVG: " << avg << '\n';
         // std::cerr << "VAR: " << var << '\n';
         std::cerr << "last_move: " << int(last_move) << '\n';
 
         std::cerr << "sons:\n";
 
         // float sqrt_log_node_vis = fastsqrtf(fastlogf(vis));
         for (int8_t move = 0; move < 4; move++) {
             BrainNode* node = (sons + move);
             
             std::cerr << " mov > " << int(node->last_move) << '\n';
             std::cerr << " vis > " << node->vis << '\n';
             std::cerr << " avg > " << node->avg << '\n';
             // std::cerr << " var > " << node->var << '\n';
 
             // float reward_variance = node->var / node->vis;
             // float variance_term = reward_variance + fastsqrtf(2 * fastlogf(vis) / node->vis);
             // float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis) * std::min(0.25f, variance_term);
 
             // std::cerr << " ucb > " << ucb_score << '\n';
             
             std::cerr << '\n';
         }
     }
 };
 
 BrainNode BrainNode::pool[BRAIN_POOL];
 uint32_t  BrainNode::last = 0;
 
 struct Brain {
     BrainNode* roots[3];
 
     inline void reset() {
         BrainNode::last = 0;
 
         for (int i = 0; i < 3; i++) {
             roots[i] = &BrainNode::pool[BrainNode::last];
             roots[i]->init(0);
 
             BrainNode::last++;
         }
     }
 
     void optimize(BrainNode** heads, State &state, float* reward) {
         if (state.is_terminal()) {
             state.get_stats(reward[0], reward[1], reward[2]);
             
             for (int i = 0; i < 3; i++) {
                 if (heads[i]) {
                     heads[i]->apply(reward[i]);
                 }
             }
             return;
         }
 
         if (heads[0] == 0 && heads[1] == 0 && heads[2] == 0) {
             do {
                 uint8_t moves = fast_rand();
                 state.play(moves & 3, (moves >> 2) & 3, (moves >> 4) & 3);
             } while (!state.is_terminal());
 
             state.get_stats(reward[0], reward[1], reward[2]);
             return;
         }
 
         BrainNode* childs[3];
         int8_t moves[3];
 
         for (int i = 0; i < 3; i++) {
             if (heads[i] == 0 || heads[i]->vis == 0) {
                 childs[i] = 0;
                 moves[i] = fast_rand() & 3;
             }
             else {
                 if (heads[i]->sons == 0) {
                     heads[i]->expand();
                 }
 
                 childs[i] = heads[i]->select();
                 moves[i] = childs[i]->last_move;
             }
         }
 
         state.play(moves[0], moves[1], moves[2]);
     
         optimize(childs, state, reward);
 
         for (int i = 0; i < 3; i++) if (heads[i]) {
             heads[i]->apply(reward[i]);
         }
     }
 
     void random_walk(BrainNode** heads, State &state, float* reward) {
         if (state.is_terminal()) {
             state.get_stats(reward[0], reward[1], reward[2]);
             
             for (int i = 0; i < 3; i++) {
                 if (heads[i]) {
                     heads[i]->apply(reward[i]);
                 }
             }
             return;
         }
 
         if (heads[0] == 0 && heads[1] == 0 && heads[2] == 0) {
             do {
                 uint8_t moves = fast_rand();
                 state.play(moves & 3, (moves >> 2) & 3, (moves >> 4) & 3);
             } while (!state.is_terminal());
 
             state.get_stats(reward[0], reward[1], reward[2]);
             return;
         }
 
         BrainNode* childs[3];
         int8_t moves[3];
 
         for (int i = 0; i < 3; i++) {
             if (heads[i] == 0 || heads[i]->vis == 0) {
                 childs[i] = 0;
                 moves[i] = fast_rand() & 3;
             }
             else {
                 if (heads[i]->sons == 0) {
                     heads[i]->expand();
                 }
 
                 childs[i] = heads[i]->random_select();
                 moves[i] = childs[i]->last_move;
             }
         }
 
         state.play(moves[0], moves[1], moves[2]);
     
         random_walk(childs, state, reward);
 
         for (int i = 0; i < 3; i++) if (heads[i]) {
             heads[i]->apply(reward[i]);
         }
     }
 
     int best_move(int player_idx) const {
         return roots[player_idx]->best_move();
     }
 
     void debug() const {
         for (int i = 0; i < 3; i++) {
             roots[i]->debug();
         }
     }
 
     void run(const State& root_state, int timeout) {
         reset();
 
         do {
             State state = root_state;
             BrainNode* heads[3] = {roots[0], roots[1], roots[2]};
             float reward[3];
 
             random_walk(heads, state, reward);
         } while (timer.get_elapsed() < timeout * 0.15 &&
                  BrainNode::last + 40 < BRAIN_POOL);
 
         // debug();
 
         do {
             State state = root_state;
             BrainNode* heads[3] = {roots[0], roots[1], roots[2]};
             float reward[3];
 
             // maybe first 20% of time use on random rollouts from the begginning and then the rest
             optimize(heads, state, reward);
         } while (timer.get_elapsed() < timeout &&
                  BrainNode::last + 40 < BRAIN_POOL);
         
     }
 };
 
 #endif // BRAIN_HPP
 // *** End of: /home/olaf/codingame/search/brain.hpp *** 

#include <iostream>
#include <string>
#include <vector>

int TURN;
int NB_GAMES;

int main() {
    
    std::cerr << "BrainNode: " << sizeof(BrainNode) << '\n';
    std::cerr << "State: " << sizeof(State) << '\n';

    std::cin >> PLAYER_IDX;
    std::cin.ignore();
    std::cin >> NB_GAMES;
    std::cin.ignore();

    timer.start();

    Brain brain;

    for (TURN = 0; ; TURN++) {
        State current_state;
        
        int final_score[3];
        for (int i = 0; i < 3; i++) {
            std::cin >> final_score[i];

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

        std::cerr << "Brain START\n";

        current_state.turn = TURN;

#ifdef PSYLEAGUE
        brain.run(current_state, 45);
        // brain.debug();
#else
        brain.run(current_state, (TURN == 0 ? 950 : 45));
        // brain.debug();
#endif // PSYLEAGUE

        // timer.start();

        // for (int i = 0; i < MCTSNode::last_node; i++) {
        //     for (int k = 0; k < 3; k++)
        //         for (int j = 0; j < 4; j++)
        //             MCTSNode::pool[i].vis[k][j] >>= 1;
        //     MCTSNode::pool[i].node_vis >>= 1;
        // }

        /*
        if i'm lossing -> attack the lowest link
        if i'm winning -> maximize the gap between me and second place
        if i'm second ->
        */
        std::cerr << "timer: " << timer.get_elapsed() << '\n';
        std::cerr << "pool: " << (float) BrainNode::last / BRAIN_POOL << '\n';
        std::cerr << "last: " << BrainNode::last << '\n';

        // int8_t greedy_moves[3];
        // current_state.greedy_moves(greedy_moves);
        // int move = greedy_moves[PLAYER_IDX];
        // std::cerr << "GREEDY: ";
        // for (int i = 0; i < 3; i++) {
        //     std::cerr << move_list[greedy_moves[i]] << ' ';
        // }
        // std::cerr << '\n';

        int move = brain.best_move(PLAYER_IDX);
        
        std::vector<std::string> move_list = {
            "UP", "LEFT", "DOWN", "RIGHT"
        };

        std::cerr << "predicting: \n";
        std::cerr << "0: " << move_list[brain.best_move(0)] << '\n';
        std::cerr << "1: " << move_list[brain.best_move(1)] << '\n';
        std::cerr << "2: " << move_list[brain.best_move(2)] << '\n';

        std::cout << move_list[move] << std::endl;

        // if (TURN == 1) {
        //     return 0;
        // }
    }

}
// *** End of: /home/olaf/codingame/brain.cpp *** 
