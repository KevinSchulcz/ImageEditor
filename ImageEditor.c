// gcc $(pkg-config --cflags gtk4) -o ImageEditor ImageEditor.c $(pkg-config --libs gtk4)

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

// Stuct for keeping track of undo/redos
typedef struct actionNode {
    guchar oldR, oldG, oldB;
    guchar newR, newG, newB;
    int x, y;
    struct actionNode *next;
    struct actionNode *prev;
} actionNode;

actionNode *currNode = NULL;
int mode = 1; // 1 for getColour - 2 for paint
static guchar red, green, blue;
GtkWidget *image;
GdkPixbuf *pixbuf;
GtkWidget *label;
GtkWidget *colourPreview;
static char labelText[40] = "X:   Y:   Colour: ";
static void activate(GtkApplication* app, gpointer user_data);
static void draw(GtkDrawingArea *cairoContainer, cairo_t *cr, int width, int height, gpointer data);
static void clickCallback(GtkGestureClick *gesture, int n_press, double x, double y, GtkWidget *area);
static void drawColourPreview(GtkDrawingArea *cairoContainer, cairo_t *cr, int width, int height, gpointer data);
static void getColour(GtkWidget *widget, gpointer data);
static void paint(GtkWidget *widget, gpointer data);
static void undo(GtkWidget *widget, gpointer data);
static void redo(GtkWidget *widget, gpointer data);
static void save(GtkWidget *widget, gpointer data);
static void redrawImage();
void clearNodesForward(struct actionNode *node);
void addAction(guchar oldR, guchar oldG, guchar oldB, guchar newR, guchar newG, guchar newB, int x, int y);

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("id.ImageEditor", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int gtkStatus = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return gtkStatus;
}

static void activate(GtkApplication* app, gpointer user_data)
{
    // Init window
    GtkWidget *window;
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Image Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);

    // Init Grid
    GtkWidget *grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(window), grid);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);

    // Init Image
    GFile *gfile = g_file_new_for_path("input.png");
    image = gtk_picture_new_for_file(gfile);
    g_object_unref(gfile);
    gtk_widget_set_hexpand(image, TRUE);
    gtk_widget_set_vexpand(image, TRUE);
    gtk_picture_set_content_fit(GTK_PICTURE(image), GTK_CONTENT_FIT_FILL);
    pixbuf = gdk_pixbuf_new_from_file("input.png", NULL);

    // Init Label
    label = gtk_label_new_with_mnemonic(labelText);

    // Init Colour Preview
    colourPreview = gtk_drawing_area_new();
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(colourPreview), 50);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(colourPreview), 50);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(colourPreview), drawColourPreview, NULL, NULL);

    // Buttons
    GtkWidget *getColourButton = gtk_button_new_with_label("Get Colour");
    g_signal_connect(getColourButton, "clicked", G_CALLBACK(getColour), NULL);
    GtkWidget *paintButton = gtk_button_new_with_label("Paint");
    g_signal_connect(paintButton, "clicked", G_CALLBACK(paint), NULL);
    GtkWidget *undoButton = gtk_button_new_with_label("Undo");
    g_signal_connect(undoButton, "clicked", G_CALLBACK(undo), NULL);
    GtkWidget *redoButton = gtk_button_new_with_label("Redo");
    g_signal_connect(redoButton, "clicked", G_CALLBACK(redo), NULL);
    GtkWidget *saveButton = gtk_button_new_with_label("Save");
    g_signal_connect(saveButton, "clicked", G_CALLBACK(save), NULL);

    // Bind Widgets to Grid
    gtk_grid_attach(GTK_GRID(grid), image, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), colourPreview, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), getColourButton, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), paintButton, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), undoButton, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), redoButton, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), saveButton, 0, 7, 1, 1);

    // Init Click Gesture
    GtkGesture *click = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 0);
    gtk_widget_add_controller(image, GTK_EVENT_CONTROLLER(click));

    // Bind Gesture
    g_signal_connect(click, "pressed", G_CALLBACK(clickCallback), image);
    
    // Load window
    gtk_window_present(GTK_WINDOW(window));
}

