# two_player_snake
Simple console game in c++ emulating snake, with no tail, but two players.
Build on MacOS using Clion with Cmake.

## A bit about the project

This is my first project in c++, intended just to explore a bunch of the basics of the language, and produce something tangible that I could play with my 
wife who was very patient waiting for me to get to do this. 
Key things that I wanted to learn/demonstrate were:

- Creating classes to represent players, and the target (i.e. fruit, in snake terminology) to make the program reasonably object-oriented
- Using a class to manage the field of competitors (defaulting to 2) and step them through the game so that play could be readily expanded to more players. 
- Using 'ncurses' library to manage the printing of the game to the console, including allowing the screen to be resized prior to play.
- Using a separate thread to take continuous input from the keboard, without blocking game-play.

## Instructions

- You'll need to have command line tools. If you don't have it, in a terminal:

`xcode-select --install`

- If you don't have Clion, you can build and run from the terminal:

- Use [homebrew](https://brew.sh/) to install cmake:

`brew install cmake`

- Clone into this repo, once in repo:

`mkdir build`

`cd build`

`cmake ..`

`make`

`open game1`


## How to play
The two players are marked 'C' and 'A' and will keep moving in the same direction until the corresponding control key on the keyboard ('C' or 'A') is pressed. 
Once the key is pressed, the direction of the player changes 90 degrees. Which direction is randomly assiged at the game start. 
When a player reaches the target 'T' they score a point, and the target resets to a new location in the screen. 
Any player that hits the wall loses, and the game ends.
Otherwise the game proceeds until a player reaches a score of 3, when they win. (Or a maximum of 10,000 steps pass.)

