# Geometric Modeling

CAD software developed as a project for Geometric Modeling 1 (Modelowanie Geometryczne 1) at Faculty of Mathematics and Information Science of Warsaw University of Technology.

Written in C++ using DirectX11 as rendering engine and Qt6 for GUI.

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
- Saving and loading scene from compatible JSON file format.

## Modes

Interaction with objects depends on current mode, which is show in message bar at the bottom.

- SELECT
- MOVE
- ROTATE (WIP)
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
