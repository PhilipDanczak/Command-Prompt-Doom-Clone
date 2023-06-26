// TerminalFirstPersonShooter.cpp 
// Author: Philip Danczak
// Date: June 26, 2023
// Description: This program creates a Doom like clone using ASCII characters to draw the walls and create boundries for the player.
//

#include <iostream>
#include <string>
#include <thread>
#include <Windows.h>
#include <chrono>

#define N_SCREEN_WIDTH 120
#define N_SCREEN_HEIGHT 40

#define N_MAP_WIDTH 16
#define N_MAP_HEIGHT 16

double playerXPOS = 8.0;
double playerYPOS = 8.0;
double playerViewingAngle = 0.0;
double depthOfField = 16.0;

constexpr double playerFOV = 3.14159 / 4.0;

int main()
{
    wchar_t* screen_handle = new wchar_t[N_SCREEN_WIDTH * N_SCREEN_HEIGHT];
    HANDLE console_handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE , 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(console_handle);
    DWORD dword_bytes_written = 0;
    std::wstring map;
    map += L"################"
           L"#         #    #"
           L"#         #    #"
           L"#              #"
           L"#              #"
           L"#              #"
           L"#              #"
           L"#              #"
           L"#         ######"
           L"#  ####        #"
           L"#     #        #"
           L"#     #        #"
           L"#######   ###  #"
           L"#         #    #"
           L"#         #    #"
           L"################";

    auto timestamp1 = std::chrono::system_clock::now();
    auto timestamp2 = std::chrono::system_clock::now();



    while (true)
    {
        timestamp2 = std::chrono::system_clock::now();
        std::chrono::duration<double> currentElapsedTime = timestamp2 - timestamp1;
        timestamp1 = timestamp2;
        double elapsedTime = currentElapsedTime.count();


        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) 
        {
            playerXPOS += sin(playerViewingAngle) * 5.0 * elapsedTime;
            playerYPOS += cos(playerViewingAngle) * 5.0 * elapsedTime;
            if (map[(int)playerYPOS * N_MAP_WIDTH + (int)playerXPOS] == '#')
            {
                playerXPOS -= sin(playerViewingAngle) * 5.5 * elapsedTime;
                playerYPOS -= cos(playerViewingAngle) * 5.5 * elapsedTime;
            }
        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000) 
        {
            playerXPOS -= sin(playerViewingAngle) * 5.0 * elapsedTime;
            playerYPOS -= cos(playerViewingAngle) * 5.0 * elapsedTime;

            if (map[(int)playerYPOS * N_MAP_WIDTH + (int)playerXPOS] == '#')
            {
                playerXPOS += sin(playerViewingAngle) * 5.5 * elapsedTime;
                playerYPOS += cos(playerViewingAngle) * 5.5 * elapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            playerViewingAngle -= (1.0) * elapsedTime;
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            playerViewingAngle += (1.0) * elapsedTime;

        for (int XCoordinate = 0; XCoordinate < N_SCREEN_WIDTH; XCoordinate++)
        {
            double rayAngle = (playerViewingAngle - playerFOV / 2.0) + ((double)XCoordinate / (double)N_SCREEN_WIDTH) * playerFOV;
            double distanceToWall = 0;

            bool hitWallFlag = false;
            bool hitBorderFlag = false;

            double unitVectorX = sin(rayAngle);
            double unitVectorY = cos(rayAngle);

            while (!hitWallFlag && distanceToWall < depthOfField)
            {
                distanceToWall += 0.1;

                int calculateDistanceX = (int)(playerXPOS + unitVectorX * distanceToWall);
                int calculateDistanceY = (int)(playerYPOS + unitVectorY * distanceToWall);

                if (calculateDistanceX < 0 || calculateDistanceX >=  N_MAP_WIDTH || calculateDistanceY < 0 || calculateDistanceY >= N_MAP_HEIGHT)
                {
                    hitWallFlag = true;
                    distanceToWall = depthOfField;
                }
                else
                {
                    if (map[calculateDistanceY * N_MAP_WIDTH + calculateDistanceX] == '#')
                    {
                        hitWallFlag = true;
                        std::vector<std::pair<double, double>> ray_corners;
                        for (size_t tileX = 0; tileX < 2; tileX++)
                        {
                            for (size_t tileY = 0; tileY < 2; tileY++)
                            {
                                double verticalX = (double)calculateDistanceX + tileX - playerXPOS;
                                double verticalY = (double)calculateDistanceY + tileY - playerYPOS;
                                double displace = sqrt(verticalX * verticalX + verticalY * verticalY);
                                double dottedLine = (unitVectorX * verticalX / displace) + (unitVectorY * verticalY / displace);
                                ray_corners.push_back(std::make_pair(displace, dottedLine));
                            }
                            std::sort(ray_corners.begin(), ray_corners.end(),
                                [](const std::pair<double, double>& left, const std::pair<double, double>& right)
                                { return left.first < right.first; });


                            double boundry = 0.01;
                            if (acos(ray_corners.at(0).second) < boundry) hitBorderFlag = true;
                            if (acos(ray_corners.at(1).second) < boundry) hitBorderFlag = true;
                        }
                    }
                }
            }


            int ceilingDistance = (double)(N_SCREEN_HEIGHT / 2.0) - N_SCREEN_HEIGHT / ((double)distanceToWall);
            int floor = N_SCREEN_HEIGHT - ceilingDistance;

            short shader = ' ';
            short bottomFloor = ' ';

            if (distanceToWall <= depthOfField / 4.0)        shader = 0x2588;
            else if (distanceToWall < depthOfField / 3.0)    shader = 0x2593;
            else if (distanceToWall < depthOfField / 2.0)    shader = 0x2592;
            else if (distanceToWall < depthOfField)          shader = 0x2591;
            else                                             shader = ' ';

            if (hitBorderFlag)         shader = ' ';
            for (int YAxis = 0; YAxis < N_SCREEN_HEIGHT; ++YAxis)
            {
                if (YAxis < ceilingDistance)
                    screen_handle[YAxis * N_SCREEN_WIDTH + XCoordinate] = ' ';
                else if (YAxis > ceilingDistance && YAxis <= floor)
                    screen_handle[YAxis * N_SCREEN_WIDTH + XCoordinate] = shader;
                else 
                {
                    double bottomDistance = 1.0 - (((double)YAxis - N_SCREEN_HEIGHT / 2.0) / ((double)N_SCREEN_HEIGHT / 2.0));
                    if (bottomDistance < 0.25)         bottomFloor = '#';
                    else if (bottomDistance < 0.50)    bottomFloor = 'x';
                    else if (bottomDistance < 0.75)    bottomFloor = '-';
                    else if (bottomDistance < 0.90)    bottomFloor = '.';
                    else                       bottomFloor = ' ';

                    screen_handle[YAxis * N_SCREEN_WIDTH + XCoordinate] = bottomFloor;
                }

            }
        }

        swprintf_s(screen_handle, 40, L"X=%3.2lf, Y=%3.2lf, A=%3.2lf FPS=%3.2lf", playerXPOS, playerYPOS, playerViewingAngle, 1.0 / elapsedTime);


        for (int XTile = 0;  XTile < N_MAP_WIDTH; XTile++)
            for (int YTile = 0; YTile < N_MAP_WIDTH; YTile++)
            {
                screen_handle[(YTile + 1) * N_SCREEN_WIDTH + XTile] = map[YTile * N_MAP_WIDTH + XTile];
            }
        screen_handle[((int)playerYPOS + 1) * N_SCREEN_WIDTH + (int)playerXPOS] = 'P';

        screen_handle[N_SCREEN_WIDTH * N_SCREEN_HEIGHT - 1] = '\0';
        WriteConsoleOutputCharacter(console_handle, screen_handle, N_SCREEN_WIDTH * N_SCREEN_HEIGHT, { 0,0 }, &dword_bytes_written);
    }

    return 0;
}