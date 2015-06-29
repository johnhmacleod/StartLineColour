#include <pebble.h>
#include "startline.h"
#include "screens.h"
  
int configLock = 0;

  void doTitleInvert()
  {
  #ifndef PBL_COLOR
  GRect a = layer_get_frame(text_layer_get_layer(s_data_title[screens[currentScreen].field_layer_map[configuring_field]]));
  a.origin.y += a.size.h / 4;
  a.size.h -= a.size.h/4 ;

  layer_set_bounds(inverter_layer_get_layer(inverter), a);
  #else
  text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], invertedTitleColour);
  #endif
}
  
void doTitleRevert()
  {
  #ifndef PBL_COLOR
  layer_set_bounds(inverter_layer_get_layer(inverter), GRect(0,0,0,0));
  #else
  text_layer_set_text_color(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], normalTitleColour);
  #endif  
}
//
//
// ****************************
// All the button handlers here
// ****************************
//

void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (messageClick && configuring)
    {
    screenMessage(""); // Check that configuring is not turned off in updatescreen
    doTitleInvert();
    return;
  }
  if (configuring)
      { // Set the current title back to normal
      doTitleRevert();
      configuring_field  = (1+configuring_field) % screens[currentScreen].num_fields;
      doTitleInvert();
    }
  else if (messageClick) 
    {
    doubleClick = false;
    screenMessage("");
  }
  else if (configLock == 1) {
    show_screens();
  }
}

  
void resetDefault() {
      screenMessage("");
    updatescreen(-1, "");
    int i;
    for (i=0; i<4; i++) // Step through 4 default screens
      {
          screens[i].num_fields = screenDefault[i].num_fields;
          int j;
          for (j=0; j<screenDefault[i].num_fields; j++) // Copy the maps from the default screen
            {
            screens[i].field_layer_map[j] = screenDefault[i].field_layer_map[j];
            screens[i].field_data_map[j] = screenDefault[i].field_data_map[j];
            }
    }
    for (i=4; i<NUM_SCREENS; i++) // Blank out the rest of the screens
      screens[i].num_fields = 0; 
      
    currentScreen = 0;
    updatescreen(-2, "");
    doubleClick = false;

}
  
//
// Up Click Handler
//
void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (messageClick && configuring) // Displayed a message whilst configuring
    {
    screenMessage("");
    doTitleInvert();
  }
  else if (configuring) {
    int *cfdm = &(screens[currentScreen].field_data_map[configuring_field]);
    doDataRevert(configuring_field);
    #ifdef CONFIG_SUPPORT
    do {
      *cfdm = (1 + *cfdm) % num_keytitles;  //Step to the next data item in the list
    } while (racebox == 0 && (*cfdm >= RACEBOX_START && *cfdm <= RACEBOX_END)); 
    #else
      *cfdm = (1 + *cfdm) % num_keytitles;  //Step to the next data item in the list
    #endif
    // Set the title & blank data
    text_layer_set_text(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], 
                        keyTitles[*cfdm].title);
    // Need to clear out data that may have just arrived & ensure that the field is not inverted
    blankNormal(currentScreen, configuring_field);
}
    else if (doubleClick) //Confirming reset to default
    resetDefault();
  else if (messageClick) // Just acknowledging a message
    screenMessage("");
  else  // Step to next screen
    {    
      holdThisScreen = TRANSITION_IDLE;
    do { // Search backwards through screens to find the next one in use
      currentScreen = (currentScreen == 0) ? NUM_SCREENS - 1 : currentScreen - 1;
    } while (screens[currentScreen].num_fields == 0);
    updatescreen(currentScreen,"");
  } // Stepping to next screen
}

//
// Down Click Handler - just like the up click handler
//
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (messageClick && configuring)
    {
    screenMessage(""); // Blank the message
    doTitleInvert();
  }
  else if (configuring) {
    int *fdm_cf = &(screens[currentScreen].field_data_map[configuring_field]);
    doDataRevert(configuring_field);
    #ifdef CONFIG_SUPPORT
    do {
        (*fdm_cf) = (*fdm_cf) == 0 ? num_keytitles - 1 : (*fdm_cf) - 1;
    } while (racebox == 0 && (*fdm_cf >= RACEBOX_START && *fdm_cf <= RACEBOX_END));
    #else
      (*fdm_cf) = (*fdm_cf) == 0 ? num_keytitles - 1 : (*fdm_cf) - 1;
    #endif
    text_layer_set_text(s_data_title[screens[currentScreen].field_layer_map[configuring_field]], 
                        keyTitles[*fdm_cf].title);
    blankNormal(currentScreen, configuring_field);
  } // Configuring
  else if (messageClick) // Cancelling reset to default
  {
    doubleClick = false;
    screenMessage("");
  }
  else { // Not configuring - just step to next screen
     holdThisScreen = TRANSITION_IDLE;
    do { // Search through screens to find the next one in use
      currentScreen = (1 + currentScreen) % NUM_SCREENS;
    } while (screens[currentScreen].num_fields == 0);
    updatescreen(currentScreen,"");
      } // Stepping to next screen
}
  
