#ifndef _SITE_H_
#define _SITE_H_


char site[] PROGMEM = R"=====(
<html>
<head>
    <style>
        body {background-color: lightgoldenrodyellow;}
        h1 {
            font-family: Helvetica, sans-serif;
            padding-left: 10px;
            padding-top: 6px;
        }
        p {font-size: large;}
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
            font-weight: bold;
            font-size: larger;
            margin-left: -10px;
            padding-left: 4px;
            padding-bottom: 10px;
            padding-top: 10px;
            margin-bottom: 5px;
            margin-right: 40px;
            border: 2px solid grey;
            border-radius: 5px;
            /* background-color: cornflowerblue; */
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

<!-- List all the available drinks -->
<!-- TODO: Her må vi fylle inn de aktuelle drinkene, på ett eller annet smart vis... -->
<div class="drinkList">
    <p>Hva vil du drikke?</p>
    <ul>
        <a href="/drink?type=Cosmopolitan"><li>Cosmopolitan</li></a>
        <a href="/drink?type=Passionate Cosmo"><li>Passionate Cosmopolitan</li></a>
        <a href="/drink?type=Sex on the Beach"><li>Sex on the Beach</li></a>
        <a href="/drink?type=Bay of Passion"><li>Bay of Passion</li></a>
        <a href="/drink?type=Null"><li>empty</li></a>
        <a href="/drink?type=Null"><li>empty</li></a>
        <a href="/drink?type=Null"><li>empty</li></a>
        <a href="/drink?type=Null"><li>empty</li></a>
        <a href="/drink?type=Null"><li>empty</li></a>
        <a href="/drink?type=Null"><li>empty</li></a>
        <a href="/drink?type=Null"><li>empty</li></a>
        <a href="/drink?type=Null"><li>empty</li></a>
    </ul>

</div>

</body>
</html>
)=====";

#endif