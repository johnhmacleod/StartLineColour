#include <pebble.h>
#include "startline.h"


  static char buffer[15][16];
  static char VMGtoWindBuffer[7];
  static char tmpbuf[16];
  static char tackLogBuffer[7][5];
int screenMessageTimer = 0;
//
// Handle incoming messages from phone
//
void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char *layDecode[] = {"SPN", "SCB", "SMD",NULL,NULL,NULL,NULL,NULL,NULL,NULL,"PPN","PCB","PMD","PPN","PCB","PMD"};
  static int tackLog[7] = {0,0,0,0,0,0,0};
  static bool warnedLineBurn = false;
  static int currentState = -1;
  static int thisTack = 0, oldTack = 0;
  int angle;
  bool earlyWarnDone = false;
  bool weAreRacing = false;
  bool doScreenTransition;
  #ifdef PBL_COLOR
  bool redAWA, highVMG;
  #endif
  int j, tmp, ii;
  bool foundKey, negNum; 
  #ifdef CONFIG_SUPPORT
  bool foundAKey = false; // Did we find any data keys in the message?
  #endif
  int startingScreen; // To remember which screen we were at when we arrived here so we don't loop forever looking for a screen with data on it
  int a;
  float b;
  static int flashFlag = 0;
  char *sa, *sb;
  bool fieldUpdated[6];
  bool canDoVMG;
  int twd, bs, hdg;
  #ifdef CONFIG_SUPPORT
    bool receivedConfig = false;
  #endif
  
  if (doubleClick || messageClick)
    return; // Get out of here if we are displaying a message - note - we may miss a tack because of this, but unlikely!!
  startingScreen = currentScreen;
#ifdef PBL_COLOR
  text_layer_set_background_color(flash, flashFlag == 0 ? GColorGreen : GColorClear);
#else
  layer_set_bounds(inverter_layer_get_layer(flash), flashFlag == 0 ? GRectZero : GRect(0,0,7,7)); 
