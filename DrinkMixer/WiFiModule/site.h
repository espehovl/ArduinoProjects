#ifndef _SITE_H_
#define _SITE_H_

char top[] PROGMEM = R"=====(
<html>
<head>
    <style>
        body {
            background-color: ivory;
        }
        h1 {
            font-family: Helvetica, sans-serif;
            padding-left: 10px;
            padding-top: 6px;
        }
        p {
            font-size: larger;
        }
        .drinkList {
            border: 3px solid grey;
            border-radius: 10px;
            background-color: azure;
            padding-left: 10px;
            padding-bottom: 10px;
        }
        .drinkList ul {
            list-style: none;
        }
        .drinkList ul li:hover {
            background-color: rgb(118, 162, 243);
        }
        .drinkList ul li {
            font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
            font-weight: normal;
            font-size: 32;
            margin-left: -10px;
            padding-left: 4px;
            padding-bottom: 10px;
            padding-top: 10px;
            margin-bottom: 5px;
            margin-right: 40px;
            border: 2px solid grey;
            border-radius: 5px;
            overflow: hidden;
            transition-duration: 0.4s;
        }
        .drinkList ul li p {
            font-size: 20;
            padding-bottom: 0;
            margin-bottom: 0;
            margin-top: 4px;
            position: relative;
            margin-left: 40px;
        }
        .drinkList ul a {
            color: black;
            text-decoration: none;
        }
    </style>
    <title>BarBot 1000</title>
</head>
<body>
<h1>BarBot 1000</h1>
<div class="drinkList">
    <ul>
)=====";


char tail[] PROGMEM =
R"=====(
    </ul>
</div>
</body>
</html>
)=====";

#endif