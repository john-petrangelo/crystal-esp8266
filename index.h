char const MAIN_page[] PROGMEM = R"=====(
<html>
  <head>
    <title>Crystal Control</title>
    <meta name="viewport" content="initial-scale=2.0">
    <style>
      body { background-color: lavender; font-family: 'Brush Script MT', cursive; }
      button { color: purple; width: 95% }
      .column { text-align: center; float: left; width: 50%; display: grid }
</style>
  </head>
  <body>
    <center><b>Dark Crystal</b></center><br/>

    <div class="column">
      <a href="/off"><button>Off</button></a><br/>
      <a href="/rainbow"><button>Rainbow Connection</button></a><br/>
      <a href="/darkcrystal"><button>Dark Crystal</button></a><br/>
      <a href="/flame"><button>Flame</button></a><br/>
    </div>

    <div class="column">
      <a href="/demo1"><button>Demo 1</button></a><br/>
      <a href="/demo2"><button>Demo 2</button></a><br/>
    </div>
  </body>
</html>
)=====";