//
// Long Select Handler
//
void long_select_handler(ClickRecognizerRef recognizer, void *context) {
    if (messageClick || configLock == 0)
      return; // Not allowed while a messgae is displayed or if configuration is locked
    if (configuring == 0) { 
      holdThisScreen = -1;
      configuring = 1; // Select configuring mode
      
      configuring_field = 0;
      doTitleInvert();
      
  
    }
    else { // End configuring, set current title back to normal display
      int i;
      bool preStart = false, postStart = false;
      for (i = 0; i < screens[currentScreen].num_fields; i++)
        {
        if (keyTitles[screens[currentScreen].field_data_map[i]].preStart)
                      preStart = true;
        if (keyTitles[screens[currentScreen].field_data_map[i]].postStart)
                      postStart = true;
      }
      if (preStart && postStart)
        {
        screenMessage("Can't mix pre & post start fields");
      }
      else // End of field configuring
        {
      configuring = 0;
      holdThisScreen = TRANSITION_IDLE;
      doTitleRevert();
      }
    }
}
  
void long_up_handler(ClickRecognizerRef recognizer, void *context) {
  if (configLock == 1 && configuring == 0 && !messageClick && !doubleClick)  // When not configuring, long up creates a new screen like the current one
  {
    Screen *cs = &screens[currentScreen];
    int i;
    for (i=0; i<NUM_SCREENS; i++) // Go find a free screen slot
      if (screens[i].num_fields == 0)
      {
          screens[i].num_fields = /* screens[currentScreen]. */ cs->num_fields;
          int j;
          for (j=0; j< /* screens[currentScreen].*/ cs->num_fields; j++) // Copy the map from the current screen
            {
            screens[i].field_layer_map[j] = /* screens[currentScreen].*/ cs->field_layer_map[j];
            screens[i].field_data_map[j] = /* screens[currentScreen].*/ cs->field_data_map[j];
            }
      break;
    }
    if (i < NUM_SCREENS)
      {
      currentScreen = i;
      updatescreen(currentScreen, "");
      }    
    else
      vibes_long_pulse();
  } 

}

void long_down_handler(ClickRecognizerRef recognizer, void *context) {
  if (configLock == 1 && configuring == 0 && !messageClick && !doubleClick) // Long down deletes the current screen so long as it's not the last one
  {
    int i;
    int ok = 0;  //Always allow delete as long as there is at least one remaining screen
    
    for (i=0; i<NUM_SCREENS; i++)
      {
      if (screens[i].num_fields != 0)
        ok++;
    }
    
            
  if (ok > 1) // OK to delete go find a new screen to display now
    {
    int i;
    for (i=0; i < NUM_SCREENS; i++)
      if (i != currentScreen && screens[i].num_fields != 0) // Avoid selecting the screen we are deleting & unused screens
        break;
    updatescreen(i, "");  // i is left set to the first in-use screen
    screens[currentScreen].num_fields = 0;  // Take current screen out of use. Can't do this earlier or it breaks updatescreen()
    currentScreen = i;
  }
    else
      vibes_long_pulse();
}
}

void select_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (messageClick)
    return; // Not allowed when a message is displayed
  int i = click_number_of_clicks_counted(recognizer);
  if (i==2)
    {
  if (configLock == 1 && !configuring)
    {
  doubleClick = true;
  screenMessage("Press UP to reset to default, DOWN otherwise");
  }
  }
  else if (i == 3)
    {
  if (!configuring && !doubleClick)
  {
  #ifdef PBL_COLOR
  screenMessage(STARTLINE_PEBBLE_TIME_VERSION);
  #else
  screenMessage(STARTLINE_PEBBLE_VERSION);
  #endif
  }
    
  } 
  
  else if (i == 4) 
    {
    configLock = 1 - configLock;
    layer_set_hidden((Layer *)s_padlockLayer, configLock == 1);
    //screenMessage(configLock == 0 ? "Config locked" : "Config unlocked");
  }
  
}

