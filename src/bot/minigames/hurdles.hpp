#ifndef HURDLES
#define HURDLES

#include "../core/const.hpp"
#include "../core/random.hpp"

#include <iostream>
#include <cassert>

const int tracks_count = 640;
const uint32_t all_tracks[tracks_count] = {8947848,17336456,559240,559240,17860744,34637960,1083528,1083528,34952,34952,34952,34952,17893512,34670728,1116296,1116296,35719304,69273736,2164872,2164872,67720,67720,67720,67720,2184,2184,2184,2184,2184,2184,2184,2184,17895560,34672776,1118344,1118344,35721352,69275784,2166920,2166920,69768,69768,69768,69768,35786888,69341320,2232456,2232456,71438472,138547336,4329608,4329608,135304,135304,135304,135304,4232,4232,4232,4232,4232,4232,4232,4232,17895688,34672904,1118472,1118472,35721480,69275912,2167048,2167048,69896,69896,69896,69896,35787016,69341448,2232584,2232584,71438600,138547464,4329736,4329736,135432,135432,135432,135432,4360,4360,4360,4360,4360,4360,4360,4360,35791112,69345544,2236680,2236680,71442696,138551560,4333832,4333832,139528,139528,139528,139528,71573768,138682632,4464904,4464904,142876936,277094664,8659208,8659208,270600,270600,270600,270600,8456,8456,8456,8456,8456,8456,8456,8456,17895696,34672912,1118480,1118480,35721488,69275920,2167056,2167056,69904,69904,69904,69904,35787024,69341456,2232592,2232592,71438608,138547472,4329744,4329744,135440,135440,135440,135440,4368,4368,4368,4368,4368,4368,4368,4368,35791120,69345552,2236688,2236688,71442704,138551568,4333840,4333840,139536,139536,139536,139536,71573776,138682640,4464912,4464912,142876944,277094672,8659216,8659216,270608,270608,270608,270608,8464,8464,8464,8464,8464,8464,8464,8464,35791376,69345808,2236944,2236944,71442960,138551824,4334096,4334096,139792,139792,139792,139792,71574032,138682896,4465168,4465168,142877200,277094928,8659472,8659472,270864,270864,270864,270864,8720,8720,8720,8720,8720,8720,8720,8720,71582224,138691088,4473360,4473360,142885392,277103120,8667664,8667664,279056,279056,279056,279056,143147536,277365264,8929808,8929808,285753872,17318416,17318416,17318416,541200,541200,541200,541200,16912,16912,16912,16912,16912,16912,16912,16912,35791392,69345824,2236960,2236960,71442976,138551840,4334112,4334112,139808,139808,139808,139808,71574048,138682912,4465184,4465184,142877216,277094944,8659488,8659488,270880,270880,270880,270880,8736,8736,8736,8736,8736,8736,8736,8736,71582240,138691104,4473376,4473376,142885408,277103136,8667680,8667680,279072,279072,279072,279072,143147552,277365280,8929824,8929824,285753888,17318432,17318432,17318432,541216,541216,541216,541216,16928,16928,16928,16928,16928,16928,16928,16928,71582752,138691616,4473888,4473888,142885920,277103648,8668192,8668192,279584,279584,279584,279584,143148064,277365792,8930336,8930336,285754400,17318944,17318944,17318944,541728,541728,541728,541728,17440,17440,17440,17440,17440,17440,17440,17440,143164448,277382176,8946720,8946720,285770784,17335328,17335328,17335328,558112,558112,558112,558112,286295072,17859616,17859616,17859616,34636832,34636832,34636832,34636832,1082400,1082400,1082400,1082400,33824,33824,33824,33824,33824,33824,33824,33824,71582784,138691648,4473920,4473920,142885952,277103680,8668224,8668224,279616,279616,279616,279616,143148096,277365824,8930368,8930368,285754432,17318976,17318976,17318976,541760,541760,541760,541760,17472,17472,17472,17472,17472,17472,17472,17472,143164480,277382208,8946752,8946752,285770816,17335360,17335360,17335360,558144,558144,558144,558144,286295104,17859648,17859648,17859648,34636864,34636864,34636864,34636864,1082432,1082432,1082432,1082432,33856,33856,33856,33856,33856,33856,33856,33856,143165504,277383232,8947776,8947776,285771840,17336384,17336384,17336384,559168,559168,559168,559168,286296128,17860672,17860672,17860672,34637888,34637888,34637888,34637888,1083456,1083456,1083456,1083456,34880,34880,34880,34880,34880,34880,34880,34880,286328896,17893440,17893440,17893440,34670656,34670656,34670656,34670656,1116224,1116224,1116224,1116224,35719232,35719232,35719232,35719232,69273664,69273664,69273664,69273664,2164800,2164800,2164800,2164800,67648,67648,67648,67648,67648,67648,67648,67648,143165568,277383296,8947840,8947840,285771904,17336448,17336448,17336448,559232,559232,559232,559232,286296192,17860736,17860736,17860736,34637952,34637952,34637952,34637952,1083520,1083520,1083520,1083520,34944,34944,34944,34944,34944,34944,34944,34944,286328960,17893504,17893504,17893504,34670720,34670720,34670720,34670720,1116288,1116288,1116288,1116288,35719296,35719296,35719296,35719296,69273728,69273728,69273728,69273728,2164864,2164864,2164864,2164864,67712,67712,67712,67712,67712,67712,67712,67712,286331008,17895552,17895552,17895552,34672768,34672768,34672768,34672768,1118336,1118336,1118336,1118336,35721344,35721344,35721344,35721344,69275776,69275776,69275776,69275776,2166912,2166912,2166912,2166912,69760,69760,69760,69760,69760,69760,69760,69760,35786880,35786880,35786880,35786880,69341312,69341312,69341312,69341312,2232448,2232448,2232448,2232448,71438464,71438464,71438464,71438464,138547328,138547328,138547328,138547328,4329600,4329600,4329600,4329600,135296,135296,135296,135296,135296,135296,135296,135296};

