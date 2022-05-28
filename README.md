# OpenGL-Tetris

A basic Tetris implementation meant as a showcase of computer graphics techniques for the University of Delaware's Computer Graphics course.
The game uses OpenGL 3.3.

Some code and OpenGL guidance from [these tutorials][tutorials].
Tetris guidance from [The Tetris Guideline], though I have not implemented it perfectly or completely.

Some attempts were made to clean the code after submitting it as the final project.

TODO:
- Further clean code
  - Move some code out of main.cpp into separate files/classes  
- Game over state
- Hard drop preview
- Board outline

## Controls

### Game

| Key | Action |
| ---: | --- |
| A | move falling piece left |
| D | move falling piece right |
| W | soft drop |
| S | hard drop |
| Left | rotate falling piece left (CCW) |
| Right | rotate falling piece right (CW) |
| LeftShift | hold piece |

### Camera

The following keys can be pressed to move the camera to the corresponding position.

| | Left | Center | Right |
| :---: | :---: | :---: | :---: |
| **Top** | Y | U | I |
| **Center** | H | J | K |
| **Bottom** | B | N | M |

G moves the camera to a side view specifically included to show that the currently held piece is displayed using the billboard/impostor technique.

### Lighting

| Keys | Action |
| ---: | --- |
| Ins/Del | increase/decrease ambient lighting component |
| Home/End | increase/decrease diffuse lighting component |
| PageUp/PageDown | increase/decrease specular lighting exponent |
| Keypad 7/9 | decrease/increase light's x position |
| Keypad 4/6 | decrease/increase light's y position |
| Keypad 1/3 | decrease/increase light's z position |

## Other Acknowledgements

.obj files made in [Blender].

[Images for the held piece billboards.][held_pieces]

All textures for the upcoming pieces from [Pixabay] under the [Pixabay License].

[tutorials]: https://www.opengl-tutorial.org/
[The Tetris Guideline]: https://tetris.wiki/Tetris_Guideline
[held_pieces]: https://commons.wikimedia.org/wiki/Category:Tetrominoes
[Blender]: https://www.blender.org/
[Pixabay]: https://pixabay.com/
[Pixabay License]: https://pixabay.com/service/license/
