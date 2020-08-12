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
    //Windows really makes things complicated...
    #define fix_rc(x) (((x) == SOCKET_ERROR) ? WSAGetLastError() : 0)
    #define sockerrno WSAGetLastError()
    
    static char* sockstrerror(int x) {
        static char line[80];
        //It's unbelievable how the Window API is so overcomplicated!
        //From https://stackoverflow.com/a/16723307/2737696
        char *s = NULL;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                       NULL, x,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPSTR)&s, 0, NULL);
        strcpy(line, s);
        LocalFree(s);
        
        //sprintf(line, "some inscrutable windows-related problem with code %d", x);
        return line;
    }
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
/* At this point, the following API can be used on either Linux or Windows:
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
*/

#define TIME_SCALE (1.0/200e-6) //realtime seconds per simulation seconds

#define NUM_KEYS 4

#define NUM_LEDS 10

#define NUM_HEX_DISPLAYS 6
#define HEX_NYBBLES (2*NUM_HEX_DISPLAYS)

typedef struct _fake_fpga {
    vpiHandle clk, buttons, keys, leds, hex; //Handles to the regs/nets in the testbench
    vpiHandle x, y, colour, plot, vga_resetn; //Nets specifically for VGA control
    int led_vc_cb_reg; //Nonzero if LED value change callback already registered
    int hex_vc_cb_reg; //Nonzero if HEX value change callback already registered
    int clk_vc_cb_reg; //Nonzero if CLK value change callback already registered
    
    int rwsync_cb_reg; //Nonzero if ReadWriteSynch callback is already registered
    int rosync_cb_reg; //Nonzero if ReadOnlySynch callback is already registered
    
    char led_new_val[NUM_LEDS + 1];    //String containing latest LED values, 
                                       //or NULs if nothing changed .
                                       //Extra byte is for NUL character
    char hex_new_val[HEX_NYBBLES + 1]; //Same idea as LEDs but for HEX
    
    char button_vals[NUM_LEDS + 1]; //Keep track of button states for displaying
    char key_vals[NUM_KEYS + 1];
    
    vpiHandle keep_alive_cb_handle; //We need to cancel keep_alive callbacks
                                    //when the simulation ends
    
} fake_fpga;

static struct timeval sim_start; //Keeps track of real time

static sockfd server = INVALID_SOCKET;

#define USAGE  "$my_task(reg clk, reg buttons[9:0], reg keys[9:0], wire leds[9:0], wire hex[47:0],\n" \
                "        wire [7:0] x, wire [6:0] y, wire [2:0] colours, wire plot, wire vga_resetn);"

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

