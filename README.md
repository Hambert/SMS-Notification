## Send notification SMS - experimental!
This little Arduino Sketch send an SMS on power on.
After boot, it read all incomming SMS and search through the text for t phone numbers to stores them in an array.

#ToDo:
* complete SMS Text parser
* save the numbers in EPROM

Requirements: [Sim800l library](https://cristiansteib.github.io/Sim800l/)
