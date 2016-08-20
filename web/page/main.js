//Copyright (C) 2015 <>< Charles Lohr, see LICENSE file for more info.
//
//This particular file may be licensed under the MIT/x11, New BSD or ColorChord Licenses.

function KickHID()
{
}

var lastMouseX;
var lastMouseY;
var TimeLastUp;
var MouseDown = false;

var UpTimeout;


function HandleUpDown( down )
{
	MouseDown = down;
	DeltaMouse( 0, 0 );

	if( down )
	{
		$("#MouseCap").css("background-color","blue");
	}
	else
	{
		$("#MouseCap").css("background-color","green");
	}

}

function MouseUpDown( down )
{
	if( !down )
	{
		TimeLastUp = Date.now();
		UpTimeout = setTimeout( function() { HandleUpDown( false ); }, 200 );
	}
	else
	{
		var now = Date.now();
	    var dt = now - TimeLastUp;
		HandleUpDown( dt < 150 );
		try{ clearTimeout( UpTimeout ); } catch(e) { }
		DeltaMouse( 0,0 );
	}
x
}

function DeltaMouse( x, y )
{
	msg = Math.round(x) + ", " + Math.round(y);
	//$("#MouseDebug").val( msg );
	var msg = "CM" + (MouseDown?"1":"0") + "\t" + Math.round(x) + "\t" + Math.round(y);
	console.log( msg );
	QueueOperation( msg );
}

function AttachMouseListener()
{
    $("#MouseCap").on("touchstart", function(event) {
		event.preventDefault();
		var msg = "Mouse:";
		var e = event.originalEvent.changedTouches[0];
		lastMouseX = e.clientX;
		lastMouseY = e.clientY;
		MouseUpDown( true );
	});

    $("#MouseCap").on("touchend", function(event) {
		event.preventDefault();
		var msg = "Mouse:";
		var e = event.originalEvent.changedTouches[0];
		lastMouseX = e.clientX;
		lastMouseY = e.clientY;
		MouseUpDown( false );
	});

	$("#MouseCap").on("touchmove", function(event)  {
		event.preventDefault();
		var msg = "Mouse:";
		var e = event.originalEvent.changedTouches[0];
		DeltaMouse( e.clientX - lastMouseX, e.clientY - lastMouseY );
		lastMouseX = e.clientX;
		lastMouseY = e.clientY;
		return false;
	});
	console.log( "Attaching." );
}

window.addEventListener("load", AttachMouseListener, false);

/*
is_leds_running = false;
pause_led = false;

function KickLEDs()
{
	$( "#LEDPauseButton" ).css( "background-color", (is_leds_running&&!pause_led)?"green":"red" );

	if( !is_leds_running && !pause_led )
		LEDDataTicker();

}

window.addEventListener("load", KickLEDs, false);

function ToggleLEDPause()
{
	pause_led = !pause_led;
	KickLEDs();
}


function GotLED(req,data)
{
	var ls = document.getElementById('LEDCanvasHolder');
	var canvas = document.getElementById('LEDCanvas');
	var ctx = canvas.getContext('2d');
	var h = ls.height;
	var w = ls.width;
	if( canvas.width != ls.clientWidth-10 )   canvas.width = ls.clientWidth-10;
	if( ctx.canvas.width != canvas.clientWidth )   ctx.canvas.width = canvas.clientWidth;

	var secs = data.split( ":" );

	$( "#LEDPauseButton" ).css( "background-color", "green" );

	var samps = Number( secs[1] );
	var data = secs[2];
	var lastsamp = parseInt( data.substr(0,4),16 );
	ctx.clearRect( 0, 0, canvas.width, canvas.height );

	for( var i = 0; i < samps; i++ )
	{
		var x2 = i * canvas.clientWidth / samps;
		var samp = data.substr(i*6,6);
		var y2 = ( 1.-samp / 2047 ) * canvas.clientHeight;

		ctx.fillStyle = "#" + samp.substr( 2, 2 ) + samp.substr( 0, 2 ) + samp.substr( 4, 2 );
		ctx.lineWidth = 0;
		ctx.fillRect( x2, 0, canvas.clientWidth / samps+1, canvas.clientHeight );
	}

	var samp = parseInt( data.substr(i*2,2),16 );

	LEDDataTicker();
} 

function LEDDataTicker()
{
	if( IsTabOpen('LEDs') && !pause_led )
	{
		is_leds_running = true;
		QueueOperation( "CL",  GotLED );
	}
	else
	{
		is_leds_running = 0;
	}
	$( "#LEDPauseButton" ).css( "background-color", (is_leds_running&&!pause_led)?"green":"red" );

}

*/


