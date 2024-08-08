//app.js
document.onkeydown = checkKeyDown; //the function which is invoked on keydown
document.onkeyup = checkKeyUp; //the function which is invoked on keyup

//Creating a new connection
var connection = new WebSocket('ws://192.168.4.1:81/', ['arduino']);

connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  console.log('Server: ', e.data);
};
connection.onclose = function () {
  console.log('WebSocket connection closed');
};

var movement = 2; // 1: BACKWARDS, 2: STOP 3: FORWARDS
var wheelDirection = 1; // 0: LEFT, 1: STRAIGHT, 2: RIGHT

function checkKeyDown(e) { //When the key is pressed

    e = e || window.event;

    if (e.keyCode == '38') {
      // up arrow
      movement = 2;
      console.log("forward");
    }
    else if (e.keyCode == '40') {
        // down arrow
        movement = 0;
        console.log("down");
    }
    else if (e.keyCode == '37') {
       // left arrow
       wheelDirection = 1;
       console.log("left");
    }
    else if (e.keyCode == '39') {
       // right arrow
       wheelDirection = 3;
       console.log("right");
    }
    var sum = (wheelDirection*10)+movement;
    connection.send(sum.toString());
    console.log("sum: "+sum);
}

function checkKeyUp(e) { //When the key is released

    e = e || window.event;

    if (e.keyCode == '38') {
      // up arrow
      movement = 1;
      console.log("stop forward");
    }
    else if (e.keyCode == '40') {
        // down arrow
        movement = 1;
        console.log("stop backwards");
    }
    else if (e.keyCode == '37') {
       // left arrow
       wheelDirection = 2;
       console.log("stop left");
    }
    else if (e.keyCode == '39') {
       // right arrow
       wheelDirection = 2;
       console.log("stop right");
    }
    var sum = (wheelDirection*10)+movement; //Summing the commands as mentioned.
    connection.send(sum.toString()); //Sending the number or command so to speak.
    console.log("sum: "+sum);
}
