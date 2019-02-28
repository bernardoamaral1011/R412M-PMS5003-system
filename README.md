# R412M-PMS5003-system
Monitoring system consisting of the PMS5003 and the SODAQ SFF R412M board, along with a 5V DC-DC converter.

Code is divided into a server component that receives udp packets with pm measures from various sensors and stores them into a MongoDB database, and the arduino code that is inserted into the R412M boards.

--The location of each board has to be inserted manually in the arduino code.
--The only way to detect that a board is out of network range is through the absence of its periodic message to the server.
--There's a mechanism to see whether the sensor is malfunctioning or not through the server, due to the errorToken in the arduino code.
