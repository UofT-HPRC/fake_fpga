#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <vpi_user.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <Ws2tcpip.h>
    
    //From https://stackoverflow.com/a/20816961/2737696
	int inet_pton(int af, const char *src, void *dst)
	{
	  struct sockaddr_storage ss;
	  int size = sizeof(ss);
	  char src_copy[INET6_ADDRSTRLEN+1];

	  ZeroMemory(&ss, sizeof(ss));
	  /* stupid non-const API */
	  strncpy (src_copy, src, INET6_ADDRSTRLEN+1);
	  src_copy[INET6_ADDRSTRLEN] = 0;

	  if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
		switch(af) {
		  case AF_INET:
		*(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
		return 1;
		  case AF_INET6:
		*(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
		return 1;
		}
	  }
	  return 0;
	}
    
    typedef SOCKET sockfd;
    
    //The things windows and MinGW make me do...
    //https://virtuallyfun.com/wordpress/2017/02/11/wsapoll-mingw/
    #ifdef __MINGW32__
        typedef struct pollfd {
            SOCKET fd;
            SHORT events;
            SHORT revents;
        } WSAPOLLFD, *PWSAPOLLFD, FAR *LPWSAPOLLFD;
        WINSOCK_API_LINKAGE int WSAAPI WSAPoll(LPWSAPOLLFD fdArray, ULONG fds, INT timeout);
        
        /* Event flag definitions for WSAPoll(). */
        #define POLLRDNORM  0x0100
        #define POLLRDBAND  0x0200
        #define POLLIN      (POLLRDNORM | POLLRDBAND)
        #define POLLPRI     0x0400

        #define POLLWRNORM  0x0010
        #define POLLOUT     (POLLWRNORM)
        #define POLLWRBAND  0x0020

        #define POLLERR     0x0001
        #define POLLHUP     0x0002
        #define POLLNVAL    0x0004
    #endif
    
    #define poll(x,y,z) WSAPoll(x,y,z)
    #define fix_rc(x) (((x) == INVALID_SOCKET) ? WSAGetLastError() : 0)
    #define sockerrno WSAGetLastError()
    #define sockstrerror(x) int_to_str(x)
    static char* int_to_str(int x) {
        static char line[80];
        sprintf(line, "some inscrutable windows-related problem with code %d", x);
        return line;
    }
#else
    #include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/time.h>
    #include <poll.h>
    typedef struct pollfd pollfd;
    typedef int sockfd;
    #define INVALID_SOCKET -1
    #define closesocket(x) close(x)
    #define fix_rc(x) (x)
    #define sockerrno errno
    #define sockstrerror(x) strerror(x)
#endif

#ifdef _WIN32
    //From https://stackoverflow.com/a/26085827/2737696

    #include <stdint.h> // portable: uint64_t   MSVC: __int64 

    struct timezone;

    int gettimeofday(struct timeval * tp, struct timezone * tzp)
    {
        // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
        // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
        // until 00:00:00 January 1, 1970 
        static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

        SYSTEMTIME  system_time;
        FILETIME    file_time;
        uint64_t    time;
        
        GetSystemTime( &system_time );
        SystemTimeToFileTime( &system_time, &file_time );
        time =  ((uint64_t)file_time.dwLowDateTime )      ;
        time += ((uint64_t)file_time.dwHighDateTime) << 32;

        tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
        tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
        
        return 0;
    }
#endif

// At this point, the following API can be used on either Linux or Windows:
// - poll can be used as normal, except for some subtle bug ms won't fix
//   (see https://curl.haxx.se/mail/lib-2012-10/0038.html). I'm just hoping
//   that I won't run into this
// - save socket file descriptors into sockfd variables
// - Use closesocket() instead of close()
// - Use INVALID_SOCKET instead of -1
// - gettimeofday can be used as normal
// - (ugh) need to wrap all calls with fix_rc() because the winsock versions
//   return INVALID_SOCKET rather than a meaningful error code. By the way,
//	 0 means success and anything else is an error number that can be passed
//	 into sockstrerror()
// - Use sockerrno instead of errno
// - Use sockstrerr(x) instead of strerror(x)

#define TIME_SCALE (1.0/5e-9) //realtime seconds per simulation seconds

typedef struct _fake_fpga {
    vpiHandle buttons, leds; //Handles to the button and LED nets
    int vc_cb_reg; //Nonzero if LED value change callback already registered
    
    char led_new_val[9]; //String containing latest LED values, or NULs if nothing changed 
                         //Extra byte is for NUL character
    int ledrd_cb_reg; //Nonzero if ReadOnlySynch callback registered for printing led values
    
    char button_vals[9]; //Keep track of button states for displaying
    
    vpiHandle keep_alive_cb_handle; //We need to cancel keep_alive callbacks
                                    //when the simulation ends
    
} fake_fpga;

static struct timeval sim_start; //Keeps track of real time

static sockfd server = INVALID_SOCKET;

#define USAGE  "fake_fpga(leds[7:0], buttons[7:0]);"

//Frees all the fake_fpga instances
static int end_of_sim_cleanup(s_cb_data *dat) {
    fake_fpga *f = (fake_fpga*) dat->user_data;
    free(f);
    
    if (server != INVALID_SOCKET) {
        closesocket(server);
    }
    
    #ifdef _WIN32
	WSACleanup();
	#endif
    
    vpi_printf("\nQuitting...\n");
    
    return 0;
}

static int start_of_sim(s_cb_data *dat) {
    vpi_printf("Time scaling: %g sim seconds per real-time second\n\n\n", 1.0f/TIME_SCALE);
    
    
    #ifdef _WIN32
	//Windows is such a thorn in my side...
	WSADATA wsa_data;
	WSAStartup(0x0202, &wsa_data);
	#endif
    
    /*
     
    // This is if the simulation is the server, but this code is commented
    // out because Ruiqi's GUI is a server instead
    
    sockfd server = socket(AF_INET, SOCK_STREAM, 0);
    
    if (server == INVALID_SOCKET) {
        vpi_printf("Could not open socket: %s\n", sockstrerror(sockerrno));
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    struct sockaddr_in local = {
        .sin_family = AF_INET,
        .sin_port = htons(5555)
    };
    
    int rc = fix_rc(bind(server, (struct sockaddr*) &local, sizeof(struct sockaddr_in)));
    if (rc < 0) {
        vpi_printf("Could not bind to port 5555: %s\n", sockstrerror(rc));
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    rc = fix_rc(listen(server, 1));
    if (rc < 0) {
        vpi_printf("Could not start listening: %s\n", sockstrerror(rc));
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    vpi_printf("Listening for incoming connections...\n");
    vpi_mcd_flush(1);
    
    client = accept(server, NULL, NULL);
    */
    
    server = socket(AF_INET, SOCK_STREAM, 0);
    
    if (server == INVALID_SOCKET) {
        vpi_printf("Could not open socket: %s\n", sockstrerror(sockerrno));
        vpi_control(vpiFinish, 1);
        return 0;
    }
    
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(54321)
    };
    
    inet_pton(AF_INET, "127.0.0.1", &(serv_addr.sin_addr));
    
    int rc = fix_rc(connect(server, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr_in)));
    
    if (rc != 0) {
        vpi_printf("Could not connect to GUI server: %s\n", sockstrerror(rc));
        vpi_control(vpiFinish, 1);
        return 0;
	}
    
    vpi_printf("Connected to server!\n");
    vpi_mcd_flush(1);
    
    //evutil_gettimeofday(&sim_start, NULL);
    gettimeofday(&sim_start, NULL);
    
    return 0;
}