//ReadWriteSynch callback that prints fpga state values and checks for 
//button presses
static int rw_sync(s_cb_data *dat) {
    //Grab FPGA state from callback user data
    fake_fpga *f = (fake_fpga*) dat->user_data;   
     
    //Mark that we've received the callback
    f->rwsync_cb_reg = 0;
    
    //Check if LED values have changed
    if (f->led_new_val[0] != 0) {
		char line[NUM_LEDS + 2 + 1];
		//%n in sprintf is broken on MinGW!!!! Why????
		sprintf(line, "l%s\n", f->led_new_val);

		//vpi_printf("Sent %s", line);
		//vpi_mcd_flush(1);
		
		//Use a regular blocking send on the socket. If internet is slow,
		//it makes sense that the simulation should also slow down
		send(server, line, NUM_LEDS + 2, 0);
		
		//Mark that we've seen the updated values
		f->led_new_val[0] = 0;
	}
    
    //Check if HEX values have changed
    if (f->hex_new_val[0] != 0) {
		char line[HEX_NYBBLES + 2 + 1];
		//%n in sprintf is broken on MinGW!!!! Why????
		sprintf(line, "h%s\n", f->hex_new_val);
		
		//Use a regular blocking send on the socket. If internet is slow,
		//it makes sense that the simulation should also slow down
		send(server, line, HEX_NYBBLES + 2, 0);
		
		//vpi_printf("Sent %s", line);
		//vpi_mcd_flush(1);
		
		//Mark that we've seen the updated values
		f->hex_new_val[0] = 0;
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
    
    //vpi_printf("Waiting for %d ms (disparity = %g ms)\n", disparity_ms, disparity * 1e3);
    //vpi_mcd_flush(1);
    
    struct pollfd pfd = {
        .fd = server,
        .events = POLLIN
    };
    
    int rc = poll(&pfd, 1, disparity_ms);
    if (fix_rc(rc) < 0) {
	#ifndef _WIN32
	//A rare situation where Linux is more complicated than Windows
	if (errno == EINTR) return 0; //This is not an error
	#endif
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
        } else if (rc == 0) {
            vpi_printf("GUI has closed the connection\n");
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
			if (rc < 2 || swnum < 0 || swnum >= NUM_LEDS) {
				vpi_printf("Warning: malformed switch command. Ignoring...\n");
				vpi_mcd_flush(1);
				return 0;
			}
			
			swnum = (NUM_LEDS - 1) - swnum;
			
            //vpi_printf("Setting SW %d to %d\n", swnum, val);
            
			vpi_mcd_flush(1);
			
			f->button_vals[swnum] = (val) ? '1' : '0';
            
            //vpi_printf("button_vals is now [%s]\n", f->button_vals);
            //vpi_mcd_flush(1);
			
			s_vpi_value new_vals = {
				.format = vpiBinStrVal,
				.value = {f->button_vals}
			};
			
			vpi_put_value(f->buttons, &new_vals, NULL, vpiNoDelay);
		} else if(!strcmp(msg, "KEY")) {
			if (!boundary) {
				vpi_printf("Warning: malformed key command. Ignoring...\n");
				vpi_mcd_flush(1);
				return 0;
			}
			
			int keynum, val;
			int rc = sscanf(boundary, "%d %d", &keynum, &val);
			if (rc < 2 || keynum < 0 || keynum >= NUM_KEYS) {
				vpi_printf("Warning: malformed key command. Ignoring...\n");
				vpi_mcd_flush(1);
				return 0;
			}
			
			keynum = (NUM_KEYS - 1) - keynum;
			
            //vpi_printf("Setting KEY %d to %d\n", keynum, val);
            
			vpi_mcd_flush(1);
			
			f->key_vals[keynum] = (val) ? '1' : '0';
			
			s_vpi_value new_vals = {
				.format = vpiBinStrVal,
				.value = {f->key_vals}
			};
			
			vpi_put_value(f->keys, &new_vals, NULL, vpiNoDelay);
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
    if (f->rwsync_cb_reg == 0) {
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
        f->rwsync_cb_reg = 1;
    }
}

//ReadOnlySynch callback that reads VGA control wires at the clock's
//rising edge. It's up to the person registering this callback that they
//check the clock's value before registering this callback
static int rising_edge(s_cb_data *dat) {
    //Grab FPGA state from callback user data
    fake_fpga *f = (fake_fpga*) dat->user_data;   
     
    //Mark that we've received the callback
    f->rosync_cb_reg = 0;
    
    //TODO vga_resetn
    
    //Check if plot is asserted
    s_vpi_value val = {
        .format = vpiIntVal
    };
    vpi_get_value(f->plot, &val);
    
    if(val.value.integer != 0) {
        //Read x, y, colour
        int x, y, col;
        
        vpi_get_value(f->x, &val);
        x = val.value.integer % 160; //Pixel coordinates wraparound
        vpi_get_value(f->y, &val);
        y = val.value.integer % 120;
        vpi_get_value(f->colour, &val);
        col = val.value.integer;
        
        char line[80];
        sprintf(line, "c %03d %03d %d\n", x, y, col);
	
	//vpi_printf("Sent %s", line);
	//vpi_mcd_flush(1);
        
        int rc = fix_rc(send(server, line, 12, 0));
        if (rc <= 0) {
            vpi_printf("Could not send command to GUI: %s\n", sockstrerror(rc));
            vpi_mcd_flush(1);
            vpi_control(vpiFinish, 1);
            return 0;
        }
    }
    
    return 0;
}

//Helper function to register ReadOnlySynch callback
static void reg_rising_edge_ro_sync_cb(fake_fpga *f) {
    //Register printer callback, if not already done
    if (f->rosync_cb_reg == 0) {
        //Desired time units
        s_vpi_time time_type = {
            .type = vpiSimTime
        };
        
        //Callback info for printing LED value at end of sim time
        s_cb_data cbdat = {
            .reason = cbReadOnlySynch,
            .cb_rtn = rising_edge,
            .time = &time_type,
            .user_data = (char *) f
        };
        
        //Register the callback
        vpiHandle cb_handle = vpi_register_cb(&cbdat);
        //We'll never need this handle, so free it
        vpi_free_object(cb_handle);
        
        //Signal that the callback is registered so we don't re-register it
        f->rosync_cb_reg = 1;
    }
}

//Value-change callback which registers a printer callback for the end of
//this sim time (so that values have "settled" by the time we print)
static int led_value_change(s_cb_data *dat) {
    //Grab FPGA state from callback user data
    fake_fpga *f = (fake_fpga*) dat->user_data;
    
    //Make sure we have the expected value format
    if (dat->value->format != vpiBinStrVal) {
        vpi_printf("Error: incorrect value format, expected vpiBinStrVal\n");
        vpi_control(vpiFinish, 1);
    }
    
    //Update the LED value string
    strncpy(f->led_new_val, dat->value->value.str, NUM_LEDS);
    f->led_new_val[NUM_LEDS] = 0;
    
    //Register fake I/O update callback
    reg_rw_sync_cb(f);
    
    return 0;
}

//Value-change callback which registers a printer callback for the end of
//this sim time (so that values have "settled" by the time we print)
static int hex_value_change(s_cb_data *dat) {
    //vpi_printf("hex is now [%s]\n", dat->value->value.str);
    //vpi_mcd_flush(1);
    //Grab FPGA state from callback user data
    fake_fpga *f = (fake_fpga*) dat->user_data;
    
    //Make sure we have the expected value format
    if (dat->value->format != vpiHexStrVal) {
        vpi_printf("Error: incorrect value format, expected vpiBinStrVal\n");
        vpi_control(vpiFinish, 1);
    }
    
    //Update the HEX value string
    strncpy(f->hex_new_val, dat->value->value.str, HEX_NYBBLES);
    f->hex_new_val[HEX_NYBBLES] = 0; //strncpy doesn't add the NUL, so we do it
    
    //The GUI has a bug where it doesn't accept 'Z' or 'X'
    char *p = f->hex_new_val;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    
    //Register fake I/O update callback
    reg_rw_sync_cb(f);
    
    return 0;
}

//Value-change callback for the clock. Handles VGA signals.
static int clk_value_change(s_cb_data *dat) {
    fake_fpga *f = (fake_fpga*) dat->user_data;
    
    //Register fake I/O update callback if this is a rising edge
    int clkval = dat->value->value.integer;
    if (clkval) reg_rising_edge_ro_sync_cb(f);
    return 0;
}

//Helper function to register a value-change callback
static void reg_vc_cb(vpiHandle net, PLI_INT32 format, char *user_data, PLI_INT32 (*callback)(struct t_cb_data *)) {
    //Say we want time in sim units
    s_vpi_time time_type = {
        .type = vpiSimTime
    };
    
    //We want a binary string
    s_vpi_value val_type = {
        .format = format
    };
    
    //Callback info
    s_cb_data cbdat = {
        .reason = cbValueChange,
        .cb_rtn = callback,
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

//Checks if arguments to my_task are sensible
static int my_compiletf(char* user_data) {
    //Get handle to this task call instance
    vpiHandle self = vpi_handle(vpiSysTfCall, NULL);
    
    vpiHandle clk, buttons, keys, leds, hex;
    vpiHandle x, y, colour, plot, vga_resetn;
    
    //Iterate through arguments
    vpiHandle args = vpi_iterate(vpiArgument, self);
    if (args == NULL) goto usage_error; //Error if no arguments
    
    //First argument is clock
    clk = vpi_scan(args);
    if (clk == NULL) goto usage_error;
    if (vpi_get(vpiType, clk) != vpiReg) goto usage_error;
    if (vpi_get(vpiSize, clk) != 1) goto usage_error;
    
    //Second argument is buttons
    buttons = vpi_scan(args);
    if (buttons == NULL) goto usage_error;
    if (vpi_get(vpiType, buttons) != vpiReg) goto usage_error;
    if (vpi_get(vpiSize, buttons) != 10) goto usage_error;
    
    //Third argument is keys
    keys = vpi_scan(args);
    if (keys == NULL) goto usage_error;
    if (vpi_get(vpiType, keys) != vpiReg) goto usage_error;
    if (vpi_get(vpiSize, keys) != 4) goto usage_error;
        
    //Fourth argument is leds
    leds = vpi_scan(args);
    if (leds == NULL) goto usage_error;
    if (vpi_get(vpiType, leds) != vpiNet) goto usage_error;
    if (vpi_get(vpiSize, leds) != 10) goto usage_error;
        
    //Fifth argument is hex displays
    hex = vpi_scan(args);
    if (hex == NULL) goto usage_error;
    if (vpi_get(vpiType, hex) != vpiNet) goto usage_error;
    if (vpi_get(vpiSize, hex) != 8*NUM_HEX_DISPLAYS) goto usage_error;
    
    //Sixth argument is x
    x = vpi_scan(args);
    if (x == NULL) goto usage_error;
    if (vpi_get(vpiType, x) != vpiNet) goto usage_error;
    if (vpi_get(vpiSize, x) != 8) goto usage_error;
    
    //Seventh argument is y
    y = vpi_scan(args);
    if (y == NULL) goto usage_error;
    if (vpi_get(vpiType, y) != vpiNet) goto usage_error;
    if (vpi_get(vpiSize, y) != 7) goto usage_error;
    
    //Eighth argument is colour
    colour = vpi_scan(args);
    if (colour == NULL) goto usage_error;
    if (vpi_get(vpiType, colour) != vpiNet) goto usage_error;
    if (vpi_get(vpiSize, colour) != 3) goto usage_error;
    
    //Ninth argument is plot
    plot = vpi_scan(args);
    if (plot == NULL) goto usage_error;
    if (vpi_get(vpiType, plot) != vpiNet) goto usage_error;
    if (vpi_get(vpiSize, plot) != 1) goto usage_error;
    
    //Tenth argument is vga_resetn
    vga_resetn = vpi_scan(args);
    if (vga_resetn == NULL) goto usage_error;
    if (vpi_get(vpiType, vga_resetn) != vpiNet) goto usage_error;
    if (vpi_get(vpiSize, vga_resetn) != 1) goto usage_error;
    
    //If extra arugments given, throw an error
    if (vpi_scan(args) != NULL) {
        vpi_free_object(args);
        goto usage_error;
    }
    
    //Allocate and initialize fake_fpga state struct
    fake_fpga *f = malloc(sizeof(fake_fpga));
    f->clk = clk;
    f->buttons = buttons;
    f->keys = keys;
    f->leds = leds;
    f->hex = hex;
    f->x = x; f->y = y; f->colour = colour; f->plot = plot; f->vga_resetn = vga_resetn;
    f->led_vc_cb_reg = 0;
    f->hex_vc_cb_reg = 0;
    f->clk_vc_cb_reg = 0;
    f->rwsync_cb_reg = 0;
    f->rosync_cb_reg = 0;
    strcpy(f->button_vals, "0000000000");
    strcpy(f->key_vals, "1111");
    
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
    reg_vc_cb(f->leds, vpiBinStrVal, (char*)f, led_value_change);
    //Mark that we've registered it
    f->led_vc_cb_reg = 1;
    
    //Register a value-change callback for f->hex. 
    reg_vc_cb(f->hex, vpiHexStrVal, (char*)f, hex_value_change);
    //Mark that we've registered it
    f->hex_vc_cb_reg = 1;
    
    //Register a value-change callback for f->clk. 
    reg_vc_cb(f->clk, vpiIntVal, (char*)f, clk_value_change);
    //Mark that we've registered it
    f->clk_vc_cb_reg = 1;
    
    //Apply initial button values immediately
    s_vpi_value init_val = {
        .format = vpiBinStrVal,
        .value = {
            .str = f->button_vals
        }
    };
    vpi_put_value(f->buttons, &init_val, NULL, vpiNoDelay);
    
    //Apply initial key values immediately
    init_val.value.str = f->key_vals;
    vpi_put_value(f->keys, &init_val, NULL, vpiNoDelay);
    
    //Also hook up our keep-alive callback
    reg_keep_alive_cb(f);
    
    return 0;
}

//Nitty-gritty VPI stuff for setting up custom tasks
void my_task_register() {
    s_vpi_systf_data tf_data = {
        .type      = vpiSysTask,
        .tfname    = "$fake_fpga",
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
