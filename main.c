#include "pebble.h"

// Number of updates per second, must be < 1000
#define UPDATES 4

static Window *s_window;
static Layer *s_hands_layer, *s_words_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GFont s_font;
static double hand1;

static void hands_update_proc(Layer *layer, GContext *ctx) {
  GPoint center = GPoint(72 , 72);
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int16_t hand_length;
  double hand_angle;
  
//work out decimal of how far through the day you are
  hand1 = (((double)t->tm_hour * 3600) + ((double)t->tm_min * 60) + (double)t->tm_sec + (double)(time_ms(NULL, NULL) / 1000.0)) / (24*60*60); 
  
//create 3rd hand
  graphics_context_set_stroke_color(ctx, GColorGreen);
  graphics_context_set_stroke_width(ctx, 5);
  hand_length = 54;
  hand_angle = TRIG_MAX_ANGLE * hand1 * 12 * 12 ;
  GPoint third_hand = {
      .x = (int16_t)(-sin_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.x ,
      .y = (int16_t)(cos_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.y ,
  };
  graphics_draw_line(ctx, third_hand, center); 
  
//create 2nd hand
  graphics_context_set_stroke_color(ctx, GColorBlue);
  graphics_context_set_stroke_width(ctx, 7);
  hand_length = 42;
  hand_angle = TRIG_MAX_ANGLE * hand1 * 12;
  GPoint second_hand = {
      .x = (int16_t)(-sin_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.x ,
      .y = (int16_t)(cos_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.y ,
  };
  graphics_draw_line(ctx, second_hand, center); 
  
//create 1st hand
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 9);
  hand_length = 30;
  hand_angle = TRIG_MAX_ANGLE * hand1;
  GPoint first_hand = {
      .x = (int16_t)(-sin_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.x ,
      .y = (int16_t)(cos_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.y ,
  };
  graphics_draw_line(ctx, first_hand, center); 

//create 4th hand
  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 3);
  hand_length = 66;
  hand_angle = TRIG_MAX_ANGLE * hand1 * 12 * 12 * 12;
  GPoint fourth_hand = {
      .x = (int16_t)(-sin_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.x ,
      .y = (int16_t)(cos_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.y ,
  };
  graphics_draw_line(ctx, fourth_hand, center); 
  
//dot in the middle
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_circle(ctx, center, 2);
}

static void words_update_proc(Layer *layer, GContext *ctx) {
  char b[100];
  char word_buf[100];
  int i;
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char dectime[16];
  strftime(dectime, sizeof(dectime), "%T", t);

  // Draw decimal time text box
  GRect s_words = GRect(2,146,72,22);  
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, dectime, s_font, s_words, GTextOverflowModeFill, GTextAlignmentLeft,NULL);
  
  // Create duodecimal time text
  
  i = (int)(hand1 * 12) % 12 ;
  if(i==10) {
    snprintf(b, sizeof(b), "%s", "X");
  }
  else if (i==11) {
    snprintf(b, sizeof(b), "%s", "E");
  }
  else {
    snprintf(b, sizeof(b),"%d", i);
  }
  
  strcpy(word_buf,b);  
  
  i = (int)(hand1 * 12 * 12) % 12 ;
  if(i==10) {
    snprintf(b, sizeof(b), "%s", "X");
  }
  else if (i==11) {
    snprintf(b, sizeof(b), "%s", "E");
  }
  else {
    snprintf(b, sizeof(b),"%d", i);
  }
  
  strcat(word_buf,b);
  
  i = (int)(hand1 * 12 * 12 * 12) % 12 ;
  if(i==10) {
    snprintf(b, sizeof(b), "%s", "X");
  }
  else if (i==11) {
    snprintf(b, sizeof(b), "%s", "E");
  }
  else {
    snprintf(b, sizeof(b),"%d", i);
  }
  
  strcat(word_buf,b);
  strcat(word_buf,".");
  
  i = (int)(hand1 * 12 * 12 * 12 * 12) % 12 ;
  if(i==10) {
    snprintf(b, sizeof(b), "%s", "X");
  }
  else if(i==11){
    snprintf(b, sizeof(b), "%s", "E");
  }
  else{
    snprintf(b, sizeof(b),"%d", i);
  }
  
  strcat(word_buf,b);
  
  i = (int)(hand1 * 12 * 12 * 12 * 12 * 12) % 12 ;
  if(i==10) {
    snprintf(b, sizeof(b), "%s", "X");
  }
  else if (i==11) {
    snprintf(b, sizeof(b), "%s", "E");
  }
  else {
    snprintf(b, sizeof(b),"%d", i);
  }
  
  strcat(word_buf,b);
  
  GRect s_duo = GRect(86,146,58,22);  
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, word_buf, s_font, s_duo, GTextOverflowModeFill, GTextAlignmentCenter,NULL);
}

static void load_background_image() {
  gbitmap_destroy(s_background_bitmap);
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FACE);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
}

static void handle_tick(struct tm *t, TimeUnits units_changed) {

  for (int i=0; i<UPDATES; i++) {
  	app_timer_register(1000 / UPDATES * i, (void*)layer_mark_dirty, window_get_root_layer(s_window));
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Load background image
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  load_background_image();
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  // Initalize hands layer
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
  
  // Initalize words layer
  s_words_layer = layer_create(bounds);
  layer_set_update_proc(s_words_layer, words_update_proc);
  layer_add_child(window_layer, s_words_layer);

  // Load custom fonts
  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_18));
}

static void window_unload(Window *window) {
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  
  layer_destroy(s_words_layer);
  layer_destroy(s_hands_layer);
  
  fonts_unload_custom_font(s_font);
}

static void init() {
  const int inbound_size = 128;
  const int outbound_size = 128;
  app_message_open(inbound_size, outbound_size);
    
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}  
