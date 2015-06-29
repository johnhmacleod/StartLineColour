#include "pebble.h"
#include "math.h"
#include "startline.h"

#ifdef PBL_COLOR
GColor8 normalTitleColour;
GColor8 invertedTitleColour;
GColor8 backgroundColour;
GColor8 normalTextColour;
GColor8 negativeTextColour;
GColor8 negativeBackgroundColour;
GColor8 greenTextColour;
GColor8 redTextColour;
GColor8 messageTextColour;
GColor8 messageOutlineColour;
GColor8 messageBackgroundColour;
#endif
  
  
GBitmap *s_res_padlock, *s_res_logo;
BitmapLayer *s_padlockLayer, *s_logo_layer;

#ifdef CONFIG_SUPPORT
// Configuration items
bool racebox = false;
bool useBold = false;
bool vibeDisconnect = true;
#ifdef PBL_COLOR
bool colourAWA = true;
#endif
#endif

Window *s_main_window;
TextLayer *s_data_layer[16];
#ifdef PBL_COLOR
TextLayer *dataInverterPT[6];
#else
InverterLayer *inverter; //Used for title inversion
InverterLayer *dataInverter[6]; // Used for negative data
#endif

TextLayer *s_data_title[20];
TextLayer *messageLayer;
#ifdef PBL_COLOR
TextLayer *messageOutline;
#endif

#ifdef CONFIG_SUPPORT  
int tickCounter = 0; 
#endif
void doupdate();
void updatescreen();


int holdThisScreen = TRANSITION_IDLE; // When this is non-zero, no auto transition
int currentScreen = 0;
int configuring = 0; // Set to 1 when configuring display
int configuring_field = 0; // Index of the title we are currently configuring
bool doubleClick = false;
bool messageClick = false;

// This structure maps the KEY values to the titles used on the screen.  
// The field_data_map array contains an index into this array so we know what data to display, so if you rearrange the order here, all stored field mappings
// will be messed up!  Always add new items to the end.
// Only add items to the end of this array

const keyTitle keyTitles[] = {
{KEY_LAY_BURN,"Lay Burn", true, false},
{KEY_LAY_DIST,"Lay Dist", true, false},
{KEY_LAY_TIME,"Lay Time", true, false},
{KEY_LINE_BURN,"Line Burn", true, false},
{KEY_LINE_DIST, "Line Dist", true, false},
{KEY_LINE_TIME,"Line Time", true, false},
{KEY_LINE_ANGLE, "Line Angle", true, false},
{KEY_SECS_TO_START, "To Start", true, false},
{KEY_LAY_SEL, "Lay Line", true, false},
{KEY_TARGET_SPEED,"Tgt Speed", false, false}, //No
{KEY_TARGET_ANGLE,"Tgt Angle", true, false}, //No
{KEY_BOAT_SOG,"SOG", false, false},
{KEY_MARK_TURN,"TURN", false, true},
{KEY_MARK_LAY_DIST,"MkLay Dst", false, true},
{KEY_HEADING_COG,"COG", false, true},
{KEY_TIME_TO_MARK, "Mins 2 Mk", false, true},
{KEY_TACK_HEADER,"Tack Hdr" , false, true},
{KEY_MARK_BEARING, "Mk Bearng", false, true}, //No
{KEY_CURRENT_MARK, "Mark", false, true},
{KEY_LAST_TACK,"Lst Tack", false, true},
{KEY_TARGET_TACK, "Tgt Tack", false, true}, //No
{KEY_MARK_DIST, "Dist 2 Mk", false, true},
{KEY_TACK_STATE, "Tack State", false, true},
{KEY_TACK_LOG, "Tack Log", false, true},
//  #define RACEBOX_START 24
{KEY_AWS, "AWS", false, true},
{KEY_AWA, "AWA", false, true},
{KEY_TWS, "TWS", false, true},
{KEY_TWA, "TWA", false, true},
{KEY_TWD, "TWD", false, true},
{KEY_DEPTH, "Depth", false, true},
{KEY_HEEL, "Heel", false, true},
{KEY_CURRENT_SPEED, "Crnt Spd", false, true},
{KEY_CURRENT_DIR, "Crnt Dir", false, true},
{KEY_BOAT_SPEED, "Boat Spd", false, true},
{KEY_VMG_WIND, "VMG Wind", false, true},
//  #define RACEBOX_END 34
};
 
int num_keytitles = sizeof(keyTitles) / sizeof(keyTitles[0]);

