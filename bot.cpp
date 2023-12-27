// *** Start of: /home/olaf/codingame/main.cpp *** 
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

 // *** Start of: /home/olaf/codingame/fish.h *** 
 #ifndef FISH_H
 #define FISH_H
 
  // *** Start of: /home/olaf/codingame/vector.h *** 
  #ifndef VECTOR_H
  #define VECTOR_H
  
  #include <cmath>
  #include <string>
  
  struct Vector {
      double x;
      double y;
  
      Vector(double _x = 0, double _y = 0) :
          x(_x), y(_y) {
  
      }
  
      Vector round() const {
          return Vector(
              (int) ::round(this->x), 
              (int) ::round(this->y)
          );
      }
  
      bool isZero() const {
          return x == 0 && y == 0;
      }
      
      Vector normalize() {
          double length = sqrt(x * x + y * y);
          if (length == 0)
              return Vector(0, 0);
          return Vector(x / length, y / length);
      }
  
      bool inRange(const Vector &v, double range) const {
          return (v.x - x) * (v.x - x) + (v.y - y) * (v.y - y) <= range * range;
      }
  
      double distance(const Vector &other) const {
          return sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
      }
  
      Vector operator+ (const Vector &other) const {
          return Vector(x + other.x, y + other.y);
      }
  
      Vector operator* (const double &scalar) const {
          return Vector(x * scalar, y * scalar);
      }
  
      std::string to_string() const {
          return std::to_string(x) + ',' + std::to_string(y);
      }
  };
  
  #endif // VECTOR_H
  // *** End of: /home/olaf/codingame/vector.h *** 
 
 struct Fish {
     Vector pos;
     Vector speed;
     int id;
     int type;
     int color;
     bool is_visible;
     bool is_scanned;
     bool is_foe_scanned;
     bool in_gamezone;
     bool is_reported;
     bool is_foe_reported;
 
     bool isMonster() const {
         return type == -1;
     }
 };
 
 #endif // FISH_H
 // *** End of: /home/olaf/codingame/fish.h *** 
 // *** Start of: /home/olaf/codingame/drone.h *** 
 #ifndef DRONE_H
 #define DRONE_H
 
 #include <string>
 
  // *** Start of: /home/olaf/codingame/const.h *** 
  #ifndef CONST_H
  #define CONST_H
  
  #include <cmath>
  
  int gameTurn;
  const int WIDTH = 10000;
  const int HEIGHT = 10000;
  
  const int DRONES_PER_PLAYER = 2;
  
  const int UGLY_UPPER_Y_LIMIT = 2500;
  const int DRONE_UPPER_Y_LIMIT = 0;
  const int DRONE_START_Y = 500;
  
  const int COLORS_PER_FISH = 4;
  const int DRONE_MAX_BATTERY = 30;
  const int LIGHT_BATTERY_COST = 5;
  const int DRONE_BATTERY_REGEN = 1;
  const int DRONE_MAX_SCANS = 1000; // Integer.MAX_VALUE;
  
  const int DARK_SCAN_RANGE = 800;
  const int LIGHT_SCAN_RANGE = 2000;
  const int UGLY_EAT_RANGE = 300;
  const int DRONE_HIT_RANGE = 200;
  const int FISH_HEARING_RANGE = (DARK_SCAN_RANGE + LIGHT_SCAN_RANGE) / 2;
  
  const int DRONE_MOVE_SPEED = 600;
  const int DRONE_SINK_SPEED = 300;
  const int DRONE_EMERGENCY_SPEED = 300;
  double DRONE_MOVE_SPEED_LOSS_PER_SCAN = 0;
  
  const int FISH_SWIM_SPEED = 200;
  const int FISH_AVOID_RANGE = 600;
  const int FISH_FLEE_SPEED = 400;
  const int UGLY_ATTACK_SPEED = (int) (DRONE_MOVE_SPEED * 0.9);
  const int UGLY_SEARCH_SPEED = (int) (UGLY_ATTACK_SPEED / 2);
  
  const int FISH_X_SPAWN_LIMIT = 1000;
  const int FISH_SPAWN_MIN_SEP = 1000;
  
  const Vector CENTER((WIDTH - 1) / 2.0, (HEIGHT - 1) / 2.0);
  
  const int MAX_TURNS = 201;
  
  // BOT CONST
  
  const int CREATURE_COUNT = 30;
  
  const double PI = acos(-1);
  const double TAU = 2 * PI;
  
  #endif // CONST_H
  // *** End of: /home/olaf/codingame/const.h *** 
 
 struct Drone {
     Vector pos;
     Vector speed;
     int id;
     int emergency;
     int battery;
     
     bool lights; // was light ON on the last turn?
 
     int scan_count;
     Fish* scans[CREATURE_COUNT];
     
     std::string move;
     std::string msg;
 };
 
 #endif // DRONE_H
 // *** End of: /home/olaf/codingame/drone.h *** 
 // *** Start of: /home/olaf/codingame/net.h *** 
 #ifndef NET_H
 #define NET_H
 
 #include <algorithm>
 
 
 struct Net {
     Vector LU; // left up
     Vector RD; // right down
 
     Net() {
 
     }
 
     Net(const Vector &_LU, const Vector &_RD) :
         LU(_LU), RD(_RD) {
         assert(LU.x <= RD.x);
         assert(LU.y <= RD.y);
     }
 
     bool inside(const Vector &point) const {
         return LU.x <= point.x && point.x <= RD.x &&
                LU.y <= point.y && point.y <= RD.y;
     }
 
     Net intersect(const Net &other) const {
         Net result;
 
         result.LU.x = std::max(LU.x, other.LU.x);
         result.LU.y = std::max(LU.y, other.LU.y);
 
         result.RD.x = std::min(RD.x, other.RD.x);
         result.RD.y = std::min(RD.y, other.RD.y);
 
         assert(result.LU.x <= result.RD.x);
         assert(result.LU.y <= result.RD.y);
 
         return result;
     }
     
     void apply_intersection(const Net &other) {
         (*this) = this->intersect(other);
     }
 
     Vector center() const {
         return Vector((LU.x + RD.x) * 0.5, (LU.y + RD.y) * 0.5);
     }
 
     void expand(double range) {
         LU.x -= range;
         LU.y -= range;
         RD.x += range;
         RD.y += range;
     }
 
     bool inRange(const Vector &point, double range) const {
         return LU.x - range <= point.x && point.x <= RD.x + range &&
                LU.y - range <= point.y && point.y <= RD.y + range;
     }
 };
 
 #endif // NET_H
 // *** End of: /home/olaf/codingame/net.h *** 
 // *** Start of: /home/olaf/codingame/collision.h *** 
 #ifndef COLLISION_H
 #define COLLISION_H
 
 #include <cmath>
 
 
 double getCollision(Drone drone, Fish ugly) {
     // Check instant collision
     if (ugly.pos.inRange(drone.pos, DRONE_HIT_RANGE + UGLY_EAT_RANGE)) {
         return 0.0;
     }
 
     // Both units are motionless
     if (drone.speed.isZero() && ugly.speed.isZero()) {
         return -1.0;
     }
 
     // Change referencial
     double x = ugly.pos.x;
     double y = ugly.pos.y;
     double ux = drone.pos.x;
     double uy = drone.pos.y;
 
     double x2 = x - ux;
     double y2 = y - uy;
     double r2 = UGLY_EAT_RANGE + DRONE_HIT_RANGE;
     double vx2 = ugly.speed.x - drone.speed.x;
     double vy2 = ugly.speed.y - drone.speed.y;
 
     // Resolving: sqrt((x + t*vx)^2 + (y + t*vy)^2) = radius <=> t^2*(vx^2 + vy^2) + t*2*(x*vx + y*vy) + x^2 + y^2 - radius^2 = 0
     // at^2 + bt + c = 0;
     // a = vx^2 + vy^2
     // b = 2*(x*vx + y*vy)
     // c = x^2 + y^2 - radius^2 
 
     double a = vx2 * vx2 + vy2 * vy2;
 
     if (a <= 0.0) {
         return -1.0;
     }
 
     double b = 2.0 * (x2 * vx2 + y2 * vy2);
     double c = x2 * x2 + y2 * y2 - r2 * r2;
     double delta = b * b - 4.0 * a * c;
 
     if (delta < 0.0) {
         return -1.0;
     }
 
     double t = (-b - sqrt(delta)) / (2.0 * a);
 
     if (t <= 0.0) {
         return -1.0;
     }
 
     if (t > 1.0) {
         return -1.0;
     }
     return t;
 }
 
 
 #endif // COLLISION_H
 // *** End of: /home/olaf/codingame/collision.h *** 

