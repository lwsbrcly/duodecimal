#include "pebble.h"

// Number of updates per second, must be < 1000
#define UPDATES 4

#define CONFIGS_KEY 0

static int s_persist_value_read, s_persist_value_written;
static AppSync s_sync;
static uint8_t s_sync_buffer[128];

typedef struct Persist {
  char subsecond[10];
  char fourthhandoption[10];
  char thirdhandoption[10];
  char secondhandoption[10];
  char showdecimal[10];
  char showduodecimal[10];
} __attribute__((__packed__)) Persist;

Persist configs = {
  .subsecond = "on",
  .fourthhandoption = "show",
  .thirdhandoption = "show",
  .secondhandoption = "show",
  .showdecimal = "show",
  .showduodecimal = "show"
};

enum {
  SubsecondUpdate_KEY = 0x0,
  ShowFourthHand_KEY = 0x1,
  ShowThirdHand_KEY = 0x2,
  ShowSecondHand_KEY = 0x3,
  ShowDecimal_KEY = 0x4,
  ShowDuodecimal_KEY = 0x5
};

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
  if (strcmp(configs.thirdhandoption, "show") == 0) {
    graphics_context_set_stroke_color(ctx, GColorLightGray);
    graphics_context_set_stroke_width(ctx, 5);
    hand_length = 54;
    hand_angle = TRIG_MAX_ANGLE * hand1 * 12 * 12 ;
    GPoint third_hand = {
        .x = (int16_t)(-sin_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.x ,
        .y = (int16_t)(cos_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.y ,
    };
    graphics_draw_line(ctx, third_hand, center); 
  }
  
//create 2nd hand
  if (strcmp(configs.secondhandoption, "show") == 0) {
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    graphics_context_set_stroke_width(ctx, 7);
    hand_length = 42;
    hand_angle = TRIG_MAX_ANGLE * hand1 * 12;
    GPoint second_hand = {
        .x = (int16_t)(-sin_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.x ,
        .y = (int16_t)(cos_lookup(hand_angle) * (int32_t)hand_length / TRIG_MAX_RATIO) + center.y ,
    };
    graphics_draw_line(ctx, second_hand, center); 
  }
  
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
  if (strcmp(configs.fourthhandoption, "show") == 0) {
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
}

static void words_update_proc(Layer *layer, GContext *ctx) {
  char b[100];
  char word_buf[100];
  int i;
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
// Draw decimal time text box
  if (strcmp(configs.showdecimal, "show") == 0) {
    char dectime[16];
    strftime(dectime, sizeof(dectime), "%T", t);
    GRect s_words = GRect(2,146,72,22);  
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, dectime, s_font, s_words, GTextOverflowModeFill, GTextAlignmentLeft,NULL);
  }
  
// Create duodecimal time text
  if (strcmp(configs.showduodecimal, "show") == 0) {
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
}

static void load_background_image() {
  gbitmap_destroy(s_background_bitmap);
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FACE);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
}

static void handle_tick(struct tm *t, TimeUnits units_changed) {
  if (strcmp(configs.subsecond, "on") == 0) {
    for (int i=0; i<UPDATES; i++) {
    	app_timer_register(1000 / UPDATES * i, (void*)layer_mark_dirty, window_get_root_layer(s_window));
    }
  }
  else {
    layer_mark_dirty(window_get_root_layer(s_window));
  }
}


static void load_persistent_config() {
  if (persist_exists(CONFIGS_KEY)) {
    s_persist_value_read = persist_read_data(CONFIGS_KEY, &configs, sizeof(configs));
  }
}

static void save_persistent_config() {
  s_persist_value_written = persist_write_data(CONFIGS_KEY, &configs, sizeof(configs));
}


static void s_sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  
  switch (key) {
    case SubsecondUpdate_KEY:  
      strcpy(configs.subsecond, new_tuple->value->cstring);
    
      break;
    case ShowFourthHand_KEY:
      strcpy(configs.fourthhandoption, new_tuple->value->cstring);
    
      break;
    case ShowThirdHand_KEY:
      strcpy(configs.thirdhandoption, new_tuple->value->cstring);
    
      break;
    case ShowSecondHand_KEY:
      strcpy(configs.secondhandoption, new_tuple->value->cstring);
    
      break;
    case ShowDecimal_KEY:
      strcpy(configs.showdecimal, new_tuple->value->cstring);
    
      break;
    case ShowDuodecimal_KEY:
      strcpy(configs.showduodecimal, new_tuple->value->cstring);
    
      break;
  }
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  handle_tick(t, HOUR_UNIT + MINUTE_UNIT + SECOND_UNIT);
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
  
  load_persistent_config();
  
  Tuplet initial_values[] = {
    TupletCString(ShowFourthHand_KEY, configs.fourthhandoption),
    TupletCString(ShowThirdHand_KEY, configs.thirdhandoption),
    TupletCString(ShowSecondHand_KEY, configs.secondhandoption),
    TupletCString(SubsecondUpdate_KEY, configs.subsecond),
    TupletCString(ShowDecimal_KEY, configs.showdecimal),
    TupletCString(ShowDuodecimal_KEY, configs.showduodecimal),
  };
  
  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values), s_sync_tuple_changed_callback, NULL, NULL);
      
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
}

static void deinit() {
  save_persistent_config();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
  
  app_sync_deinit(&s_sync);
}

int main() {
  init();
  app_event_loop();
  deinit();
}  