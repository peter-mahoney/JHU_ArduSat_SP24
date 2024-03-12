//www.elegoo.com
//2016.12.9
int transistorPin = 3;
int tempPin = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(transistorPin, OUTPUT); // Set transistorPin as an output
}
void loop()
{
  int tempReading = analogRead(tempPin);
  // This is OK
  double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );       //  Temp Kelvin
  float tempC = tempK - 273.15;            // Convert Kelvin to Celcius
  float tempF = (tempC * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
  // Display Temperature in F
  Serial.println(tempF);
  
  if (tempF < 73.5) {
    // If the temperature is below 80 degrees Fahrenheit, set the transistor pin high
    digitalWrite(transistorPin, HIGH);
  } else if (tempF > 76.0) {
    // If the temperature is 80 degrees Fahrenheit or above, set the transistor pin low
    digitalWrite(transistorPin, LOW);
  }
  delay(500);
}
