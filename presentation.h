#ifndef PRESENTATION_H
#define PRESENTATION_H

// Logical game pixels are fitted inside the physical client area.
struct GameViewport { int x, y, width, height; };
inline GameViewport FitGameViewport(int clientWidth, int clientHeight, int gameWidth, int gameHeight)
{
    GameViewport v = {0, 0, clientWidth, clientHeight};
    if(clientWidth <= 0 || clientHeight <= 0 || gameWidth <= 0 || gameHeight <= 0) return v;
    if((long long)clientWidth * gameHeight > (long long)clientHeight * gameWidth)
        v.width = (int)((long long)clientHeight * gameWidth / gameHeight);
    else
        v.height = (int)((long long)clientWidth * gameHeight / gameWidth);
    if(v.width < 1) v.width = 1;
    if(v.height < 1) v.height = 1;
    v.x = (clientWidth - v.width) / 2;
    v.y = (clientHeight - v.height) / 2;
    return v;
}
#endif
