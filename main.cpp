#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

// ── Constants ────────────────────────────────────────────────────────────────
const int   WW = 800, WH = 640, BAR = 90, CS = 40;
const int   COLS = WW / CS, ROWS = (WH - BAR) / CS;
const int   APL  = 5, LB_MAX = 5;
const float BASE = 0.140f, STEP = 0.012f, MINSPD = 0.055f;

// ── Types ─────────────────────────────────────────────────────────────────────

/** @brief Leaderboard entry. */
struct Entry { string name; int score; };

/** @brief Screen flash effect state. */
struct Flash { sf::Color col; float t = 0; bool on = false; };

/** @brief All game screens. */
enum class Screen { Start, Playing, GameOver, Board };

// ── State ─────────────────────────────────────────────────────────────────────
Screen           screen = Screen::Start;
int              apples = 0, score = 0, hi = 0, level = 1;
sf::Vector2i     dir = {1,0}, ndir = {1,0}, apple;
deque<sf::Vector2i> snake;
Flash            flash;
vector<Entry>    lb;
string           lbFile = "leaderboard.txt";
sf::Texture      menuTex, bgTex;
bool             menuOk = false, bgOk = false;

// ── Leaderboard ───────────────────────────────────────────────────────────────

/**
 * @brief Loads leaderboard from file, sorts descending, keeps top LB_MAX.
 * @param f Path to leaderboard file.
 */
void loadLB(const string& f) {
    lb.clear();
    ifstream in(f); if (!in) return;
    int sc; string nm;
    while (in >> sc && getline(in, nm)) {
        if (!nm.empty() && nm[0] == ' ') nm = nm.substr(1);
        lb.push_back({nm, sc});
    }
    sort(lb.begin(), lb.end(), [](auto& a, auto& b){ return a.score > b.score; });
    if ((int)lb.size() > LB_MAX) lb.resize(LB_MAX);
}

/**
 * @brief Saves leaderboard to file.
 * @param f Path to leaderboard file.
 * @throws runtime_error if file cannot be opened.
 */
void saveLB(const string& f) {
    ofstream out(f);
    if (!out) throw runtime_error("Cannot write: " + f);
    for (auto& e : lb) out << e.score << " " << e.name << "\n";
}

/**
 * @brief Adds entry, re-sorts, trims, saves.
 * @param nm Player name.
 * @param sc Score to add.
 * @param f  Leaderboard file path.
 */
void addLB(const string& nm, int sc, const string& f) {
    lb.push_back({nm, sc});
    sort(lb.begin(), lb.end(), [](auto& a, auto& b){ return a.score > b.score; });
    if ((int)lb.size() > LB_MAX) lb.resize(LB_MAX);
    try { saveLB(f); } catch (...) {}
}

// ── Game Logic ────────────────────────────────────────────────────────────────

/**
 * @brief Returns true if pos is occupied by any snake segment.
 * @param pos Grid cell to check.
 * @return true if occupied, false otherwise.
 */
bool hasSnake(sf::Vector2i pos) {
    for (auto& p : snake) if (p == pos) return true;
    return false;
}

/**
 * @brief Returns movement delay in seconds for current level.
 * @return Clamped delay value.
 */
float delay() { return max(BASE - (level-1)*STEP, MINSPD); }

/**
 * @brief Returns snake colour for current level (cycles every 6 levels).
 * @return SFML Color.
 */
sf::Color snakeCol() {
    static const array<sf::Color,6> c = {
        sf::Color(65,130,230), sf::Color(40,200,180), sf::Color(230,130,40),
        sf::Color(180,60,220), sf::Color(220,190,30), sf::Color(220,60,130)};
    return c[(level-1) % 6];
}

/** @brief Spawns apple at random empty cell. */
void spawnApple() {
    do { apple = {rand()%COLS, rand()%ROWS}; } while (hasSnake(apple));
}

/** @brief Resets all state and begins a new game. */
void reset() {
    snake.clear();
    snake.push_back({5,6}); snake.push_back({4,6}); snake.push_back({3,6});
    dir = ndir = {1,0};
    apples = score = 0; level = 1;
    flash.on = false;
    spawnApple();
    screen = Screen::Playing;
}

/**
 * @brief Moves snake one step; handles wall/self collision and apple eating.
 *
 * Sets screen to GameOver on any collision.
 */