const Screen screenDefault[4] = {
                                      {2,
                                      {2,3},
                                      {6,7},
                                      {6,7},
                                      true
                                      },
                                      {3,
                                      {18,11,12},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      true
                                      },
                                      {4,
                                      {0,1,2,3},
                                      {8,9,10,11},
                                      {8,9,10,11},
                                      true
                                      },
                                      {6,
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      true
                                        }
                                    };





Screen screens[NUM_SCREENS] = {
                                      {2,
                                      {2,3},
                                      {6,7},
                                      {6,7},
                                      true
                                      },
                                      {3,
                                      {0,1,2},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      {THREE_FIELD_INDEX, THREE_FIELD_INDEX+1, THREE_FIELD_INDEX+2},
                                      true
                                      },
                                      {4,
                                      {0,1,2,3},
                                      {8,9,10,11},
                                      {8,9,10,11},
                                      true
                                      },
                                      {6,
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      {0,1,2,3,4,5},
                                      true
                                      }
                                    };  // Code relies on the rest of the array being zero to indicate screens not in use.

static GFont s_2_font, s_title_font, s_4_font, s_3_font,
              s_6_font, s_medium_title_font, s_large_title_font;

#ifdef PBL_COLOR
TextLayer *flash;
#else
InverterLayer *inverter;
InverterLayer *flash;
#endif
static Layer *dataLayer, *titleLayer; /* Layers to hold all the titles & data - for Z control */

void doDataInvert(int field)  // Fixed up due to loss of inverter_layer
  {
  GRect a = layer_get_frame(text_layer_get_layer(s_data_layer[screens[currentScreen].field_layer_map[field]]));
  a.origin.y += a.size.h / 4;
  a.size.h -= a.size.h/4 ;
  #ifdef PBL_COLOR
  int fd = keyTitles[screens[currentScreen].field_data_map[field]].key;
  if (fd == KEY_LAY_BURN || fd == KEY_LINE_BURN || fd == KEY_SECS_TO_START) {  // Set BURN fields green when negative
    text_layer_set_text_color(s_data_layer[screens[currentScreen].field_layer_map[field]], greenTextColour);
  }
  else { // All other fields invert (sort of!!)
    layer_set_frame(text_layer_get_layer(dataInverterPT[field]), a);
    layer_set_hidden(text_layer_get_layer(dataInverterPT[field]), false);
    text_layer_set_background_color(dataInverterPT[field], negativeBackgroundColour);
    text_layer_set_text_color(s_data_layer[screens[currentScreen].field_layer_map[field]], negativeTextColour);
  }
  #else // On old Pebble, just invert
  layer_set_bounds(inverter_layer_get_layer(dataInverter[field]), a);
  #endif

}

void setupField(int i, GFont dataFont, GFont titleFont) {
      text_layer_set_font(s_data_layer[i], dataFont);    
      text_layer_set_font(s_data_title[i], titleFont);
}

void doDataRevert(int field)
  {
  #ifdef PBL_COLOR
  layer_set_hidden(text_layer_get_layer(dataInverterPT[field]), true);
  text_layer_set_text_color(s_data_layer[screens[currentScreen].field_layer_map[field]], normalTextColour);
  #else
  layer_set_bounds(inverter_layer_get_layer(dataInverter[field]), GRectZero);
  #endif
}

#ifdef PBL_COLOR
void draw_layer(Layer *layer, GContext *gctxt)
{
  if (!layer_get_hidden(layer)) {
      graphics_context_set_fill_color(gctxt, negativeBackgroundColour);
      GRect rect = layer_get_bounds(layer);
      graphics_fill_rect(gctxt, rect, 5, GCornersAll);
  }
}
#endif

static void main_window_load(Window *window) {
#ifdef PBL_COLOR
normalTitleColour = (GColor8){.argb=((uint8_t)(0xC0|63))};
invertedTitleColour = (GColor8){.argb=((uint8_t)(0xC0|49))};
backgroundColour = (GColor8){.argb=((uint8_t)(0xC0|0))};
normalTextColour = (GColor8){.argb=((uint8_t)(0xC0|63))};
messageTextColour= (GColor8){.argb=((uint8_t)(0xC0|0))};
messageOutlineColour = (GColor8){.argb=((uint8_t)(0xC0|3))};
messageBackgroundColour = (GColor8){.argb=((uint8_t)(0xC0|63))};
greenTextColour = (GColor8){.argb=((uint8_t)(0xC0|12))};
negativeTextColour = (GColor8){.argb=((uint8_t)(0xC0|0))};
negativeBackgroundColour = GColorYellow;
redTextColour = GColorFolly;
#endif
  //APP_LOG(APP_LOG_LEVEL_ERROR, "In Main_window_load");
  // Use system font, apply it and add to Window
  #ifdef CONFIG_SUPPORT
  if (useBold) {
  s_3_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTB_64));
  s_2_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTB_59));
  s_4_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTB_48));
  s_6_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTB_47));
}
  else 
  #endif
  {
  s_3_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_64));
  s_2_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_59));
  s_4_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_50));
  s_6_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PTN_47));
  }
 
  
  s_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_large_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_medium_title_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