#endif
  flashFlag = 1 - flashFlag;
  if (holdThisScreen > 0)
    holdThisScreen--;
  
  weAreRacing = false; // Assume we're not racing unless we receive a mark name
  do
    {
    for (ii = 0; ii < screens[currentScreen].num_fields; ii++) {
      fieldUpdated[ii] = false;
    }
    canDoVMG = false;
    #ifdef PBL_COLOR
    redAWA = false;
    highVMG = false;
    #endif
    doScreenTransition = false;
    weAreRacing = false;
    // Read first item
    Tuple *t = dict_read_first(iterator);
    j = 0;
    while(t != NULL) {
    foundKey = true;
    char *bj;
    bj = buffer[j];
    // Which key was received?
    //APP_LOG(APP_LOG_LEVEL_INFO, "Key %d Value %d", (int)t->key, (int)t->value->int32);
    switch(t->key) {
      
      case KEY_LINE_BURN:      
      case KEY_LAY_TIME:
      case KEY_LINE_TIME:
      case KEY_SECS_TO_START:
      case KEY_LAY_BURN:
      case KEY_TIME_TO_MARK:
      negNum = false;    
      tmp = abs((int)t->value->int32) ;
      negNum = ((int)t->value->int32 < 0);

      if (tmp >= 100)
      {
        if ((tmp < 6000 && isBigField(t->key)) || tmp < 600) // We have room for mins & seconds & always when mins < 10
        {
          snprintf(bj, sizeof(buffer[0]),"%d:%02d", tmp / 60, tmp % 60);
        }
        else if (tmp < 3600)
        {
          snprintf(bj, sizeof(buffer[0]),"%dm", tmp / 60 + ((tmp % 60) >= 30 ? 1:0));
        }
        else
          {
          snprintf(bj, sizeof(buffer[0]),"%dh", tmp / 3600 + ((tmp % 3600) >= 1800 ? 1:0));          
        }
      }
      else // (tmp < 100)
      {
        snprintf(bj, sizeof(buffer[0]),"%ds", tmp);
      }
      break;

      case KEY_LAST_TACK:
      oldTack = thisTack; // Remember the tack from the last message 
      thisTack = (int)t->value->int32;
      negNum = false; // This definitely shouldn't happen!
      if (currentState != 1)
        bj[0] = '\0';
      else
        snprintf(bj, sizeof(buffer[0]),"%d", abs((int)t->value->int32));
      break;
      
      case KEY_TARGET_TACK:
      negNum = false;
      snprintf(bj, sizeof(buffer[0]),"%d", abs((int)t->value->int32));
      
      case KEY_TWD:
      if (t->key == KEY_TWD) {
        canDoVMG = true;
        twd = t->value->int32;
      }
      case KEY_LAY_DIST:
      case KEY_LINE_DIST:

      case KEY_TARGET_ANGLE:
      case KEY_MARK_BEARING:
      case KEY_HEADING_COG:
      case KEY_HEADING:
      if (t->key == KEY_HEADING)
        hdg = t->value->int32;
      case KEY_AWS:

      case KEY_TWS:
      case KEY_DEPTH:
      case KEY_HEEL:
      case KEY_CURRENT_DIR:
      negNum = ((int)t->value->int32 < 0);
      snprintf(bj, sizeof(buffer[0]),"%d", abs((int)t->value->int32));
      break;
 
      case KEY_TWA:
      case KEY_AWA:
      negNum = false;
      angle = (int)t->value->int32;
      #ifdef PBL_COLOR
      redAWA = angle > 180;
      if (colourAWA) {
              snprintf(bj, sizeof(buffer[0]),"%d", angle <= 180 ? angle : abs(angle - 360));
      } else {
              snprintf(bj, sizeof(buffer[0]),"%d%s", angle <= 180 ? angle : abs(angle - 360), angle <= 180 ? "S":"P");
      }
      #else
      snprintf(bj, sizeof(buffer[0]),"%d%s", angle <= 180 ? angle : abs(angle - 360), angle <= 180 ? "S":"P");
      #endif
      break;
      
      case KEY_BOAT_SOG:
      case KEY_BOAT_SPEED:
      if (t->key == KEY_BOAT_SPEED)
        bs = t->value->int32;
      case KEY_TARGET_SPEED: 
      case KEY_CURRENT_SPEED:
      snprintf(bj, sizeof(buffer[0]),"%d.%d", abs((int)t->value->int32)/10, abs((int)t->value->int32) % 10);
      negNum = ((int)t->value->int32 < 0);
      break;
      
      case KEY_LINE_ANGLE: // If this is negative, we will be over the line early - red burn time
      a = t->value->int32;
      #ifdef PBL_COLOR
      highVMG = (a < 0);
      #endif
      a = abs(a);
      negNum = false;
      snprintf(bj, sizeof(buffer[0]),"%d", a);
      break;
      
      case KEY_MARK_DIST:
      case KEY_MARK_LAY_DIST:
      negNum = false;
      if (currentState != 1  && t->key == KEY_MARK_LAY_DIST)
        bj[0] = '\000';
      else
        {
        a = (int)t->value->int32;
        negNum = (a < 0);
        a = abs(a);
        b = a / 18.52;
        bool bf = isBigField((int)t->key);
        if (a < 1000) // Less than 1000m - just show m
          snprintf(bj, sizeof(buffer[0]), "%d", a);
        else if (b < 1000 || bf) // less than 100nm or it's a big field - show nm.n
          {
          int d1, d2, d3;
          d1 = (int)b/100;
          d2 = (((int)b % 100)/10);
          d3 = ((((int)b % 10) > 4) ? 1 : 0);
          if (d3 == 1)
            d2 += 1;
          if (d2 == 10)
            {
            d1 += 1;
            d2 = 0;
          }
          snprintf(bj, sizeof(buffer[0]), "%d.%d", d1, d2);
        }
        else
            {
            snprintf(bj, sizeof(buffer[0]), "%d", (int)b/10);
        }
      }
      break;
      
      case KEY_LAY_SEL:
      //APP_LOG(APP_LOG_LEVEL_INFO, "Key %d Value %d", (int)t->key, (int)t->value->int32);
      negNum = false;
      snprintf(bj, sizeof(buffer[0]),"%s", layDecode[(int)t->value->int32]);
      break;
      
      /* These are the turn style values - Rnn & Lnn */
      case KEY_MARK_TURN:
      case KEY_TACK_HEADER:
      negNum = false;
      if (currentState != 1 && t-> key == KEY_TACK_HEADER)
        bj[0] = '\000';
      else
        {
//        if (t->value->int32 >= 0)
          snprintf(bj, sizeof(buffer[0]), "%c%02d", t->value->int32 >= 0 ? 'R' : 'L', abs((int)t->value->int32));
//        else
//          snprintf(buffer[j], sizeof(buffer[j]), "L%02d", -(int)(t->value->int32));
      }
      break;
      
      case KEY_CURRENT_MARK:
      weAreRacing = true; //This data only arrives once the start is over, we must be racing
      negNum = false;
      sa = t->value->cstring;
      sb = tmpbuf;
             // APP_LOG(APP_LOG_LEVEL_ERROR, "Mark->%s", sa);
      while (*sa != '\000') {
        *sb = *sa; // Copy the current char
        if (*sb == ':' && *(sa+1) == ':') { // Found :: don't increment b so we ignore the extra :
          sa++;
          continue;
        }
        else if (*sb == ':' && *(sa+1) == '\000') { // End of string coming up & last char was : - don't increment b - it will be zapped
          sa++;
          continue;
        }
        else {
          sa++;
          sb++;
        }
      }
      *sb = '\000';
      snprintf(bj, sizeof(buffer[0]), "%s", tmpbuf);
      break;
      
      case KEY_TACK_STATE:
      a = (int)t->value->int32;
      negNum = a < 0;
      snprintf(bj, sizeof(buffer[0]), "%d", abs((int)(t->value->int32)));
      if (currentState == 1 && a != 1) // Just detected a tack
        {
        tackLog[0] = oldTack;
        int i;
        for (i = 6; i > 0; i--) // Shuffle them all up one
          tackLog[i] = tackLog[i-1];
      }
      currentState = a;
      // Now display the log
      int k = 1; // Start the log at 1 - 0 is the current tack ready to be shuffled up
      int i = 0;
      for (i=0; i < screens[currentScreen].num_fields; i++) // Go look for TACK LOG on the screen
      {
        if (keyTitles[screens[currentScreen].field_data_map[i]].key == KEY_TACK_LOG) // Tack Log is displayed
        {
          if (tackLog[k] == 0)
            tackLogBuffer[k][0] = '\000';
          else
            snprintf(tackLogBuffer[k], sizeof(tackLogBuffer[0]), "%d", tackLog[k]);
          setField(i, false, tackLogBuffer[k]);
          fieldUpdated[i] = true;
          k++; // Step to the next tacklog entry
        }
      }
      break;
      #ifdef CONFIG_SUPPORT
      case KEY_CONFIG_BOLD: useBold = (t->value->int32 != 0);
      foundKey = false;
      receivedConfig = true;
      break;
      
      case KEY_CONFIG_RACEBOX: racebox = t->value->int32;
      foundKey = false;
      receivedConfig = true;
      break;
      
      case KEY_CONFIG_VIBEDISCONNECT: vibeDisconnect = t->value->int32;
      foundKey = false;
      receivedConfig = true;
      break;
      
      
      case KEY_CONFIG_COLOURAWA: 
      #ifdef PBL_COLOR
        colourAWA = t->value->int32 != 0;
      #endif // PBL_COLOUR
      foundKey = false;
      receivedConfig = true;
      break;

      #endif //CONFIG_SUPPORT
      default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized, value %d", (int)t->key, (int)t->value->int32);
      foundKey = false;
      break;
    }
    if (foundKey) // Now look through the fields on the current screen to see if it is displayed
      {
      #ifdef CONFIG_SUPPORT
      foundAKey = true;
      #endif
      if (t->key == KEY_LINE_BURN) // Do vibrate regardless of what is displayed
              {
              if (!earlyWarnDone && t->value->int32 < 10 && t->value->int32 >0)
                {
                vibes_double_pulse ();
                earlyWarnDone = true;
              }
              else if (t->value->int32 <= 0)
                {
                if (!warnedLineBurn)
                    {
                    vibes_long_pulse();
                    warnedLineBurn = true;
                }
              }
              else
                {
                warnedLineBurn = false;
              }
            }
                
      int i;
        for (i=0; i<screens[currentScreen].num_fields; i++)
          {
          if (weAreRacing && keyTitles[screens[currentScreen].field_data_map[i]].preStart) // We are racing & we have pre-start data displayed
              doScreenTransition = true; // Force a transition if we are racing with a screen displaying prestart data
          if (keyTitles[screens[currentScreen].field_data_map[i]].key == (int)t->key) // Did we find a match?
            { // To this point we have only decoded the message into the buffer
            setField(i, negNum, bj);  // Display the data - we found a match
            fieldUpdated[i] = true;
          }
        }
    // Use next buffer - the buffer remains in use even after the data actually appears on the screen
    // So we need to use a different buffer for each message.
      j++;
    }
    t = dict_read_next(iterator); // Look for next item
  }
#ifdef CONFIG_SUPPORT    
  if (receivedConfig) {
    screenMessageTimer = 3;
    screenMessage("Configuration received");
}
  if (foundAKey) {
  // Messages are still arriving
    if (tickCounter >= 7 && vibeDisconnect)
      vibes_double_pulse();
    tickCounter = 0;
  }
#endif    
  if (weAreRacing && doScreenTransition && holdThisScreen == 0) // We are racing, are showing some prestart data & don't need to hold this screen
    {
      do // Loop around looing for the next in use screen
        {
        currentScreen = (1 + currentScreen) % NUM_SCREENS;
      } while (screens[currentScreen].num_fields == 0);
    if (currentScreen == startingScreen)
      doScreenTransition = false; // Stop looking - we're back where we started!
    updatescreen(currentScreen, NULL); 
  }
  } while (weAreRacing && doScreenTransition && holdThisScreen == 0);
  
  int VMGtoWind;
  
  if (canDoVMG) {
    VMGtoWind = mycos(M_PI * ((hdg - twd) / 180.0)) * (bs * 10);
    VMGtoWind = VMGtoWind / 10 + (VMGtoWind % 10 >=5 ? 1 : 0);
    snprintf(VMGtoWindBuffer, sizeof(VMGtoWindBuffer), "%d.%d", abs(VMGtoWind)/10, abs(VMGtoWind)%10);
  }
  
  // Post process screen fields - VMGWind, blank non-updated fields etc.
  for (ii = 0; ii < screens[currentScreen].num_fields; ii++) {
    int fieldKeyTitle = keyTitles[screens[currentScreen].field_data_map[ii]].key;
    if (canDoVMG) {
      if (fieldKeyTitle == KEY_VMG_WIND) {
        setField(ii, false, VMGtoWindBuffer);  
        fieldUpdated[ii] = true;
      }
    }
    #ifdef PBL_COLOR
    if ((fieldKeyTitle == KEY_LINE_BURN && highVMG) || (colourAWA && (fieldKeyTitle == KEY_AWA || fieldKeyTitle == KEY_TWA) && redAWA)) {
      // APP_LOG(APP_LOG_LEVEL_ERROR, "High VMG/AWA Port in post process");
      text_layer_set_text_color(s_data_layer[screens[currentScreen].field_layer_map[ii]], redTextColour); 
    }
    else if (colourAWA && (fieldKeyTitle == KEY_AWA || fieldKeyTitle == KEY_TWA) && !redAWA) {
      // APP_LOG(APP_LOG_LEVEL_ERROR, "AWA Starboard in post process");
      text_layer_set_text_color(s_data_layer[screens[currentScreen].field_layer_map[ii]], greenTextColour);       
    }
    #endif
    if (!fieldUpdated[ii])
      {
      setField(ii, false, ""); // If a field has not been updated, blank it out - it will be mapped to the wrong buffer[] element
    }
  }
  
}
