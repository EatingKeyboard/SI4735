#include <SI4735.h>
#include <avr/pgmspace.h>
#include "patch_content.h"

#define RESET_PIN 12
#define AM_FUNCTION 1

// You have to set APPLY_PATCH to 1 if you reaaly want to start this process
bool APPLY_PATCH = true;
bool PATCH_FINISIHED = false;

SI4735 si4735;

void setup()
{

  Serial.begin(9600);

  if (!APPLY_PATCH)
  {
    showWarning();
  }
  else
  {
    delay(1000);

    si4735.setup(RESET_PIN, 15);
    delay(1000);
    showFirmwareInformation();
  }
}

void showWarning()
{
  // Serial.println("Read before the files attention.txt, reademe.txt or leiame.txt.");
  
}

void showFirmwareInformation()
{

  si4735.getFirmware();

  Serial.println("Firmware Information.");
  Serial.print("Part Number (HEX)........: ");
  Serial.println(si4735.getFirmwarePN(), HEX);
  Serial.print("Firmware Major Revision..: ");
  Serial.println(si4735.getFirmwareFWMAJOR());
  Serial.print("Firmware Minor Revision..: ");
  Serial.println(si4735.getFirmwareFWMINOR());
  Serial.print("Patch ID ................: ");
  Serial.print(si4735.getFirmwarePATCHH(), HEX);
  Serial.println(si4735.getFirmwarePATCHL(), HEX);
  Serial.print("Component Major Revision.: ");
  Serial.println(si4735.getFirmwareCMPMAJOR());
  Serial.print("Component Minor Revision.: ");
  Serial.println(si4735.getFirmwareCMPMINOR());
  Serial.print("Chip Revision............: ");
  Serial.println(si4735.getFirmwareCHIPREV());
}

void confirmationYouAreSureAndApply()
{

  if (!PATCH_FINISIHED)
  {

    if (Serial.available() > 0)
    {
      char key = Serial.read();
      if (key == 'Y')
      {
        applyPatch();
      }
      else if (key == '?')
      {
        showWarning();
      }
      else
      {
        // Serial.println("Type Y if you agree with the terms or ? to see the terms!");
        delay(5000);
      }
    }
  }
  else
  {
    // Serial.println("PATCH applyed. Upload a clean or new sketch to this Arduino and Shutdown it!");
    delay(5000);
  }
}




/* 
 * Power Up with patch configuration
 * See Si47XX PROGRAMMING GUIDE; page 219 and 220
 */
void prepereSi4735ToPatch()
{
  si4735.waitToSend();
  Wire.beginTransmission(SI473X_ADDR);
  Wire.write(POWER_UP);
  Wire.write(0xE2); // Set to FM Transmit, set patch enable, enable interrupts.
  Wire.write(0x50); // Set to Analog Line Input.
  Wire.endTransmission();
  delayMicroseconds(2500);
  si4735.waitToSend();
}

void applyPatch()
{

  int offset = 0;
  int i = 0;
  byte content;

  Serial.println("Applying.");
  delay(500);
  prepereSi4735ToPatch();
  /*
  // Send patch for whole SSBRX initialization string
  for (offset = 0; offset < size_content_initialization; offset += 8)
  {
    Wire.beginTransmission(SI473X_ADDR);
    for (i = 0; i < 8; i++)
    {
      content = pgm_read_byte_near(ssb_patch_content_initialization + (i + offset));
      Wire.write(content);
    }
    Wire.endTransmission();
    if ( offset > 80 and (offset % 80) == 0 )  Serial.println(offset);
    si4735.waitToSend();
    delayMicroseconds(600);
  }
  */
  Serial.println("2");
  // Send patch for whole SSBRX full download
  for (offset = 0; offset < size_content_full; offset += 8)
  {
    Wire.beginTransmission(SI473X_ADDR);
    for (i = 0; i < 8; i++)
    {
      content = pgm_read_byte_near(ssb_patch_content_full + (i + offset));
      Wire.write(content);
    }
    Wire.endTransmission();
    if ( offset > 80 and (offset % 80) == 0 ) Serial.println(offset);
    si4735.waitToSend();
    delayMicroseconds(600);
  }

  delay(250);

  Serial.println("Applyed!");
  delay(1000);
  si4735.setup(RESET_PIN, AM_FUNCTION);
  si4735.setSsbConfig(1, 0, 0, 1, 0, 1);
  si4735.setSSB(700, 7200,  7100, 1,1);
  si4735.setVolume(62);
  si4735.frequencyUp();
  si4735.frequencyDown();

  while (1) {
    si4735.setSsbBfo(1000);
    delay(500);
    si4735.setSsbBfo(-1000);
  }
  
  PATCH_FINISIHED = true;
}

void loop()
{
  if (APPLY_PATCH)
  {
    confirmationYouAreSureAndApply();
  }

}