#ifdef PBL_COLOR
  flash = text_layer_create(GRect(2,7,7,7));
  text_layer_set_background_color(flash, GColorDarkCandyAppleRed);
  
  int jj;
  for (jj = 0; jj < 6; jj++) {
  dataInverterPT[jj] = text_layer_create(GRect(0,0,144,168));
  text_layer_set_background_color(dataInverterPT[jj], GColorBlack);
  // layer_set_bounds(text_layer_get_layer(dataInverterPT[jj]),GRectZero);  
  
  layer_set_update_proc(text_layer_get_layer(dataInverterPT[jj]), draw_layer);
  layer_set_hidden(text_layer_get_layer(dataInverterPT[jj]), true);
  }
#else
  inverter = inverter_layer_create(GRect(0,0,144,168));
  layer_set_bounds(inverter_layer_get_layer(inverter),GRectZero);
  
  flash = inverter_layer_create(GRect(2,7,7,7));
  
  int jj;
  for (jj = 0; jj < 6; jj++) {
  dataInverter[jj] = inverter_layer_create(GRect(0,0,144,168));
  layer_set_bounds(inverter_layer_get_layer(dataInverter[jj]),GRectZero);  
  }
#endif
  
  // Create Display RectAngles
  
  // Six data fields & their titles
  #define SIX_FIELD_INDEX 0
  #define SIX_FIELD_MAX 5
  const GRect sixData[6] =  {GRect(0, 2, 71, 49),GRect(73, 2, 71, 49),GRect(0, 53, 71, 49),GRect(73, 53, 71, 49),GRect(0, 105, 71, 49),GRect(73, 105, 71, 49)};
  const GRect sixTitle[6] = {GRect(0, 49, 71, 15),GRect(73, 49, 71, 15),GRect(0, 102, 71, 14),GRect(73, 102, 71, 14),GRect(0, 154, 71, 15),GRect(73, 154, 71, 15)};
  for (int i = 0; i < 6; i++) {
      s_data_layer[i] = text_layer_create(sixData[i]);
      s_data_title[i] = text_layer_create(sixTitle[i]);  
  }
  
  #define TWO_FIELD_INDEX 6
  #define TWO_FIELD_MAX 7
  s_data_layer[6] = text_layer_create(GRect(0, 2, 288, 60));
  #ifdef PBL_COLOR
  layer_set_frame((Layer *) s_data_layer[6], GRect(0, 2, 144, 60));
  layer_set_bounds((Layer *) s_data_layer[6], GRect(0, 0, 288, 60));
  #else
  layer_set_frame((Layer *) s_data_layer[6], GRect(0, 2, 144, 60));
  #endif
  s_data_title[6] = text_layer_create(GRect(0, 64, 144, 28));
  
  s_data_layer[7] = text_layer_create(GRect(0, 79, 288, 60));
  #ifdef PBL_COLOR
  layer_set_frame((Layer *) s_data_layer[7], GRect(0, 79, 144, 60));
  layer_set_bounds((Layer *) s_data_layer[7], GRect(0, 0, 288, 60));
  #else
  layer_set_frame((Layer *) s_data_layer[7], GRect(0, 79, 144, 60));
  #endif
  s_data_title[7] = text_layer_create(GRect(0, 140, 144, 28));

  

  // Four data fields & their titles
  #define FOUR_FIELD_INDEX 8
  #define FOUR_FIELD_MAX 11
  s_data_layer[8] = text_layer_create(GRect(0, 12, 142, 51));
  layer_set_frame((Layer *) s_data_layer[8], GRect(0, 12, 71, 51));
  #ifdef PBL_COLOR
  layer_set_bounds((Layer *) s_data_layer[8], GRect(0, 0, 142, 51));
  #endif
  s_data_title[8] = text_layer_create(GRect(0, 65, 71, 24));
  
  #ifdef PBL_COLORX // We can scroll fields that do not touch the left border on the Time
  s_data_layer[9] = text_layer_create(GRect(73, 12, 142, 51));
  layer_set_frame((Layer *) s_data_layer[9], GRect(73, 12, 71, 51));
  layer_set_bounds((Layer *) s_data_layer[9], GRect(0, 0, 142, 51));
  #else
  s_data_layer[9] = text_layer_create(GRect(73, 12, 71, 51));
  #endif
  s_data_title[9] = text_layer_create(GRect(73, 65, 71, 24));
  
  s_data_layer[10] = text_layer_create(GRect(0, 91, 142, 51));
  layer_set_frame((Layer *) s_data_layer[10], GRect(0, 91, 71, 51));
  #ifdef PBL_COLOR
  layer_set_bounds((Layer *) s_data_layer[10], GRect(0, 0, 142, 51));
  #endif
  s_data_title[10] = text_layer_create(GRect(0, 144, 71, 24));
  
  #ifdef PBL_COLORX
  s_data_layer[11] = text_layer_create(GRect(73, 91,  142, 51));
  layer_set_frame((Layer *) s_data_layer[11], GRect(73, 91, 71, 51));
  layer_set_bounds((Layer *) s_data_layer[11], GRect(0, 0, 142, 51));
  #else
  s_data_layer[11] = text_layer_create(GRect(73, 91,  71, 51));
  #endif
  s_data_title[11] = text_layer_create(GRect(73, 144, 71, 24));
    /*
  const GRect fourTitle[4] = {GRect(0, 65, 71, 24),GRect(73, 65, 71, 24),GRect(0, 144, 71, 24),GRect(73, 144, 71, 24)};
  for (int i = 0; i <= 4; i++)
    {
    s_data_title[i+FOUR_FIELD_INDEX] = text_layer_create(fourTitle[i]);
  }
  */
  
  // Three fields - One big, two small
  //#define THREE_FIELD_INDEX 12
  #define THREE_FIELD_MAX 14
  s_data_layer[12] = text_layer_create(GRect(0, 10, 332, 65));
  layer_set_frame((Layer *) s_data_layer[12], GRect(0, 10, 144, 65));
  #ifdef PBL_COLOR
  layer_set_bounds((Layer *) s_data_layer[12], GRect(0, 0, 332, 65));
  #endif
  s_data_title[12] = text_layer_create(GRect(0, 75, 144, 28));

  s_data_layer[13] = text_layer_create(GRect(0, 91, 150, 51));
  layer_set_frame((Layer *) s_data_layer[13], GRect(0, 91, 71, 51));
  #ifdef PBL_COLOR
  layer_set_bounds((Layer *) s_data_layer[13], GRect(0, 0, 150, 51));
  #endif
  s_data_title[13] = text_layer_create(GRect(0, 144, 71, 24));
  
  #ifdef PBL_COLORX
  s_data_layer[14] = text_layer_create(GRect(73, 91, 142, 51));
  layer_set_frame((Layer *) s_data_layer[14], GRect(73, 91, 71, 51));
  layer_set_bounds((Layer *) s_data_layer[14], GRect(0, 0, 150, 51));
  #else
  s_data_layer[14] = text_layer_create(GRect(73, 91,  71, 51));
  #endif
  s_data_title[14] = text_layer_create(GRect(73, 144, 71, 24));
  
  
  // Top title
  s_data_layer[TITLE_INDEX] = text_layer_create(GRect(0, 0, 144, 16));
  
  

  // Set up top title area
