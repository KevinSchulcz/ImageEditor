ImageEditor
By: Kevin Schulcz

Description:
    A simple interface that allows one to open an image file and directly
    change the pixels of the image. One is able to use tools such as a 
    colour picker and undo/redo buttons. Finally, using the save button, 
    the user is able to save their changed image to a newly created file.
    There is a sample input image in the repo.

To Run:
    Note - Requires GTK4 and pkg-config for library managment, both can be 
            downloaded through Homebrew if on Mac

    gcc $(pkg-config --cflags gtk4) -o ImageEditor ImageEditor.c $(pkg-config --libs gtk4)

To Use: 
    1. Takes only .png files as input with the name "input.png" in the same 
        directory. Example is attached. (Can change input name on lines 62/68)
    
    2. Pressing "Get Colour"/"Paint" buttons will switch between their
        respective modes. In "Get Colour" mode, clicking the picture will 
        copy that pixel's colour into your paintbrush, previewed by the solid 
        box. In "Paint" mode, clicking the picture will change that pixel's 
        colour to that in your paintbrush.
    
    3. Undo will revert the last pixel painted to whichever colour it was
        last. Redo works in the opposite direction, only being able to 
        be pressed x times if undo was pressed x times.

    4. Save will save the current state of the image to a new file named
        "inputModified.png".

To Know:
    1. The pixels are painted one by one, so using a larger resolution image 
        will cause the pixels to be small and thus harder to see when painting.
        The given example image is not the hardest to see if you paint with 
        the white colour of the moon.

    2. The undo/redo functionality uses a doubly linked list of arbitrary 
        length to track actions. Thus you shall be able to paint/undo/redo 
        as much as the program has memory for. Each action is like 32bytes. 
        Could set a limit on how many to store if wanting to be able to paint 
        endlessly.

    3. If you press undo a bit, then paint before redo-ing, the original 
        redo actions are lost and the current paint action will be the 
        "newest" action stored by the program.