#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <map>

using namespace std;

bool is_archery_lost(string gpu, vector<int> regs, int player_idx) {
    if (gpu == "GAME_OVER")
        return false;

    int worse_enemy_score = 1e9;
    int player_score = -1e9;

    for (int i = 0; i < 3; i++) {
        if (i == player_idx) {
            int x = abs(regs[2 * i + 0]);
            int y = abs(regs[2 * i + 1]);

            vector<vector<bool>> dp(21, vector<bool>(21, false));

            dp[x][y] = true;

            for (char c : gpu) {
                vector<vector<bool>> pd(21, vector<bool>(21, false));

                int wind = char(c - '0');

                for (int a = 0; a <= 20; a++) {
                    for (int b = 0; b <= 20; b++) {
                        if (dp[a][b]) {
                            pd[min(a + wind, 20)][b] = true;
                            pd[abs(a - wind)][b] = true;
                            pd[a][min(b + wind, 20)] = true;
                            pd[a][abs(b - wind)] = true;
                        }
                    }
                }

                swap(pd, dp);
            }

            for (int a = 0; a <= 20; a++) {
                for (int b = 0; b <= 20; b++) {
                    if (dp[a][b]) {
                        int now = -(a * a + b * b);

                        if (player_score < now) {
                            player_score = now;
                        }
                    }
                }
            }
        }
        else {
            int x = abs(regs[2 * i + 0]);
            int y = abs(regs[2 * i + 1]);

            vector<vector<bool>> dp(21, vector<bool>(21, false));

            dp[x][y] = true;

            for (char c : gpu) {
                vector<vector<bool>> pd(21, vector<bool>(21, false));

                int wind = char(c - '0');

                for (int a = 0; a <= 20; a++) {
                    for (int b = 0; b <= 20; b++) {
                        if (dp[a][b]) {
                            pd[min(a + wind, 20)][b] = true;
                            pd[abs(a - wind)][b] = true;
                            pd[a][min(b + wind, 20)] = true;
                            pd[a][abs(b - wind)] = true;
                        }
                    }
                }

                swap(pd, dp);
            }

            for (int a = 0; a <= 20; a++) {
                for (int b = 0; b <= 20; b++) {
                    if (dp[a][b]) {
                        int now = -(a * a + b * b);

                        if (worse_enemy_score > now) {
                            worse_enemy_score = now;
                        }
                    }
                }
            }
        }
    }

    cerr << "worse enemy in archery: " << worse_enemy_score << " vs " << player_score << '\n';

    return worse_enemy_score > player_score;
}

bool is_archery_win(string gpu, vector<int> regs, int player_idx) {
    if (gpu == "GAME_OVER")
        return false;

    int best_enemy_score = -1e9;
    int player_score = 1e9;

    for (int i = 0; i < 3; i++) {
        if (i == player_idx) {
            int x = abs(regs[2 * i + 0]);
            int y = abs(regs[2 * i + 1]);

            vector<vector<bool>> dp(21, vector<bool>(21, false));

            dp[x][y] = true;

            for (char c : gpu) {
                int wind = char(c - '0');
                
                vector<vector<bool>> pd(21, vector<bool>(21, false));

                for (int a = 0; a <= 20; a++) {
                    for (int b = 0; b <= 20; b++) {
                        if (dp[a][b]) {
                            pd[min(a + wind, 20)][b] = true;
                            pd[a][min(b + wind, 20)] = true;
                        }
                    }
                }

                swap(dp, pd);
            }

            for (int a = 0; a <= 20; a++) {
                for (int b = 0; b <= 20; b++) {
                    if (dp[a][b]) {
                        int now = -(a * a + b * b);

                        if (player_score > now) {
                            player_score = now;
                        }
                    }
                }
            }
        }
        else {
            int x = abs(regs[2 * i + 0]);
            int y = abs(regs[2 * i + 1]);

            vector<vector<bool>> dp(21, vector<bool>(21, false));

            dp[x][y] = true;

            for (char c : gpu) {
                vector<vector<bool>> pd(21, vector<bool>(21, false));

                int wind = char(c - '0');

                for (int a = 0; a <= 20; a++) {
                    for (int b = 0; b <= 20; b++) {
                        if (dp[a][b]) {
                            pd[min(a + wind, 20)][b] = true;
                            pd[abs(a - wind)][b] = true;
                            pd[a][min(b + wind, 20)] = true;
                            pd[a][abs(b - wind)] = true;
                        }
                    }
                }

                swap(pd, dp);
            }

            int now = -1e9;

            for (int a = 0; a <= 20; a++) {
                for (int b = 0; b <= 20; b++) {
                    if (dp[a][b]) {
                        now = max(now, -(a * a + b * b));
                    }
                }
            }
            
            if (best_enemy_score < now) {
                best_enemy_score = now;
            }
        }
    }

    cerr << "best enemy in archery: " << best_enemy_score << " vs " << player_score << '\n';


    return best_enemy_score <= player_score;
}