#ifdef PBL_COLOR
    text_layer_set_text_color(s_data_layer[TITLE_INDEX], normalTextColour);
#else
    text_layer_set_text_color(s_data_layer[TITLE_INDEX], GColorWhite);
#endif
    text_layer_set_text_alignment(s_data_layer[TITLE_INDEX], GTextAlignmentCenter);
    text_layer_set_text(s_data_layer[TITLE_INDEX], "StartLine");
    text_layer_set_font(s_data_layer[TITLE_INDEX], s_title_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_data_layer[TITLE_INDEX])); 
 #ifdef PBL_COLOR
  text_layer_set_background_color(s_data_layer[TITLE_INDEX], GColorClear);
  window_set_background_color(window, backgroundColour);
#else
  text_layer_set_background_color(s_data_layer[TITLE_INDEX], GColorBlack);
  window_set_background_color(window, GColorBlack);
#endif
  // Set up the messgage layer
  messageLayer = text_layer_create(GRect(10,30,124,120));
  #ifdef PBL_COLOR // On the Time, do a nice outline to the messagebox by putting a larger rectangle behind it
    messageOutline = text_layer_create(GRect(5,25,134,130));
    text_layer_set_background_color(messageOutline, GColorClear);
  #endif
  text_layer_set_background_color(messageLayer, GColorClear);
  #ifdef PBL_COLOR
  text_layer_set_text_color(messageLayer, messageTextColour);
  #else
  text_layer_set_text_color(messageLayer, GColorWhite);
  #endif
  text_layer_set_text_alignment(messageLayer, GTextAlignmentCenter);
  text_layer_set_font(messageLayer, s_large_title_font);
  
  // Add the Title layer first so it is behind the data layer
  titleLayer = layer_create(GRect(0, 0, 144, 168));
  layer_insert_below_sibling(titleLayer, (Layer *)s_data_layer[TITLE_INDEX]);
  
  
  // Now do the data layer - in front of the title layer
  dataLayer = layer_create(GRect(0, 0, 144, 168)); 
  layer_insert_below_sibling(dataLayer, titleLayer); 

  #ifdef PBL_COLOR
    // On the Time, we create some textlayers that sit behind the fields & are turned on when the data is negative
    // Do them before the data fields so they sit behind
  int ii;
  for (ii = 0; ii < 6; ii++) {
     layer_add_child(dataLayer, text_layer_get_layer(dataInverterPT[ii]));
  }
  #endif
    

