#ifndef MY_GUI_H
#define MY_GUI_H 1

void *start_gtk (void *arg);

//VPI code uses pthread_cond_timedwait, and GTK thread signals the condition
//when a button is clicked in the GUI
typedef struct _gui_inputs {
    int changed;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    
    int switch_states[8];
    
    int quit_sim;
} gui_inputs;

extern gui_inputs inputs;

//When VPI "notices" that an LED has changed value, it schedules an event in
//the GTK main loop with this function, which internally makes a local copy
//of the LED data and calls gdk_threads_add_idle.
//Currently, I'm using the ASCII strings that VPI generates 
void update_LEDs(char *ledstr);

#endif
