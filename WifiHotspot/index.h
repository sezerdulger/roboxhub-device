const char MAIN_page[] PROGMEM = R"=====(
<html>
	<head>
			<title>Doorlock</title>
	</head>
	<body>
		<form method="post" action="/save">
			<input type="text" value="@@ssid@@" name="ssid" />
			<input type="password" value="@@pass@@" name="password" />
			<input type="submit" value="Save" name="submit"/>
		</form>
	</body>
</html>
)=====";