// Create all the fields we will use on all screen layouts  
  int i;
  for (i =0; i < TITLE_INDEX; i++)
    {
    // Data
    text_layer_set_background_color(s_data_layer[i], GColorClear);
#ifdef PBL_COLOR
    text_layer_set_text_color(s_data_layer[i], normalTextColour);
#else
    text_layer_set_text_color(s_data_layer[i], GColorWhite);
#endif
    text_layer_set_text_alignment(s_data_layer[i], GTextAlignmentCenter);
    text_layer_set_overflow_mode(s_data_layer[i], GTextOverflowModeWordWrap);
    layer_add_child(dataLayer, text_layer_get_layer(s_data_layer[i]));
    
    //Title

    text_layer_set_background_color(s_data_title[i], GColorClear);
    #ifdef PBL_COLOR
    text_layer_set_text_color(s_data_title[i], normalTitleColour);
    #else
    text_layer_set_text_color(s_data_title[i], GColorWhite);
    #endif
    text_layer_set_text_alignment(s_data_title[i], GTextAlignmentCenter);
    
    if (i >= SIX_FIELD_INDEX && i <= SIX_FIELD_MAX) // Small title fonts on the 6 field layout
      {
      setupField(i, s_6_font, s_title_font);
    }
    else if (i >= TWO_FIELD_INDEX && i <= TWO_FIELD_MAX) // This is 2 fields
      {
      setupField(i, s_2_font, s_large_title_font);
    }

    else if (i >= FOUR_FIELD_INDEX && i <= FOUR_FIELD_MAX) // 4 field layout
      {
      setupField(i, s_4_font, s_medium_title_font);
    }
    else if (i >= THREE_FIELD_INDEX && i <= THREE_FIELD_MAX)
      {
      if (i == THREE_FIELD_INDEX) // First field is big
        {
        setupField(i, s_3_font, s_large_title_font);
      } else
        {
        setupField(i, s_4_font, s_medium_title_font);
//        text_layer_set_font(s_data_layer[i], s_4_font);    
//        text_layer_set_font(s_data_title[i], s_medium_title_font);        
      }
    }      
   
    layer_add_child(titleLayer, text_layer_get_layer(s_data_title[i]));
    
  }

  // Add the heartbeat
 #ifdef PBL_COLOR
 layer_add_child(window_get_root_layer(window), text_layer_get_layer(flash));
 #else
 layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter));

  // On the old pebble, the inverter goes in front of the data fields, so happens here
  // Consider doing the white backing field technique for both pebbles

  int ii;
  for (ii = 0; ii < 6; ii++) {
     layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(dataInverter[ii]));
  }

   layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(flash));
  #endif
    
  // Add the message windows last so they are in front of everything else
  #ifdef PBL_COLOR // Add the outline box behind the message box
      layer_add_child(window_get_root_layer(window), text_layer_get_layer(messageOutline)); 
  #endif

    layer_add_child(window_get_root_layer(window), text_layer_get_layer(messageLayer)); 
  
  // Go find a screen with some fields in use
  for (currentScreen = 0; screens[currentScreen].num_fields == 0; currentScreen++)
    ;

  // Add the padlock icon - steals the rest of my heap!!
  
  s_padlockLayer = bitmap_layer_create(GRect(128, 2, 16, 16));
  s_res_padlock = gbitmap_create_with_resource(RESOURCE_ID_PADLOCK);
  bitmap_layer_set_bitmap(s_padlockLayer, s_res_padlock);
  layer_add_child(window_get_root_layer(window), (Layer *)s_padlockLayer);
  layer_set_hidden((Layer *)s_padlockLayer, configLock == 1);
  
  // And make it the current screen
  updatescreen(currentScreen,"00"); // Make this too long & it crashes the Pebble!!!!!!
  //doDataInvert(1);
  // if (screens[currentScreen].num_fields > 2)
  //  doDataInvert(2);
  #ifdef PBL_COLOR
    s_logo_layer = bitmap_layer_create(GRect(0,0,144,168));
    s_res_logo = gbitmap_create_with_resource(RESOURCE_ID_SLPNG);
    bitmap_layer_set_bitmap(s_logo_layer, s_res_logo);
   
    layer_add_child(window_get_root_layer(window), (Layer *)s_logo_layer);
  
  static PropertyAnimation *s_property_animation;

  GRect from_frame = layer_get_frame((Layer *)s_logo_layer);
  GRect to_frame = GRect(72, 84, 0, 0);

  // Create the animation
  s_property_animation = property_animation_create_layer_frame((Layer *)s_logo_layer, &from_frame, &to_frame);
  animation_set_delay((Animation*) s_property_animation, 1000);
  animation_set_duration((Animation*) s_property_animation, 500);
  animation_set_curve((Animation*) s_property_animation, AnimationCurveEaseInOut);
  animation_schedule((Animation*) s_property_animation);

  #endif
}