//Cheesy helper function to generate nice LED display string
char *led_disp(char const *binstr) {
    static char ret[160];
    char *p = ret;
    
    int i;
    for (i = 0; i < 8; i++) {
        int incr;
        int on = (binstr[i] == '1');
        //sprintf(p, "%s  \e[49m %n", on ? "\e[41m" : "\e[44m", &incr);
        sprintf(p, "%s %n", on ? "ON" : "OF", &incr);
        //p += incr;
        p += 3;
    }
    
    return ret;
}

//Cheesy helper function to generate nice button display string
char *button_disp(char const *binstr) {
    static char ret[160];
    char *p = ret;
    
    int i;
    for (i = 0; i < 8; i++) {
        int incr;
        int on = (binstr[i] == '1');
        sprintf(p, "%s %n", on ? "^^" : "vv", &incr);
        //sprintf's %n is broken on windows!!! wtf!!!
        //p += incr;
        p += 3;
    }
    
    return ret;
}

//Dumb ReadWriteSynch callback that prints fpga state values and checks for 
//button presses
static int rw_sync(s_cb_data *dat) {
    //Grab FPGA state from callback user data
    fake_fpga *f = (fake_fpga*) dat->user_data;   
     
    //Mark that we've received the callback
    f->ledrd_cb_reg = 0;
    
    //Check if LED values have changed
    if (f->led_new_val[0] != 0) {
		char line[13];
		//%n in sprintf is broken on MinGW!!!! Why????
		sprintf(line, "l00%s\n", f->led_new_val);
		
		//Use a regular blocking send on the socket. If internet is slow,
		//it makes sense that the simulation should also slow down
		send(server, line, 12, 0);
		
		//Mark that we've seen the updated values
		f->led_new_val[0] = 0;
	}
    
    //Get current time and compare with scaled sim time
    struct timeval now;    
    gettimeofday(&now, NULL);
    
    //TODO: make time scaling a run-time argument
    double real_time = ((double) (now.tv_sec - sim_start.tv_sec) 
                        + 1e-6 * (double) (now.tv_usec - sim_start.tv_usec));
    double sim_time_ns = (double) dat->time->low / 1000.0;
    double scaled_sim_time = sim_time_ns*1e-9 * TIME_SCALE;
    double disparity = (scaled_sim_time - real_time);
    
    int disparity_ms = disparity * 1e3;
    
    //Don't actually wait if the waiting time is less than 5 ms
    if (disparity_ms < 5) disparity_ms = 0;
    
    struct pollfd pfd = {
        .fd = server,
        .events = POLLIN
    };
    
    int rc = poll(&pfd, 1, disparity_ms);
    if (rc < 0) {
        vpi_printf("poll() got some kind of error: %s", sockstrerror(sockerrno));
        vpi_mcd_flush(1);
        vpi_control(vpiFinish, 1);
        return 0;
    } else if (rc > 0) {
        //vpi_printf("Got network data! Draining it...\n");
        char msg[80];
        int rc = recv(server, msg, sizeof(msg) - 1, 0);
        if (rc < 0) {
            vpi_printf("Could not read network data: %s", sockstrerror(sockerrno));
            vpi_mcd_flush(1);
            vpi_control(vpiFinish, 1);
            return 0;
        }
        msg[rc] = 0; //NUL-terminate just to cover edge cases
        
        char *boundary = strpbrk(msg, " \t\r\n\v");
        if (boundary) *boundary++ = 0;
        
        if(!strcmp(msg, "SW")) {
			if (!boundary) {
				vpi_printf("Warning: malformed switch command. Ignoring...\n");
				vpi_mcd_flush(1);
				return 0;
			}
			
			int swnum, val;
			int rc = sscanf(boundary, "%d %d", &swnum, &val);
			if (rc < 2 || swnum < 0 || swnum > 7) {
				vpi_printf("Warning: malformed switch command. Ignoring...\n");
				vpi_mcd_flush(1);
				return 0;
			}
			
			swnum = 7 - swnum;
			
			vpi_mcd_flush(1);
			
			f->button_vals[swnum] = (val) ? '1' : '0';
			
			s_vpi_value new_vals = {
				.format = vpiBinStrVal,
				.value = {f->button_vals}
			};
			
			vpi_put_value(f->buttons, &new_vals, NULL, vpiNoDelay);
		} else if (!strcmp(msg, "end")) {
			vpi_control(vpiFinish, 1);
			return 0;
		} else {
			vpi_printf("Unrecognized command: [%s]. Ignoring...\n", msg);
			vpi_mcd_flush(1);
		}
    }
    
    return 0;
}

