#include <pebble.h>
#define SETTINGS_KEY 1

typedef struct ClaySettings {
  GColor BackgroundColor;
  GColor NoseColor;
} ClaySettings;

static ClaySettings settings;

static void prv_default_settings() {
  settings.BackgroundColor = GColorMintGreen;
  settings.NoseColor = GColorRed;
}

static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_load_settings() {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Check for Clay settings
  Tuple *bg_color_t = dict_find(iterator, MESSAGE_KEY_BackgroundColor);
  if (bg_color_t) {
    settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);
  }

  Tuple *nose_color_t = dict_find(iterator, MESSAGE_KEY_NoseColor);
  if (nose_color_t) {
    settings.NoseColor = GColorFromHEX(nose_color_t->value->int32);
  }
  if (bg_color_t || nose_color_t) {
    prv_save_settings();
    //prv_update_display();
  }

}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


//Battery
static int s_battery_level;
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
}

// Render Screen
static Window *s_main_window;
static Layer *s_canvas_layer;


static void dither(GContext *ctx, int boundx, int boundy){
  //graphics_context_set_stroke_color(ctx,  COLOR_FALLBACK (GColorMintGreen, GColorBlack));
  for(int y = 0; y < boundy; y++) {
  for(int x = 0; x < boundx; x++) {
    if((x + y) % 2 == 0) {
      graphics_draw_pixel(ctx, GPoint(x, y));
    }
  }
}
}

static void draw_eye(GContext *ctx, GPoint center, int radius, float value, float max_value) {
  // Convert value to angle
  float angle = TRIG_MAX_ANGLE * value / max_value;

  int x = (int)(sin_lookup(angle) * (radius - (radius / 3) - 2) / TRIG_MAX_RATIO) + center.x;
  int y = (int)(-cos_lookup(angle) * (radius - (radius / 3) - 2) / TRIG_MAX_RATIO) + center.y;

  // Outer circle
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, radius);
  graphics_draw_circle(ctx, center, radius);

  // Moving dot
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(x, y), (radius / 3));

  // Center pivot (optional, looks nice)
  //graphics_context_set_fill_color(ctx, GColorClear);
  //graphics_fill_circle(ctx, center, 2);
}

static void draw_nose(GContext *ctx, GPoint center, int radius) {
  PBL_IF_COLOR_ELSE(graphics_context_set_fill_color(ctx, settings.NoseColor), graphics_context_set_fill_color(ctx, GColorBlack));
  //graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_circle(ctx, center, (radius));
  
  float angle = TRIG_MAX_ANGLE * 90/100;
  int x = (int)(sin_lookup(angle) * (radius - (radius / 3) - 2) / TRIG_MAX_RATIO) + center.x;
  int y = (int)(-cos_lookup(angle) * (radius - (radius / 3) - 2) / TRIG_MAX_RATIO) + center.y;
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(x, y), (radius / 6 ));
  }

static void draw_smile(GContext *ctx, int width, int height){
  GRect arc_rect = GRect(
  (width/2)-(width/6),
  height*7 / 12,
  width / 3,
  width / 3
  );
  
  if (gcolor_equal(settings.BackgroundColor, GColorBlack)){
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);
  } else {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_fill_color(ctx, GColorBlack);
  }
  graphics_context_set_stroke_width(ctx, 5);
  if (s_battery_level <= 20) {
    graphics_fill_circle(ctx, GPoint((width/2),height*10 / 12), (width/10));
  } else{
    graphics_draw_arc(ctx,arc_rect,GOvalScaleModeFitCircle,TRIG_MAX_ANGLE  /4 , TRIG_MAX_ANGLE * 3 /4);
  }
  
  //graphics_draw_round_rect(ctx,arc_rect,0);
}
  

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Smooth time values
  float seconds = tick_time->tm_sec;
  float minutes = tick_time->tm_min + seconds / 60.0;
  float hours = (tick_time->tm_hour % 12) + minutes / 60.0;

  GRect bounds = layer_get_bounds(layer);
  // Background
  graphics_context_set_fill_color(ctx,  COLOR_FALLBACK (settings.BackgroundColor, GColorWhite));
  graphics_context_set_stroke_color(ctx,  COLOR_FALLBACK (GColorMintGreen, GColorBlack));
  PBL_IF_COLOR_ELSE(graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone), dither(ctx,bounds.size.w,bounds.size.h));
  //graphics_context_set_stroke_color(ctx,  COLOR_FALLBACK (GColorMintGreen, GColorBlack));
  //dither(ctx,bounds.size.w,bounds.size.h);
  
  // Setup Points for eyes and nose
  GPoint left_center = GPoint(bounds.size.w / 4, bounds.size.h / 2.5);
  GPoint right_center = GPoint(bounds.size.w * 3 / 4, bounds.size.h / 2.5);
  GPoint nose_center = GPoint(bounds.size.w / 2, bounds.size.h*7 / 12);

  // Hour dial (Left Eye)
  draw_eye(ctx, left_center, (bounds.size.w / 4.5), hours, 12.0);

  // Minute dial (Right Eye)
  draw_eye(ctx, right_center, bounds.size.w / 4.5, minutes, 60.0);
  
  // Nose 
  draw_nose(ctx, nose_center, bounds.size.w / 8);
  
  draw_smile(ctx, bounds.size.w, bounds.size.h);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_canvas_layer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}






static void init() {
  // Register AppMessage callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
  prv_load_settings();
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);

  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());

  //Render stuff
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  //update every second
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}