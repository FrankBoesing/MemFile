/*
  MEMFile file dump
  This example code is in the public domain.
*/

#include <MEMFILE.h>

char testData[] = {'1', '2', '3', '4', '5', '6', '7', '8'};

MemFS  myfs;

void setup()
{
  // open the file.
  File dataFile = myfs.open((char*) &testData, sizeof(testData), FILE_WRITE);

  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    Serial.println();
    dataFile.seek(1);
    dataFile.write('!');
    dataFile.seek(0);
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }
}

void loop()
{
}