void screenMessage(char *message)
  {
  static int preHoldThisScreen = TRANSITION_IDLE;
  
  if (*message != '\0') // Setting a message
    {
    updatescreen(-1,""); //Blank
    #ifdef PBL_COLOR
    #else
    // text_layer_set_background_color(messageLayer, GColorBlack);
    layer_set_bounds(inverter_layer_get_layer(inverter), GRect(0,0,0,0)); // Turn off the title inverter layer
    #endif
    preHoldThisScreen = holdThisScreen; // Remember what it was
    holdThisScreen = -1;
    messageClick = true;
    #ifdef PBL_COLOR
    text_layer_set_background_color(messageLayer, messageBackgroundColour);
    text_layer_set_background_color(messageOutline, messageOutlineColour);
    #endif
    text_layer_set_text(messageLayer, message);
  }
  else //Clearing a message
    {
    messageClick = false;
    holdThisScreen = preHoldThisScreen; // Restore it to what it was
    text_layer_set_text(messageLayer,"");  
    #ifdef PBL_COLOR
    text_layer_set_background_color(messageLayer, GColorClear);
    text_layer_set_background_color(messageOutline, GColorClear);
    #endif
    updatescreen(currentScreen, "");
  }
  
}

static void main_window_unload(Window *window) {
  int i;
  for (i=0; i<TITLE_INDEX; i++)
    {
    text_layer_destroy(s_data_layer[i]);
    text_layer_destroy(s_data_title[i]);
  }
  
  layer_destroy(dataLayer);
  layer_destroy(titleLayer);
  #ifdef PBL_COLOR
  text_layer_destroy(messageOutline);
  #endif
  text_layer_destroy(messageLayer);
  text_layer_destroy(s_data_layer[TITLE_INDEX]);  
  #ifdef PBL_COLOR
  text_layer_destroy(flash);
  #else
  inverter_layer_destroy(inverter);
  inverter_layer_destroy(flash);
  #endif

  
  int jj;
  for (jj = 0; jj < 6; jj++) {
    #ifdef PBL_COLOR
    text_layer_destroy(dataInverterPT[jj]);
    #else
    inverter_layer_destroy(dataInverter[jj]);
    #endif
  }
  

  bitmap_layer_destroy(s_padlockLayer);
  gbitmap_destroy(s_res_padlock);

    
  #ifdef PBL_COLOR
  bitmap_layer_destroy(s_logo_layer);
  gbitmap_destroy(s_res_logo);
  #endif
  
  fonts_unload_custom_font(s_2_font);
  fonts_unload_custom_font(s_3_font);
  fonts_unload_custom_font(s_4_font);
  fonts_unload_custom_font(s_6_font);
}

// setField displays some data in a field & handles the reverse font

