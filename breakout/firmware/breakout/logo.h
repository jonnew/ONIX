#include <vector>
#include <array>
#include <cmath>
#include <random>
#include <queue>
#include "oe_draw.h"

class Adafruit_SSD1351;

// Line history
using Line = std::array<uint16_t, 5> ;
using Lines = std::vector<Line>;
using LineBuffer = std::queue<Lines>;

// Point class
template <typename RandGen>
struct Point {

    Point(int idx, RandGen &rng, float speed = 1.0, int size_px = 50, int x_px = 0, int y_px = 0)
    : idx(idx)
    {
        // Initial locations
        std::uniform_real_distribution<float> d_pos(0, size_px);
        x = x_px + d_pos(rng);
        y = y_px + d_pos(rng);

        // Movement freq and phase
        std::uniform_real_distribution<float> d_freq(-3.14, 3.14);
        freq = d_freq(rng);
        dir = speed * d_freq(rng) / 3.14;
    }

    int idx;

    float x;
    float y;

    float freq;
    float dir;

    int flag{0};
};

template <int NumPoints, typename RandGen = std::default_random_engine>
class Logo {

    // Verticies
    //std::vector<Point<RandGen>> points;

    // LineBuffer
    LineBuffer line_buffer;

    // Random generator
    std::default_random_engine rng;

    // Bounding box
    const uint16_t XL;
    const uint16_t XH;
    const uint16_t YL;
    const uint16_t YH;

    // Amount of frames to keep previously drawn lines around
    size_t draw_history;

    // Connection matrix, encodes line colors
    uint16_t connected[NumPoints][NumPoints];

public:
    // Verticies
    std::vector<Point<RandGen>> points;
    Logo(float speed = 1.0, int size_px = 50, int x_px = 0, int y_px = 0, size_t history = 4)
    : XL(x_px)
    , XH(x_px + size_px)
    , YL(y_px)
    , YH(y_px + size_px)
    , draw_history(history)
    , connected{0}
    {
        std::random_device rd;
        rng = std::default_random_engine(rd());

        for (int i = 0; i < NumPoints; i++)
            points.emplace_back(i, rng, speed, size_px, x_px, y_px);

        make_connection_matrix(rng);
    }

    //void draw(Adafruit_SSD1351 &tft)
    //{
    //    update_positions();
    //    update_line_buffer();

    //    // Delete line about to be popped
    //    for (const auto &a : line_buffer.front())
    //        tft.drawLine(a[0], a[1], a[2], a[3], BACKGROUND);

    //    // Draw latest line
    //    for (const auto &a : line_buffer.back())
    //        tft.drawLine(a[0], a[1], a[2], a[3], a[4]);
    //}

private:

    void update_positions (void)
    {
        for (auto &p : points)
            update_position(p);
    }

    void update_position(Point<RandGen> &pt)
    {
        // Move
        pt.x += pt.dir * std::sin(pt.freq + 1);
        pt.y += pt.dir * std::cos(pt.freq + 1);

        // Reflect if required
        if (pt.flag == 0 && (pt.x <= XL || pt.x >= XH || pt.y <= YL || pt.y >= YH)) {
            pt.dir = -pt.dir;
            pt.flag = 1;
        } else if (pt.flag == 1 && !(pt.x <= XL || pt.x >= XH || pt.y <= YL || pt.y >= YH)) {
            pt.flag = 0;
        }

        // Bound
        pt.x = pt.x <= XL ? XL : pt.x;
        pt.y = pt.y <= YL ? YL : pt.y;
        pt.x = pt.x >= XH ? XH : pt.x;
        pt.y = pt.y >= YH ? YH : pt.y;
    }

    void update_line_buffer(void)
    {
        Lines buf;

        for (int i = 0; i < NumPoints; i++) {
            for (int j = 0; j < NumPoints; j++) {
                if (connected[i][j] != 0) {
                    buf.push_back({{
                            static_cast<uint16_t>(points[i].x),
                            static_cast<uint16_t>(points[i].y),
                            static_cast<uint16_t>(points[j].x),
                            static_cast<uint16_t>(points[j].y),
                            connected[i][j]
                    }});
                }
            }
        }

        line_buffer.push(buf);
        if (line_buffer.size() > draw_history)
            line_buffer.pop();
    }

    void make_connection_matrix(RandGen& rng)
    {
        std::array<uint16_t, 6> colors {{OE_YELLOW, OE_CYAN, OE_RED, OE_GREY, OE_VIOLET, OE_BLUE}};
        //std::array<uint16_t, 6> colors {{GREEN, YELLOW, MAGENTA, RED, BLUE, CYAN}};
        std::discrete_distribution<> d_colors({1, 2, 1, 3, 1, 1});
        std::uniform_real_distribution<float> d_link(0, 1);

        for (int i = 0; i < NumPoints; i++) {
            for (int j = 0; j < NumPoints; j++) {
                if (i < 4 && j < 4)
                    connected[i][j] = colors[d_colors(rng)];
                else if (d_link(rng) < 0.25)
                    connected[i][j] = colors[d_colors(rng)];
            }
        }
    }
};
