/*

Modified 11 September 2015 by Ben Rose for TinyCircuits to work on TinyScreen+

Copyright (c) 2004-2007, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>

TinyScreen display = TinyScreen(0);

#define screenWidth 96
#define screenHeight 64
#define texWidth 64
#define texHeight 64
#define mapWidth 24
#define mapHeight 24

#define w screenWidth
#define h screenHeight

int worldMap[mapWidth][mapHeight] =
{
  {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 7, 7, 7, 7, 7, 7, 7},
  {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7},
  {4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7},
  {4, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7},
  {4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7},
  {4, 0, 4, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 0, 7, 7, 7, 7, 7},
  {4, 0, 5, 0, 0, 0, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1},
  {4, 0, 6, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8},
  {4, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 1},
  {4, 0, 8, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8},
  {4, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1},
  {4, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 1},
  {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
  {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
  {6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
  {4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 6, 0, 6, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3},
  {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2},
  {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2},
  {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2},
  {4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 2},
  {4, 0, 0, 5, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2},
  {4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2},
  {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2},
  {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3}
};

uint8_t buffer[screenWidth * screenHeight * 2];


extern const uint16_t* texture[8];

void setup() {
  Wire.begin();
  SerialUSB.print("hello!");
  display.begin();
  display.setFlip(0);
  display.setBrightness(8);
  display.setBitDepth(1);
}

void loop() {
  float posX = 22.0, posY = 11.5;  //x and y start position
  float dirX = -1.0, dirY = 0.0; //initial direction vector
  float planeX = 0.0, planeY = 0.66; //the 2d raycaster version of camera plane

  float time = 0; //time of current frame
  float oldTime = 0; //time of previous frame

  //start the main loop
  while (1)
  {
    for (int x = 0; x < w; x++)
    {
      //calculate ray position and direction
      float cameraX = 2 * x / float(w) - 1; //x-coordinate in camera space
      float rayPosX = posX;
      float rayPosY = posY;
      float rayDirX = dirX + planeX * cameraX;
      float rayDirY = dirY + planeY * cameraX;

      //which box of the map we're in
      int mapX = int(rayPosX);
      int mapY = int(rayPosY);

      //length of ray from current position to next x or y-side
      float sideDistX;
      float sideDistY;

      //length of ray from one x or y-side to next x or y-side
      float deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
      float deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
      float perpWallDist;

      //what direction to step in x or y-direction (either +1 or -1)
      int stepX;
      int stepY;

      int hit = 0; //was there a wall hit?
      int side; //was a NS or a EW wall hit?

      //calculate step and initial sideDist
      if (rayDirX < 0)
      {
        stepX = -1;
        sideDistX = (rayPosX - mapX) * deltaDistX;
      }
      else
      {
        stepX = 1;
        sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
      }
      if (rayDirY < 0)
      {
        stepY = -1;
        sideDistY = (rayPosY - mapY) * deltaDistY;
      }
      else
      {
        stepY = 1;
        sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
      }
      //perform DDA
      while (hit == 0)
      {
        //jump to next map square, OR in x-direction, OR in y-direction
        if (sideDistX < sideDistY)
        {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        }
        else
        {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if (worldMap[mapX][mapY] > 0) hit = 1;
      }

      //Calculate distance of perpendicular ray (oblique distance will give fisheye effect!)
      if (side == 0) perpWallDist = fabs((mapX - rayPosX + (1 - stepX) / 2) / rayDirX);
      else           perpWallDist = fabs((mapY - rayPosY + (1 - stepY) / 2) / rayDirY);

      //Calculate height of line to draw on screen
      int lineHeight = abs(int(h / perpWallDist));

      //calculate lowest and highest pixel to fill in current stripe
      int drawStart = -lineHeight / 2 + h / 2;
      if (drawStart < 0) drawStart = 0;
      int drawEnd = lineHeight / 2 + h / 2;
      if (drawEnd >= h) drawEnd = h - 1;

      //texturing calculations
      int texNum = worldMap[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!

      //calculate value of wallX
      float wallX; //where exactly the wall was hit
      if (side == 1) wallX = rayPosX + ((mapY - rayPosY + (1 - stepY) / 2) / rayDirY) * rayDirX;
      else           wallX = rayPosY + ((mapX - rayPosX + (1 - stepX) / 2) / rayDirX) * rayDirY;
      wallX -= floor((wallX));

      //x coordinate on the texture
      int texX = int(wallX * float(texWidth));
      if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
      if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

      for (int y = drawStart; y < drawEnd; y++)
      {
        int d = y * 256 - h * 128 + lineHeight * 128;  //256 and 128 factors to avoid floats
        int texY = ((d * texHeight) / lineHeight) / 256;
        uint16_t color = texture[texNum][texHeight * texY + texX];
        //make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
        //if (side == 1) color = (color >> 1) & 8355711;
        buffer[(x+(y * w))*2] = color>>8;
        buffer[(x+(y * w))*2+1] = color;
      }
    }

    display.goTo(0, 0);
    display.startData();
    display.writeBuffer(buffer, w * h * 2);
    display.endTransfer();
    
    for (int i = 0; i < w*h; i++){
      buffer[i++] = 0b01111011;
      buffer[i] = 0b11101111;
    }
    for (int i = w*h; i < w*h*2; i++){
      buffer[i++] = 0b00111001;
      buffer[i] = 0b11100111;
    }

    
    //timing for input and FPS counter
    oldTime = time;
    time = millis();
    float frameTime = (time - oldTime) / 1000.0; //frametime is the time this frame has taken, in seconds
    SerialUSB.println(1.0 / frameTime); //FPS counter

    //speed modifiers
    float moveSpeed = frameTime * 5.0; //the constant value is in squares/second
    float rotSpeed = frameTime * 3.0; //the constant value is in radians/second

    uint8_t buttonStates = display.getButtons();
    //move forward if no wall in front of you
    if (buttonStates & 2)
    {
      if (worldMap[int(posX + dirX * moveSpeed)][int(posY)] == false) posX += dirX * moveSpeed;
      if (worldMap[int(posX)][int(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
    }
    //move backwards if no wall behind you
    if (buttonStates & 1)
    {
      if (worldMap[int(posX - dirX * moveSpeed)][int(posY)] == false) posX -= dirX * moveSpeed;
      if (worldMap[int(posX)][int(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;
    }
    //rotate to the right
    if (buttonStates & 8)
    {
      //both camera direction and camera plane must be rotated
      float oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
      float oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    //rotate to the left
    if (buttonStates & 4)
    {
      //both camera direction and camera plane must be rotated
      float oldDirX = dirX;
      dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
      dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
      float oldPlaneX = planeX;
      planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
      planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }
  }
}
