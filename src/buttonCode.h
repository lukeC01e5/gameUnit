
//int buttonValue = -1; // Declare the buttonValue variable



volatile int buttonValue = -1; // Global variable to hold the button value

void yesButtonPressed()
{
    buttonValue = 1;
}

void noButtonPressed()
{
    buttonValue = 0;
}

const int yesButtonPin = 35; // Button1 on the TTGO T-Display
const int noButtonPin = 0;   // Button2 on the TTGO T-Display

int buttonConfirm()
{
  attachInterrupt(digitalPinToInterrupt(yesButtonPin), yesButtonPressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(noButtonPin), noButtonPressed, FALLING);

  // Wait for a button press
  while (buttonValue == -1)
  {
    delay(100); // Optional delay to prevent the loop from running too fast
  }

  int returnValue = buttonValue; // Save the button value

  // Reset the button value
  buttonValue = -1;

  detachInterrupt(digitalPinToInterrupt(yesButtonPin));
  detachInterrupt(digitalPinToInterrupt(noButtonPin));

  return returnValue; // Return the button value
}