void setField(int i,  bool negNum, char* value)
  {
  static PropertyAnimation *pa1[6] = {NULL}, *pa2[6] = {NULL}; //Arrays to cope with 6 fields
    {
    static GSize textContent;
    static GRect gfrom, gto, gframe;
    TextLayer *flm = s_data_layer[screens[currentScreen].field_layer_map[i]];
    text_layer_set_text_alignment(flm, GTextAlignmentLeft);
    text_layer_set_text(flm, value); // This line only
    textContent = text_layer_get_content_size(flm);
    gfrom = layer_get_bounds((Layer *)flm);
    gframe = layer_get_frame((Layer *)flm);
    //APP_LOG(APP_LOG_LEVEL_INFO, "frame.size.w=%d bounds.size.w=%d", gframe.size.w, gfrom.size.w);
    //APP_LOG(APP_LOG_LEVEL_INFO, "gframe.size.w=%d textContent.w=%d i=%d", gframe.size.w, textContent.w, i);
    if (textContent.w > gframe.size.w) // Overflowed
      {
      // APP_LOG(APP_LOG_LEVEL_INFO, "setfield11 value=%s", value);
      if ( (pa1[i] == NULL || !animation_is_scheduled((Animation*)pa1[i])) 
          && (pa2[i] == NULL || !animation_is_scheduled((Animation*) pa2[i]))) // We are not already animating
        {
         //APP_LOG(APP_LOG_LEVEL_INFO, "setField 10");
        gto = gfrom;
        gfrom.origin.x = 0;
        gto.origin.x = (gframe.size.w - textContent.w)/2; //Work out har far left to move animate the text
         //APP_LOG(APP_LOG_LEVEL_INFO, "setField11 gfrom.x=%d gfrom.y=%d", gfrom.origin.x, gfrom.origin.y);
         //APP_LOG(APP_LOG_LEVEL_INFO, "setField11 gto.x=%d gto.y=%d", gto.origin.x, gto.origin.y);
        int tim = (int)(-2000.0 * ((float)gto.origin.x) / 20.0);
        animate_layer_bounds(&pa1[i], (Layer *)flm, &gfrom, &gto, tim, 0);
        animate_layer_bounds(&pa2[i], (Layer *)flm, &gto, &gfrom, tim, tim);
        }
      else
        {
        //APP_LOG(APP_LOG_LEVEL_INFO, "Already scheduled screen=%d i=%d %d %d %d %d", currentScreen, i, (int)pa1[i], animation_is_scheduled((Animation*)pa1[i]), (int)pa2[i], animation_is_scheduled((Animation*)pa2[i]) );
        text_layer_set_text(flm, value); // Animation running - just set the text
      }
    }
    else // We need to redraw the text centred in the reset bounds
      {
      // APP_LOG(APP_LOG_LEVEL_INFO, "setfield11 value=%s", value);
      GRect bF = layer_get_bounds((Layer *)flm);
      GRect fF = layer_get_frame((Layer *)flm);
      if (bF.size.w != fF.size.w) // is there extra space?
        {
        bF.origin.x = -(bF.size.w / 2 - fF.size.w / 2) /2;
        // APP_LOG(APP_LOG_LEVEL_INFO, "origin.x =%d", bF.origin.x);
        layer_set_bounds((Layer *)flm, bF); // Centre the Bounds below the Frame
      }
      text_layer_set_text_alignment(flm, GTextAlignmentCenter); //Should be Center but need to work out how!
      text_layer_set_text(flm, value); // This line only
    }
    
  }
  if (negNum) // Did we get a negative number
  {
    doDataInvert(i);
  } 
  else // No, positive number
  {
    doDataRevert(i);
  }
}

//
// Returns true if it can find the current key in a large field on the screen. Used to work out how to format data
//

bool isBigField(int key)
  {
  int i;
  
  int nf = screens[currentScreen].num_fields;
  for (i = 0; i < nf; i++)
    if (keyTitles[screens[currentScreen].field_data_map[i]].key == key)
      break;
  if (i == nf) // Didn't find it
    return false;
  if (nf == 2 || /* nf == 4 || */ (nf == 3 && i == 0))
    return true;
  else
    return false;
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}


// Blank Normal
void blankNormal(int screen, int field)
  {
  TextLayer *tl = s_data_layer[screens[screen].field_layer_map[field]];
    text_layer_set_text(tl, "");
    text_layer_set_background_color(tl, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(tl, normalTextColour);
  #else
  text_layer_set_text_color(tl, GColorWhite);
  #endif
}


static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, long_select_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 0, long_up_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 0, long_down_handler, NULL);
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 4, 200, true, select_multi_click_handler);
}

