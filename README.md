### Abstract
A cannon game where the objective is to score as many points as possible by wrecking havoc. Destroying objects gives you points and if you successfully destroy all the destructible objects before the time runs out, you win. Otherwise you lose.

### Gameplay
![Screenshot of Cannon Game](gameplay.gif "Screenshot")
* (The quality of the gif had to be reduced to reduce its size)

### Controls:

##### Mouse:
* Scroll up/down to zoom in and zoom out
* Right click and drag to pan the scene
* Left click anywhere on the screen to get the cannon ready, and release to launch a cannonball

##### Keyboard:
* 'R' to reset the cannonball so it can be launched again if you are in a hurry
* 'UP' to zoom in
* 'DOWN' to zoom out
* 'RIGHT' to pan right
* 'LEFT' to pan left
* 'N' to pan up
* 'M' to pan down
* 'SPACE' to launch the cannon
* 'S' to decrease the launch power
* 'F' to increase the launch power
* 'A' to increase the launch angle
* 'B' to decrease the launch angle


### About the game:

* Get a score as high as possible.
* There are coins scattered around collecting which give you a 100 points
* There is a switch in the game which the cannonball can activate (by jumping on it), this would give you the locations of the 3 goals. These goals each give you 200 points.
* Killing a pig gives you a 100 points
* Breaking boxes would give you 50 points


### Features:

* No images used anywhere in the game. Everything is create by using shapes in openGL. This ensures the loading of the game is quick and efficient.
* Rendered text/numbers without the help of any libraries (only using shapes).
* A power bar on the top left of the screen.
* Cannon recoil after firing a shot.
* Collision using boxes(not circles), this is a lot more effective when blocks are of uneven size.
* Small animations where pigs/boxes rotate/topple over and pigs get black eyes indicating their health.
* Used elastic collisions between movable objects and laws or reflection for collisions with immovable objects.
* A switch/button which unlocks goals.


#### Note:

All objects are sorted into different layers. Each layer is drawn one at a time. Some layers are more prefered and will be drawn last (Like the pig layers) whereas others will be drawn earlier (Like the background layer). Within a layer objects are drawn in a lexicographical order. These two together give you the ability to draw complex objects with ease.


### Dependencies:
##### Linux/Windows/ Mac OSX - Dependencies: (Recommended)
* GLFW
* GLAD
* GLM

##### Linux - Dependencies: (alternative)
* FreeGLUT
* GLEW
* GLM