//Helper function to register fake FPGA input/output callback
static void reg_rw_sync_cb(fake_fpga *f) {
    //Register printer callback, if not already done
    if (f->ledrd_cb_reg == 0) {
        //Desired time units
        s_vpi_time time_type = {
            .type = vpiSimTime
        };
        
        //Callback info for printing LED value at end of sim time
        s_cb_data cbdat = {
            .reason = cbReadWriteSynch,
            .cb_rtn = rw_sync,
            .time = &time_type,
            .user_data = (char *) f
        };
        
        //Register the callback
        vpiHandle cb_handle = vpi_register_cb(&cbdat);
        //We'll never need this handle, so free it
        vpi_free_object(cb_handle);
        
        //Signal that the callback is registered so we don't re-register it
        f->ledrd_cb_reg = 1;
    }
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
    f->led_new_val[8] = 0;
    
    //Register fake I/O update callback
    reg_rw_sync_cb(f);
    
    return 0;
}

//Prototype to fix circular dependency
static void reg_keep_alive_cb(fake_fpga *f);

//Callback called every 5000 sim ticks just to make sure the simulation 
//stays open.
static int keep_alive(s_cb_data *dat) {
    fake_fpga *f = (fake_fpga*) dat->user_data;
    
    //Old handle is no longer meaningful, so free it:
    vpi_free_object(f->keep_alive_cb_handle);
    //Register a new keep-alive callback
    reg_keep_alive_cb(f);
    
    //Register callback for updating I/O
    reg_rw_sync_cb(f);
    
    return 0;
}