// #ifdef PBL_COLOR
#ifdef CONFIG_SUPPORT
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
static bool alternate;
tickCounter++;
if (screenMessageTimer > 0) {
  screenMessageTimer--;
  if (screenMessageTimer == 0)
    screenMessage("");
} else if (tickCounter == 7) {
    alternate = false;
    if (vibeDisconnect)
      vibes_short_pulse();
  updatescreen(currentScreen, "");
} else if (tickCounter > 7) {
    if (alternate) {
      updatescreen(currentScreen,"--");
    } else {
      updatescreen(currentScreen,"");
    }
    alternate = !alternate;
}
}
#endif

  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  #ifndef PBL_COLOR
  window_set_fullscreen(s_main_window, true);
  #else
  
  #endif
    
    #ifdef CONFIG_SUPPORT
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    #endif
  // Set click handlers
  window_set_click_config_provider(s_main_window, click_config_provider);
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  
//   APP_LOG(APP_LOG_LEVEL_INFO, "Inbox: %d OutBox %d", (int)app_message_inbox_size_maximum(),(int)app_message_outbox_size_maximum());
  
  // Open AppMessage
  #ifdef PBL_COLOR
  app_message_open(app_message_inbox_size_maximum(), 0 /*app_message_outbox_size_maximum()*/);
  #else
  app_message_open(126/*app_message_inbox_size_maximum()*/, 0 /*app_message_outbox_size_maximum()*/);
  #endif
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

//
// Main
//
int main(void) {
  int i;
  
  //Read configuration back from storage - also need a way to reset to defaults
  for (i=0; i<NUM_SCREENS; i++)
      if (persist_exists(MAPPING_PKEY + i))
        persist_read_data(MAPPING_PKEY + i, &screens[i], sizeof(screens[i]));

  configLock = screens[0].special; // Read back current state of config lock
  #ifdef CONFIG_SUPPORT
  useBold = screens[1].special ==1; // Read back current state of bold font 
  racebox = screens[2].special;
  vibeDisconnect = screens[3].special;
  #ifdef PBL_COLOUR
  colourAWA = screens[4].special;
  #endif
  #endif
  
  
  init();
  
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED); // Beware!! Increased power usage, but much better responsiveness

  app_event_loop();

  app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);

  deinit();
  
  screens[0].special = configLock; // Store configuration lock state
  #ifdef CONFIG_SUPPORT
  screens[1].special = ((useBold) ? 1 : 0);
  screens[2].special = racebox;
  screens[3].special = vibeDisconnect;
  #ifdef PBL_COLOUR
  screens[4].special = colourAWA;
  #endif
  #endif
   //Save configuration to storage
  for (i=0; i<NUM_SCREENS; i++)
          persist_write_data(MAPPING_PKEY + i, &screens[i], sizeof(screens[i]));
}

//
// Change screen modes if needed - blank out old values, show new
//
void updatescreen(int thisScreen, char *initialValue)
  {  
    static int lastScreen = -1;  // Remember where we came from 
    int i;
  if (thisScreen == -2) //Act as if we are starting from scratch -- there is no last screen
    {
    lastScreen = -1;
    thisScreen = 0;
  }
  
  if (lastScreen != -1)  // If we had a last screen, blank out fields
    {
    
    for (i=0; i< screens[lastScreen].num_fields; i++)
        {
        TextLayer *dl = s_data_layer[screens[lastScreen].field_layer_map[i]];
      
        text_layer_set_text(dl, "");
        text_layer_set_text(s_data_title[screens[lastScreen].field_layer_map[i]], "");

        text_layer_set_background_color(dl, GColorClear);
#ifdef PBL_COLOR      
        text_layer_set_text_color(dl, normalTextColour);
#else
        text_layer_set_text_color(dl, GColorWhite);
#endif
      } 
  }
  
if (thisScreen != -1) // -1 if there is no screen to go to -- just blanking out lastScreen prior to default restore;
  {
    for (i=0; i<screens[thisScreen].num_fields; i++) // For now - put something in the fields
      {
      if (initialValue != NULL)
        //text_layer_set_text(s_data_layer[screens[thisScreen].field_layer_map[i]], initialValue);
        setField(i, false, initialValue);
    }
    
    // Set up titles
    for (i=0; i<screens[thisScreen].num_fields; i++)
      {
      if (screens[thisScreen].field_data_map[i] >= num_keytitles)
        screens[thisScreen].field_data_map[i] = 0;
      // XXX text_layer_set_background_color(s_data_title[screens[thisScreen].field_layer_map[i]], GColorClear);
      text_layer_set_text(s_data_title[screens[thisScreen].field_layer_map[i]], keyTitles[screens[thisScreen].field_data_map[i]].title);
    }
  lastScreen = thisScreen;
  int jj;
  for (jj = 0; jj < 6; jj++) {
    doDataRevert(jj);
  }
  static char buf[25];
  snprintf(buf, sizeof(buf), "StartLine    Screen %d", thisScreen + 1);
  text_layer_set_text(s_data_layer[TITLE_INDEX], buf);
  }
}
  
