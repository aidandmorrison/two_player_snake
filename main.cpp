#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>
#include <ncurses.h>
#include <string>


// A few headline inputs which determine how the game plays.
int win_score = 3;
int game_length = 10000; // game ends after this many steps
int game_speed = 400; // ms per step

// This is the main class definition for the 'Player' of the game.
/* It's excessive because I wanted to exercise a little of what I had learned about constructors, including overloading,
 public and private members etc. There's no particularly strong rationale for much of the bulk here,
 or exactly how it's laid out.
I probably had lofty goals of making the game more powerful/general than it would become, including allowing for
less discreet movements, positions and orientations. */
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

    // This function actually moves the player by one unit. Allowing control over speed could be a good extension.
    void step(){
        x += sin(o)*speed;
        y += cos(o)*speed;
    }

    // This just generates plus or minus 1, randomly.  Seems like a lot of code for that!
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

    // This increments the orientation by 90 degrees, if required
    void increment_o(){
        if (change){
            o += (M_PI/2 * helicity);
            change = false;
        }
    }

};

// The idea of this class is to bundle all the players together, and provide a means to 'step' them all through the
// game with a single call.  The idea was to make the game more scalable, and also was a great exercise in managing
// arrays of pointers, which was an important lesson.
class Competitors{
public:
    Competitors() = default;
    int player_count = 0;
    const static int max_players = 2;
    Player * field[max_players];
/*    The line below was an alternative implementation which seemed to work, and would allow an in-game addition
    of more players. I think. But I had difficulty getting the threading required for smooth inputs, and reliable
     response to inputs to work this way round, since this mean that I was actually creating a new player instance
    in the array when adding players, and changes made to the p1 and p2 in main, but outside Competitors, weren't
    reflected inside Competitors, and vice-versa.  So using an array of pointers turned out to be better.
    Just keeping the alternative line here in comments, as it was a key learning I fumbled around for a while.*/
    //Player *field = new Player[max_players];
    void add_player(Player &p){
        if (player_count < max_players) {
            field[player_count] = &p;
            player_count ++;
        } else {
            std::cout << "Game is full!" << std::endl;
        }
    }

    // Here's the function I hoped would simplify a lot of code later.
    void step_all_players() const{
        for(int i = 0; i < player_count; i ++){
            field[i]->increment_o();
            field[i]->step();
        }
    }

};

// Class for the Target, which starts at a random position on the screen, and finds another one whenever required.
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

// I left this as a function, as there was really no need to hold any state.  The referee just check to see whether
// a player has scored, lost, or won.
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

// This just takes in the field of Competitors, Target, and the window to print out on the console.
void printer(Competitors c, Target t, WINDOW *w){
    for (int i = 0; i < c.player_count; i++){
        mvwprintw(w, c.field[i]->getinty(), c.field[i]->getintx(), c.field[i]->icon);
    }
    mvwprintw(w, t.y, t.x, t.icon);
    wrefresh(w);
}

// This is is the crucial function that gets called on a separate thread to take input from the keyboard.
// This, and calling it on the separate thread, getting everything to work smoothly and reliably was quite a lesson.
// It's relatively easy to create race conditions wind up with things being very intermittent and dysfunctional.
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

    // Get ncurses to initialise the screen, hide input and the cursor, and build a window for the introduction window
    initscr();
    noecho();
    curs_set(0);
    int start_y, start_x;
    start_y = start_x = 0;
    WINDOW * introwin = newwin(10, 60, start_y, start_x);

    // Countdown 5 second so that players can expand the console screen to the desired size
    for (int i = 0; i < 5; i++){
        mvwprintw(introwin, 0,0, "Game will start in %d, adjust screen now.", 5-i);
        wrefresh(introwin);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Clear the screen, and give a final message
    werase(introwin);
    mvwprintw(introwin, 0, 0, "Game starting, good luck!!");
    wrefresh(introwin);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Now get the measurements of the new screen for gameplay
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    // Declare variables for initialising player instances
    int x;
    int y;
    int o;
    // Prepare random generator with a time dependent seed so we get new positions every game
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::default_random_engine e(seed);

    // Initialise Player 1 and Player 2, so that they don't appear too close to the walls
    x = e() % (maxx - 8) + 4;
    y = e() % (maxy - 8) + 4;
    o = e();
    Player p1(x, y, o*M_PI/2);
    x = e() % (maxx - 8) + 8;
    y = e() % (maxy - 8) + 4;
    o = e();
    Player p2(x, y, o*M_PI/2);
    p1.icon = "A";
    p1.control = 'a';
    p2.icon = "C";
    p2.control = 'c';

    // Initialise the Competitors class, and pass in references to the players
    Competitors ThesePlayers = Competitors();
    ThesePlayers.add_player(p1);
    ThesePlayers.add_player(p2);

    // Create the target
    Target CurrentTarget = Target(maxy, maxx);

    // Set up an ncurses window of the size of the screen and print this
    WINDOW * win = newwin(maxy, maxx, start_y, start_x);
    werase(win);
    refresh();
    char wall = '|';
    char roof = '-';
    box(win, (int)wall, (int)roof);
    wrefresh(win);

    // Get ready to play by declaring and initialising key params
    int frame_count = 0;
    bool game_continues = true;
    // Tell ncurses to not wait for keyboard before continuing, crucial for getch() calls in gameplay
    timeout(0);

    // Start a thread that calls the function player_input, getting keyboard input to control players
    std::thread input_thread(player_input, std::ref(ThesePlayers), &game_continues);

    // And actually start the game loop!
    while(game_continues){

        // Clear the screen and reprint the latest state
        werase(win);
        box(win, (int)wall, (int)roof);
        printer(ThesePlayers, CurrentTarget, win);

        // Check to see if anyone has scored, won or lost
        game_continues = referee(ThesePlayers, &CurrentTarget, maxx, maxy, win);

        // Wait just a bit (game_speed dependent) so players can see and respond
        std::this_thread::sleep_for(std::chrono::milliseconds(game_speed));

        // Change the positions of the players
        ThesePlayers.step_all_players();

        // Check for exceeding limit on game
        frame_count += 1;
        if (frame_count >= game_length){
            game_continues = false;
        }
    }

    // For completeness, join the input thread back into the main thread
    input_thread.join();

    // Print out the final scores
    for (int i = 0; i < ThesePlayers.player_count; i ++){
        mvwprintw(win, maxy/2 + i, maxx/3, "Final score of ");
        wprintw(win, ThesePlayers.field[i]->icon);
        wprintw(win, " is %d", ThesePlayers.field[i]->score);
    }
    mvwprintw(win, 1, maxx/3, "Thanks for Playing");
    wrefresh(win);

    // Either wait 5 seconds on final screen with scores, or any key press to continue
    timeout(5000);
    getch();

    // Close the ncurses window and exit
    endwin();
    return 0;
}
