# ImageEditor

A simple interface that allows one to open an image file and directly change 
the pixels of the image. It is made with C and the GTK4 libraries.

One is able to use tools such as a colour picker and undo/redo buttons. 
Finally, using the save button, the user is able to save their changed image 
to a newly created file. See [User Guide](##user-guide) for more info.

## Demo



## Getting Started

### Dependencies

* [GTK4](https://www.gtk.org)
* [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)
* A C compiler

### Installing Dependencies

#### On macOS

All dependencies can be installed on macOS easily through Homebrew, or manually if you prefer.
1. Install Homebrew [here](https://docs.brew.sh/Installation).
2. Install the 2 dependencies with Homebrew.
```
{
    brew install gtk4
    brew install pkgconf
}
```
3. macOS already comes with a C compiler installed (clang) so installing one isn't necessary.

### Compiling

```
gcc $(pkg-config --cflags gtk4) -o ImageEditor ImageEditor.c $(pkg-config --libs gtk4)
```

### Executing
See user guide below.
```
./ImageEditor
```

## User Guide

### Before Using

* Takes only .png files as input with the name "input.png" in the same directory. Example is attache (Can change input name/path on lines 63/69)
* The pixels are painted one by one, so using a larger resolution image will cause the pixels to be small and thus harder to see when painting.

### Tool Descriptions

* Pressing "Get Colour"/"Paint" buttons will switch between their respective modes. In "Get Colour" mode, clicking the picture will copy that pixel's colour into your paintbrush, previewed by the solid box. In "Paint" mode, clicking the picture will change that pixel's colour to that in your paintbrush.
* Undo will revert the last pixel painted to whichever colour it was last. Redo works in the opposite direction, only being able to be pressed x times if undo was pressed x times.
* Save will save the current state of the image to a new file named "inputModified.png".
* If you press undo a bit, then paint before redo-ing, the original redo actions are lost and the current paint action will be the "newest" action stored by the program.

### Implementation Info/Future Modifications
* The undo/redo functionality uses a doubly linked list of arbitrary length to track actions. Thus you shall be able to paint/undo/redo as much as the program has memory for. Each action is like 32bytes Could set a limit on how many actions to store if wanting to be able to paint endlessly.

## Authors

Kevin Schulcz