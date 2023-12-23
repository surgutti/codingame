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

int monsters_count;
Fish* monsters_list[CREATURE_COUNT];

Drone drones[PLAYERS][DRONES_PER_PLAYER];
Drone* drone_from_id[CREATURE_COUNT];

Fish visible_creatures[CREATURE_COUNT];

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

    for (;;) {
        int my_score;
        std::cin >> my_score; std::cin.ignore();
        int foe_score;
        std::cin >> foe_score; std::cin.ignore();
        int my_scan_count;
        std::cin >> my_scan_count; std::cin.ignore();
        for (int i = 0; i < my_scan_count; i++) {
            int creature_id;
            std::cin >> creature_id; std::cin.ignore();
        }
        int foe_scan_count;
        std::cin >> foe_scan_count; std::cin.ignore();
        for (int i = 0; i < foe_scan_count; i++) {
            int creature_id;
            std::cin >> creature_id; std::cin.ignore();
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

            std::cerr << "SCAN: " << drone_id << ' ' << creature_id << '\n';
        }
        int visible_creature_count;
        std::cin >> visible_creature_count; std::cin.ignore();
        for (int i = 0; i < visible_creature_count; i++) {
            std::cin >> visible_creatures[i].id >>
                        visible_creatures[i].pos.x >>
                        visible_creatures[i].pos.y >>
                        visible_creatures[i].speed.x >>
                        visible_creatures[i].speed.y; std::cin.ignore();
            
            std::cerr << "I SEE:\n";
            std::cerr << " ID   : " << visible_creatures[i].id << '\n';
            std::cerr << " POS  : " << visible_creatures[i].pos.to_string() << '\n';
            std::cerr << " SPEED: " << visible_creatures[i].speed.to_string() << '\n';
            std::cerr << "---\n";

            Fish *creature = creatures_from_id[visible_creatures[i].id];

            creature->pos = visible_creatures[i].pos;
            creature->speed = visible_creatures[i].speed;

            // check whether fish net is exact
            if(fish_nets[visible_creatures[i].id].inside(visible_creatures[i].pos) == false) {
                std::cerr << "FISH: " << visible_creatures[i].id << '\n';
                std::cerr << "POS: " << visible_creatures[i].pos.to_string() << '\n';
                std::cerr << "SPEED: " << visible_creatures[i].speed.to_string() << '\n';
                std::cerr << "NET: " << fish_nets[visible_creatures[i].id].LU.to_string() << ' ' << fish_nets[visible_creatures[i].id].RD.to_string() << '\n';
                std::cerr << "FAILED!!!\n";
                assert(false);
            }

            fish_nets[visible_creatures[i].id].LU = visible_creatures[i].pos;
            fish_nets[visible_creatures[i].id].RD = visible_creatures[i].pos;
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

        for (int i = 0; i < CREATURE_COUNT; i++) {
            Fish* creature = creatures_from_id[i];

            if (creature == 0)
                continue;
            
            // std::cerr << creature->id << " => " << fish_nets[creature->id].LU.to_string() << ' ' << fish_nets[creature->id].RD.to_string() << '\n';
            std::cerr << creature->id << " => " << fish_nets[creature->id].center().to_string() << '\n';
        }

        double best_score = 1e9;
        Vector best_speed0;
        Vector best_speed1;

        const int REP = 300;

        int scans_todo = - my_scan_count;

        for (int i = 0; i < CREATURE_COUNT; i++) {
            const Fish* creature = creatures_from_id[i];

            if (creature == 0 || creature->type == -1) {
                continue;
            }

            bool done = false;
            for (int drone = 0; drone < 2; drone++) {
                for (int j = 0; j < drones[0][drone].scan_count; j++) {
                    if (drones[0][drone].scans[j]->id == creature->id) {
                        done = true;
                    }
                }
            }

            if (done == false) {
                scans_todo++;
            }
        }

        std::cerr << "SCANS TODO: " << scans_todo << '\n';

        for (int angle0 = 0; angle0 < REP; angle0++) {
            drones[0][0].speed = (Vector(cos(angle0 * TAU / REP), sin(angle0 * TAU / REP)) * DRONE_MOVE_SPEED).round();
            
            // std::cerr << "speed0: " << drones[0][0].speed.to_string() << '\n';
            for (int angle1 = 0; angle1 < REP; angle1++) {
                drones[0][1].speed = (Vector(cos(angle1 * TAU / REP), sin(angle1 * TAU / REP)) * DRONE_MOVE_SPEED).round();

                bool ok = true;
                
                if (inside(drones[0][0].pos + drones[0][0].speed) == false ||
                    inside(drones[0][1].pos + drones[0][1].speed) == false) {
                    ok = false;
                }

                for (int i = 0; i < visible_creature_count; i++) {
                    const Fish *monster = creatures_from_id[visible_creatures[i].id];

                    if (monster->type != -1)
                        continue;

                    if (getCollision(drones[0][0], *monster) >= 0 ||
                        getCollision(drones[0][1], *monster) >= 0) {
                        ok = false;
                        break;
                    }
                }

                if (ok) {
                    double score = 0;

                    if (scans_todo == 0) {
                        score += (drones[0][0].pos.y + drones[0][0].speed.y - 495) * HEIGHT;
                        score += (drones[0][1].pos.y + drones[0][1].speed.y - 495) * HEIGHT;
                    }
                    else {
                        if (drones[0][0].scan_count >= 4) {
                            score += (drones[0][0].pos.y + drones[0][0].speed.y - 495) * HEIGHT;
                        }

                        if (drones[0][1].scan_count >= 4) {
                            score += (drones[0][1].pos.y + drones[0][1].speed.y - 495) * HEIGHT;
                        }

                        for (int i = 0; i < CREATURE_COUNT; i++) {
                            const Fish *creature = creatures_from_id[i];

                            if (creature == 0 || creature->type == -1)
                                continue;

                            bool is_scanned = false;

                            for (int k = 0; k < 2; k++)
                                for (int j = 0; j < drones[0][k].scan_count; j++)
                                    if (drones[0][k].scans[j]->id == creature->id)
                                        is_scanned = true;

                            if (is_scanned)
                                continue;

                            double d1 = (drones[0][0].pos + drones[0][0].speed).distance(fish_nets[creature->id].center());
                            double d2 = (drones[0][1].pos + drones[0][1].speed).distance(fish_nets[creature->id].center());
                            double dd = 20000;

                            if (drones[0][0].scan_count < 4) {
                                dd = std::min(dd, d1);
                            }

                            if (drones[0][1].scan_count < 4) {
                                dd = std::min(dd, d2);
                            }

                            score += dd;
                        }
                    }

                    if (best_score > score) {
                        best_score = score;
                        best_speed0 = drones[0][0].speed;
                        best_speed1 = drones[0][1].speed;

                        // std::cerr << "NEW SCORE: " << best_score << '\n';
                    }
                }
            }
        }

        std::cerr << "SCORE: " << best_score << '\n';

        drones[0][0].move = "MOVE " + std::to_string(int(drones[0][0].pos.x + best_speed0.x)) + ' ' + std::to_string(int(drones[0][0].pos.y + best_speed0.y)) + " 1";
        drones[0][1].move = "MOVE " + std::to_string(int(drones[0][1].pos.x + best_speed1.x)) + ' ' + std::to_string(int(drones[0][1].pos.y + best_speed1.y)) + " 1";

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
                    std::cerr << "MONSTER: " << i << " => AGGRESSIVE MODE\n";
                    fish_nets[creature->id].expand(540);
                }
                else {
                    std::cerr << "MONSTER: " << i << " => NORMAL MODE\n";
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
                    std::cerr << "FISH: " << i << " => FRIGHTENED MODE\n";
                    fish_nets[creature->id].expand(400);
                }
                else {
                    std::cerr << "FISH: " << i << " => NORMAL MODE\n";
                    fish_nets[creature->id].expand(200);
                }
            }
        }

        for (int i = 0; i < visible_creature_count; i++) {
            int creature_id = visible_creatures[i].id;

            Vector next_pos = visible_creatures[i].pos + visible_creatures[i].speed;
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
            
            std::cerr << "ID    : " << creature->id << '\n';
            std::cerr << "OLD   : " << fish_nets[creature->id].LU.to_string() << ' ' << fish_nets[creature->id].RD.to_string() << '\n';
            std::cerr << "BORDER: " << fish_borders[creature->type].LU.to_string() << ' ' << fish_borders[creature->type].RD.to_string() << '\n';
            fish_nets[creature->id].apply_intersection(fish_borders[creature->type]);
        }

        for (int i = 0; i < my_drone_count; i++) {
            std::cout << drones[0][i].move << std::endl;
        }
    }
}
// *** End of: /home/olaf/codingame/main.cpp *** 