bool is_hurdle_lost(string gpu, vector<int> regs, int player_idx) {
    if (gpu == "GAME_OVER")
        return false;

    int worse_enemy_score = 1e9;
    int player_score = 0;

    int len = (int) gpu.size();
    gpu += "...";

    for (int i = 0; i < 3; i++) {
        if (i == player_idx) {
            int stun = regs[i + 3];
            player_score -= stun;

            int pos = regs[i];
            
            while (pos < len) {
                if (gpu[pos + 1] == '#') {
                    pos += 2;
                }
                else
                if (gpu[pos + 2] == '#') {
                    pos += 1;
                }
                else
                if (gpu[pos + 3] == '#') {
                    pos += 2;
                }
                else {
                    pos += 3;
                }

                player_score -= 1;
            }
        }
        else {
            int stun = regs[i + 3];
            int now = -stun;

            for (int j = regs[i]; j < len; j++) {
                now -= 1;

                if (gpu[j] == '#')
                    now -= 3;
            }

            if (worse_enemy_score > now) {
                worse_enemy_score = now;
            }
        }
    }

    cerr << "worse enemy in hurdle race: " << worse_enemy_score << " vs " << player_score << '\n';

    return worse_enemy_score > player_score;
}

bool is_hurdle_win(string gpu, vector<int> regs, int player_idx) {
    if (gpu == "GAME_OVER")
        return false;

    int best_enemy_score = -1e9;
    int player_score = 0;

    int len = (int) gpu.size();
    gpu += "...";

    for (int i = 0; i < 3; i++) {
        if (i == player_idx) {
            int stun = regs[i + 3];
            player_score -= stun;

            for (int j = regs[i]; j < len; j++) {
                player_score -= 1;

                if (gpu[j] == '#')
                    player_score -= 3; 
            }
        }
        else {
            int stun = regs[i + 3];

            int now = -stun;

            int pos = regs[i];

            while (pos < len) {
                if (gpu[pos + 1] == '#') {
                    pos += 2;
                }
                else
                if (gpu[pos + 2] == '#') {
                    pos += 1;
                }
                else
                if (gpu[pos + 3] == '#') {
                    pos += 2;
                }
                else {
                    pos += 3;
                }

                now -= 1;
            }
        
            if (best_enemy_score < now) {
                best_enemy_score = now;
            }
        }
    }

    cerr << "best enemy in hurdle race: " << best_enemy_score << " vs " << player_score << '\n';

    return best_enemy_score <= player_score;
}

bool is_diving_lost(string gpu, vector<int> regs, int player_idx) {
    if (gpu == "GAME_OVER")
        return false;

    int worse_enemy_score = 1e9;
    int player_score = 0;

    for (int i = 0; i < 3; i++) {
        if (i == player_idx) {
            player_score += regs[i];
            int combo = regs[i + 3];

            for (int j = 0; j < (int) gpu.size(); j++) {
                combo++;
                player_score += combo;
            }
        }
        else {
            int now = regs[i];

            if (worse_enemy_score > now) {
                worse_enemy_score = now;
            }
        }
    }

    cerr << "worse enemy in diving: " << worse_enemy_score << " vs " << player_score << '\n';

    return worse_enemy_score > player_score;
}