//Helper function to register a keep-alive callback. This is actually just
//a cbAfterDelay callback for 5000 sim ticks
static void reg_keep_alive_cb(fake_fpga *f) {
    s_vpi_time delay = {
        .type = vpiSimTime,
        .low = 5000,
        .high = 0
    };
    
    s_cb_data cbdat = {
        .reason = cbAfterDelay,
        .cb_rtn = keep_alive,
        .time = &delay,
        .user_data = (char *) f
    };
    
    f->keep_alive_cb_handle = vpi_register_cb(&cbdat);
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
    
    //vpi_printf("buttons has type %d and name %s\n", vpi_get(vpiType, buttons), vpi_get_str(vpiFullName, buttons));
    
    //Second argument is leds
    leds = vpi_scan(args);
    if (leds == NULL) goto usage_error;
    
    //vpi_printf("leds has type %d and name %s\n", vpi_get(vpiType, leds), vpi_get_str(vpiFullName, leds));
    
    //If extra arugments given, throw an error
    if (vpi_scan(args) != NULL) {
        vpi_free_object(args);
        goto usage_error;
    }
    
    //Allocate and initialize fake_fpga state struct
    fake_fpga *f = malloc(sizeof(fake_fpga));
    f->buttons = buttons;
    f->leds = leds;
    f->vc_cb_reg = 0;
    f->ledrd_cb_reg = 0;
    strcpy(f->button_vals, "00000000");
    
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
            .str = f->button_vals
        }
    };
    
    //Apply initial button values immediately
    vpi_put_value(f->buttons, &init_buttons, NULL, vpiNoDelay);
    
    //Also hook up our keep-alive callback
    reg_keep_alive_cb(f);
    
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

void start_of_sim_cb_register() {
    s_vpi_time time_type = {
        .type = vpiSimTime
    };
    
    s_cb_data cbdat = {
        .reason = cbStartOfSimulation,
        .cb_rtn = start_of_sim,
        .time = &time_type
    };
    
    vpiHandle cb_handle = vpi_register_cb(&cbdat);
    vpi_free_object(cb_handle);
}

void (*vlog_startup_routines[])() = {
    my_task_register,
    start_of_sim_cb_register,
    NULL
};
