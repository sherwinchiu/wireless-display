const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <p>Please unplug and replug after username and password is set!</p>
  <form action="/get">
    ssid: <input type="text" name="ssid"><br>
    pass: <input type="text" name="pass"><br>
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";