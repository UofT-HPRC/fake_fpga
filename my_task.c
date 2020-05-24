#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> //for usleep
#include <vpi_user.h>

typedef struct {
    vpiHandle buttons, leds; //Handles to the button and LED nets
    int vc_cb_reg; //Nonzero if LED value change callback already registered
    
    char led_new_val[9]; //String containing latest LED values, or NULL if nothing changed 
                         //Extra byte is for NUL character
    int ledrd_cb_reg; //Nonzero if ReadOnlyynch callback registered for printing led values
} fake_fpga;

#define USAGE  "fake_fpga(leds[7:0], buttons[7:0]);"

//Frees all the fake_fpga instances
static int end_of_sim_cleanup(s_cb_data *dat) {
    vpi_printf("Freeing memory\n");
    fake_fpga *f = (fake_fpga*) dat->user_data;
    
    free(f);
    
    return 0;
}

//Dumb ReadWriteSynch callback that prints fpga state values
static int rw_sync(s_cb_data *dat) {
    //Grab FPGA state from callback user data
    fake_fpga *f = (fake_fpga*) dat->user_data;
    
    //Time+value info
    vpi_printf("At time %05u, value = %s\n", dat->time->low, f->led_new_val);
    
    //Mark that we've finished the callback
    f->ledrd_cb_reg = 0;
    
    //End simulation past a certain point
    if (dat->time->low >= 50000) {
        vpi_control(vpiFinish, 1);
    }
    
    return 0;
}

//Value-change callback which registers a printer callback for the end of
//this sim time (so that values have "settled" by the time we print)
static int value_change(s_cb_data *dat) {
    //Grab FPGA state from callback user data
    fake_fpga *f = (fake_fpga*) dat->user_data;
    
    //Make sure we have the expected value format
    if (dat->value->format != vpiBinStrVal) {
        vpi_printf("Error: incorrect value format, expected vpiBinStrVal\n");
        vpi_control(vpiFinish, 1);
    }
    
    //Update the LED value string
    strncpy(f->led_new_val, dat->value->value.str, 8);
    
    //Register printer callback, if not already done
    if (f->ledrd_cb_reg == 0) {
        //Desired time units
        s_vpi_time time_type = {
            .type = vpiSimTime
        };
        
        //Callback info for printing LED value at end of sim time
        s_cb_data cbdat = {
            .reason = cbReadOnlySynch,
            .cb_rtn = rw_sync,
            .time = &time_type,
            .user_data = (char *) f
        };
        
        //Register the callback
        vpiHandle cb_handle = vpi_register_cb(&cbdat);
        //We'll never need this handle, so free it
        vpi_free_object(cb_handle);
        
        //Don't re-register printer callback
        f->ledrd_cb_reg = 1;
    }
    
    return 0;
}

//Helper function to register a value-change callback
static void reg_vc_cb(vpiHandle net, char *user_data) {
    //Say we want time in sim units
    s_vpi_time time_type = {
        .type = vpiSimTime
    };
    
    //We want a binary string
    s_vpi_value val_type = {
        .format = vpiBinStrVal
    };
    
    //Callback info
    s_cb_data cbdat = {
        .reason = cbValueChange,
        .cb_rtn = value_change,
        .time = &time_type,
        .value = &val_type,
        .obj = net,
        .user_data = user_data
    };
    
    //Register the callback
    vpiHandle cb_handle = vpi_register_cb(&cbdat);
    //We don't need the handle to the callback (since we never plan to cancel it)
    vpi_free_object(cb_handle);
}

//Checks if arguments to my_task are sensible
static int my_compiletf(char* user_data) {
    //Get handle to this task call instance
    vpiHandle self = vpi_handle(vpiSysTfCall, NULL);
    
    vpiHandle buttons, leds;
    
    //Iterate through arguments
    vpiHandle args = vpi_iterate(vpiArgument, self);
    if (args == NULL) goto usage_error; //Error if no arguments
    
    //First argument is buttons
    buttons = vpi_scan(args);
    if (buttons == NULL) goto usage_error;
    
    vpi_printf("buttons has type %d and name %s\n", vpi_get(vpiType, buttons), vpi_get_str(vpiFullName, buttons));
    
    //Second argument is leds
    leds = vpi_scan(args);
    if (leds == NULL) goto usage_error;
    
    vpi_printf("leds has type %d and name %s\n", vpi_get(vpiType, leds), vpi_get_str(vpiFullName, leds));
    
    //If extra arugments given, throw an error
    if (vpi_scan(args) != NULL) {
        vpi_free_object(args);
        goto usage_error;
    }
    
    vpi_printf("Allocating memory\n");
    //Allocate and initialize fake_fpga state struct
    fake_fpga *f = malloc(sizeof(fake_fpga));
    f->buttons = buttons;
    f->leds = leds;
    f->vc_cb_reg = 0;
    f->ledrd_cb_reg = 0;
    
    //Attach the allocated FPGA state struct to this task call instance
    vpi_put_userdata(self, f);
    
    //Callback info for end of sim cleanup (for freeing fake_fpga structs)
    s_cb_data dat = {
        .reason = cbEndOfSimulation,
        .cb_rtn = end_of_sim_cleanup,
        .user_data = (void*) f
    };
    
    //Register callback
    vpiHandle cb_handle = vpi_register_cb(&dat);
    //We don't need the handle (since we'll never cancel the callback)
    vpi_free_object(cb_handle);
    
    return 0;
    
    //Print helpful message and end the simulation
    usage_error:
    vpi_printf(USAGE "\n");
    vpi_control(vpiFinish, 1);
    vpi_put_userdata(self, NULL);
    return 0;
}

//Intention is to only call this once in a single initial block
//Registers value-change callbacks for LED outputs
static int my_calltf(char* user_data) {
    //Get reference to the object representing this task call instance
    vpiHandle self = vpi_handle(vpiSysTfCall, NULL);
    
    //Get the saved data for this instance
    fake_fpga *f = vpi_get_userdata(self);
    
    //Register a value-change callback for f->leds. 
    reg_vc_cb(f->leds, (char*)f);
    //Mark that we've already registered it
    f->vc_cb_reg = 1;
    
    //Initial button values
    s_vpi_value init_buttons = {
        .format = vpiBinStrVal,
        .value = {
            .str = "00001000"
        }
    };
    
    //Apply initial button values immediately
    vpi_put_value(f->buttons, &init_buttons, NULL, vpiNoDelay);
    
    return 0;
}

//Nitty-gritty VPI stuff for setting up custom tasks

void my_task_register() {
    s_vpi_systf_data tf_data = {
        .type      = vpiSysTask,
        .tfname    = "$my_task",
        .calltf    = my_calltf,
        .compiletf = my_compiletf,
        .user_data = 0
    };
    
    vpi_register_systf(&tf_data);
}

void (*vlog_startup_routines[])() = {
    my_task_register,
    NULL
};
