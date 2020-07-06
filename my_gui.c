#include <gtk/gtk.h>
#include <string.h>
#include "my_gui.h"

//Named colour constants for LEDs
static double const led_on[3] = {1.0, 0.0, 0.2};
static double const led_off[3] = {0.5, 0.0, 0.1};

//Easiest way to keep track of LED states was in a global. Meh it's not
//that big a deal
static GtkWidget *sws[8];
static GtkWidget *leds[8];
static double const *led_states[8];

static gboolean print_msg(GtkSwitch *sw, gboolean state, gpointer data) {
    double const **ls = (double const **) data;
    int ind = ls - led_states;
    g_print ("Switch %d was turned %s\n", ind, state ? "on" : "off");
    //*ls = (state == TRUE) ? led_on : led_off;
    //gtk_widget_queue_draw(leds[ind]);
    return 0;
}

static gboolean draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data) {    
    guint width, height;
    GtkStyleContext *context;

    context = gtk_widget_get_style_context (widget);

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    gtk_render_background (context, cr, 0, 0, width, height);

    cairo_arc (cr,
             width / 2.0, height / 2.0,
             MIN (width, height) / 2.0,
             0, 2 * G_PI);
    
    double const *col = *(double const **)data;
    cairo_set_source_rgb(cr, col[0], col[1], col[2]);

    cairo_fill (cr);

    return TRUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *grid;

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
    
    grid = gtk_grid_new ();
    gtk_container_add ((GtkContainer*)window, grid);
    
    int i;
    for (i = 0; i < 8; i++) {
        led_states[i] = led_off;
        
        leds[i] = gtk_drawing_area_new();
        gtk_widget_set_size_request (leds[i], 32, 32);
        g_signal_connect (G_OBJECT (leds[i]), "draw",
                          G_CALLBACK (draw_callback), &(led_states[i]));

        gtk_grid_attach(GTK_GRID(grid), leds[i], i, 0, 1, 1);
        
        
        sws[i] = gtk_switch_new();
        g_signal_connect(sws[i], "state-set", G_CALLBACK (print_msg), &(led_states[i]));
        gtk_grid_attach(GTK_GRID(grid), sws[i], i, 1, 1, 1);
    }
    gtk_widget_show_all (window);
}

void *start_gtk (void *arg){
    char *fake_argv[] = {"test"};
    GtkApplication *app;

    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    g_application_run (G_APPLICATION (app), 1, fake_argv);
    g_object_unref (app);

    return NULL;
}

static int update_LEDs_gtk_thread(void *arg) {
    char *new_vals = (char *) arg;
    
    int i;
    for (i = 0; i < 8; i++) {
        int change = (new_vals[i] == '1') ^ (led_states[i] == led_on);
        if (change) {
            led_states[i] = (new_vals[i] == '1') ? led_on : led_off;
            gtk_widget_queue_draw(leds[i]);
        }
    }
    
    return G_SOURCE_REMOVE;
}

void update_LEDs(char *ledstr) {
    char *local_copy = malloc(8);
    strncpy(local_copy, ledstr, 8);
    gdk_threads_add_idle(update_LEDs_gtk_thread, local_copy);
    //printf("Got LED update: %s\n", ledstr);
}