void step() {
    dir = ndir;
    sf::Vector2i h = snake.front() + dir;
    if (h.x < 0 || h.x >= COLS || h.y < 0 || h.y >= ROWS) { screen = Screen::GameOver; return; }
    for (int i = 1; i < (int)snake.size(); ++i)
        if (snake[i] == h) { screen = Screen::GameOver; return; }
    snake.push_front(h);
    if (h == apple) {
        score += 10 * level; if (score > hi) hi = score;
        level = ++apples / APL + 1;
        flash = {sf::Color(255,255,150,120), 0.25f, true};
        spawnApple();
    } else snake.pop_back();
}

// ── Drawing ───────────────────────────────────────────────────────────────────

/**
 * @brief Draws gameplay background (image or checkerboard) + flash overlay.
 * @param w Render window.
 */
void drawBG(sf::RenderWindow& w) {
    if (bgOk) {
        sf::Sprite s(bgTex);
        s.setScale({(float)WW/bgTex.getSize().x, (float)WH/bgTex.getSize().y});
        w.draw(s);
    } else {
        for (int y = 0; y < ROWS; ++y)
            for (int x = 0; x < COLS; ++x) {
                sf::RectangleShape c({(float)CS,(float)CS});
                c.setFillColor((x+y)%2 ? sf::Color(25,60,18) : sf::Color(30,70,20));
                c.setPosition({(float)(x*CS),(float)(BAR+y*CS)});
                w.draw(c);
            }
    }
    if (flash.on) {
        sf::RectangleShape ov({(float)WW,(float)(WH-BAR)});
        ov.setPosition({0,(float)BAR});
        sf::Color fc = flash.col;
        fc.a = (uint8_t)(flash.col.a * min(flash.t/0.25f,1.f));
        ov.setFillColor(fc); w.draw(ov);
    }
}

/**
 * @brief Draws apple (red circle + green leaf).
 * @param w Render window.
 */
void drawApple(sf::RenderWindow& w) {
    sf::CircleShape b(16); b.setFillColor(sf::Color(230,45,35));
    b.setPosition({(float)(apple.x*CS+4),(float)(BAR+apple.y*CS+4)}); w.draw(b);
    sf::CircleShape l(5); l.setFillColor(sf::Color(40,160,55));
    l.setPosition({(float)(apple.x*CS+22),(float)(BAR+apple.y*CS)}); w.draw(l);
}

/**
 * @brief Draws snake body, head, and directional eyes.
 * @param w Render window.
 */
void drawSnake(sf::RenderWindow& w) {
    auto col = snakeCol();
    for (int i = (int)snake.size()-1; i >= 1; --i) {
        sf::CircleShape s(CS/2-3); s.setFillColor(col);
        s.setPosition({(float)(snake[i].x*CS+3),(float)(BAR+snake[i].y*CS+3)}); w.draw(s);
    }
    float hx = (float)(snake.front().x*CS), hy = (float)(BAR+snake.front().y*CS);
    sf::CircleShape h(CS/2-2); h.setFillColor(col); h.setPosition({hx+2,hy+2}); w.draw(h);

    sf::CircleShape e1(6),e2(6),p1(2.5f),p2(2.5f);
    e1.setFillColor(sf::Color::White); e2.setFillColor(sf::Color::White);
    p1.setFillColor(sf::Color::Black); p2.setFillColor(sf::Color::Black);
    if      (dir.x== 1){e1.setPosition({hx+23,hy+ 9});e2.setPosition({hx+23,hy+24});p1.setPosition({hx+27,hy+13});p2.setPosition({hx+27,hy+28});}
    else if (dir.x==-1){e1.setPosition({hx+ 7,hy+ 9});e2.setPosition({hx+ 7,hy+24});p1.setPosition({hx+ 9,hy+13});p2.setPosition({hx+ 9,hy+28});}
    else if (dir.y==-1){e1.setPosition({hx+ 9,hy+ 7});e2.setPosition({hx+24,hy+ 7});p1.setPosition({hx+13,hy+ 9});p2.setPosition({hx+28,hy+ 9});}
    else               {e1.setPosition({hx+ 9,hy+24});e2.setPosition({hx+24,hy+24});p1.setPosition({hx+13,hy+28});p2.setPosition({hx+28,hy+28});}
    w.draw(e1);w.draw(e2);w.draw(p1);w.draw(p2);
}