struct Hurdles {
    
    static int dp[tracks_count][TRACK_LENGTH + 2];
    static int pd[tracks_count][TRACK_LENGTH + 2];
    static uint8_t dp_opt[tracks_count][TRACK_LENGTH + 2];
    static uint8_t next_pos[tracks_count][TRACK_LENGTH + 2][MOVE_COUNT];
    static uint8_t next_hit[tracks_count][TRACK_LENGTH + 2][MOVE_COUNT];
    
    int track_index;

    uint32_t track;

    int8_t pos[PLAYER_COUNT], stun[PLAYER_COUNT];
    bool end;

    void find_track_index() {
        for (int i = 0; i < tracks_count; i++) {
            if (all_tracks[i] == track) {
                track_index = i;
                return;
            }
        }

        assert(false);
    }

    static void build_dp() {
        for (int track_id = 0; track_id < tracks_count; track_id++) {
            const uint32_t track_mask = all_tracks[track_id];
            const auto has_obstacle = [track_mask](int p) {
                if (p > TRACK_LENGTH + 1) {
                    return false;
                }
                return ((track_mask >> p) & 1) != 0;
            };

            for (int pos = 0; pos <= TRACK_LENGTH + 1; pos++) {
                for (int move = 0; move < MOVE_COUNT; move++) {
                    int p = pos;

                    if (move == 0) {
                        p += 2;
                    }
                    else
                    if (move == 1) {
                        p++;
                    }
                    else
                    if (move == 2) {
                        p++;

                        if (!has_obstacle(p)) {
                            p++;
                        }
                    }
                    else
                    if (move == 3) { 
                        p++;

                        if (!has_obstacle(p)) {
                            p++;

                            if (!has_obstacle(p)) {
                                p++;
                            }
                        }
                    }
                    else {
                        assert(false);
                    }

                    if (p > TRACK_LENGTH + 1) {
                        p = TRACK_LENGTH + 1;
                    }

                    next_pos[track_id][pos][move] = static_cast<uint8_t>(p);
                    next_hit[track_id][pos][move] = has_obstacle(p) ? 1 : 0;
                }
            }

            dp[track_id][TRACK_LENGTH - 1] = 0;
            dp[track_id][TRACK_LENGTH - 0] = 0;
            dp[track_id][TRACK_LENGTH + 1] = 0;
            
            pd[track_id][TRACK_LENGTH - 1] = 0;
            pd[track_id][TRACK_LENGTH - 0] = 0;
            pd[track_id][TRACK_LENGTH + 1] = 0;

            dp_opt[track_id][TRACK_LENGTH - 1] = 0;
            dp_opt[track_id][TRACK_LENGTH - 0] = 0;
            dp_opt[track_id][TRACK_LENGTH + 1] = 0;

            for (int i = TRACK_LENGTH - 2; i >= 0; i--) {
                int& val = dp[track_id][i];
                int& lav = pd[track_id][i];
                uint8_t& opt = dp_opt[track_id][i];

                val = 10000;
                lav = 0;
                opt = 0;

                for (int move = 0; move < MOVE_COUNT; move++) {
                    int now = 1;
                    const int p = next_pos[track_id][i][move];

                    if (next_hit[track_id][i][move]) {
                        now += 2;
                    }

                    if (lav < now + pd[track_id][p]) {
                        lav = now + pd[track_id][p];
                    }

                    now += dp[track_id][p];

                    if (val > now) {
                        val = now;
                        opt = uint8_t(1) << move;
                    }
                    else
                    if (val == now) {
                        opt |= uint8_t(1) << move;
                    }
                }
            }
        }
    }

