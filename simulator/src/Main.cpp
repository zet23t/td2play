/*
 * This file was quickly hacked together to provide simple graphics output and
 * button controls for simulating a TinyScreen. It's really bad quality but does
 * its job: Providing an environment with visual output for compiling code that
 * runs on the TinyScreen+ in a similar way.
 */

#include <GLFW/glfw3.h>
#include <gl/GL.h>
#include <stdlib.h>
#include <stdio.h>
#include <TinyScreen.h>

#define TINYSCREEN_WIDTH 96
#define TINYSCREEN_HEIGHT 64


#define SCREEN_TEXTURE_SIZE 128
#define SCREEN_UMAX (96.0f / (float)SCREEN_TEXTURE_SIZE)
#define SCREEN_VMAX (64.0f / (float)SCREEN_TEXTURE_SIZE)
typedef struct {
    GLuint screenTexture;
    unsigned char screenData[SCREEN_TEXTURE_SIZE*SCREEN_TEXTURE_SIZE * 3];
    unsigned char x,y;
    bool is16bit;
    GLFWwindow* window;
} Emulator;

Emulator emulator;

static void updateScreen() {
    glBindTexture(GL_TEXTURE_2D, emulator.screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, SCREEN_TEXTURE_SIZE, SCREEN_TEXTURE_SIZE, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, emulator.screenData);

}

int digitalRead(int pin) {
    GLFWwindow* window = emulator.window;
    switch (pin) {
        case 4: return (glfwGetKey(window, GLFW_KEY_G) ? 1 : 0);
        case 5: return (glfwGetKey(window, GLFW_KEY_H) ? 1 : 0);
        default: return 0;
    }
    return 0;
}

int analogRead(int pin) {
    GLFWwindow* window = emulator.window;
    switch (pin) {
        case 2: return (glfwGetKey(window, GLFW_KEY_RIGHT) ? -511 : 0) + (glfwGetKey(window, GLFW_KEY_LEFT) ? 511 : 0)+511;
        case 3: return (glfwGetKey(window, GLFW_KEY_UP) ? -511 : 0) + (glfwGetKey(window, GLFW_KEY_DOWN) ? 511 : 0)+511;
        default: return 0;
    }
    return 0;
}

void setup();

void loop();

static void drawCircle(float x, float y, float radius, int div) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i=0;i<=div;i+=1) {
        float ang = (float)i/(float)div * 3.141593f * 2.0f;
        float px = sinf(ang) * radius;
        float py = cosf(ang) * radius;
        glVertex2f(px+x,py+y);
    }
    glEnd();
}

