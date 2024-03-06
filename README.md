Required platforms:
Android Studio
Arduino IDE 2.0.4
Google Firebase

Components:
Arduino Code, inside the "ProjectFinalArduinoCode" file

Android App Code, inside the "Registration" file

Firebase Database, on the firebase website


Setup Instructions:
Arduino - Connect an ESP32 board and run the Arduino Code on it, 
the only values that need to be changed are related to the local wifi login.
An entire circuit setup is needed to actually run the functions of the arduino to observe results.
Some commented Serial.prints are available (commented) to view the values the arduino gets from the database without needing any circuit setup.

Android App - 1ºLaunch the App on the built in android studio emulator (we used the Pixel 2 API 30 device for our testing).
	      2ºCreate an account using a real email and password, or login using this account (email: zemor.morgado@gmail.com, password: 1234567)
	      3ºYou should then have access to the various functionalities of the app, create your own profiles, or use the presets, 
		there are also some profiles already created if you login using the provided account.
