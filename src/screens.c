#include <pebble.h>
#include "screens.h"
#include "startline.h"


// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_18_bold;
static GBitmap *s_res_bwarrow;
static TextLayer *s_textlayer_11;
static BitmapLayer *s_bitmaplayer_1;
#ifdef PBL_COLOR
#else
static InverterLayer *s_inverterlayer_1;
#endif
char *sTxt[10];
int sNum[10] = {0,1,2,3,4,5,6,7,8,9};
static char sTxtBuf[10][13];
TextLayer *l[10] = {};

void moveArrow(int ss) {
  GRect g, h;
  g = layer_get_frame((Layer *)l[ss]);
  h = layer_get_frame((Layer *)s_bitmaplayer_1);
  h.origin.x = 0;
  h.origin.y = g.origin.y + 4;
  layer_set_frame((Layer *)s_bitmaplayer_1, h);
}

static TextLayer *setTL(GRect g, GColor colour, char *name, Window *window) {
  TextLayer *t;
  t = text_layer_create(g);
  text_layer_set_background_color(t, colour);
  text_layer_set_font(t, s_res_gothic_18_bold);
  text_layer_set_text(t, name);
  layer_add_child(window_get_root_layer(window), (Layer *)t);
  return t;
}


static void initialise_ui(void) {
  s_window = window_create();
  #ifdef PBL_COLOR
     window_set_background_color(s_window, GColorBabyBlueEyes);
  #else
  window_set_fullscreen(s_window, true); // Not needed on Time
  #endif
  int i;
  
  s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_res_bwarrow = gbitmap_create_with_resource(RESOURCE_ID_BWARROW);
  
  for (i = 0; i < 10; i++) {
    if (screens[i].num_fields == 0) {
      sTxt[i] = "Blank";
    } else {
      snprintf(sTxtBuf[i], sizeof(sTxtBuf[i]), "Screen %d(%d)", i+1, screens[i].num_fields);      
      sTxt[i] = sTxtBuf[i];
    }
  }
 
  l[0] = setTL(GRect(16, 20, 100, 18), GColorClear, sTxt[0], s_window);
  l[1] = setTL(GRect(16, 34, 100, 18), GColorClear, sTxt[1], s_window);
  l[2] = setTL(GRect(16, 48, 100, 18), GColorClear, sTxt[2], s_window);
  l[3] = setTL(GRect(16, 62, 100, 18), GColorClear, sTxt[3], s_window);
  l[4] = setTL(GRect(16, 76, 100, 18), GColorClear, sTxt[4], s_window);
  l[5] = setTL(GRect(16, 90, 100, 18), GColorClear, sTxt[5], s_window);
  l[6] = setTL(GRect(16, 104, 100, 18), GColorClear, sTxt[6], s_window);
  l[7] = setTL(GRect(16, 118, 100, 18), GColorClear, sTxt[7], s_window);
  l[8] = setTL(GRect(16, 132, 100, 18), GColorClear, sTxt[8], s_window);
  l[9] = setTL(GRect(16, 146, 100, 18), GColorClear, sTxt[9], s_window);
  s_textlayer_11 = setTL(GRect(3,0,138,24), GColorClear, "Reorder Screens", s_window);
  text_layer_set_text_alignment(s_textlayer_11, GTextAlignmentCenter);
  
 // Movable Arrow
  s_bitmaplayer_1 = bitmap_layer_create(GRect(0, 28, 16, 16));
  bitmap_layer_set_background_color(s_bitmaplayer_1, GColorWhite);
  bitmap_layer_set_bitmap(s_bitmaplayer_1, s_res_bwarrow);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_1);
  moveArrow(0);
  // Inverter Layer
  #ifdef PBL_COLOR
  #else
  s_inverterlayer_1 = inverter_layer_create(GRect(14, 29, 65, 15));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_inverterlayer_1);
  #endif
}

static void destroy_ui(void) {
  window_destroy(s_window);
  for (int i=0; i<10; i++)
    text_layer_destroy(l[i]);
  text_layer_destroy(s_textlayer_11);
  bitmap_layer_destroy(s_bitmaplayer_1);
  #ifdef PBL_COLOR
  #else
  inverter_layer_destroy(s_inverterlayer_1);
  #endif
  gbitmap_destroy(s_res_bwarrow);
}
// END AUTO-GENERATED UI CODE

static int s = 0;
static bool moving = false;
void moveInverter(int from, int to) {
  #ifndef PBL_COLOR
  GRect i;
  
  i.size.h = 15;
  i.size.w = 85;
  i.origin = layer_get_frame((Layer *)l[s]).origin;
  i.origin.x -= 2;
  i.origin.y += 5;
  layer_set_frame((Layer *)s_inverterlayer_1, i);
  #else //PBL_COLOR
  text_layer_set_text_color(l[to], GColorDarkCandyAppleRed);
  if (from != -1)
    text_layer_set_text_color(l[from], GColorBlack);
  #endif
}

void blankInverter() {
  #ifdef PBL_COLOR
    text_layer_set_text_color(l[s], GColorBlack);
  #else
  layer_set_frame((Layer *)s_inverterlayer_1, GRectZero);
  #endif
}

static void s_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (moving) {
    moving = false;
    blankInverter();
  }
  else {
    moving = true;
    moveInverter(-1, s);
  }
}

void swapText(int old, int new) {
 int tmp;
  tmp = sNum[old];
  sNum[old] = sNum[new];
  sNum[new] = tmp;

    text_layer_set_text(l[new], sTxt[sNum[new]]);
    text_layer_set_text(l[old], sTxt[sNum[old]]);  
}

static void s_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  int old_s;
  
  old_s = s;
  s--;
  if (s < 0)
    s = 9;
  moveArrow(s);
  if (moving) {
    moveInverter(old_s, s);
    swapText(old_s, s);
  }
}

static void s_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  int old_s;
  
  old_s = s;
  s++;
  s = s % 10;
  moveArrow(s);
  if (moving) {
    moveInverter(old_s, s);
    swapText(old_s, s);
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, s_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, s_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, s_down_click_handler);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
  int i;
  Screen scr[NUM_SCREENS];
  
  updatescreen(-1, "");
  for (i = 0; i < NUM_SCREENS; i++){
          scr[i].num_fields = screens[sNum[i]].num_fields;
          int j;
          for (j=0; j<screens[sNum[i]].num_fields; j++) // Copy the map from the current screen
            {
            scr[i].field_layer_map[j] = screens[sNum[i]].field_layer_map[j];
            scr[i].field_data_map[j] = screens[sNum[i]].field_data_map[j];
            }
  }
  for (i = 0; i< NUM_SCREENS; i++) {
          screens[i].num_fields = scr[i].num_fields;
          int j;
          for (j=0; j<scr[i].num_fields; j++) // Copy the map from the current screen
            {
            screens[i].field_layer_map[j] = scr[i].field_layer_map[j];
            screens[i].field_data_map[j] = scr[i].field_data_map[j];
            }
  }
  currentScreen = 0;
  updatescreen(currentScreen, "");
}

void show_screens(void) {
  initialise_ui();
  moving = false;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "done ui free=%d", (int)heap_bytes_free());
  int i;
  for (i = 0; i< NUM_SCREENS; i++)
    sNum[i] = i;
  s = 0;
  #ifdef PBL_COLOR
  #else
  layer_set_frame((Layer *)s_inverterlayer_1, GRectZero);
  #endif
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });

  window_stack_push(s_window, true);
}

void hide_screens(void) {
  window_stack_remove(s_window, true);
}
