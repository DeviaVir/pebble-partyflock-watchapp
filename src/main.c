/*
 * main.c
 * Sets up a Window object and pushes it onto the stack.
 */

#include <pebble.h>
#include "festiwatch.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;

static void load_homepage() {
  festiwatch_window_push();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  festiwatch_window_push();
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}