void TinyScreen::startData(void) {}
void TinyScreen::startCommand(void) {}
void TinyScreen::endTransfer(void) {
    GLFWwindow* window = emulator.window;
    updateScreen();
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.25f,0.25f,0.5f,0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
   // glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
    /*glBegin(GL_TRIANGLES);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(-0.6f, -0.4f, 0.f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.6f, -0.4f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.f, 0.6f, 0.f);
    glEnd();*/
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, emulator.screenTexture);
    float scale = ratio > 1.3f ? 2.25f : (2.25f/1.3f) * ratio;

    glScalef(scale,scale,scale);
    glTranslatef(-0.48f, -0.12f,0);

    glColor3f(.0f,.0f,0.f);
    drawCircle(.15f,-.25f,.18f,16);
    glColor3f(.8f,.8f,0.8f);
    float stickX = -(float)analogRead(2) / 1023.f + .5f;
    float stickY = -(float)analogRead(3) / 1023.f + .5f;
    drawCircle(.15f + stickX*.15f,-.25f + stickY * .15f,.10f,12);



    glColor3f(.0f,.0f,0.f);
    drawCircle(.6f,-.325f,.075f,12);
    drawCircle(.8f,-.225f,.075f,12);
    glColor3f(.8f,.2f,0.f);
    float buttonY = digitalRead(4) ? -.32f : -.3f;
    drawCircle(.6f,buttonY,.075f,12);
    buttonY = digitalRead(5) ? -.22f : -.2f;
    drawCircle(.8f,buttonY,.075f,12);
    int buttons = getButtons();
    glBegin(GL_QUADS);
    // bottom left
    float buttonX = buttons & 1 ? -.05f : -.075f;
    glVertex2f(buttonX, .2f);
    glVertex2f(buttonX, .1f);
    glVertex2f(.1f, .1f);
    glVertex2f(.1f, .2f);
    glEnd();

    buttonX = (buttons & 2) ? -.05f : -.075f;
    glColor3f(.8f,.2f,0.f);
    glBegin(GL_QUADS);
    glVertex2f(buttonX, .54f);
    glVertex2f(buttonX, .44f);
    glVertex2f(.1f, .44f);
    glVertex2f(.1f, .54f);
    glEnd();
    glColor3f(.8f,.2f,0.f);
    buttonX = (buttons & 4) ? 1.01f : 1.035f;
    glBegin(GL_QUADS);
    glVertex2f(.5f, .54f);
    glVertex2f(.5f, .44f);
    glVertex2f(buttonX, .44f);
    glVertex2f(buttonX, .54f);
    glEnd();
    buttonX = (buttons & 8) ? 1.01f : 1.035f;
    glBegin(GL_QUADS);
    glVertex2f(.5f, .2f);
    glVertex2f(.5f, .1f);
    glVertex2f(buttonX, .1f);
    glVertex2f(buttonX, .2f);
    glEnd();


    float margin = .02f;
    glColor3f(0,0,0);
    glBegin(GL_QUADS);
    glVertex3f(-margin, -margin, 0);
    glVertex3f(-margin, 0.64f+margin, 0);
    glVertex3f(0.96f+margin, 0.64f+margin, 0);
    glVertex3f(0.96f+margin, -margin, 0);
    glEnd();



    glEnable(GL_TEXTURE_2D);
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, SCREEN_VMAX); glVertex3f(0, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(0, 0.64f, 0);
    glTexCoord2f(SCREEN_UMAX, 0); glVertex3f(0.96f, 0.64f, 0);
    glTexCoord2f(SCREEN_UMAX, SCREEN_VMAX); glVertex3f(0.96f, 0, 0);
    glEnd();



    glfwSwapBuffers(window);
    glfwPollEvents();

    if (glfwWindowShouldClose(window)) {

        glfwDestroyWindow(window);
        glfwTerminate();
        exit(EXIT_SUCCESS);
    }
}
    void TinyScreen::begin(void) {}
    void TinyScreen::begin(uint8_t) {}
    void TinyScreen::on(void) {}
    void TinyScreen::off(void) {}
    void TinyScreen::setFlip(uint8_t) {}
    void TinyScreen::setMirror(uint8_t) {}
    void TinyScreen::setBitDepth(uint8_t is16bit) {
        emulator.is16bit = is16bit ? true : false;
    }
    void TinyScreen::setBrightness(uint8_t) {}
    //void TinyScreen::writeRemap(void) {}
    //accelerated drawing commands
    void TinyScreen::drawPixel(uint8_t, uint8_t, uint16_t) {}
    void TinyScreen::drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) {}
    void TinyScreen::drawLine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) {}
    void TinyScreen::drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) {}
    void TinyScreen::drawRect(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) {}
    void TinyScreen::clearWindow(uint8_t, uint8_t, uint8_t, uint8_t) {}
    //basic graphics commands
    void TinyScreen::writePixel(uint16_t) {
    }
    void TinyScreen::writeBuffer(uint8_t *rgb, int num) {
        //uint16_t *rgb565_16 = (uint16_t*)&rgb565[0];
        int idx = emulator.x + emulator.y * SCREEN_TEXTURE_SIZE;
        uint8_t *rgb565 = rgb;
        for (int i=0;i<num; i+=1) {
            uint8_t r,g,b;
            if (emulator.is16bit) {
                uint16_t word = 0;
                word = rgb565[i]<<8 | rgb565[i+1];
                r = word & 31;
                g = (word >> 5) & 63;
                b = word >> 11;

                r = (r << 3 | r >> 2);
                g = (g << 2 | g >> 4);
                b = (b << 3 | b >> 2);
                i+=1;
            } else {
                uint8_t rgb233 = rgb[i];
                r = rgb233 & 3;
                r = (r << 6) | (r << 4) | (r << 2) | r;
                g = (rgb233 >> 2) & 7;
                g = g << 5 | g << 2 | g >> 1;
                b = rgb233 >> 5 & 7;
                b = b << 5 | b << 2 | b >> 1;
            }
            // my gif screencsat program doesn't like 00ff00
            if (r == 0 && g >= 250 && b == 0) g = 250;
            emulator.screenData[idx*3+0] = r;
            emulator.screenData[idx*3+1] = g;
            emulator.screenData[idx*3+2] = b;
            if (idx % SCREEN_TEXTURE_SIZE == TINYSCREEN_WIDTH-1) {
                idx = emulator.x + (++emulator.y) * SCREEN_TEXTURE_SIZE;//SCREEN_TEXTURE_SIZE - TINYSCREEN_WIDTH + 1;
            } else {
                idx += 1;
            }
        }
        //emulator.y+=1;
    }
    /*void TinyScreen::setX(uint8_t, uint8_t);
    void TinyScreen::setY(uint8_t, uint8_t);*/
    void TinyScreen::goTo(uint8_t x, uint8_t y) {
        emulator.x = x;
        emulator.y = y;
    }

    //I2C GPIO related
    uint8_t TinyScreen::getButtons(void) {
        GLFWwindow* window = emulator.window;
        int tr = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
        int br = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
        int tl = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
        int bl = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
        return tl << 1 | bl | tr << 2 | br << 3;
    }
    /*void TinyScreen::writeGPIO(uint8_t, uint8_t);
    //font
    void TinyScreen::setFont(const FONT_INFO&);
    void TinyScreen::setCursor(uint8_t, uint8_t);
    void TinyScreen::fontColor(uint8_t, uint8_t); */
    size_t TinyScreen::write(uint8_t) { return 0; }


static void init() {

    for (int x=0;x<TINYSCREEN_WIDTH;x+=1) {
        for (int y=0;y<TINYSCREEN_HEIGHT;y+=1) {
            int idx = (x + y * SCREEN_TEXTURE_SIZE) * 3;
            emulator.screenData[idx] = x * 255 / TINYSCREEN_WIDTH;
            emulator.screenData[idx + 1] = y * 255 / TINYSCREEN_HEIGHT;
        }
    }

    glGenTextures(1, &emulator.screenTexture);
    updateScreen();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

unsigned long millis() {
    return clock();
}

static void error_callback(int /*error*/, const char* description)
{
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
int main(void)
{
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(320, 320, "TinyScreen Simulator", NULL, NULL);
    emulator.window = window;
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
    init();
    setup();
    while (!glfwWindowShouldClose(window))
    {
        loop();

    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
