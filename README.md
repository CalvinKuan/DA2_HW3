# Object Viewer (GDS & Qt)

A Qt 5 program for drawing, selecting, querying and editing Rectangle, Boundary and Path shapes.

## Features

### File

| Feature | Shortcut | Description |
|---|---|---|
| Open... | - | Open a GDS file and print its records. Press Cancel to show the sample shapes |
| Query Selected | Q | Show the type, layer and coordinates of the selected objects (several at once are supported) |
| Save Image... | Ctrl+S | Save the canvas as a png, jpg or bmp image |
| Quit | - | Exit the program |

### Edit

| Feature | Shortcut | Description |
|---|---|---|
| Rectangle | R | Click two points to draw a rectangle |
| Boundary | B | Click points, press Enter to finish; draws a polygon |
| Path | P | Click points, press Enter to finish; draws a horizontal/vertical path |
| Delete | Delete | Delete all selected objects |
| Merge Paths | M | Join the selected paths end to end into one path. Paths must be on the same layer, have the same width, and have end points within 15 px of each other |
| Move | V | Click a base point, then a destination point, to move all selected objects |
| Duplicate | Ctrl+D | Copy the selected objects; the copies become the selection |
| Set Layer... | L | Change the layer of the selected objects (0-19) |
| Rotate 90° | Ctrl+R | Rotate clockwise about the center of the selection |
| Flip Horizontal | Ctrl+H | Mirror left-right |
| Flip Vertical | Ctrl+Shift+H | Mirror top-bottom |
| Select All | Ctrl+A | Select every shape |
| Delete All | - | Remove all shapes |

### View

| Feature | Shortcut | Description |
|---|---|---|
| Fit | F | Show all shapes |
| Zoom In / Zoom Out | `[` / `]` | Zoom about the center of the window |
| Pan | Arrow keys | Pan up, down, left, right |

### Mouse

| Action | Description |
|---|---|
| Click a shape | Select it |
| Shift + click | Add a shape to, or remove it from, the selection (multi-select) |
| Click empty space | Clear the selection |
| Mouse wheel | Zoom about the cursor (disabled while a shape is being drawn) |
| Move the mouse | The cursor position (user coordinates) is shown at the bottom right of the window |

## Build and Run

Requires Qt 5 (5.12 or 5.15), g++ with C++11 support, and make.

### School CentOS-7 machine (Qt 5.12.12)

```bash
export LD_LIBRARY_PATH=/opt/qt5.12.12/5.12.12/gcc_64/lib:$LD_LIBRARY_PATH
export QT_QPA_FONTDIR=/usr/share/fonts/dejavu
/opt/qt5.12.12/5.12.12/gcc_64/bin/qmake -o Makefile object_viewer.pro
make
./object_viewer
```

### EE machine (ws26, CentOS-8, Qt 5.15.3)

```tcsh
setenv QT_QPA_FONTDIR /usr/share/fonts/dejavu
/usr/lib64/qt5/bin/qmake -o Makefile object_viewer.pro
make
./object_viewer
```

If you use bash, replace the first line with `export QT_QPA_FONTDIR=/usr/share/fonts/dejavu`.

### Ubuntu / Debian

```bash
sudo apt install build-essential qtbase5-dev qt5-qmake
qmake -o Makefile object_viewer.pro
make
./object_viewer
```

### Notes

- Always run `qmake` again to generate a fresh `Makefile`. A `Makefile` copied from another machine contains that machine's Qt paths and will not work.
- If you see strange link errors, run `make clean` and then `make` again.
- On a remote machine without a display, use X11 forwarding (`ssh -X`) so the window can be shown.

## Known Limitations

- Open parses a GDS file and prints its records to the terminal, but the window still shows the sample shapes; the GDS contents are not drawn yet.
- There is no Undo / Redo.