const int PLAYERS = 2;

int creature_count;
Fish creatures[CREATURE_COUNT];
Fish* creatures_from_id[CREATURE_COUNT];

Fish* fish_color_type_table[4][3];

int monsters_count;
Fish* monsters_list[CREATURE_COUNT];

Drone drones[PLAYERS][DRONES_PER_PLAYER];
Drone* drone_from_id[CREATURE_COUNT];

Fish* visible_creatures[CREATURE_COUNT];

int unvisible_unscanned_creature_count;
Fish* unvisible_unscanned_creatures[CREATURE_COUNT];

Net fish_nets[CREATURE_COUNT];

const Net fish_borders[4] = {
    Net(Vector(0, 2500), Vector(WIDTH, 10000)),
    Net(Vector(0, 2500), Vector(WIDTH, 5000)),
    Net(Vector(0, 5000), Vector(WIDTH, 7500)),
    Net(Vector(0, 7500), Vector(WIDTH, 10000))
};
#define fish_borders (fish_borders + 1) // -1 indexed

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
            fish_color_type_table[creatures[i].color][creatures[i].type] = &creatures[i];
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

    for (int player = 0; player < 2; player++)
        for (int drone = 0; drone < 2; drone++)
            drones[player][drone].battery = 30;

    for (int game_turn = 0; ; game_turn++) {
        int my_score;
        std::cin >> my_score; std::cin.ignore();
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
        for (int i = 0; i < my_drone_count; i++) {
            int old_battery = drones[0][i].battery;

            std::cin >> drones[0][i].id >>
                        drones[0][i].pos.x >> 
                        drones[0][i].pos.y >>
                        drones[0][i].emergency >>
                        drones[0][i].battery; std::cin.ignore();

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

            // std::cerr << "SCAN: " << drone_id << ' ' << creature_id << '\n';
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
             
            // std::cerr << "I SEE:\n";
            // std::cerr << " ID   : " << creature_id << '\n';
            // std::cerr << " POS  : " << Vector(x, y).to_string() << '\n';
            // std::cerr << " SPEED: " << Vector(vx, vy).to_string() << '\n';
            // std::cerr << "---\n";

            Fish *creature = creatures_from_id[creature_id];

            visible_creatures[i] = creature;

            creature->pos = Vector(x, y);
            creature->speed = Vector(vx, vy);
            creature->is_visible = true;

            // if (fish_nets[creature_id].inside(creature->pos) == false) {
            //     std::cerr << "FISH: " << creature->id << '\n';
            //     std::cerr << "POS: " << creature->pos.to_string() << '\n';
            //     std::cerr << "SPEED: " << creature->speed.to_string() << '\n';
            //     std::cerr << "NET: " << fish_nets[creature_id].LU.to_string() << ' ' << fish_nets[creature_id].RD.to_string() << '\n';
            //     std::cerr << "FAILED!!!\n";
            //     assert(false);
            // }

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

            // std::cerr << creature_id << ' ' << drone_id << " x: " << drone_x << " y: " << drone_y << " => " << radar << std::endl;

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

        const int REP = 290;

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
                    
                    if (getCollision(drones[0][drone], *monster) >= 0) {
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
                Fish* creature = fish_color_type_table[color][type];

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
                Fish* creature = fish_color_type_table[color][type];

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

        bool rush_raport = (points_on_raport >= 48 && (drones[0][0].scan_count + drones[0][1].scan_count) > 0);

        drones[0][0].msg = std::to_string(points_on_raport);
        drones[0][1].msg = (rush_raport ? "📈" : "📉");

        for (const Vector &speed0 : possible_moves[0]) {
            drones[0][0].speed = speed0;
            for (const Vector &speed1 : possible_moves[1]) {
                drones[0][1].speed = speed1;

                double score = 0;

                if (scans_todo == 0) {
                    score += (drones[0][0].pos.y + drones[0][0].speed.y - 495) * HEIGHT;
                    score += (drones[0][1].pos.y + drones[0][1].speed.y - 495) * HEIGHT;
                }
                else {
                    if (rush_raport && drones[0][0].scan_count > 0) {
                        score += fabs(drones[0][0].pos.y + drones[0][0].speed.y - 495) * HEIGHT;
                    }

                    if (rush_raport && drones[0][1].scan_count > 0) {
                        score += fabs(drones[0][1].pos.y + drones[0][1].speed.y - 495) * HEIGHT;
                    }

                    for (int i = 0; i < unvisible_unscanned_creature_count; i++) {
                        const Fish *creature = unvisible_unscanned_creatures[i];

                        double d1 = (drones[0][0].pos + drones[0][0].speed).distance(fish_nets[creature->id].center()) / 1000;
                        double d2 = (drones[0][1].pos + drones[0][1].speed).distance(fish_nets[creature->id].center()) / 1000;
                        double dd = 2000;

                        if (rush_raport == false) {
                            dd = std::min(dd, d1);
                        }

                        if (rush_raport == false) {
                            dd = std::min(dd, d2);
                        }

                        double weight = creature->type + 1;
                        if (creature->is_foe_scanned == false)
                            weight *= 2;

                        score += dd * dd * weight + (d1 + d2 - dd) / 1000;
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

        bool unscanned_fish_in_light_radius = false;

        for (int i = 0; i < CREATURE_COUNT; i++) {
            Fish *creature = creatures_from_id[i];

            if (creature == 0 || creature->isMonster() || creature->is_visible || creature->is_scanned || creature->in_gamezone == false)
                continue;

            if (fish_nets[creature->id].inRange(drones[0][0].pos, 2000) ||
                fish_nets[creature->id].inRange(drones[0][1].pos, 2000)) {
                unscanned_fish_in_light_radius = true;
            }
        }

        if (game_turn > 5 && unscanned_fish_in_light_radius) {
            drones[0][0].move += " 1";
            drones[0][1].move += " 1";
        }
        else {
            drones[0][0].move += " 0";
            drones[0][1].move += " 0";
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
                    fish_nets[creature->id].expand(/*270*/127);
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
                    fish_nets[creature->id].expand(/*400*/190);
                }
                else {
                    // std::cerr << "FISH: " << i << " => NORMAL MODE\n";
                    fish_nets[creature->id].expand(/*200*/90);
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
// *** End of: /home/olaf/codingame/main.cpp *** 
