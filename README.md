# Geometric Modeling

CAD software developed as a project for Geometric Modeling 1 (Modelowanie Geometryczne 1) at Faculty of Mathematics and Information Science of Warsaw University of Technology.

Written in C++ using DirectX11 as rendering engine and Qt6 for GUI.

### Table of Contents
**[Available primitives](#available-primitives)**<br>
**[Available functions](#available-functions)**<br>
**[Modes](#modes)**<br>
**[Usage](#usage)**<br>
**[Screenshots](#screenshots)**<br>

## Available primitives

- Point,
- Torus,
- Bezier Curve C0,
- Bezier Curve C2,
- Interpolation Curve C2,
- Surface C0,
- Surface C2.

## Available functions

- Stereoscopic rendering,
- Selecting objects from screen space (single or group),
- Filling holes between compatible C0 surfaes using Gregory patches,
- Calculating intersections between parametric objects (torus and surfaces),
- Saving and loading of scene from compatible JSON file format.
- Simulating work of 3C CNC machine utilising height maps with GPU acceleration.
- Generating 3C CNC gcode for predefined model.

## Modes

Interaction with objects depends on current mode, which is show in message bar at the bottom.

- SELECT
- MOVE
- SCALE

## Usage

New objects can be added using buttons on left side of application.
Objects in scene are displayed on list and can be selected by clicking on respective item.
Selected object's properties are displayed on right side of the screen.

### All modes

RMB - rotate camera
SCROLL - room in/out
CTRL + LMB - add new point at cursor position (if currently selected object is a curve, new point will be added to curve)

### Select mode

LMB - select object at cursor
SHIFT + LMB - add object at cursor to currently selected group

Selecting object with LMB and moving mouse without releasing button allows to move object

### Move

LMB - move selected object to position under cursor (can be holded to move continously)

### Scale

LMB + Drag - scale object with value dependend on distance moved since press down. Scaling directions are dictated by object's orientation

## Screenshots
![image](https://user-images.githubusercontent.com/35574506/214943208-3ded3b70-7fcf-445b-8613-e428c9bdeef4.png)

![image](https://user-images.githubusercontent.com/35574506/214943291-9b1a8206-10f7-4862-a144-348545a85923.png)

![image](https://user-images.githubusercontent.com/35574506/214943384-9928c54b-d194-4c51-88b5-ff446b22ef4b.png)

![image](https://user-images.githubusercontent.com/35574506/214943934-41e0af28-9267-4338-81ac-b148cec8d25a.png)

![image](https://user-images.githubusercontent.com/35574506/214944292-0f87f714-7116-425a-a942-6c556516bfa2.png)

![image](https://user-images.githubusercontent.com/35574506/214945288-aaa8470d-8b5d-4008-a556-69e230a30287.png)

![image](https://user-images.githubusercontent.com/35574506/214945502-43a47119-aee2-41f4-8605-decf49bba891.png)
