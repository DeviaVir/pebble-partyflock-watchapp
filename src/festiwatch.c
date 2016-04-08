#include <pebble.h>
#include "festiwatch.h"
#include "detail.h"

static Window *s_main_window;

static TextLayer *s_visitor_layer;
static TextLayer *s_name_layer;
static TextLayer *s_location_layer;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL, *s_tick_bitmap, *s_up_bitmap, *s_down_bitmap;
static ActionBarLayer *s_action_bar_layer;

static AppSync s_sync;
static uint8_t s_sync_buffer[256];

static uint32_t partyId;

enum MusicKey {
  PARTY_NAME_KEY = 0x0,         // TUPLE_CSTRING
  PARTY_LOCATION_KEY = 0x1,     // TUPLE_CSTRING
  PARTY_VISITOR_KEY = 0x2,      // TUPLE_CSTRING
  PARTY_ID_KEY = 0x3,           // TUPLE_CSTRING
};

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case PARTY_NAME_KEY:
      // App Sync keeps new_tuple in s_sync_buffer, so we may use it directly
      text_layer_set_text(s_name_layer, new_tuple->value->cstring);
    break;

    case PARTY_LOCATION_KEY:
      text_layer_set_text(s_location_layer, new_tuple->value->cstring);
    break;

    case PARTY_VISITOR_KEY:
      text_layer_set_text(s_visitor_layer, new_tuple->value->cstring);
    break;

    case PARTY_ID_KEY:
      partyId = new_tuple->value->uint32;
    break;
  }
}

static void request_parties(void) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    return;
  }

  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "BUTTON DOWN");
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "BUTTON SELECT, id: %d", (int) partyId);
  detail_window_push(partyId);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "BUTTON UP");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_single_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Name 
  #if defined(PBL_RECT)
    s_name_layer = text_layer_create(GRect(0, (bounds.size.h/2)-14, bounds.size.w-20, 32));
  #elif defined(PBL_ROUND)
    s_name_layer = text_layer_create(GRect(0, (bounds.size.h/2)-14, bounds.size.w-40, 32));
  #endif
  text_layer_set_text_color(s_name_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  text_layer_set_background_color(s_name_layer, GColorClear);
  text_layer_set_font(s_name_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_name_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_name_layer));

  // Visitors
  #if defined(PBL_RECT)
    s_icon_layer = bitmap_layer_create(GRect(30, 5, bounds.size.w/2-10, 30));
  #elif defined(PBL_ROUND)
    s_icon_layer = bitmap_layer_create(GRect(30, 5, bounds.size.w/2+25, 30));
  #endif
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));
  s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_PEOPLE);
  #ifdef PBL_SDK_3
    bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  #endif
  bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  #if defined(PBL_RECT)
    s_visitor_layer = text_layer_create(GRect(0, 27, bounds.size.w-20, 32));
  #elif defined(PBL_ROUND)
    s_visitor_layer = text_layer_create(GRect(0, 27, bounds.size.w, 32));
  #endif
  text_layer_set_text_color(s_visitor_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  text_layer_set_background_color(s_visitor_layer, GColorClear);
  text_layer_set_font(s_visitor_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_visitor_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_visitor_layer));

  #if defined(PBL_RECT)
  // Location
    s_location_layer = text_layer_create(GRect(0, bounds.size.h-60, bounds.size.w-20, 32));
  #elif defined(PBL_ROUND)
    s_location_layer = text_layer_create(GRect(0, bounds.size.h-60, bounds.size.w, 32));
  #endif
  text_layer_set_text_color(s_location_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  text_layer_set_background_color(s_location_layer, GColorClear);
  text_layer_set_font(s_location_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_location_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_location_layer));

  // Action bar
  s_tick_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_TICK);
  s_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
  s_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);
  s_action_bar_layer = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar_layer, window);
  action_bar_layer_set_click_config_provider(s_action_bar_layer,
                                             click_config_provider);
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_up_bitmap);
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_down_bitmap);
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_tick_bitmap);

  // Tuplets
  Tuplet initial_values[] = {
    TupletCString(PARTY_NAME_KEY, "Loading..."),
    TupletCString(PARTY_LOCATION_KEY, ""),
    TupletCString(PARTY_VISITOR_KEY, ""),
    TupletCString(PARTY_ID_KEY, ""),
  };
  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
  request_parties();
}

static void window_unload(Window *window) {
  if (s_icon_bitmap) {
    gbitmap_destroy(s_icon_bitmap);
  }

  text_layer_destroy(s_location_layer);
  text_layer_destroy(s_name_layer);
  text_layer_destroy(s_visitor_layer);
  bitmap_layer_destroy(s_icon_layer);

  window_destroy(window);
  s_main_window = NULL;
}

void festiwatch_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_background_color(s_main_window, COLOR_FALLBACK(GColorRajah, GColorWhite));
    window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);

  app_message_open(64, 64);
}