static void clickCallback(GtkGestureClick *gesture, int n_press, double x, double y, GtkWidget *area)
{
    // Get colour and paint functionality depending on state
    if (mode == 1) {
        int width = gdk_pixbuf_get_width(pixbuf);
        int height = gdk_pixbuf_get_height(pixbuf);
        g_assert(x >= 0 && x < width);
        g_assert(y >= 0 && y < height);

        int imageWidth = gtk_widget_get_width(image);
        int imageHeight = gtk_widget_get_height(image);
        int pixbufWidth = gdk_pixbuf_get_width(pixbuf);
        int pixbufHeight = gdk_pixbuf_get_height(pixbuf);
        int newX = (int)(x * pixbufWidth / (double)imageWidth);
        int newY = (int)(y * pixbufHeight / (double)imageHeight);

        int numChannels = gdk_pixbuf_get_n_channels(pixbuf);
        int strideLen = gdk_pixbuf_get_rowstride(pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
        guchar *p = pixels + newY * strideLen + newX * numChannels;
        red = p[0];
        green = p[1];
        blue = p[2];
        sprintf(labelText, " X: %d Y: %d Colour: (%d, %d, %d)", (int)x, (int)y, red, green, blue);
        gtk_label_set_label(GTK_LABEL(label), labelText);
        gtk_widget_queue_draw(colourPreview);
    }
    else {
        int width = gdk_pixbuf_get_width(pixbuf);
        int height = gdk_pixbuf_get_height(pixbuf);
        g_assert(x >= 0 && x < width);
        g_assert(y >= 0 && y < height);

        int imageWidth = gtk_widget_get_width(image);
        int imageHeight = gtk_widget_get_height(image);
        int pixbufWidth = gdk_pixbuf_get_width(pixbuf);
        int pixbufHeight = gdk_pixbuf_get_height(pixbuf);
        int newX = (int)(x * pixbufWidth / (double)imageWidth);
        int newY = (int)(y * pixbufHeight / (double)imageHeight);

        int numChannels = gdk_pixbuf_get_n_channels(pixbuf);
        int strideLen = gdk_pixbuf_get_rowstride(pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
        guchar *p = pixels + newY * strideLen + newX * numChannels;

        // Save as an action before changing
        addAction(p[0], p[1], p[2], red, green, blue, newX, newY);

        p[0] = red;
        p[1] = green;
        p[2] = blue;
        redrawImage();
    }
}

// Box to preview selected colour
static void drawColourPreview(GtkDrawingArea *colourPreview, cairo_t *cr, int width, int height, gpointer data)
{
    cairo_set_source_rgb(cr, red/255.0, green/255.0, blue/255.0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
}

// Called when Get Colour button is pressed
//  switch to getColour state
static void getColour(GtkWidget *widget, gpointer data)
{
    mode = 1;
}

// Called when Paint button is pressed
//  switch to getColour state
static void paint(GtkWidget *widget, gpointer data)
{
    mode = 2;
}

// Called when Undo button is pressed
static void undo(GtkWidget *widget, gpointer data)
{
    if (currNode == NULL)
        return;
    
    // simulate what happened last action
    int numChannels = gdk_pixbuf_get_n_channels(pixbuf);
    int strideLen = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    guchar *p = pixels + currNode->y * strideLen + currNode->x * numChannels;
    p[0] = currNode->oldR;
    p[1] = currNode->oldG;
    p[2] = currNode->oldB;
    if (currNode->prev != NULL)
        currNode = currNode->prev;
    redrawImage();
}

// Called when redo button is pressed
static void redo(GtkWidget *widget, gpointer data)
{
    if (currNode == NULL)
        return;

    // simulate what happened in the next action
    int numChannels = gdk_pixbuf_get_n_channels(pixbuf);
    int strideLen = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    guchar *p = pixels + currNode->y * strideLen + currNode->x * numChannels;
    p[0] = currNode->newR;
    p[1] = currNode->newG;
    p[2] = currNode->newB;
    if (currNode->next != NULL)
        currNode = currNode->next;
    redrawImage();
}

// Called when save button is pressed
static void save(GtkWidget *widget, gpointer data)
{
    gdk_pixbuf_save(pixbuf, "inputModified.png", "png", NULL, NULL);
    printf("Picture Saved As: \"inputModified.png\".\n");
}

void addAction(guchar oldR, guchar oldG, guchar oldB, guchar newR, guchar newG, guchar newB, int x, int y) {
    actionNode *newAction = malloc(sizeof(actionNode));
    newAction->oldR = oldR;
    newAction->oldG = oldG;
    newAction->oldB = oldB;
    newAction->newR = newR;
    newAction->newG = newG;
    newAction->newB = newB;
    newAction->x = x;
    newAction->y = y;
    newAction->next = NULL;
    newAction->prev = NULL;

    // this is the first node
    if (currNode == NULL) {
        currNode = newAction;
        return;
    }

    // need to clear "next" nodes if existing
    if (currNode->next != NULL) {
        clearNodesForward(currNode->next);
        currNode->next = NULL;
    }

    // Set new action to currNode
    currNode->next = newAction;
    newAction->prev = currNode;
    currNode = newAction;

    printf("New Action Created: painted (%d, %d, %d) at (%d, %d)\n", newR, newG, newB, x, y);
}

// Recursively frees the node memory towards the front of the list
void clearNodesForward(actionNode *node)
{
    if (node->next != NULL)
        clearNodesForward(node->next);
    
    free(node);
    return;
}

// Helper function to redraw image when changes are made
static void redrawImage()
{
    gtk_picture_set_pixbuf(GTK_PICTURE(image), pixbuf);
    gtk_widget_queue_draw(image);
}