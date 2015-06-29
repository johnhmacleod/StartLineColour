#include <pebble.h>
#include "startline.h"
  
  

int16_t getLayerBounds(void *subject)
{
  GRect temp = layer_get_bounds((Layer*)subject);
  return(temp.origin.x);
};
 
void setLayerBounds(void *subject, int16_t new_bounds)
{
  GRect b;
  b = layer_get_bounds(subject);
  b.origin.x = new_bounds;
  layer_set_bounds((Layer *)subject, b);
};


static const PropertyAnimationImplementation my_implementation = {
  .base = {
    // using the "stock" update callback:
    .update = (AnimationUpdateImplementation) property_animation_update_int16,
    // .update = (AnimationUpdateImplementation) myUpdateInt16,
  },
  .accessors = {
    // my accessors that get/set a int16 from/onto my subject:
    .setter = { .int16 = (const Int16Setter)setLayerBounds, },
    .getter = { .int16 = (const Int16Getter)getLayerBounds, },
  },
};
 
static void animation_stopped(PropertyAnimation *animation, bool finished, void *data) {
  property_animation_destroy(animation);
  *((Animation **)data) = NULL;
}

void animate_layer_bounds(PropertyAnimation **anim, Layer* layer, GRect *start, GRect *finish, int duration, int delay)
{
  static int s, f;
  s = start->origin.x;
  f = finish->origin.x; 

  
  *anim = property_animation_create(&my_implementation, layer, &s, &f);
  
  // Workaround SDK bug
  #ifdef PBL_COLOR
  property_animation_set_from_gpoint(*anim, &s);
  property_animation_set_to_gpoint(*anim, &f); // Was from_gpoint !!
  #else
  (*anim)->values.from.int16 = s;
  (*anim)->values.to.int16 = f;
  #endif
  
  AnimationHandlers handlers = {
        //The reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) animation_stopped
    };
  
  animation_set_handlers((Animation*) *anim, handlers, anim);
  animation_set_duration((Animation*) *anim, duration);
  animation_set_delay((Animation*) *anim, delay);
  animation_schedule((Animation*) *anim);

}


