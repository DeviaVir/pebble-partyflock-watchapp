#include <pebble.h>
#include "detail.h"
#include "festiwatch.h"

static Window *s_main_window;

static TextLayer *s_area_layer;
static TextLayer *s_dj_layer;
static TextLayer *s_time_layer;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;

static AppSync s_sync;
static uint8_t s_sync_buffer[128];

static uint32_t partyId;

enum MusicKey {
  MUSIC_ICON_KEY = 0x0,         // TUPLE_INT
  MUSIC_DJ_KEY = 0x1,           // TUPLE_CSTRING
  MUSIC_TIME_KEY = 0x2,         // TUPLE_CSTRING
  MUSIC_AREA_KEY = 0x3,         // TUPLE_CSTRING
  PARTY_ID_KEY = 0x4,           // TUPLE_CSTRING
};

static const uint32_t MUSIC_ICONS[] = {
  RESOURCE_ID_IMAGE_MUSIC // 0
};

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case MUSIC_ICON_KEY:
      if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }

      s_icon_bitmap = gbitmap_create_with_resource(MUSIC_ICONS[new_tuple->value->uint8]);

      #ifdef PBL_SDK_3
        bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
      #endif

      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
    break;

    case MUSIC_DJ_KEY:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "MUSIC_DJ_KEY %s", new_tuple->value->cstring);
      // App Sync keeps new_tuple in s_sync_buffer, so we may use it directly
      text_layer_set_text(s_dj_layer, new_tuple->value->cstring);
    break;

    case MUSIC_TIME_KEY:
      text_layer_set_text(s_time_layer, new_tuple->value->cstring);
    break;

    case MUSIC_AREA_KEY:
      text_layer_set_text(s_area_layer, new_tuple->value->cstring);
    break;
  }
}

static void request_party() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Party ID: %d", (int)partyId);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    return;
  }

  // Add a key-value pair
  dict_write_uint32(iter, PARTY_ID_KEY, partyId);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  #if defined(PBL_RECT)
    s_icon_layer = bitmap_layer_create(GRect(32, 0, bounds.size.w/2, 80));
  #elif defined(PBL_ROUND)
    s_icon_layer = bitmap_layer_create(GRect(45, 0, bounds.size.w/2, 80));
  #endif
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_dj_layer = text_layer_create(GRect(0, 70, bounds.size.w, 32));
  text_layer_set_text_color(s_dj_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  text_layer_set_background_color(s_dj_layer, GColorClear);
  text_layer_set_font(s_dj_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_dj_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_dj_layer));

  s_time_layer = text_layer_create(GRect(0, 102, bounds.size.w, 32));
  text_layer_set_text_color(s_time_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_area_layer = text_layer_create(GRect(0, 130, bounds.size.w, 32));
  text_layer_set_text_color(s_area_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  text_layer_set_background_color(s_area_layer, GColorClear);
  text_layer_set_font(s_area_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_area_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_area_layer));

  Tuplet initial_values[] = {
    TupletInteger(MUSIC_ICON_KEY, (uint8_t) 0),
    TupletCString(MUSIC_DJ_KEY, "FRONTLINER"),
    TupletCString(MUSIC_TIME_KEY, "23:30 - 01:00"),
    TupletCString(MUSIC_AREA_KEY, "Hardstyle"),
  };

  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  request_party();
}

static void window_unload(Window *window) {
  if (s_icon_bitmap) {
    gbitmap_destroy(s_icon_bitmap);
  }

  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_dj_layer);
  text_layer_destroy(s_area_layer);
  bitmap_layer_destroy(s_icon_layer);
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "BUTTON DOWN");
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "BUTTON SELECT");
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = (Window *)context;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "BUTTON UP");
}

static void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
}

void detail_window_push(uint32_t id) {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);
    window_set_background_color(s_main_window, COLOR_FALLBACK(GColorRajah, GColorWhite));
    window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_main_window, true);

  app_message_open(64, 64);
}