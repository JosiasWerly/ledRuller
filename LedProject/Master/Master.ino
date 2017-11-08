#include "DataStructures.h"

//decoder
//direcional 1, direcional 2, click
//dec->begin(2, 3, 4);

//lach, dt, clk
//bit->begin(12, 11, 10);

Director dir;
void setup()
{
  Serial.begin(9600);
  dir.changeProcedure<scSelectionExer>();
  
}
void loop()
{
  dir.loop();
}

