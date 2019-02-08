#include <emscripten/emscripten.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

typedef struct Color {
    uint8_t r, g, b, a;
} Color;

#define EMPTY { 0, 0, 0, 255 }

#define RED { 255, 0, 0, 255 }
#define GREEN { 0, 255, 0, 255 }
#define BLUE { 0, 0, 255, 255 }
#define ORANGE { 255, 165, 0, 255 }
#define PURPLE { 128, 0, 128, 255 }
#define YELLOW { 255, 255, 0, 255 }
#define TURQ { 64, 224, 208, 255 }
#define GREY { 128, 128, 128, 255 }
#define VIOLET { 238, 130, 238, 255 }
#define WHITE { 255, 255, 255, 255 }
#define BROWN { 165, 42, 42, 255 }
#define PUCE { 204, 136, 153, 255 }

#define NUMCOLORS 12

Color colors[NUMCOLORS] = { RED, GREEN, BLUE, ORANGE, 
                          PURPLE, YELLOW, TURQ, GREY,
                          VIOLET, WHITE, BROWN, PUCE };
Color currColor;
Color empty = EMPTY;

struct {
    int width;
    int height;
    int size;
    uint8_t* buf;
} view;

void setPixel(int x, int y) {
    if(x >= view.width || y >= view.height || x < 0 || y < 0) 
        return;
    
    uint8_t* p = view.buf + 4 * (x + view.width * y); 

    *p = currColor.r;
    *(p + 1) = currColor.g;
    *(p + 2) = currColor.b;
}

void setPixelColor(int x, int y, float intensity) {
    // Check for out of bounds
    if(x >= view.width || y >= view.height || x < 0 || y < 0) 
        return;

    uint8_t* p = view.buf + 4 * (x + view.width * y); 

    // Only draw if pixel hasn't already been drawn
    if(*p != empty.r || *(p + 1) != empty.g || *(p + 2) != empty.b) 
        return;

    float colorScalar = 1 - (intensity / 255);

    *p = currColor.r * colorScalar;
    *(p + 1) = currColor.g * colorScalar;
    *(p + 2) = currColor.b * colorScalar;
}

// An adaptation of Xiaolin Wu's antialiased line rasterization algorithm
void plotLineWidth(int x0, int y0, int x1, int y1, float w) {
    if(w < 1)
        w = 1;

    // To check for oob, ensure x is increasing
    if(x0 > x1) {
        int t = x0;
        x0 = x1;
        x1 = t;
        t = y0;
        y0 = y1;
        y1 = t;
    }

    // Check out of bounds
    // Since x is increasing, if the endpoint is < 0 there's nothing to draw
    // Similar if the start point is > maxX
    if(x0 > view.width || x1 < 0) 
        return;
 
    int sy = y0 < y1 ? 1 : -1;
    // If y is increasing,...
    if(sy == 1 && (y0 > view.width || y1 < 0))
        return;
    // If y is decreasing...
    if(sy == -1 && (y1 > view.width || y0 < 0))
        return;

    float dx = (float) abs(x1 - x0);
    float dy = (float) abs(y1 - y0);
    float e2 = sqrt(dx * dx + dy * dy);
    float err;

    // Calculate incremental values
    dx *= 255 / e2;
    dy *= 255 / e2;
    w = 255 * (w - 1);
    if(dx < dy) { // steep, so we render rows
        x1 = round((e2 + w / 2) / dy);
        err = x1 * dy - w / 2;    
        x0 -= x1;
        for(;; y0 += sy) {
            // if we've exited the viewport, stop
            if((sy == 1 && y0 > view.height) || (sy == -1 && y0 < 0))
                break;

            x1 = x0;
            setPixelColor(x1, y0, err);  // aliased border pixel
                
            for(e2 = dy - err - w; e2 + dy < 255; e2 += dy) {
                x1++;
                setPixel(x1, y0); // pixel on line, full color
            }
            setPixelColor(x1 + 1, y0, e2);
            
            if(y0 == y1) // end of the line?
                break;

            err += dx;
            if(err > 255) {
                err -= dy;
                x0++;
            }
        }
    } else { // flat
        y1 = round((e2 + w / 2) / dx);
        err = y1 * dx - w / 2;    
        y0 -= y1 * sy;
        for(;; x0++) {
            // if we're off the right side of the viewport, stop
            if(x0 > view.width)
              break;

            y1 = y0;
            setPixelColor(x0, y1, err); // aliased border pixel 
            for(e2 = dx - err - w; e2 + dx < 255; e2 += dx) {
                y1 += sy;
                setPixel(x0, y1); // pixel on line, full color
            }
            setPixelColor(x0, y1 + sy, e2); // other aliased border pixel
            
            if(x0 == x1) // end of the line?
                break;

            err += dy;
            if(err > 255) {
                err -= dx;
                y0 += sy;
            }
        }
    }
}

// x, y: coordinates
// a: angle
// l: length of line, ls: line scaling factor
// bf: branching factor
// ta: total angle of branches
// d: depth
void drawSplittingLine(int x, int y, float a, int l, float ls, int bf, float ta, int d) {
    if(d < 0 || l == 0)
        return;

    currColor = colors[d];
    float rad = M_PI / 180 * a;
    int x1 = x + l * cos(rad);
    int y1 = y + l * sin(rad);
    
    plotLineWidth(x, y, x1, y1, 1 + d);

    float ai = ta / (bf - 1); // angle increment = total angle / branching factor
    float ca = a - (ta / 2);
    for(int i = 0; i < bf; i++) {
        drawSplittingLine(x1, y1, ca, l * ls, ls, bf, ta, d - 1);
        ca += ai;
    }
}

void clearViewBuffer() {
    // Set all RGBA values in the buffer to empty
    for(uint8_t* p = view.buf; p < view.buf + view.size; p += 4) {
        *p = empty.r;
        *(p + 1) = empty.g;
        *(p + 2) = empty.b;
        *(p + 3) = empty.a;
    }
}

EMSCRIPTEN_KEEPALIVE
int drawTree(float x, float y, float a, float l, float ls, float bf, float ta, float d) {
    if(!view.buf)
        return 1;
    
    clearViewBuffer();

    int x0 = view.width * x;
    int y0 = view.height * y;

    drawSplittingLine(x0, y0, a, l, ls, bf, ta, d);
    return 0;
}

EMSCRIPTEN_KEEPALIVE
uint8_t* initView(int width, int height) {
    srand(time(NULL));

    view.width = width;
    view.height = height;
    view.size = width * height * 4 * sizeof(uint8_t);	
    view.buf = malloc(view.size);
    
    return view.buf;
}