bool is_diving_win(string gpu, vector<int> regs, int player_idx) {
    if (gpu == "GAME_OVER")
        return false;

    int best_enemy_score = 0;

    for (int i = 0; i < 3; i++) {
        if (i == player_idx) {
            continue;
        }

        int now = regs[i];

        int combo = regs[3 + i];
        for (int j = 0; j < (int) gpu.size(); j++) {
            combo++;
            now += combo;
        }

        if (best_enemy_score < now) {
            best_enemy_score = now;
        }
    }

    cerr << "best enemy in diving: " << best_enemy_score << " vs " << regs[player_idx] << '\n';

    return best_enemy_score <= regs[player_idx];
}

bool is_roller_win(string gpu, vector<int> regs, int player_idx) {
    if (gpu == "GAME_OVER")
        return false;

    int best_enemy_score = 0;
    int player_score = 0;

    for (int i = 0; i < 3; i++) {
        if (i == player_idx) {
            int risk = regs[i + 3];
            int turns = regs[6];

            if (risk < 0) {
                turns -= -risk;
                risk = 0;
            }

            player_score = regs[i];
            while (turns > 0) {
                player_score += 2;
                risk += 1;
                
                turns -= 1;
                if (risk >= 5) {
                    risk = 0;
                    turns -= 2;
                }
            }
        }
        else {
            int turns = regs[6];
            int risk = regs[i + 3];

            if (risk < 0) {
                turns -= -risk;
                risk = 0;
            }

            int now = regs[i];

            if (turns > 0) {
                int times3 = min(turns, (4 - risk) / 2);
                turns -= times3;

                now += times3 * 3;
                now += turns * 2;
            }

            if (best_enemy_score < now) {
                best_enemy_score = now;
            }
        }
    }
    
    cerr << "best enemy in scating: " << best_enemy_score << " vs " << player_score << '\n';


    return player_score >= best_enemy_score;
}

