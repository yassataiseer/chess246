#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>
#include <unistd.h>

int main() {
    std::cout << "X11 Test Program" << std::endl;
    std::cout << "DISPLAY environment variable: " 
              << (getenv("DISPLAY") ? getenv("DISPLAY") : "not set") << std::endl;
    
    // Open connection to X server
    std::cout << "Attempting to connect to X server..." << std::endl;
    Display* display = XOpenDisplay(nullptr);
    
    if (!display) {
        std::cerr << "ERROR: Could not open display" << std::endl;
        std::cerr << "Make sure X11 forwarding is enabled and DISPLAY is set correctly" << std::endl;
        return 1;
    }
    
    std::cout << "Successfully connected to X server!" << std::endl;
    
    // Get default screen
    int screen = DefaultScreen(display);
    
    // Create a simple window
    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100, 100,    // position
        300, 200,    // size
        1,           // border width
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );
    
    // Set window title
    XStoreName(display, window, "X11 Test Window");
    
    // Select input events
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    
    // Show window
    XMapWindow(display, window);
    
    std::cout << "Window created and mapped. Should be visible for 5 seconds." << std::endl;
    
    // Flush output buffer
    XFlush(display);
    
    // Keep window open for a few seconds
    sleep(5);
    
    // Close display
    XCloseDisplay(display);
    
    std::cout << "Test completed successfully." << std::endl;
    return 0;
} 