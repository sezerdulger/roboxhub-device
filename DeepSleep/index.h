const char MAIN_page[] PROGMEM = R"=====(
<html>
	<head>
			<title>Doorlock</title>
	</head>
	<body>
		<form method="post" action="/save">
			<table>
			<tr>
			<td>
				<label>SSID:</label>
			</td>
			<td>
				<input type="text" value="@@ssid@@" name="ssid" placeholder="SSID"/>
			</td>
			</tr>
			<tr>
			<td>
				<label>Password:</label>
			</td>
			<td>
				<input type="password" value="@@pass@@" name="password" placeholder="Password" />
			</td>
			</tr>
			<tr>
			<td>
				Connecto to wifi
			</td>
			<td>
			<input type="submit" value="Save" name="submit"/>
			</td>
			</tr>
			</table>
		</form>
	</body>
</html>
)=====";