int main() {
    srand(2137);

    int player_idx;
    cin >> player_idx; cin.ignore();
    int nb_games;
    cin >> nb_games; cin.ignore();

    vector<int> pos(nb_games, 0);
    vector<int> stun(nb_games, 0);

    int previous_min_index = -1;

    // game loop
    while (1) {
        vector<int> scores(nb_games);

        for (int i = 0; i < 3; i++) {
            int final_score;
            cin >> final_score;

            cerr << "final: " << final_score << ' ';

            for (int j = 0; j < nb_games; j++) {
                int gold, silver, bronze;
                cin >> gold >> silver >> bronze;

                if (i == player_idx) {
                    scores[j] = gold * 3 + silver;
                }

                cerr << gold << ' ' << silver << ' ' << bronze << ",";
            }
            cerr << '\n';
            // string score_info;
            // getline(cin, score_info);
            // cerr << "i: " << i << ' ' << score_info << '\n';
            cin.ignore();
        }

        vector<string> gpu(nb_games);
        vector<vector<int>> reg(nb_games);

        for (int i = 0; i < nb_games; i++) {
            cin >> gpu[i];
            
            reg[i].resize(7);
            for (int j = 0; j < 7; j++)
                cin >> reg[i][j];
            cin.ignore();
            cerr << "gpu(" << i << ") " << gpu[i] << '\n';

            if (gpu[i] == "GAME_OVER") {
                scores[i] += 1e5;
            }
        }

        vector<string> moves = {
            "UP", "RIGHT", "DOWN", "LEFT"
        };

        if (is_hurdle_win(gpu[0], reg[0], player_idx) ||
            is_hurdle_lost(gpu[0], reg[0], player_idx)) {
            scores[0] += 1e5;
        }

        if (is_archery_win(gpu[1], reg[1], player_idx) ||
            is_archery_lost(gpu[1], reg[1], player_idx)) {
            scores[1] += 1e5;
        }

        if (is_diving_win(gpu[3], reg[3], player_idx) ||
            is_diving_lost(gpu[3], reg[3], player_idx)) {
            scores[3] += 1e5;
        }

        if (is_roller_win(gpu[2], reg[2], player_idx)) {
            scores[2] += 1e5;
        }

        cerr << "scores: ";
        for (int i = 0; i < nb_games; i++) {
            cerr << scores[i] << ' ';
        }
        cerr << '\n';

        int min_value = *min_element(scores.begin(), scores.end());

        vector<int> min_indexes;

        int min_index = -1;

        for (int j = 0; j < 4; j++) {
            if (scores[j] == min_value) {
                if (j == previous_min_index) {
                    min_index = j;
                    previous_min_index = -1;
                    break;
                }
                min_indexes.push_back(j);
            }
        }

        if (min_index == -1) {
            min_index = min_indexes[rand() % (int) min_indexes.size()];
        }

        previous_min_index = min_index;
        
        string best_move = moves[rand() % 4];

        map<char, string> to_move;
        to_move['U'] = "UP";
        to_move['R'] = "RIGHT";
        to_move['D'] = "DOWN";
        to_move['L'] = "LEFT";

        cerr << "min_index: " << min_index << '\n';

        if (min_index == 0) {
            if (gpu[0] != "GAME_OVER") {
                gpu[0] += "......";

                int pos = reg[0][player_idx];
                int stun = reg[0][3 + player_idx];

                cerr << "pos: " << pos << ' ' << stun << ' ' << gpu[0][pos + 1] << ' ' << gpu[0][pos + 2] << '\n';
                if (stun == 0) {
                    if (gpu[0][pos + 1] == '#') {
                        best_move = "UP";
                    }
                    else
                    if (gpu[0][pos + 2] == '#') {
                        best_move = "LEFT";
                    }
                    else
                    if (gpu[0][pos + 3] == '#') {
                        best_move = "DOWN";
                    }
                    else {
                        best_move = "RIGHT";
                    }
                }
            }
        }
        else
        if (min_index == 1) {
            if (gpu[1] != "GAME_OVER") {

                vector<vector<int>> dp(21, vector<int>(21, 0));

                for (int a = 0; a <= 20; a++) {
                    for (int b = 0; b <= 20; b++) {
                        dp[a][b] = a * a + b * b;
                    }
                }

                for (int j = (int) gpu[1].size() - 1; j > 0; j--) {
                    int wind = gpu[1][j] - '0';

                    vector<vector<int>> pd(21, vector<int>(21, 0));
                    for (int a = 0; a <= 20; a++) {
                        for (int b = 0; b <= 20; b++) {
                            pd[a][b] = min(
                                {
                                    dp[min(a + wind, 20)][b],
                                    dp[abs(a - wind)][b],
                                    dp[a][min(b + wind, 20)],
                                    dp[a][abs(b - wind)]
                                }
                            );
                        }
                    }

                    swap(pd, dp);
                }

                const int DX[4] = {0, +1, 0, -1};
                const int DY[4] = {-1, 0, +1, 0};

                int best = 100000;

                int wind = gpu[1][0] - '0';
                int x = reg[1][player_idx * 2 + 0];
                int y = reg[1][player_idx * 2 + 1];

                cerr << "> " << x << ' ' << y << ' ' << wind << '\n';

                for (int dir = 0; dir < 4; dir++) {
                    int dx = x + wind * DX[dir];
                    int dy = y + wind * DY[dir];
                    
                    int now = dp[min(20, abs(dx))][min(20, abs(dy))];

                    if (best > now) {
                        best = now;
                        best_move = moves[dir];
                        cerr << "EXPECTING: " << dx << ' ' << dy << '\n';
                    }
                }
            }
        }
        else
        if (min_index == 2) {
            if (gpu[2] != "GAME_OVER") {
                int risk = reg[2][player_idx + 3];
                
                if (risk + 2 >= 4) {
                    best_move = to_move[gpu[2][1]];
                }
                else {
                    best_move = to_move[gpu[2][3]];
                }
            }
        }
        else
        if (min_index == 3) {
            if (gpu[3] != "GAME_OVER") {
                best_move = to_move[gpu[3][0]];
                // if (gpu[3][0] == 'U')
                //     best_move = "UP";
                // else
                // if (gpu[3][0] == 'R')
                //     best_move = "RIGHT";
                // else
                // if (gpu[3][0] == 'D')
                //     best_move = "DOWN";
                // else
                // if (gpu[3][0] == 'L')
                //     best_move = "LEFT";
                // else
                //     assert(false);
            }
        }

        cerr << "best: " << best_move << '\n';

        cout << best_move << endl;
    }
}