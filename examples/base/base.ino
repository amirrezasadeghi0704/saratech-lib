#include <saratech.h>

void setup() {
  saratechBeginGTD35(&Wire, 0x10, Serial1, -1, HIGH, 115200);
  amirreza.setTdaxisGain(200);
}

void loop() {
  //forati king
  aghaparsa();  //update sensor
  king.forati(15, 0, 0); // speed,xtarg,ytarg
  //amirreza
  amirreza.move(20, 0, 100, 1);  //speed,x,y,shetab
  amirreza.shoot(PA13,PA4);
  amirreza.stop();
  //sina
  sina.rotate(5.00, 45);  //dgy,servo
  sina.stop();
  //sadra
  sadra.move(90, 90, 10);  //xtarg,ytarg,speed
  sadra.stop();
  //spin
  spinBackMotor.setSpeed(255);
  //gtd35 farhan
  gtd35Adapter.setMotorIDs(0، 1، 2، 3);
  gtd35Adapter.setAngle(4000);
  gtd35Adapter.setMotorRaw(1000، 1000، - 1000، - 1000);
}