/**
 * @brief Draws HUD top bar with score, level, controls hint.
 * @param w Render window.
 * @param f Font.
 */
void drawBar(sf::RenderWindow& w, sf::Font& f) {
    sf::RectangleShape b({(float)WW,(float)BAR}); b.setFillColor(sf::Color(5,10,25,210)); w.draw(b);
    sf::CircleShape a(15); a.setFillColor(sf::Color(230,45,35)); a.setPosition({18,10}); w.draw(a);
    sf::CircleShape l(4);  l.setFillColor(sf::Color(40,160,55)); l.setPosition({36,7});  w.draw(l);
    sf::Text nm(f,"Abdallah",20); nm.setFillColor(sf::Color(255,215,0)); nm.setPosition({55,8}); w.draw(nm);
    sf::Text st(f,"Score:"+to_string(score)+"   Apples:"+to_string(apples)+"   Length:"+to_string((int)snake.size())+"   High:"+to_string(hi)+"   Lvl:"+to_string(level),20);
    st.setFillColor(sf::Color::White); st.setPosition({55,35}); w.draw(st);
    sf::Text c(f,"WASD=Move  R=Retry  L=Leaderboard  ESC=Exit",16);
    c.setFillColor(sf::Color(80,230,255)); c.setPosition({55,63}); w.draw(c);
}

/**
 * @brief Draws start screen (menu image or text fallback).
 * @param w Render window.
 * @param f Font.
 */
void drawStart(sf::RenderWindow& w, sf::Font& f) {
    if (menuOk) {
        sf::Sprite s(menuTex);
        s.setScale({(float)WW/menuTex.getSize().x,(float)WH/menuTex.getSize().y}); w.draw(s);
    } else {
        w.clear(sf::Color(18,40,12));
        sf::Text t(f,"Snake Arena Game",48); t.setFillColor(sf::Color(150,255,100)); t.setPosition({160,220}); w.draw(t);
    }
    sf::Text a(f,"Press ENTER to Start",28); a.setFillColor(sf::Color::White);          a.setPosition({250,310}); w.draw(a);
    sf::Text b(f,"Press L for Leaderboard",20); b.setFillColor(sf::Color(180,230,255)); b.setPosition({280,355}); w.draw(b);
    sf::Text c(f,"Press ESC to Exit",20);        c.setFillColor(sf::Color(200,200,200)); c.setPosition({310,390}); w.draw(c);
}

/**
 * @brief Draws game-over overlay.
 * @param w Render window.
 * @param f Font.
 */
void drawOver(sf::RenderWindow& w, sf::Font& f) {
    sf::RectangleShape ov({(float)WW,(float)WH}); ov.setFillColor(sf::Color(0,0,0,180)); w.draw(ov);
    sf::Text t(f,"GAME OVER",58); t.setFillColor(sf::Color(255,60,60)); t.setPosition({215,175}); w.draw(t);
    sf::Text i(f,"Score:"+to_string(score)+"  |  Apples:"+to_string(apples)+"  |  Level:"+to_string(level)+"  |  Length:"+to_string((int)snake.size()),22);
    i.setFillColor(sf::Color::White); i.setPosition({130,270}); w.draw(i);
    sf::Text r(f,"R = Retry",24);        r.setFillColor(sf::Color(150,255,100));  r.setPosition({310,330}); w.draw(r);
    sf::Text l(f,"L = Leaderboard",24);  l.setFillColor(sf::Color(150,200,255)); l.setPosition({270,370}); w.draw(l);
    sf::Text e(f,"ESC = Exit",24);       e.setFillColor(sf::Color(255,200,100)); e.setPosition({315,410}); w.draw(e);
}

/**
 * @brief Draws leaderboard / Hall-of-Fame screen.
 * @param w Render window.
 * @param f Font.
 */
