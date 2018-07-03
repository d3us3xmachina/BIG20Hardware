
// This function blits a full screen, raw, 16 bit 565 RGB color image to the display from the SD card.
void rawFullSPI(char* filename) {   //void rawFullSPI(fs::FS &fs, char* filename) {
  //uint8_t *b, *bmax; // Pointers into the buffer.
  //int pixelframes = frames*128;
  File f;
  // Draw bitmap.
  
    tft.setAddrWindow(0,0,127,127);
    //tft.writecommand(ST7735_RAMWR);// Tell display we're going to send it image data in a moment. (Not sure if necessary.) 
    digitalWrite(TFT_DC, HIGH); // Set DATA/COMMAND pin to DATA.

    f = SD.open(filename);
    if(!f){
      Serial.println("File not found!");
      //filepull(filename);
      return;
    } 

    uint8_t buffer[4096];
    while(f.available()) { // 2.79FPS without SPI_FULL_SPEED in SPI.begin, 3.75FPS with it.
      uint16_t readuntil = f.read(buffer, sizeof(buffer));
      uint16_t file_size = f.size();
      digitalWrite(TFT_CS, LOW); // Tell display to pay attention to the incoming data.
      digitalWrite(SD_CS, HIGH);
      digitalWrite(TFT_DC, HIGH);
      SPI.beginTransaction(SPISettings(70000000, MSBFIRST, SPI_MODE0)); //70M stable  // SPI.beginTransaction(SPISettings(70000000, MSBFIRST, SPI_MODE0)); //70M stable
      SPI.writeBytes(buffer, readuntil);
      SPI.endTransaction();
      //interrupts();         
      digitalWrite(TFT_CS, HIGH); // Tell display we're done talking to it for now, so the next SD read doesn't corrupt the screen.
    }  
    f.close(); // Close the file. 
}

  
