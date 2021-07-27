#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>
#include <ncurses.h>
#include <string>

int win_score = 3;
int game_length = 250;
int game_speed = 400;

class Player{
    double x;
    double y;
    double o = 0;
    int helicity;

public:
    const char *icon;
    char control;
    bool change = false;
    double speed = 1.0;
    int score = 0;

    void setx(double x){
        this->x = x;
    }
    void sety(double y){
        this->y = y;
    }
    void seto(double o){
        this->o = o;
    }
    void setx(int x){
        this->x = (double) x;
    }
    void sety(int y){
        this->y = (double) y;
    }
    void seto(int o){
        this->o = (double) o;
    }
    double getx(){
        return x;
    }
    double gety(){
        return y;
    }
    double geto(){
        return o;
    }
    int getintx(){
        return std::round(x);
    }
    int getinty(){
        return std::round(y);
    }
    int getinto(){
        return std::round(o);
    }

    Player(){
        helicity = get_plus_minus();
    }

    Player(double x, double y){
        setx(x);
        sety(y);
        helicity = get_plus_minus();
    }
    Player(int x, int y){
        setx(x);
        sety(y);
        helicity = get_plus_minus();
    }
    Player(double x, double y, double o){
        setx(x);
        sety(y);
        seto(o);
        helicity = get_plus_minus();
    }
    Player(int x, int y, double o){
        setx(x);
        sety(y);
        seto(o);
        helicity = get_plus_minus();
    }

    void switch_on_change(){
        change = true;
    }

    void step(){
        x += sin(o)*speed;
        y += cos(o)*speed;
    }
    int get_plus_minus(){
        unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
        std::default_random_engine e(seed);
        int i;
        i = e();
        if (i % 2 == 0){
            return 1;
        } else {
            return -1;
        }
    }


    void increment_o(){
        if (change){
            o += (M_PI/2 * helicity);
            change = false;
        }
    }

};

class Competitors{
public:
    Competitors() = default;
    int player_count = 0;
    const static int max_players = 2;
    //Player *field = new Player[max_players];
    Player * field[max_players];
    void add_player(Player &p){
        if (player_count < max_players) {
            field[player_count] = &p;
            player_count ++;
        } else {
            std::cout << "Game is full!" << std::endl;
        }

    }


    void step_all_players() const{
        for(int i = 0; i < player_count; i ++){
            field[i]->increment_o();
            field[i]->step();
        }
    }

};

class Target{
public:
    int x;
    int y;
    int screen_x, screen_y;
    const char *const icon = "T";
    std::default_random_engine e;
    Target(int y, int x){
        screen_x = x;
        screen_y = y;
        unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
        std::default_random_engine e(seed);
        this->x = e() % (screen_x - 2) + 1;
        this->y = e() % (screen_y - 2) + 1;
    }
    void reset_target(){
        unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
        std::default_random_engine e(seed);
        x = e() % (screen_x - 2) + 1;
        y = e() % (screen_y - 2) + 1;
    }

};

bool referee(Competitors c, Target * t, int x_max, int y_max, WINDOW *w){
    for (int i = 0; i < c.player_count; i++){
        int xp = c.field[i]->getintx();
        int yp = c.field[i]->getinty();
        if (xp <=0 || xp >= x_max || yp <=0 || yp >= y_max){
            mvwprintw(w, y_max -2, x_max/3, reinterpret_cast<const char *>(c.field[i]->icon));
            mvwprintw(w, y_max -2, x_max/3 + 1, " Loses!!");
            wrefresh(w);
            return false;
        }
        if (xp == t->x && yp == t->y){
            c.field[i]->score += 1;
            mvwprintw(w, y_max -3, x_max/(i + 1), reinterpret_cast<const char *>(c.field[i]->icon));
            mvwprintw(w, y_max -3, x_max/(i + 1) + 1, " Scored!!");
            t->reset_target();
            wrefresh(w);
            if (c.field[i]->score >= win_score){
                mvwprintw(w, y_max -2, x_max/3, reinterpret_cast<const char *>(c.field[i]->icon));
                mvwprintw(w, y_max -2, x_max/3 + 1, " Wins!!");
                wrefresh(w);
                return false;
            }
        }
    }
    return true;
}


void cprinter(Competitors c, Target t, WINDOW *w){
    for (int i = 0; i < c.player_count; i++){
        mvwprintw(w, c.field[i]->getinty(), c.field[i]->getintx(), c.field[i]->icon);
    }
    mvwprintw(w, t.y, t.x, t.icon);
    wrefresh(w);
}


void player_input(Competitors c, bool * game_continues){
    while(*game_continues){
        char d;
        d = getch();
        for (int i = 0; i < c.player_count; i++){

            if (d == c.field[i]->control){
                c.field[i]->switch_on_change();
            }
        }
    }
}


int main() {
    initscr();
    noecho();
    curs_set(0);
    int start_y, start_x;
    start_y = start_x = 0;
    WINDOW * introwin = newwin(10, 60, start_y, start_x);
    for (int i = 0; i < 5; i++){
        mvwprintw(introwin, 0,0, "Game will start in %d, adjust screen now.", 5-i);
        wrefresh(introwin);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    werase(introwin);
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);
    mvwprintw(introwin, 0, 0, "Game starting, good luck!!");
    wrefresh(introwin);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    timeout(0);
    int x;
    int y;
    int o;
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::default_random_engine e(seed);

    x = e() % (maxx - 8) + 4;
    y = e() % (maxy - 8) + 4;
    o = e();
    Player p1(x, y, o*M_PI/2);
    x = e() % (maxx - 8) + 8;
    y = e() % (maxy - 8) + 4;
    o = e();
    Player p2(x, y, o*M_PI/2);
    Competitors ThesePlayers = Competitors();
    p1.icon = "A";
    p1.control = 'a';
    p2.icon = "C";
    p2.control = 'c';
    ThesePlayers.add_player(p1);
    ThesePlayers.add_player(p2);
    bool game_continues = true;
    std::thread p1in(player_input, std::ref(ThesePlayers), &game_continues);
    int frame_count = 0;
    Target CurrentTarget = Target(maxy, maxx);
    WINDOW * win = newwin(maxy, maxx, start_y, start_x);
    werase(win);
    refresh();
    char wall = '|';
    char roof = '-';
    box(win, (int)wall, (int)roof);
    wrefresh(win);
    while(game_continues){
        werase(win);
        box(win, (int)wall, (int)roof);
        cprinter(ThesePlayers, CurrentTarget, win);

        ThesePlayers.step_all_players();

        game_continues = referee(ThesePlayers, &CurrentTarget, maxx, maxy, win);
        frame_count += 1;

        if (frame_count >= game_length){
            game_continues = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(game_speed));
        if (game_continues == 0){
            break;
        }
    }
    p1in.join();

    for (int i = 0; i < ThesePlayers.player_count; i ++){
        mvwprintw(win, maxy/2 + i, maxx/3, "Final score of ");
        wprintw(win, ThesePlayers.field[i]->icon);
        wprintw(win, " is %d", ThesePlayers.field[i]->score);
    }
    wrefresh(win);
    timeout(5000);
    getch();
    mvwprintw(win, 1, 2, "Thanks for Playing");
    wrefresh(win);
    endwin();
    return 0;
}