void drawBoard(sf::RenderWindow& w, sf::Font& f) {
    w.clear(sf::Color(10,20,40));
    sf::Text t(f,"Hall of Fame",44); t.setFillColor(sf::Color(255,215,0)); t.setPosition({260,60}); w.draw(t);
    if (lb.empty()) {
        sf::Text e(f,"No records yet - play to set a high score!",24);
        e.setFillColor(sf::Color(180,180,180)); e.setPosition({130,260}); w.draw(e);
    } else {
        for (int i = 0; i < (int)lb.size(); ++i) {
            sf::Text e(f, to_string(i+1)+".  "+lb[i].name+"  -  "+to_string(lb[i].score), 28);
            e.setFillColor(i==0 ? sf::Color(255,215,0) : sf::Color::White);
            e.setPosition({160,(float)(150+i*60)}); w.draw(e);
        }
    }
    sf::Text b(f,"Press ENTER or L to go back",20); b.setFillColor(sf::Color(150,200,255)); b.setPosition({245,560}); w.draw(b);
}

// ── Font Loading ──────────────────────────────────────────────────────────────

/**
 * @brief Tries each path in candidates; loads first that succeeds.
 * @param font       SFML font to load into.
 * @param candidates List of TTF paths to try.
 * @throws runtime_error if no path works.
 */
void loadFont(sf::Font& font, const vector<string>& candidates) {
    for (auto& p : candidates) if (font.openFromFile(p)) return;
    throw runtime_error("No font found. Place arial.ttf next to the executable.");
}

// ── Main ──────────────────────────────────────────────────────────────────────

/**
 * @brief Entry point. Accepts optional paths: leaderboard, menu.png, bg.png, font1, font2.
 * @param argc Argument count.
 * @param argv Argument values.
 * @return 0 on success, 1 on fatal error.
 */
int main(int argc, char* argv[]) {
    srand((unsigned)time(nullptr));

    string lbF  = argc>1 ? argv[1] : "leaderboard.txt";
    string mPng = argc>2 ? argv[2] : "menu.png";
    string bPng = argc>3 ? argv[3] : "background.png";
    lbFile = lbF;

    menuOk = menuTex.loadFromFile(mPng);
    bgOk   = bgTex.loadFromFile(bPng);

    try { loadLB(lbF); } catch (...) {}
    if (!lb.empty()) hi = lb[0].score;

    sf::RenderWindow win(sf::VideoMode({(unsigned)WW,(unsigned)WH}), "Snake Arena - Abdallah's Edition");
    win.setFramerateLimit(60);

    vector<string> fonts;
    if (argc>4) fonts.push_back(argv[4]);
    if (argc>5) fonts.push_back(argv[5]);
    fonts.insert(fonts.end(), {"arial.ttf","fonts/arial.ttf",
        "C:/Windows/Fonts/arialbd.ttf","C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"});

    sf::Font font;
    try { loadFont(font, fonts); } catch (...) { return 1; }

    sf::Clock mv, fl;
    bool entR=true, rR=true, lR=true, saved=false;

    while (win.isOpen()) {
        float dt = fl.restart().asSeconds();
        while (auto ev = win.pollEvent()) if (ev->is<sf::Event::Closed>()) win.close();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) win.close();

        bool ent = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
        bool r   = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R);
        bool l   = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L);

        if (screen == Screen::Start) {
            if (ent&&entR) { reset(); saved=false; mv.restart(); }
            if (l&&lR)     screen = Screen::Board;
        } else if (screen == Screen::Playing) {
            if      (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)&&dir.y!=1)  ndir={0,-1};
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)&&dir.y!=-1) ndir={0, 1};
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)&&dir.x!=1)  ndir={-1,0};
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)&&dir.x!=-1) ndir={ 1,0};
            if (flash.on) { flash.t -= dt; if (flash.t<=0) flash.on=false; }
            if (mv.getElapsedTime().asSeconds() >= delay()) {
                step(); mv.restart();
                if (screen==Screen::GameOver && !saved) {
                    addLB("Abdallah", score, lbF);
                    if (score>hi) hi=score;
                    saved=true;
                }
            }
        } else if (screen == Screen::GameOver) {
            if (r&&rR) { reset(); saved=false; mv.restart(); }
            if (l&&lR)  screen = Screen::Board;
        } else if (screen == Screen::Board) {
            if ((ent&&entR)||(l&&lR)) screen = Screen::Start;
        }

        entR=!ent; rR=!r; lR=!l;
        win.clear();

        if      (screen==Screen::Start)   drawStart(win,font);
        else if (screen==Screen::Board)   drawBoard(win,font);
        else {
            drawBG(win); drawBar(win,font); drawApple(win); drawSnake(win);
            if (screen==Screen::GameOver) drawOver(win,font);
        }
        win.display();
    }
    return 0;
}
