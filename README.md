# THIS REPO IS A WORK IN PROGRESS


# Arduino Web Telemetry

This project provides the necessary tools to remotely monitor your arduino-compatible projects using a web GUI instead of a simple terminal.


As of writing this, the code is mostly functional, but definitely not finished. If you would like to make use of this system before I get a chance to finish it up, please contact me on Twitter/X at aWildOcti


In order to reduce the processing load of having to handle clients, I decided to move the server aspect from the microcontroller to any available computer on the same network, which can be established at home via a router or on the go through mobile data. From there, whatever arduino compatible board you are using can connect to your computer's IP address on port 8084 and send/receive data at regular intervals, which can then be updated on the webpage, freeing up plenty of CPU cycles to handle the rest of your program. So far, this has been tested on a Pi Pico W and ESP32.

In order to access the web interface, you must go to any browser, type the computer's IP address followed by port 80 (`192.168.abc.cde:80`) or `localhost:80` if you're accessing the web UI from the computer that's hosting the server. Please note that I am not a web developer, so there is definitely plenty of room for improvement, but the framework is (mostly) here for those who need it.



# TODO

- parse CSV data coming from the microcontroller
- make the CSV data actually affect the web UI
- provide better examples
- add arduino code to repo
