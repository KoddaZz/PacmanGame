#include <iostream>
#include <vector>
#include <fstream>
#include "types.h"
#include "mapManagement.h"

using namespace std;

mapGrid loadMapFromFile(const string& fileName, const settings& config) {
    mapGrid roomGrid;
    ifstream mapFile (fileName);
    if (!mapFile.good()) {
        cerr << "Couldn't access map file! (" << fileName << ")" << endl;
        exit(2);
    }
    size_t height;
    mapFile >> height;
    size_t width;
    mapFile >> width;
    string input;
    getline(mapFile, input);  // gets rid of doors info
    roomGrid.assign(height, mapLine (width, config.KEmpty));
    for (mapGrid::iterator iter = roomGrid.begin(); iter != roomGrid.end() && getline(mapFile, input); ++iter) {
        size_t tmpIndex = 0;
        for (mapLine::iterator subIter = iter->begin(); subIter != iter->end() && tmpIndex < input.size(); ++subIter) {
            if (input[tmpIndex] == '\n') continue;
            *subIter = (input[tmpIndex] == ' ' ? config.KEmpty : input[tmpIndex]);
            ++tmpIndex;
        }
    }
    mapFile.close();
    return roomGrid;
}

// FUCKING DONE EZ
/* ToDo: add a map building process that uses a seed to procedurally generate a map out of rooms.
 *          -Each room has one or more doors with a certain direction
 *          -When player steps on a door, it generates a room in that direction
 *          -Rooms are named or have content to tell which directions they can accomodate
 *          -If no room fits the map, the door gets replaced with a tiny, tiny closet
 */


int placeRoom(mapGrid& gameGrid, const mapGrid& roomGrid, const size_t& x, const size_t& y) {
    if (roomGrid.size() + y > gameGrid.size()) return 2;
    if (roomGrid[0].size() + x > gameGrid[0].size()) return 3;

    mapGrid::iterator mapIter = gameGrid.begin() + y;

    for (const mapLine& lin : roomGrid) {
        mapLine::iterator mapLineIter = mapIter->begin() + x;
        for (const char& car : lin) {
            if (*mapLineIter != '#') *mapLineIter = car;
            ++mapLineIter;
        }
        ++mapIter;
    }
    return 0;
}


int loadAndPlace(mapGrid& gameGrid, const string& fileName, const size_t& x, const size_t y, const settings& config) {
    mapGrid roomGrid;
    roomGrid = loadMapFromFile(fileName, config);
    return placeRoom(gameGrid, roomGrid, x, y);
}

template<typename T>
bool isInVect(const vector<T>& vect, const T elem) {
    typename vector<T>::const_iterator iter = vect.begin();
    for (;iter < vect.end() && *iter != elem;++iter);
    return !(iter == vect.end());
}



void generateRoom(mapGrid& gameGrid, const char& desiredDoor, const CPosition& pos, const settings& config) {
    // Select a random room for the desired direction
    vector<string> blacklistedMaps;
    char desiredDirection;
    bool cannotPlace = false;
    // attempt to place room until able to (or not)
    for(;;) {
        string fileName = "rooms/";
        switch (desiredDoor) {
            case '1':
                if (blacklistedMaps.size() == roomsUp.size()){
                    cannotPlace = true;
                    break;
                }
                fileName += roomsUp[rand() % roomsUp.size()];
                desiredDirection = 'u';
                break;
            case '2':
                if (blacklistedMaps.size() == roomsRight.size()) {
                    cannotPlace = true;
                    break;
                }
                fileName += roomsRight[rand() % roomsRight.size()];
                desiredDirection = 'r';
                break;
            case '3':
                if (blacklistedMaps.size() == roomsDown.size()) {
                    cannotPlace = true;
                    break;
                }
                fileName += roomsDown[rand() % roomsDown.size()];
                desiredDirection = 'd';
                break;
            case '4':
                if (blacklistedMaps.size() == roomsLeft.size()) {
                    cannotPlace = true;
                    break;
                }
                fileName += roomsLeft[rand() % roomsLeft.size()];
                desiredDirection = 'l';
                break;
        };

        if (cannotPlace) break;

        if (isInVect(blacklistedMaps, fileName)) continue;

        // Calculate position of the room in gameGrid with door metadata in file
        ifstream tmpFile (fileName);
        if (!tmpFile.good()) {
            cerr << "Couldn't access map file! (" << fileName << ")" << endl;
            exit(2);
        }
        for (; tmpFile.get() != desiredDirection && tmpFile.peek() != tmpFile.eof(););  // get to desired door metadata
        size_t y;
        tmpFile >> y;  // get door y position relative to room origin
        size_t x;
        tmpFile >> x;  // get door x position relative to room origin
        CPosition roomOrigin = {pos.first - y, pos.second - x};
        if (loadAndPlace(gameGrid, fileName, roomOrigin.second, roomOrigin.first, config) == 0) break;
        blacklistedMaps.push_back(fileName);  // avoid trying again to place down a map that didn't work once
    }
    // if couldn't place a room, place a wall instead lmao get rekt nob
    if (cannotPlace) {
        // BIG potential for edge cases overflows lmao (get it? edge case? nvm)
        switch (desiredDoor) {
            case '1':
                //moveToken(gameGrid, 's', pos);
                gameGrid[pos.first - 1][pos.second]     = '#';
                gameGrid[pos.first - 1][pos.second + 1] = '#';
                gameGrid[pos.first - 1][pos.second - 1] = '#';
                break;
            case '2':
                gameGrid[pos.first][pos.second + 1]     = '#';
                gameGrid[pos.first + 1][pos.second + 1] = '#';
                gameGrid[pos.first - 1][pos.second + 1] = '#';
                break;
            case '3':
                gameGrid[pos.first + 1][pos.second]     = '#';
                gameGrid[pos.first + 1][pos.second + 1] = '#';
                gameGrid[pos.first + 1][pos.second - 1] = '#';
                break;
            case '4':
                gameGrid[pos.first][pos.second - 1]     = '#';
                gameGrid[pos.first + 1][pos.second - 1] = '#';
                gameGrid[pos.first - 1][pos.second - 1] = '#';
                break;
        };
    }
}


