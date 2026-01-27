/**
 * @file xdriver.h
 * @brief Simulation driver interface for X11.
 */

#ifndef XDRIVER_H
#define XDRIVER_H


/** @brief Default window width */
#define WWIDTH 512
/** @brief Default window height */
#define WHEIGHT 384


/** @brief Connect to the display. */
void xconnect();

/**
 * @brief Initializes main window.
 *
 * This funciton does not fail.
 */
void xinit_window();

/**
 * @brief Cleanup resources used by X11.
 *
 * This thread blocks until event loop of the main window is halted.
 */
void xdisconnect();


#endif
