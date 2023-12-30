#ifndef LOCAL
    #undef _GLIBCXX_DEBUG                // disable run-time bound checking, etc
    #pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

    #pragma GCC target("bmi,bmi2,lzcnt,popcnt")                      // bit manipulation
    #pragma GCC target("movbe")                                      // byte swap
    #pragma GCC target("aes,pclmul,rdrnd")                           // encryption
    #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD
#endif

#include <iostream>
#include <string>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <vector>
#include <array>
#include <chrono>

#include "fish.h"
#include "drone.h"
#include "const.h"
#include "net.h"
#include "collision.h"
// #include "grid.h"

int last_light[DRONES_PER_PLAYER];

bool inside(const Vector &point) {
    return 0 <= point.x && point.x <= WIDTH &&
           0 <= point.y && point.y < HEIGHT;
}

int main() {
    std::cin >> creature_count; std::cin.ignore();
    for (int i = 0; i < creature_count; i++) {
        std::cin >> creatures[i].id >> 
                    creatures[i].color >> 
                    creatures[i].type; std::cin.ignore();
        
        creatures_from_id[creatures[i].id] = &creatures[i];

        if (creatures[i].isMonster() == false) {
            fish_table[creatures[i].color][creatures[i].type] = &creatures[i];
        }
        else {
            monsters[monster_count++] = &creatures[i];
        }

        if (creatures[i].type == 0) {
            fish_nets[creatures[i].id] =
                Net(Vector(0, 2500), Vector(WIDTH, 5000));
        }
        else
        if (creatures[i].type == 1) {
            fish_nets[creatures[i].id] =
                Net(Vector(0, 5000), Vector(WIDTH, 7500));
        }
        else
        if (creatures[i].type == 2) {
            fish_nets[creatures[i].id] = 
                Net(Vector(0, 7500), Vector(WIDTH, 10000));
        }
        else
        if (creatures[i].type == -1) {
            monsters_list[monsters_count++] = &creatures[i];

            fish_nets[creatures[i].id] = 
                Net(Vector(0, 5000), Vector(WIDTH, 10000));
        }
        else {
            assert(false);
        }
    }

    // grid::initialize_probability();

    for (int player = 0; player < 2; player++)
        for (int drone = 0; drone < 2; drone++)
            drones[player][drone].battery = 30;

    for (int game_turn = 0; ; game_turn++) {
        int my_score;
        std::cin >> my_score; std::cin.ignore();

        auto start = std::chrono::high_resolution_clock::now();

        int foe_score;
        std::cin >> foe_score; std::cin.ignore();
        int my_scan_count;
        std::cin >> my_scan_count; std::cin.ignore();
        for (int i = 0; i < my_scan_count; i++) {
            int creature_id;
            std::cin >> creature_id; std::cin.ignore();

            Fish *creature = creatures_from_id[creature_id];

            creature->is_scanned = true;
            creature->is_reported = true;
        }
        int foe_scan_count;
        std::cin >> foe_scan_count; std::cin.ignore();
        for (int i = 0; i < foe_scan_count; i++) {
            int creature_id;
            std::cin >> creature_id; std::cin.ignore();

            Fish *creature = creatures_from_id[creature_id];

            creature->is_foe_scanned = true;
            creature->is_foe_reported = true;
        }
        int my_drone_count;
        std::cin >> my_drone_count; std::cin.ignore();

        std::vector<Fish*> old_scans[2][2];
        for (int i = 0; i < my_drone_count; i++) {
            int old_battery = drones[0][i].battery;

            std::cin >> drones[0][i].id >>
                        drones[0][i].pos.x >> 
                        drones[0][i].pos.y >>
                        drones[0][i].emergency >>
                        drones[0][i].battery; std::cin.ignore();

            for (int j = 0; j < drones[0][i].scan_count; j++) {
                old_scans[0][i].push_back(drones[0][i].scans[j]);
            }

            drones[0][i].lights = (drones[0][i].battery < old_battery);
            drones[0][i].scan_count = 0;
            drone_from_id[drones[0][i].id] = &drones[0][i];
        }
        int foe_drone_count;
        std::cin >> foe_drone_count; std::cin.ignore();
        for (int i = 0; i < foe_drone_count; i++) {
            int old_battery = drones[1][i].battery;

            std::cin >> drones[1][i].id >>
                        drones[1][i].pos.x >> 
                        drones[1][i].pos.y >>
                        drones[1][i].emergency >>
                        drones[1][i].battery; std::cin.ignore();


            for (int j = 0; j < drones[1][i].scan_count; j++) {
                old_scans[1][i].push_back(drones[1][i].scans[j]);
            }

            drones[1][i].lights = (drones[1][i].battery < old_battery);
            drones[1][i].scan_count = 0;
            drone_from_id[drones[1][i].id] = &drones[1][i];
        }
        int drone_scan_count;
        std::cin >> drone_scan_count; std::cin.ignore();

        for (int i = 0; i < drone_scan_count; i++) {
            int drone_id;
            int creature_id;
            std::cin >> drone_id >> creature_id;

            Drone *drone = drone_from_id[drone_id];
            Fish *creature = creatures_from_id[creature_id];

            drone->scans[drone->scan_count++] = creature;

            if (drone->id == drones[0][0].id || drone->id == drones[0][1].id) {
                creature->is_scanned = true;
            }

        }
        int visible_creature_count;
        std::cin >> visible_creature_count; std::cin.ignore();
        for (int i = 0; i < visible_creature_count; i++) {
            int creature_id, x, y, vx, vy;
            std::cin >> creature_id >>
                        x >>
                        y >>
                        vx >> 
                        vy; std::cin.ignore();

            Fish *creature = creatures_from_id[creature_id];

            visible_creatures[i] = creature;

            creature->pos = Vector(x, y);
            creature->speed = Vector(vx, vy);
            creature->is_visible = true;

            fish_nets[creature_id].LU = creature->pos;
            fish_nets[creature_id].RD = creature->pos;
        }
        int radar_blip_count;
        std::cin >> radar_blip_count; std::cin.ignore();
        for (int i = 0; i < radar_blip_count; i++) {
            int drone_id;
            int creature_id;
            std::string radar;
            std::cin >> drone_id >> 
                        creature_id >> 
                        radar; std::cin.ignore();
            
            creatures_from_id[creature_id]->in_gamezone = true;

            double drone_x = drone_from_id[drone_id]->pos.x;
            double drone_y = drone_from_id[drone_id]->pos.y;

            if (radar == "TL") {
                fish_nets[creature_id].apply_intersection(
                    Net(Vector(0, 0), Vector(drone_x + 0, drone_y + 0))
                );
            }
            else
            if (radar == "TR") {
                fish_nets[creature_id].apply_intersection(
                    Net(Vector(drone_x + 1, 0), Vector(WIDTH, drone_y + 0))
                );
            }
            else
            if (radar == "BL") {
                fish_nets[creature_id].apply_intersection(
                    Net(Vector(0, drone_y + 1), Vector(drone_x + 0, HEIGHT))
                );
            }
            else
            if (radar == "BR") {
                fish_nets[creature_id].apply_intersection(
                    Net(Vector(drone_x + 1, drone_y + 1), Vector(WIDTH, HEIGHT))
                );
            }
            else {
                assert(false);
            }
        }

        std::cerr << "BLIB: " << radar_blip_count << '\n';

        // new scans by opponent
        for (int drone = 0; drone < 2; drone++) {
            for (int i = 0; i < drones[1][drone].scan_count; i++) {
                Fish *fish = drones[1][drone].scans[i];

                bool is_new = true;
                for (Fish* other_fish : old_scans[1][drone]) {
                    if (fish->id == other_fish->id) {
                        is_new = false;
                    }
                }

                if (is_new) {
                    Vector pos = drones[1][drone].pos;
                    Net drone_border(pos, pos);
                    drone_border.expand(drones[1][drone].lights ? LIGHT_SCAN_RANGE : DARK_SCAN_RANGE);

                    std::cerr << "FISH IN FOE DRONE TERITORY: " << fish->id << ' ' << pos.x << ' ' << pos.y << ' ' << drones[1][drone].lights << '\n';

                    fish_nets[fish->id].apply_intersection(drone_border);
                }
            }
        }

        unvisible_unscanned_creature_count = 0;
        for (int i = 0; i < CREATURE_COUNT; i++) {
            Fish* creature = creatures_from_id[i];
            // std::cerr << creature->id << " => " << fish_nets[creature->id].center().to_string() << '\n';

            if (creature == 0 || creature->isMonster() || creature->is_visible || creature->is_scanned || creature->in_gamezone == false)
                continue;
            
            unvisible_unscanned_creatures[unvisible_unscanned_creature_count++] = creature;

            std::cerr << "UNVISIBLE UNSCANNED: " << creature->id << '\n';
            std::cerr << "         CENTER    : " << fish_nets[creature->id].center().to_string() << '\n';
        }

        int scans_todo = unvisible_unscanned_creature_count;

        std::cerr << "SCANS TODO: " << scans_todo << '\n';
        std::cerr << "unvisible unscanned fishes: " << unvisible_unscanned_creature_count << '\n';

        double best_score = 1e10;
        Vector best_speed0;
        Vector best_speed1;

        const int REP = 400;

        std::array<std::vector<Vector>, 2> possible_moves;

        for (int drone = 0; drone < 2; drone++) {
            for (int angle = 0; angle < REP; angle++) {
                double alpha = angle * TAU / REP;

                drones[0][drone].speed = (Vector(cos(alpha), sin(alpha)).normalize() * DRONE_MOVE_SPEED).round();

                if (inside(drones[0][drone].pos + drones[0][drone].speed) == false) {
                    continue;
                }
                
                bool ok = true;

                for (int i = 0; i < visible_creature_count; i++) {
                    const Fish* monster = visible_creatures[i];

                    if (monster->isMonster() == false)
                        continue;
                    
                    int monster_final_x = monster->pos.x + monster->speed.x;
                    int monster_final_y = monster->pos.y + monster->speed.y;

                    if (monster_final_x <= fish_borders[monster->type].LU.x)
                        monster_final_x = fish_borders[monster->type].LU.x;
                        
                    if (monster_final_x >= fish_borders[monster->type].RD.x)
                        monster_final_x = fish_borders[monster->type].LU.x;
                        
                    if (monster_final_y <= fish_borders[monster->type].LU.y)
                        monster_final_y = fish_borders[monster->type].LU.y;
                        
                    if (monster_final_y >= fish_borders[monster->type].RD.y)
                        monster_final_y = fish_borders[monster->type].RD.y;
                    
                    monster_final_x -= drones[0][drone].pos.x + drones[0][drone].speed.x;
                    monster_final_y -= drones[0][drone].pos.y + drones[0][drone].speed.y;

                    monster_final_x *= monster_final_x;
                    monster_final_y *= monster_final_y;

                    if (getCollision(drones[0][drone], *monster) >= 0 ||
                        monster_final_x + monster_final_y <= 500 * 500) {
                        ok = false;
                        break;
                    }
                }

                if (ok) {
                    possible_moves[drone].push_back(drones[0][drone].speed);
                }
            }
        }

        std::cerr << "# MOVE 0: " << possible_moves[0].size() << '\n';
        std::cerr << "# MOVE 1: " << possible_moves[1].size() << '\n';

        int points_on_raport = my_score;

        for (int color = 0; color < 4; color++) {
            bool all_scanned = true;
            bool one_unreported = false;
            bool foe_unreported = false;

            for (int type = 0; type < 3; type++) {
                Fish* creature = fish_table[color][type];

                if (creature->is_scanned == false) {
                    all_scanned = false;
                }
                
                if (creature->is_reported == false) {
                    one_unreported = true;
                }

                if (creature->is_foe_reported == false) {
                    foe_unreported = true;
                }

                if (creature->is_reported == false && creature->is_scanned) {
                    if (creature->is_foe_reported) {
                        points_on_raport += (creature->type + 1) * 1;
                    }
                    else {
                        points_on_raport += (creature->type + 1) * 2;
                    }
                }
            }

            if (all_scanned && one_unreported) {

                if (foe_unreported) {
                    points_on_raport += 2 * 3;
                }
                else {
                    points_on_raport += 1 * 3;
                }
            }
        }

        for (int type = 0; type < 3; type++) {
            bool all_scanned = true;
            bool one_unreported = false;
            bool foe_unreported = false;

            for (int color = 0; color < 4; color++) {
                Fish* creature = fish_table[color][type];

                if (creature->is_scanned == false) {
                    all_scanned = false;
                }
                
                if (creature->is_reported == false) {
                    one_unreported = true;
                }

                if (creature->is_foe_reported == false) {
                    foe_unreported = true;
                }
            }

            if (all_scanned && one_unreported) {
                if (foe_unreported) {
                    points_on_raport += 2 * 4;
                }
                else {
                    points_on_raport += 1 * 4;
                }
            }
        }

        bool rush_raport = (points_on_raport >= 49 && (drones[0][0].scan_count + drones[0][1].scan_count) > 0);

        // if (points_on_raport > 64) {
        //     int my_speed = std::max(drones[0][0].pos.y, drones[0][1].pos.y) / DRONE_MOVE_SPEED;
        //     int op_speed = std::min(drones[1][0].pos.y, drones[1][1].pos.y) / DRONE_MOVE_SPEED;
        
        //     if (my_speed <= op_speed) {
        //         rush_raport = true;
        //     }
        // }

        for (const Vector &speed0 : possible_moves[0]) {
            drones[0][0].speed = speed0;
            for (const Vector &speed1 : possible_moves[1]) {
                drones[0][1].speed = speed1;

                double score = 0;

                if (scans_todo == 0) {
                    score += (drones[0][0].pos.y + drones[0][0].speed.y - 495);
                    score += (drones[0][1].pos.y + drones[0][1].speed.y - 495);
                }
                else {
                    if (rush_raport && drones[0][0].scan_count > 0) {
                        score += fabs(drones[0][0].pos.y + drones[0][0].speed.y - 495);
                    }

                    if (rush_raport && drones[0][1].scan_count > 0) {
                        score += fabs(drones[0][1].pos.y + drones[0][1].speed.y - 495);
                    }

                    for (int i = 0; i < unvisible_unscanned_creature_count; i++) {
                        const Fish *creature = unvisible_unscanned_creatures[i];

                        double d1 = (drones[0][0].pos + drones[0][0].speed).distance(fish_nets[creature->id].center()) / 100;
                        double d2 = (drones[0][1].pos + drones[0][1].speed).distance(fish_nets[creature->id].center()) / 100;
                        double dd = 2000;

                        if (rush_raport == false) {
                            dd = std::min(d1, d2);
                        }

                        double weight = creature->type + 1;

                        if (creature->is_foe_reported == false) {
                            if (creature->is_foe_scanned == false) {
                                weight *= 4;
                            }
                            else {
                                weight *= 2.5;
                            }
                        }

                        // uncertanity lines in [0, log(2500 * 10000)] = [0, 17.034] -> get easy fishes early
                        // double uncertanity = (1 - (log(fish_nets[creature->id].getArea()) / 18));

                        score += dd * dd * weight + (d1 + d2 - dd) / 1000;
                    }

                    // for (int i = 0; i < monster_count; i++) {
                    //     double d1 = (drones[0][0].pos + drones[0][0].speed).distance(fish_nets[monsters[i]->id].center()) / 3000;
                    //     double d2 = (drones[0][1].pos + drones[0][1].speed).distance(fish_nets[monsters[i]->id].center()) / 3000;
                    //     double p = 1 - log(fish_nets[monsters[i]->id].getArea()) / 20;

                    //     score += d1 * p;
                    //     score += d2 * p;
                    // }

                    if (game_turn < 13) {
                        score += (drones[0][0].speed.y + drones[0][1].speed.y) / 100;
                    }
                }

                if (best_score > score) {
                    best_score = score;
                    best_speed0 = drones[0][0].speed;
                    best_speed1 = drones[0][1].speed;
                }
            }
        }

        std::cerr << "SCORE: " << best_score << '\n';
        std::cerr << "SCANS0: " << drones[0][0].scan_count << '\n';
        std::cerr << "SCANS1: " << drones[0][1].scan_count << '\n';

        drones[0][0].move = "MOVE " + std::to_string(int(drones[0][0].pos.x + best_speed0.x)) + ' ' + std::to_string(int(drones[0][0].pos.y + best_speed0.y));
        drones[0][1].move = "MOVE " + std::to_string(int(drones[0][1].pos.x + best_speed1.x)) + ' ' + std::to_string(int(drones[0][1].pos.y + best_speed1.y));

        for (int drone = 0; drone < 2; drone++) {
            bool unscanned_fish_in_light_radius = false;

            for (int i = 0; i < CREATURE_COUNT; i++) {
                Fish *creature = creatures_from_id[i];

                if (creature == 0 || 
                    creature->isMonster() || 
                    creature->is_visible || 
                    creature->is_scanned || 
                    creature->is_reported || 
                    creature->in_gamezone == false)
                    continue;

                if (fish_nets[creature->id].inRange(drones[0][0].pos, 2000) ||
                    fish_nets[creature->id].inRange(drones[0][1].pos, 2000)) {
                    unscanned_fish_in_light_radius = true;
                }
            }

            if (game_turn > 5 && unscanned_fish_in_light_radius) {
                drones[0][drone].move += " 1";
                last_light[drone] = game_turn;
            }
            else {
                drones[0][drone].move += " 0";
            }
        }

        // UPDATING FISH NETS
        for (int i = 0; i < CREATURE_COUNT; i++) {
            Fish* creature = creatures_from_id[i];

            if (creature == 0)
                continue;
            
            if (creature->isMonster()) {
                bool aggressiveMode = false;
                
                for (int ii = 0; ii < 2; ii++) {
                    for (int jj = 0; jj < 2; jj++) {
                        if (fish_nets[creature->id].inRange(drones[ii][jj].pos, drones[ii][jj].lights ? LIGHT_SCAN_RANGE : DARK_SCAN_RANGE)) {
                            aggressiveMode = true;
                        }
                    }
                }

                if (aggressiveMode) {
                    // std::cerr << "MONSTER: " << i << " => AGGRESSIVE MODE\n";
                    fish_nets[creature->id].expand(540);
                }
                else {
                    // std::cerr << "MONSTER: " << i << " => NORMAL MODE\n";
                    fish_nets[creature->id].expand(270);
                }
            }
            else {
                bool frightenedMode = false;

                for (int ii = 0; ii < 2; ii++) {
                    for (int jj = 0; jj < 2; jj++) {
                        if (fish_nets[creature->id].inRange(drones[ii][jj].pos, 1400 - 1E-9)) {
                            frightenedMode = true;
                        }
                    }
                }

                if (frightenedMode) {
                    // std::cerr << "FISH: " << i << " => FRIGHTENED MODE\n";
                    fish_nets[creature->id].expand(400);
                }
                else {
                    // std::cerr << "FISH: " << i << " => NORMAL MODE\n";
                    fish_nets[creature->id].expand(200);
                }
            }
        }

        for (int i = 0; i < visible_creature_count; i++) {
            int creature_id = visible_creatures[i]->id;

            Vector next_pos = visible_creatures[i]->pos + visible_creatures[i]->speed;
            int fish_type = creatures_from_id[creature_id]->type;

            // snap to fish zone
            if (next_pos.y > HEIGHT - 1) {
                next_pos.y = HEIGHT - 1;
            }
            else
            if (next_pos.y > fish_borders[fish_type].RD.y) {
                next_pos.y = fish_borders[fish_type].RD.y;
            }
            else
            if (next_pos.y < fish_borders[fish_type].LU.y) {
                next_pos.y = fish_borders[fish_type].LU.y;
            }

            if (next_pos.x < 0) {
                next_pos.x = 0;
            }

            if (next_pos.x > WIDTH) {
                next_pos.x = WIDTH;
            }

            fish_nets[creature_id] = Net(next_pos, next_pos);
        }

        for (int i = 0; i < CREATURE_COUNT; i++) {
            Fish* creature = creatures_from_id[i];

            if (creature == 0)
                continue;
            
            // std::cerr << "ID    : " << creature->id << '\n';
            // std::cerr << "OLD   : " << fish_nets[creature->id].LU.to_string() << ' ' << fish_nets[creature->id].RD.to_string() << '\n';
            // std::cerr << "BORDER: " << fish_borders[creature->type].LU.to_string() << ' ' << fish_borders[creature->type].RD.to_string() << '\n';
            fish_nets[creature->id].apply_intersection(fish_borders[creature->type]);
        }

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        drones[0][0].msg = std::to_string(points_on_raport) + " | " + std::to_string(best_score);
        drones[0][1].msg = std::string(rush_raport ? "📈" : "📉") + " | " + std::to_string(duration.count()) + "ms";

        for (int i = 0; i < my_drone_count; i++) {
            std::cout << drones[0][i].move << ' ' << drones[0][i].msg << std::endl;
        }

        for (int i = 0; i < CREATURE_COUNT; i++) {
            Fish* creature = creatures_from_id[i];

            if (creature == 0)
                continue;
            
            creature->is_visible = false;
            creature->in_gamezone = false;
            creature->is_scanned = false;
            creature->is_foe_scanned = false;
        }
    }
}