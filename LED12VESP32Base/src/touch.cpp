#include <Button2.h>

#include "touch.h"

Button2 touch1, touch2, touch3, touch4;

void touched(Button2 &btn)
{
}

void touchSetup()
{
  touch1.begin(TOUCH1_PIN);
  touch1.setClickHandler(touched);
  touch2.begin(TOUCH2_PIN);
  touch2.setClickHandler(touched);
  touch3.begin(TOUCH3_PIN);
  touch3.setClickHandler(touched);
  touch4.begin(TOUCH4_PIN);
  touch4.setClickHandler(touched);
}

void touchLoop()
{
    
}