    inline bool is_useless(int8_t player_idx) const {
        return stun[player_idx] > 0;
    }

    inline uint8_t greedy_moves(int8_t player_idx) const {
        return dp_opt[track_index][pos[player_idx]];
    }

    bool operator== (const Hurdles &other) const {
        if (end != other.end)
            return false;
        
        if (end)
            return true;
        
        for (int i = 0; i < PLAYER_COUNT; i++) {
            if (pos[i] != other.pos[i])
                return false;
            if (stun[i] != other.stun[i])
                return false;
        }

        return track == other.track;
    }

    void debug() const {
        std::cerr << "TRACK: ";
        for (int i = 0; i < TRACK_LENGTH; i++) {
            if (track & (1u << i))
                std::cerr << '#';
            else
                std::cerr << '.';
        }
        std::cerr << "|\n";

        for (int i = 0; i < PLAYER_COUNT; i++) {
            std::cerr << "i: " << i << " => " << int(pos[i]) << ' ' << int(stun[i]) << '\n';
        }
    }

    inline int expected_end() const {
        const int t0 = pd[track_index][pos[0]] + stun[0];
        const int t1 = pd[track_index][pos[1]] + stun[1];
        const int t2 = pd[track_index][pos[2]] + stun[2];

        if (t0 <= t1 && t0 <= t2)
            return t0;
        else
        if (t1 <= t0 && t1 <= t2)
            return t1;

        return t2;
    }

    inline void generate_places(float* places) const {
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

    inline void randomize() {
        track_index = fast_rand() % tracks_count;
        track = all_tracks[track_index];

        for (int i = 0; i < PLAYER_COUNT; i++) {
            pos[i] = 0;
            stun[i] = 0;
        }

        end = false;
    }

    inline void play(const int8_t* move) {
        if (end) {
            return;
        }

        for (int i = 0; i < PLAYER_COUNT; i++) {
            if (stun[i]) {
                stun[i]--;
            }
            else {
                const uint8_t prev_pos = static_cast<uint8_t>(pos[i]);
                const uint8_t move_idx = static_cast<uint8_t>(move[i]);
                const uint8_t next = next_pos[track_index][prev_pos][move_idx];
                pos[i] = static_cast<int8_t>(next);

                if (next_hit[track_index][prev_pos][move_idx]) {
                    stun[i] = 2;
                }
                else
                if (next >= TRACK_LENGTH - 1) {
                    end = true;
                }
            }
        }
    }

    inline bool playable([[maybe_unused]] const int8_t player_idx) const {
        // const int8_t enemy1_idx = (player_idx + 1) % 3;
        // const int8_t enemy2_idx = (player_idx + 2) % 3;

        // if (dp[track_index][pos[player_]])

        // if (pd[pos[player_idx]] + stun[player_idx] <=
        //     dp[pos[enemy1_idx]] + stun[enemy1_idx] &&
            
        //     pd[pos[player_idx]] + stun[player_idx] <=
        //     dp[pos[enemy2_idx]] + stun[enemy2_idx]) {
        //     return false; // inevitable 1st place
        // }

        // if (dp[pos[player_idx]] + stun[player_idx] >
        //     pd[pos[enemy1_idx]] + stun[enemy1_idx] &&

        //     dp[pos[player_idx]] + stun[player_idx] >
        //     pd[pos[enemy2_idx]] + stun[enemy2_idx]) {
        //     return false; // inevitable 3rd place
        // }

        // // ranking: <enemy1> <player> <enemy2>
        // if (dp[pos[player_idx]] + stun[player_idx] >
        //     pd[pos[enemy1_idx]] + stun[enemy1_idx] &&
            
        //     pd[pos[player_idx]] + stun[player_idx] <=
        //     dp[pos[enemy2_idx]] + stun[enemy2_idx]) {
        //     return false; // inevitable 2nd place
        // }

        // // ranking: <enemy2> <player> <enemy1>
        // if (dp[pos[player_idx]] + stun[player_idx] >
        //     pd[pos[enemy2_idx]] + stun[enemy2_idx] &&
            
        //     pd[pos[player_idx]] + stun[player_idx] <=
        //     dp[pos[enemy1_idx]] + stun[enemy1_idx]) {
        //     return false; // inevitable 2nd place
        // }

        return true;
    }

};

int Hurdles::dp[tracks_count][TRACK_LENGTH + 2];
int Hurdles::pd[tracks_count][TRACK_LENGTH + 2];
uint8_t Hurdles::dp_opt[tracks_count][TRACK_LENGTH + 2];
uint8_t Hurdles::next_pos[tracks_count][TRACK_LENGTH + 2][MOVE_COUNT];
uint8_t Hurdles::next_hit[tracks_count][TRACK_LENGTH + 2][MOVE_COUNT];

#endif // HURDLES
