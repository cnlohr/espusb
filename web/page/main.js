//Copyright (C) 2015 <>< Charles Lohr, see LICENSE file for more info.
//
//This particular file may be licensed under the MIT/x11, New BSD or ColorChord Licenses.

function KickHID()
{
}

var lastMouseX;
var lastMouseY;
var TimeLastUp;
var ButtonsDown = 0;

var UpTimeout;


function HandleUpDown( button, down )
{
	console.log( "HandleUpDown: " + button + " " + down );
	var Mask = 1<<button;
	if( down )
		ButtonsDown |= Mask;
	else
		ButtonsDown &= ~Mask;

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
		UpTimeout = setTimeout( function() { HandleUpDown( 0, false ); }, 200 );
	}
	else
	{
		var now = Date.now();
	    var dt = now - TimeLastUp;
		HandleUpDown( 0, dt < 150 );
		try{ clearTimeout( UpTimeout ); } catch(e) { }
	}
}

function DeltaMouse( x, y )
{
	msg = Math.round(x) + ", " + Math.round(y);
	var msg = "CM" + ButtonsDown + "\t" + Math.round(x) + "\t" + Math.round(y);
	QueueOperation( msg );	
	console.log( msg );
}

var KeyboardModifiers = 0;

function HandleNewText()
{
	var code = $("#KeyboardInput").val().charCodeAt(0);
	var ucode = 0;
	var CurrentModifier = KeyboardModifiers;
	//USB Codes: http://www.freebsddiary.org/APC/usb_hid_usages.php
	if( code >= 65 && code <= 90 )
	{
		ucode = code - 65 + 4; //Convert to "A"
		CurrentModifier |= 2; //Force left shift.
	}
	else if( code >= 97 && code <= 122 )
	{
		ucode = code - 97 + 4; //Convert to "a"
		CurrentModifier &= ~2; //Force un-left-shift
	}
	else if( code >= 49 && code <= 57 )
	{
		ucode = code - 49 + 0x1e; //Convert to "A"
		CurrentModifier &= ~2; //Force un-left-shift
	}
	else if( code == 33 ) { ucode = 0x1e; CurrentModifier = 2; }
	else if( code == 64 ) { ucode = 0x1f; CurrentModifier = 2; }
	else if( code == 35 ) { ucode = 0x20; CurrentModifier = 2; }
	else if( code == 36 ) { ucode = 0x21; CurrentModifier = 2; }
	else if( code == 37 ) { ucode = 0x22; CurrentModifier = 2; }
	else if( code == 94 ) { ucode = 0x23; CurrentModifier = 2; }
	else if( code == 38 ) { ucode = 0x24; CurrentModifier = 2; } //&
	else if( code == 42 ) { ucode = 0x25; CurrentModifier = 2; }
	else if( code == 40 ) { ucode = 0x26; CurrentModifier = 2; } //(
	else if( code == 48 ) { ucode = 0x27; CurrentModifier = 0; } //0
	else if( code == 41 ) { ucode = 0x27; CurrentModifier = 2; }

	if( ucode > 0 )
	{
		var msg = "CK" + CurrentModifier + "\t" + ucode;
		QueueOperation( msg );
	}
	$("#KeyboardInput").val("");
}

function AttachMouseListener()
{
    $("#MouseCap").on("touchstart", function(event) {
		event.preventDefault();
		var e = event.originalEvent.changedTouches[0];
		lastMouseX = e.clientX;
		lastMouseY = e.clientY;
		MouseUpDown( true );
	});

    $("#MouseCap").on("touchend", function(event) {
		event.preventDefault();
		var e = event.originalEvent.changedTouches[0];
		lastMouseX = e.clientX;
		lastMouseY = e.clientY;
		MouseUpDown( false );
	});

	$("#MouseCap").on("touchmove", function(event)  {
		event.preventDefault();
		var e = event.originalEvent.changedTouches[0];
		DeltaMouse( e.clientX - lastMouseX, e.clientY - lastMouseY );
		lastMouseX = e.clientX;
		lastMouseY = e.clientY;
		return false;
	});

	//This is for desktop mice.
	for( var b = 0; b < 3; b++ )
	{
		let ths = b; //Thank you, Mortiz from youtube live chat!!!
		$("#Mouse"+b).on("mouseup", function(event) { HandleUpDown( ths, 0 ); } );
		$("#Mouse"+b).on("mousedown", function(event) { HandleUpDown( ths, 1 ); } );
		$("#Mouse"+b).on("touchend", function(event) { HandleUpDown( ths, 0 ); } );
		$("#Mouse"+b).on("touchstart", function(event) { HandleUpDown( ths, 1 ); } );
	}

	$("#KeyboardInput").on( "input", function() { HandleNewText() } );

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


